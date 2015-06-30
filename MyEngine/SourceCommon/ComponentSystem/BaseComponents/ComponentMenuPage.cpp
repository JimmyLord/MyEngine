//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if LEGACYHACK
#include "../../SharedGameCode/Screens/ScreenManager.h"
#include "../../SharedGameCode/Screens/Screen_Base.h"
#include "../../SharedGameCode/Menus/MenuScrollBox.h"
#endif //LEGACYHACK

#if MYFW_USING_WX
bool ComponentMenuPage::m_PanelWatchBlockVisible = true;
#endif

ComponentMenuPage::ComponentMenuPage()
: ComponentBase()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_MenuPage;

    m_pComponentTransform = 0;
    m_pComponentCamera = 0;
    m_pComponentLuaScript = 0;

    m_MenuItemsCreated = false;
    m_LayoutChanged = false;

    m_pInputBoxWithKeyboardFocus = 0;

    m_MenuItemsUsed = 0;
    for( unsigned int i=0; i<MAX_MENU_ITEMS; i++ )
    {
        m_pMenuItems[i] = 0;
    }
    m_pMenuItemHeld = 0;

    m_pMenuLayoutFile = 0;

    m_MenuLayouts = 0;
    m_CurrentLayout = 0;
    m_CurrentWidth = 0;
    m_CurrentHeight = 0;

    m_MenuPageActionCallbackStruct.pFunc = 0;
    m_MenuPageActionCallbackStruct.pObj = 0;

    m_MenuPageVisibleCallbackStruct.pFunc = 0;
    m_MenuPageVisibleCallbackStruct.pObj = 0;

    // Register callbacks.
    MYFW_REGISTER_COMPONENT_CALLBACK( Tick );
    MYFW_REGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
    MYFW_REGISTER_COMPONENT_CALLBACK( Draw );
    MYFW_REGISTER_COMPONENT_CALLBACK( OnTouch );
    MYFW_REGISTER_COMPONENT_CALLBACK( OnButtons );
    MYFW_REGISTER_COMPONENT_CALLBACK( OnKeys );

#if MYFW_USING_WX
    m_ControlID_Filename = -1;
    h_RenameInProgress = false;
#endif
}

ComponentMenuPage::~ComponentMenuPage()
{
    SAFE_RELEASE( m_pMenuLayoutFile );

    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        SAFE_DELETE( m_pMenuItems[i] );
    }
    for( unsigned int i=m_MenuItemsUsed; i<MAX_MENU_ITEMS; i++ )
    {
        MyAssert( m_pMenuItems[i] == 0 );
    }

    cJSON_Delete( m_MenuLayouts );

    // Unregister callbacks.
    MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
}

void ComponentMenuPage::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;

    m_Visible = true;
    m_LayersThisExistsOn = Layer_HUD;

    SAFE_RELEASE( m_pMenuLayoutFile );
    m_MenuItemsCreated = false;
    m_LayoutChanged = false;

    // find the first ortho cam component in the scene, settle for a perspective if that's all there is.
    m_pComponentCamera = (ComponentCamera*)g_pComponentSystemManager->GetFirstComponentOfType( "CameraComponent" );
    MyAssert( m_pComponentCamera->IsA( "CameraComponent" ) );
    while( m_pComponentCamera != 0 && m_pComponentCamera->m_Orthographic == false )
    {
        m_pComponentCamera = (ComponentCamera*)g_pComponentSystemManager->GetNextComponentOfType( m_pComponentCamera );
        MyAssert( m_pComponentCamera == 0 || m_pComponentCamera->IsA( "CameraComponent" ) );
    }

    m_pComponentLuaScript = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX

#if LEGACYHACK
void ComponentMenuPage::LEGACYHACK_GrabMenuItemPointersFromCurrentScreen()
{
    Screen_Base* pScreen = g_pScreenManager->GetTopScreen();
    if( pScreen )
    {
        for( int i=0; i<MAX_MENUITEMS; i++ )
        {
            m_pMenuItems[m_MenuItemsUsed] = pScreen->GetMenuItem( i );
            if( m_pMenuItems[m_MenuItemsUsed] )
            {
                // Bring in the scrollbox menu items as static items in the menu page.
                if( m_pMenuItems[m_MenuItemsUsed]->m_MenuItemType == MIT_ScrollBox )
                {
                    MenuScrollBox* pScrollBox = (MenuScrollBox*)m_pMenuItems[m_MenuItemsUsed];
                    for( int i=0; i<pScrollBox->m_NumMenuItems; i++ )
                    {
                        m_pMenuItems[m_MenuItemsUsed] = pScrollBox->GetMenuItem( i );
                        if( m_pMenuItems[m_MenuItemsUsed] )
                        {
                            if( m_pMenuItems[m_MenuItemsUsed]->m_MenuItemType == MIT_Button )
                            {
                                MenuButton* pButton = (MenuButton*)m_pMenuItems[m_MenuItemsUsed];
                                sprintf_s( pButton->m_ButtonAction, 32, "%d", (int)pButton->m_ButtonAction[0] );
                                int bp = 1;
                            }
                            m_MenuItemsUsed++;
                        }
                    }
                }
                else
                {
                    if( m_pMenuItems[m_MenuItemsUsed]->m_MenuItemType == MIT_Button )
                    {
                        MenuButton* pButton = (MenuButton*)m_pMenuItems[m_MenuItemsUsed];
                        sprintf_s( pButton->m_ButtonAction, 32, "%d", (int)pButton->m_ButtonAction[0] );
                        int bp = 1;
                    }
                    m_MenuItemsUsed++;
                }
            }
        }
    }
}
#endif //LEGACYHACK

void ComponentMenuPage::SaveMenuPageToDisk(const char* fullpath)
{
    LOGInfo( LOGTag, "Saving Menu File: %s\n", fullpath );

    FILE* pFile = 0;
    fopen_s( &pFile, fullpath, "wb" );
    if( pFile == 0 )
    {
        // TODO: handle creation of folders if user types in something that doesn't exist.
        LOGError( LOGTag, "Menu file failed to save: %s - folder doesn't exist?\n", fullpath );
        return;
    }

    //cJSON* jMenuItems = Menu_ImportExport::ExportMenuLayout( m_pMenuItems, m_MenuItemsUsed );
    cJSON* jMenuItems = m_MenuLayouts;
    
    char* string = cJSON_Print( jMenuItems );
    //cJSON_Delete( jMenuItems );

    fprintf( pFile, "%s", string );
    fclose( pFile );

    cJSONExt_free( string );
}

void ComponentMenuPage::RenameMenuPage(const char* newfullpath)
{
    // TODO: clicking on the filename box should pop up a file-open dialog, instead of this mess.

    // if the filename didn't change, return.
    if( m_pMenuLayoutFile != 0 && strcmp( newfullpath, m_pMenuLayoutFile->m_FullPath ) == 0 )
        return;

    // if new file exists load it, otherwise delete this file, save as the new name and load it.
    if( m_pMenuLayoutFile != 0 )
    {
        // filename changed, delete the old file, save the new one and 
        if( m_pMenuLayoutFile->GetRefCount() == 1 )
        {
            LOGInfo( LOGTag, "Renaming menu file from %s to %s, file is not be deleted from disk.\n", m_pMenuLayoutFile->m_FullPath, newfullpath );

            m_pMenuLayoutFile->Release();
                    
            // TODO: all ref's are gone, delete the old file from disk.
        }
        else
        {
            m_pMenuLayoutFile->Release();
        }
    }

    // if a new file is requested, either create it or load it.
    if( newfullpath[0] != 0 )
    {
        if( g_pFileManager->DoesFileExist( newfullpath ) )
        {
            // launching this messagebox causes a change focus message on the editbox, causing OnValueChanged to get call again, avoiding issue with this hack.
            h_RenameInProgress = true;

            int answer = wxMessageBox( "File already exists.\nDiscard changes and Load it?", "Menu Load Confirm", wxYES_NO, g_pEngineMainFrame );

            // destroy the current menu items and load the new menu page.
            if( answer == wxYES )
            {
                ClearAllMenuItems();
                MyFileObject* pFile = g_pFileManager->RequestFile( newfullpath );
                SetMenuLayoutFile( pFile );
                pFile->Release();
                m_MenuItemsCreated = false;
                return; // new file was loaded and menu items from that file will be created in Tick.
            }

            // the file existed, so load it up and associate it with this menupage, but don't save over it unless scene is saved.
            //   the user might have just typed the wrong name.
            MyFileObject* pFile = g_pFileManager->RequestFile( newfullpath );
            SetMenuLayoutFile( pFile );
            pFile->Release();
            return;
        }

        // the file didn't exist, so save as new name
        SaveMenuPageToDisk( newfullpath );

        // request the file, so it's part of the scene.
        MyFileObject* pFile = g_pFileManager->RequestFile( newfullpath );
        SetMenuLayoutFile( pFile );
        pFile->Release();
    }

    h_RenameInProgress = false;
}

void ComponentMenuPage::SaveCurrentLayoutToJSON()
{
    if( m_CurrentWidth != 0 && m_CurrentHeight != 0 )
    {
        if( m_CurrentWidth == m_CurrentHeight )
        {
            m_CurrentLayout = SaveLayoutToJSON( "Square" );
        }
        else if( m_CurrentWidth < m_CurrentHeight )
        {
            m_CurrentLayout = SaveLayoutToJSON( "Tall" );
        }
        else
        {
            m_CurrentLayout = SaveLayoutToJSON( "Wide" );
            m_CurrentLayout = SaveLayoutToJSON( "Wide" );
        }
    }
}

cJSON* ComponentMenuPage::SaveLayoutToJSON(const char* layoutname)
{
    cJSON* layout = cJSON_GetObjectItem( m_MenuLayouts, layoutname );
    if( layout )
        cJSON_DeleteItemFromObject( m_MenuLayouts, layoutname );

    if( m_MenuItemsUsed > 0 )
    {
        layout = Menu_ImportExport::ExportMenuLayout( m_pMenuItems, m_MenuItemsUsed );
        cJSON_AddItemToObject( m_MenuLayouts, layoutname, layout );
    }

    return layout;
}

void ComponentMenuPage::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMenuPage::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Menu Page" );
}

void ComponentMenuPage::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentMenuPage::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Menu Page", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentBase::FillPropertiesWindow( clear );

        g_pPanelWatch->AddBool( "Visible", &m_Visible, 0, 1 );
        g_pPanelWatch->AddUnsignedInt( "Layers", &m_LayersThisExistsOn, 0, 63 );

        const char* desc = "none";
        if( m_pComponentCamera )
            desc = m_pComponentCamera->m_pGameObject->GetName();
        m_ControlID_ComponentCamera = g_pPanelWatch->AddPointerWithDescription( "Camera Component", m_pComponentCamera, desc, this, ComponentMenuPage::StaticOnDropComponent, ComponentMenuPage::StaticOnValueChanged );

        if( m_pMenuLayoutFile )
            m_ControlID_Filename = g_pPanelWatch->AddPointerWithDescription( "Menu file", m_pMenuLayoutFile, m_pMenuLayoutFile->m_FullPath, this, 0, ComponentMenuPage::StaticOnValueChanged );
        else
            m_ControlID_Filename = g_pPanelWatch->AddPointerWithDescription( "Menu file", m_pMenuLayoutFile, "no file", this, 0, ComponentMenuPage::StaticOnValueChanged );
    }
}

void ComponentMenuPage::AppendItemsToRightClickMenu(wxMenu* pMenu)
{
    ComponentBase::AppendItemsToRightClickMenu( pMenu );

    pMenu->Append( RightClick_AddButton, "Add Button" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );

    pMenu->Append( RightClick_AddSprite, "Add Sprite" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );

    pMenu->Append( RightClick_AddText, "Add Text" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );

#if LEGACYHACK
    pMenu->Append( 9876, "Grab menu item pointers from current screen" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );
#endif
}

void ComponentMenuPage::OnPopupClick(wxEvent &evt)
{
    ComponentBase::OnPopupClick( evt );

    ComponentMenuPage* pComponent = (ComponentMenuPage*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    MyAssert( pComponent->IsA( "MenuPageComponent" ) );

    wxTreeItemId componentID = g_pPanelObjectList->FindObject( pComponent );    

    int id = evt.GetId();
    if( id == RightClick_AddButton )
    {
        MenuButton* pMenuItem = MyNew MenuButton( 100 );
        pMenuItem->SetString( "Test Button" );
        pComponent->m_pMenuItems[pComponent->m_MenuItemsUsed] = pMenuItem;
        pComponent->m_MenuItemsUsed++;
        g_pPanelObjectList->AddObject( pMenuItem, MenuButton::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Button" );
        pMenuItem->RegisterMenuItemDeletedCallback( pComponent, StaticOnMenuItemDeleted );
    }

    if( id == RightClick_AddSprite )
    {
        MenuSprite* pMenuItem = MyNew MenuSprite();
        pComponent->m_pMenuItems[pComponent->m_MenuItemsUsed] = pMenuItem;
        pComponent->m_MenuItemsUsed++;
        g_pPanelObjectList->AddObject( pMenuItem, MenuSprite::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Sprite" );
        pMenuItem->RegisterMenuItemDeletedCallback( pComponent, StaticOnMenuItemDeleted );
    }

    if( id == RightClick_AddText )
    {
        MenuText* pMenuItem = MyNew MenuText();
        pComponent->m_pMenuItems[pComponent->m_MenuItemsUsed] = pMenuItem;
        pComponent->m_MenuItemsUsed++;
        g_pPanelObjectList->AddObject( pMenuItem, MenuText::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Text" );
        pMenuItem->RegisterMenuItemDeletedCallback( pComponent, StaticOnMenuItemDeleted );
    }

#if LEGACYHACK
    if( id == 9876 )
    {
        pComponent->ClearAllMenuItems();
        pComponent->LEGACYHACK_GrabMenuItemPointersFromCurrentScreen();
        //pComponent->SaveMenuPageToDisk( "Data/Menus/LegacyHackMenuPage.menu" );
        pComponent->SaveCurrentLayoutToJSON();
        pComponent->m_MenuItemsUsed = 0; // wipe old menu pointers, "screen_base" object should still have them
        for( unsigned int i=0; i<MAX_MENU_ITEMS; i++ )
            pComponent->m_pMenuItems[i] = 0;
        pComponent->UpdateLayout( pComponent->m_CurrentLayout );
    }
#endif
}

void ComponentMenuPage::OnValueChanged(int controlid, bool finishedchanging)
{
    if( finishedchanging && controlid == m_ControlID_Filename )
    {
        if( h_RenameInProgress == false )
        {
            wxString newfullpath = g_pPanelWatch->m_pVariables[controlid].m_Handle_TextCtrl->GetValue();
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( controlid, newfullpath );
            RenameMenuPage( newfullpath );
            if( m_pMenuLayoutFile )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( controlid, m_pMenuLayoutFile->m_FullPath );
            }
            else
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( controlid, "no file" );
            }
        }
    }
}

void ComponentMenuPage::OnMenuItemDeleted(MenuItem* pMenuItem)
{
    bool found = false;
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        if( found == false && m_pMenuItems[i] == pMenuItem )
        {
            SAFE_DELETE( m_pMenuItems[i] );
            found = true;
        }

        if( found )
        {
            if( i == m_MenuItemsUsed-1 )
                m_pMenuItems[i] = 0;
            else
                m_pMenuItems[i] = m_pMenuItems[i+1];
        }
    }

    if( found )
    {
        g_pPanelObjectList->RemoveObject( pMenuItem );
        m_MenuItemsUsed -= 1;
    }

    if( pMenuItem == m_pInputBoxWithKeyboardFocus )
        m_pInputBoxWithKeyboardFocus = 0;

    MyAssert( found );
}

void ComponentMenuPage::OnDropComponent(int controlid, wxCoord x, wxCoord y)
{
    ComponentBase* pComponent = 0;

    if( controlid == m_ControlID_ComponentCamera )
    {
        if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
        {
            pComponent = (ComponentBase*)g_DragAndDropStruct.m_Value;
        }

        if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
        {
            pComponent = ((GameObject*)g_DragAndDropStruct.m_Value)->GetFirstComponentOfBaseType( BaseComponentType_Camera );
        }

        if( pComponent && pComponent != this )
        {
            if( pComponent->IsA( "CameraComponent" ) )
            {
                assert( dynamic_cast<ComponentCamera*>( pComponent ) );
                m_pComponentCamera = dynamic_cast<ComponentCamera*>( pComponent );
            }

            // update the panel so new OBJ name shows up.
            if( m_pComponentCamera )
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( controlid, m_pComponentCamera->m_pGameObject->GetName() );
            else
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( controlid, "none" );
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMenuPage::ExportAsJSONObject(bool savesceneid)
{
#if MYFW_USING_WX
    // Scene is saving so also save menu file to disk.
    if( m_pMenuLayoutFile == 0 )
    {
        wxFileDialog FileDialog( g_pEngineMainFrame, _("Save Menu file"), "./Data/Menus", "", "Menu files (*.menu)|*.menu", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
        if( FileDialog.ShowModal() != wxID_CANCEL )
        {
            wxString wxpath = FileDialog.GetPath();
            char fullpath[MAX_PATH];
            sprintf_s( fullpath, MAX_PATH, "%s", (const char*)wxpath );
            const char* relativepath = g_pEngineMainFrame->GetRelativePath( fullpath );
            m_pMenuLayoutFile = g_pFileManager->RequestFile( relativepath );
        }
    }

    if( m_pMenuLayoutFile )
        SaveMenuPageToDisk( m_pMenuLayoutFile->m_FullPath );
    else
        LOGError( LOGTag, "MENU FILE NOT SAVED!\n" );

    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );

    cJSON_AddNumberToObject( jComponent, "Visible", m_Visible );
    cJSON_AddNumberToObject( jComponent, "Layers", m_LayersThisExistsOn );

    if( m_pMenuLayoutFile )
        cJSON_AddStringToObject( jComponent, "MenuFile", m_pMenuLayoutFile->m_FullPath );
    
    return jComponent;
#else
    MyAssert( false ); // no saving menus in runtime.
    return 0;
#endif //MYFW_USING_WX
}

void ComponentMenuPage::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jComponent, sceneid );

    cJSONExt_GetBool( jComponent, "Visible", &m_Visible );
    cJSONExt_GetUnsignedInt( jComponent, "Layers", &m_LayersThisExistsOn );

    cJSON* jFilename = cJSON_GetObjectItem( jComponent, "MenuFile" );
    if( jFilename )
    {
        MyFileObject* pFile = g_pFileManager->RequestFile( jFilename->valuestring ); // will add ref.
        SetMenuLayoutFile( pFile ); // will add ref.
        pFile->Release();
    }
}

ComponentMenuPage& ComponentMenuPage::operator=(const ComponentMenuPage& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    this->m_Visible = other.m_Visible;
    this->m_LayersThisExistsOn = other.m_LayersThisExistsOn;

    SetMenuLayoutFile( other.m_pMenuLayoutFile );

    return *this;
}

void ComponentMenuPage::FindLuaScriptComponentPointer()
{
    // if we don't have a luascript, find the first one attached to this gameobject
    if( m_pComponentLuaScript == 0 )
    {
        m_pComponentLuaScript = (ComponentLuaScript*)m_pGameObject->GetFirstComponentOfType( "LuaScriptComponent" );
    }
}

void ComponentMenuPage::OnLoad()
{
    // if we don't have a luascript, find the first one attached to this gameobject
    FindLuaScriptComponentPointer();
}

void ComponentMenuPage::OnPlay()
{
    // if we don't have a luascript, find the first one attached to this gameobject
    FindLuaScriptComponentPointer();
}

// will return true if input is used.
bool ComponentMenuPage::Callback_OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    if( m_Visible == false )
        return false;

    //ComponentBase::Callback_OnTouch( action, id, x, y, pressure, size );

    switch( action )
    {
    case GCBA_Down: // new finger down
        {
            //g_Fingers[id].set( x, y, id );

            // hold down the first button we collide with.
            for( int i=m_MenuItemsUsed-1; i>=0; i-- )
            {
                if( m_pMenuItems[i] )
                {
                    if( m_pMenuItems[i]->HoldOnCollision( id, x, y, true ) )
                        return true;
                }
            }
        }
        break;

    case GCBA_Held: // any finger might have moved
        {
            int fingerindex = id; //-1;

            //for( int i=0; i<10; i++ )
            //{
            //    if( g_Fingers[i].id == id )
            //        fingerindex = i;
            //}

            if( fingerindex != -1 )
            {
                //g_Fingers[fingerindex].set( x, y, id );

                //BasicMenuHandleHeldFunc( fingerindex, x, y );
                if( fingerindex == -1 )
                    return false;

                for( int i=m_MenuItemsUsed-1; i>=0; i-- )
                {
                    if( m_pMenuItems[i] )
                    {
                        m_pMenuItems[i]->ReleaseOnNoCollision( fingerindex, x, y );
                    }
                }
            }
        }
        break;

    case GCBA_Up: // any finger up
        {
            //g_Fingers[id].reset();

            for( int i=m_MenuItemsUsed-1; i>=0; i-- )
            {
                if( m_pMenuItems[i] )
                {
                    const char* action = m_pMenuItems[i]->TriggerOnCollision( id, x, y, true );

                    // set this menu item as the selected input box.
                    if( m_pMenuItems[i]->m_MenuItemType == MIT_InputBox )
                        m_pInputBoxWithKeyboardFocus = (MenuInputBox*)m_pMenuItems[i];

                    if( action != 0 )//&& OnMenuAction( action ) )
                    {
                        if( m_MenuPageActionCallbackStruct.pFunc )
                        {
                            m_MenuPageActionCallbackStruct.pFunc( m_MenuPageActionCallbackStruct.pObj, this, action, m_pMenuItems[i] );
                        }
#if MYFW_USING_WX
                        // in editor, there's a chance the script component was created and not associated with this object.
                        FindLuaScriptComponentPointer();
#endif
                        if( m_pComponentLuaScript )
                        {
                            m_pComponentLuaScript->CallFunction( action );//"PressedMenuButton" );
                            return true;
                        }

                        return true;
                    }
                }
            }
        }
        break;
    }

    return false;
}

// will return true if input is used.
bool ComponentMenuPage::Callback_OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    if( m_Visible == false )
        return false;

    //ComponentBase::Callback_OnButtons( action, id );

    return false;
}

// will return true if input is used.
bool ComponentMenuPage::Callback_OnKeys(GameCoreButtonActions action, int keycode, int unicodechar)
{
    if( m_Visible == false )
        return false;

    //ComponentBase::Callback_OnButtons( action, keycode, unicodechar );

    //if( Screen_Base::OnKeyDown( keycode, unicodechar ) )
    //    return true;

    //LOGInfo( LOGTag, "Screen_PlayerCreation::OnKeyDown %p\n", m_pInputBoxWithKeyboardFocus );

    if( action == GCBA_Down )
    {
        if( m_pInputBoxWithKeyboardFocus )
        {
            return m_pInputBoxWithKeyboardFocus->OnKeyDown( keycode, unicodechar );
        }
    }

    return false;
}

void ComponentMenuPage::Callback_Tick(double TimePassed)
{
    //ComponentBase::Callback_Tick( TimePassed );

    if( m_MenuItemsCreated == false )
    {
        if( m_pMenuLayoutFile && m_pMenuLayoutFile->m_FileLoadStatus == FileLoadStatus_Success )
        {
            MyAssert( m_MenuLayouts == 0 );
            m_MenuLayouts = cJSON_Parse( m_pMenuLayoutFile->m_pBuffer );
            if( m_MenuLayouts == 0 )
                m_MenuLayouts = cJSON_CreateObject();

            LoadLayoutBasedOnCurrentAspectRatio();

            m_MenuItemsCreated = true;
        }
    }

    // Tick all the menu items.
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        if( m_pMenuItems[i] )
        {
            m_pMenuItems[i]->Tick( TimePassed );
        }
    }
}

void ComponentMenuPage::Callback_OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    //ComponentBase::Callback_OnSurfaceChanged( TimePassed );

#if MYFW_USING_WX
    if( m_CurrentWidth != 0 && m_CurrentHeight != 0 )
    {
        if( m_Visible )
        {
            if( m_MenuItemsCreated == true )
                SaveCurrentLayoutToJSON();
        }
    }
#endif //MYFW_USING_WX

    m_CurrentWidth = m_pComponentCamera->m_WindowWidth; //width;//desiredaspectwidth;
    m_CurrentHeight = m_pComponentCamera->m_WindowHeight; //height;//desiredaspectheight;

    // since we reloaded all items, trigger the onvisible callback.
    if( m_Visible )
    {
        LoadLayoutBasedOnCurrentAspectRatio();
        m_Visible = false;
        SetVisible( true );
    }
    else
    {
        m_LayoutChanged = true;
    }
}

void ComponentMenuPage::LoadLayoutBasedOnCurrentAspectRatio()
{
    if( m_MenuLayouts == 0 )
        return;

    cJSON* obj = 0;

    if( m_CurrentWidth == m_CurrentHeight )
    {
        obj = cJSON_GetObjectItem( m_MenuLayouts, "Square" );
    }
    else if( m_CurrentWidth < m_CurrentHeight )
    {
        obj = cJSON_GetObjectItem( m_MenuLayouts, "Tall" );
    }
    else
    {
        obj = cJSON_GetObjectItem( m_MenuLayouts, "Wide" );
    }

    if( obj != 0 )
        UpdateLayout( obj );

    // grab the first input box, if there is one and set it as focused.
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        MyAssert( m_pMenuItems[i] );
        if( m_pMenuItems[i] && m_pMenuItems[i]->m_MenuItemType == MIT_InputBox )
            m_pInputBoxWithKeyboardFocus = (MenuInputBox*)m_pMenuItems[i];
    }
}

void ComponentMenuPage::UpdateLayout(cJSON* layout)
{
    m_CurrentLayout = layout;

    ClearAllMenuItems();
    m_MenuItemsUsed = Menu_ImportExport::ImportMenuLayout( m_CurrentLayout, m_pMenuItems, MAX_MENU_ITEMS );

#if MYFW_USING_WX
    wxTreeItemId componentID = g_pPanelObjectList->FindObject( this );    
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        switch( m_pMenuItems[i]->m_MenuItemType )
        {
        case MIT_Sprite:
            g_pPanelObjectList->AddObject( m_pMenuItems[i], MenuSprite::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Sprite" );
            break;

        case MIT_Text:
            g_pPanelObjectList->AddObject( m_pMenuItems[i], MenuText::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Text" );
            break;

        case MIT_Button:
            g_pPanelObjectList->AddObject( m_pMenuItems[i], MenuButton::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Button" );
            break;

        case MIT_InputBox:
            g_pPanelObjectList->AddObject( m_pMenuItems[i], MenuButton::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "InputBox" );
            break;

        case MIT_ScrollingText:
            g_pPanelObjectList->AddObject( m_pMenuItems[i], MenuButton::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "ScrollingText" );
            break;

        case MIT_Base:
        case MIT_ScrollBox:
        case MIT_CheckBox:
        case MIT_NumMenuItemTypes:
        default:
            MyAssert( false );
        }
    }
#endif //MYFW_USING_WX
}

void ComponentMenuPage::Callback_Draw(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    //ComponentBase::Callback_Draw( pCamera, pMatViewProj, pShaderOverride );

    if( m_Visible == false || (m_LayersThisExistsOn & pCamera->m_LayersToRender) == 0 )
        return;

    if( m_pComponentCamera == 0 )
        return;

    glDisable( GL_DEPTH_TEST );
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        if( m_pMenuItems[i] )
        {
            m_pMenuItems[i]->Draw( &m_pComponentCamera->m_Camera2D.m_matViewProj );
        }
    }
    glEnable( GL_DEPTH_TEST );
}

void ComponentMenuPage::ClearAllMenuItems()
{
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
#if MYFW_USING_WX
        g_pPanelObjectList->RemoveObject( m_pMenuItems[i] );
#endif //MYFW_USING_WX

        SAFE_DELETE( m_pMenuItems[i] );
    }
    m_MenuItemsUsed = 0;
    m_pInputBoxWithKeyboardFocus = 0;
}

void ComponentMenuPage::SetMenuLayoutFile(MyFileObject* pFile)
{
    if( pFile )
        pFile->AddRef();
    SAFE_RELEASE( m_pMenuLayoutFile );
    m_pMenuLayoutFile = pFile;
}

MenuItem* ComponentMenuPage::GetMenuItemByName(const char* name)
{
    for( int i=0; i<MAX_MENU_ITEMS; i++ )
    {
        if( m_pMenuItems[i] && strcmp( m_pMenuItems[i]->m_Name, name ) == 0 )
        {
            return m_pMenuItems[i];
        }
    }

    return 0;
}

void ComponentMenuPage::RegisterMenuPageActionCallback(void* pObj, MenuPageActionCallbackFunc pFunc)
{
    m_MenuPageActionCallbackStruct.pFunc = pFunc;
    m_MenuPageActionCallbackStruct.pObj = pObj;
}

void ComponentMenuPage::RegisterMenuPageVisibleCallback(void* pObj, MenuPageVisibleCallbackFunc pFunc)
{
    m_MenuPageVisibleCallbackStruct.pFunc = pFunc;
    m_MenuPageVisibleCallbackStruct.pObj = pObj;
}

void ComponentMenuPage::SetVisible(bool visible)
{
    if( m_Visible == visible )
        return;

    if( visible == true && m_LayoutChanged )
    {
        // recreate all the menu items
        LoadLayoutBasedOnCurrentAspectRatio();
    }

    m_Visible = visible;

    if( m_MenuPageVisibleCallbackStruct.pFunc )
    {
        m_MenuPageVisibleCallbackStruct.pFunc( m_MenuPageVisibleCallbackStruct.pObj, this, visible );
    }
}
