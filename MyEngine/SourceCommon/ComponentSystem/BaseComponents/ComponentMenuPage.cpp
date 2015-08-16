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

    m_InputEnabled = true;
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

    m_MenuPageTickCallbackStruct.pFunc = 0;
    m_MenuPageTickCallbackStruct.pObj = 0;

    m_MenuPageDrawCallbackStruct.pFunc = 0;
    m_MenuPageDrawCallbackStruct.pObj = 0;

    m_MenuPageOnTouchCallbackStruct.pFunc = 0;
    m_MenuPageOnTouchCallbackStruct.pObj = 0;

    m_MenuPageOnButtonsCallbackStruct.pFunc = 0;
    m_MenuPageOnButtonsCallbackStruct.pObj = 0;

    m_MenuPageOnKeysCallbackStruct.pFunc = 0;
    m_MenuPageOnKeysCallbackStruct.pObj = 0;

    // Runtime vars
    m_ItemSelected = -1;

    m_ExtentsSetWhenLoaded = false;

    for( int i=0; i<3; i++ )
    {
        m_ButtonActions[i][0] = 0;
    }

    m_RelativeCursorSize.Set( 0, 0 );

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
}

void ComponentMenuPage::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;

    m_Visible = true;
    m_LayersThisExistsOn = Layer_HUD;

    m_InputEnabled = true;

    SAFE_RELEASE( m_pMenuLayoutFile );
    m_MenuItemsCreated = false;

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

ComponentMenuPage* CastAs_ComponentMenuPage(ComponentBase* pComponent)
{
    MyAssert( pComponent->IsA( "MenuPageComponent" ) );
    return (ComponentMenuPage*)pComponent;
}

void ComponentMenuPage::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate ).addFunction( "CastAs_ComponentMenuPage", CastAs_ComponentMenuPage );

    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentMenuPage>( "ComponentMenuPage" )
            //.addData( "localmatrix", &ComponentMenuPage::m_LocalTransform )
            
            .addFunction( "GetMenuItemByName", &ComponentMenuPage::GetMenuItemByName )
            .addFunction( "IsEnabled", &ComponentMenuPage::IsEnabled )
            
            .addFunction( "SetSceneID", &ComponentMenuPage::SetSceneID )
            .addFunction( "GetSceneID", &ComponentMenuPage::GetSceneID )
            
            .addFunction( "SetID", &ComponentMenuPage::SetID )
            .addFunction( "GetID", &ComponentMenuPage::GetID )
        .endClass();
}

ComponentMenuPage& ComponentMenuPage::operator=(const ComponentMenuPage& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    this->m_Visible = other.m_Visible;
    this->m_InputEnabled = other.m_InputEnabled;
    this->m_LayersThisExistsOn = other.m_LayersThisExistsOn;

    for( int i=0; i<3; i++ )
    {
        strcpy_s( m_ButtonActions[i], MAX_BUTTON_ACTION_LENGTH, other.m_ButtonActions[i] );
    }

    SetMenuLayoutFile( other.m_pMenuLayoutFile );

    return *this;
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

void ComponentMenuPage::SaveMenuPageToDisk()
{
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
}

void ComponentMenuPage::SaveMenuPageToDisk(const char* fullpath)
{
    if( m_MenuLayouts == 0 )
        return;

    LOGInfo( LOGTag, "Saving Menu File: %s\n", fullpath );

    SaveCurrentLayoutToJSON();

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
    if( m_MenuLayouts == 0 )
        return;

    if( m_CurrentWidth != 0 && m_CurrentHeight != 0 )
    {
        cJSON* square = cJSON_DetachItemFromObject( m_MenuLayouts, "Square" );
        cJSON* tall = cJSON_DetachItemFromObject( m_MenuLayouts, "Tall" );
        cJSON* wide = cJSON_DetachItemFromObject( m_MenuLayouts, "Wide" );

        if( m_ExtentsSetWhenLoaded == false )
        {
            m_ExtentsBLTRWhenPageLoaded.x = m_pComponentCamera->m_Camera2D.m_OrthoBottom;
            m_ExtentsBLTRWhenPageLoaded.y = m_pComponentCamera->m_Camera2D.m_OrthoLeft;
            m_ExtentsBLTRWhenPageLoaded.z = m_pComponentCamera->m_Camera2D.m_OrthoTop;
            m_ExtentsBLTRWhenPageLoaded.w = m_pComponentCamera->m_Camera2D.m_OrthoRight;
        }

        if( m_MenuItemsUsed > 0 )
        {
            if( m_CurrentWidth == m_CurrentHeight )
            {
                if( square )
                    cJSON_Delete( square );
                square = Menu_ImportExport::ExportMenuLayout( m_pMenuItems, m_MenuItemsUsed, m_ExtentsBLTRWhenPageLoaded );
            }
            else if( m_CurrentWidth < m_CurrentHeight )
            {
                if( tall )
                    cJSON_Delete( tall );
                tall = Menu_ImportExport::ExportMenuLayout( m_pMenuItems, m_MenuItemsUsed, m_ExtentsBLTRWhenPageLoaded );
            }
            else
            {
                if( wide )
                    cJSON_Delete( wide );
                wide = Menu_ImportExport::ExportMenuLayout( m_pMenuItems, m_MenuItemsUsed, m_ExtentsBLTRWhenPageLoaded );
            }
        }

        if( wide )
            cJSON_AddItemToObject( m_MenuLayouts, "Wide", wide );
        if( tall )
            cJSON_AddItemToObject( m_MenuLayouts, "Tall", tall );
        if( square )
            cJSON_AddItemToObject( m_MenuLayouts, "Square", square );
    }
}

void ComponentMenuPage::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId treeid = g_pPanelObjectList->AddObject( this, ComponentMenuPage::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Menu Page" );
    g_pPanelObjectList->SetDragAndDropFunctions( treeid, 0, ComponentMenuPage::StaticOnDropMenuItemOnMenuPage );
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
        g_pPanelWatch->AddBool( "Input Enabled", &m_InputEnabled, 0, 1 );
        
        g_pPanelWatch->AddUnsignedInt( "Layers", &m_LayersThisExistsOn, 0, 63 );

        const char* desc = "none";
        if( m_pComponentCamera )
            desc = m_pComponentCamera->m_pGameObject->GetName();
        m_ControlID_ComponentCamera = g_pPanelWatch->AddPointerWithDescription( "Camera Component", m_pComponentCamera, desc, this, ComponentMenuPage::StaticOnDropComponent, ComponentMenuPage::StaticOnValueChanged );

        if( m_pMenuLayoutFile )
            m_ControlID_Filename = g_pPanelWatch->AddPointerWithDescription( "Menu file", m_pMenuLayoutFile, m_pMenuLayoutFile->m_FullPath, this, 0, ComponentMenuPage::StaticOnValueChanged );
        else
            m_ControlID_Filename = g_pPanelWatch->AddPointerWithDescription( "Menu file", m_pMenuLayoutFile, "no file", this, 0, ComponentMenuPage::StaticOnValueChanged );

        g_pPanelWatch->AddString( "Action B", &m_ButtonActions[0][0], MAX_BUTTON_ACTION_LENGTH );
        g_pPanelWatch->AddString( "Action C", &m_ButtonActions[1][0], MAX_BUTTON_ACTION_LENGTH );
        g_pPanelWatch->AddString( "Action D", &m_ButtonActions[2][0], MAX_BUTTON_ACTION_LENGTH );

        g_pPanelWatch->Add2Floats( "Cursor Size", "x", "y", &m_RelativeCursorSize.x, &m_RelativeCursorSize.y, -10, 10 );
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

    pMenu->AppendSeparator();

    pMenu->Append( RightClick_ForceReload, "Force Reload" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );

    pMenu->Append( RightClick_SavePage, "Save Page" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );

    pMenu->AppendSeparator();

    pMenu->Append( RightClick_CopyToOtherLayouts, "Copy unique items to other layouts" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );

#if LEGACYHACK
    pMenu->AppendSeparator();

    pMenu->Append( 9876, "Grab menu item pointers from current screen" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );
#endif
}

void ComponentMenuPage::OnPopupClick(wxEvent &evt)
{
    ComponentBase::OnPopupClick( evt );

    ComponentMenuPage* pComponent = (ComponentMenuPage*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    MyAssert( pComponent->IsA( "MenuPageComponent" ) );

    int id = evt.GetId();

    switch( id )
    {
    case RightClick_AddButton:          pComponent->AddNewMenuItemToTree( MIT_Button );     break;
    case RightClick_AddSprite:          pComponent->AddNewMenuItemToTree( MIT_Sprite );     break;
    case RightClick_AddText:            pComponent->AddNewMenuItemToTree( MIT_Text );       break;
    case RightClick_ForceReload:        pComponent->LoadLayoutBasedOnCurrentAspectRatio();  break;
    case RightClick_SavePage:           pComponent->SaveMenuPageToDisk();                   break;
    case RightClick_CopyToOtherLayouts: pComponent->CopyUniqueItemsToOtherLayouts();        break;
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

MenuItem* ComponentMenuPage::AddNewMenuItemToTree(int type)
{
    MenuItem* pMenuItem = 0;
    PanelObjectListCallbackLeftClick pLeftClickFunc = 0;
    const char* desc = 0;

    if( type == MIT_Button )
    {
        pMenuItem = MyNew MenuButton( 100 );
        ((MenuButton*)pMenuItem)->SetString( "Test Button" );
        pLeftClickFunc = MenuButton::StaticFillPropertiesWindow;
        desc = "Button";
    }
    else if( type == MIT_Sprite )
    {
        pMenuItem = MyNew MenuSprite();
        pLeftClickFunc = MenuSprite::StaticFillPropertiesWindow;
        desc = "Sprite";
    }
    else if( type == MIT_Text )
    {
        pMenuItem = MyNew MenuText();
        pLeftClickFunc = MenuText::StaticFillPropertiesWindow;
        desc = "Text";
    }

    MyAssert( pMenuItem != 0 );
    if( pMenuItem == 0 )
        return 0;

    m_pMenuItems[m_MenuItemsUsed] = pMenuItem;
    m_MenuItemsUsed++;

    AddMenuItemToTree( m_MenuItemsUsed-1, pLeftClickFunc, desc );

    return pMenuItem;
}

void ComponentMenuPage::AddMenuItemToTree(unsigned int index, PanelObjectListCallbackLeftClick pLeftClickFunc, const char* desc)
{
    MenuItem* pMenuItem = m_pMenuItems[index];

    wxTreeItemId componentID = g_pPanelObjectList->FindObject( this );

    wxTreeItemId treeid = g_pPanelObjectList->AddObject( pMenuItem, pLeftClickFunc, StaticOnMenuItemRightClick, componentID, desc );
    g_pPanelObjectList->SetLabelEditFunction( treeid, MenuItem::StaticOnLabelEdit );
    g_pPanelObjectList->SetDragAndDropFunctions( treeid, MenuItem::StaticOnDrag, ComponentMenuPage::StaticOnDropMenuItemOnMenuItem );
    g_pPanelObjectList->SetCustomObjectForCallback_RightClick( treeid, this );
    g_pPanelObjectList->SetCustomObjectForCallback_Drop( treeid, this );
    pMenuItem->RegisterMenuItemDeletedCallback( this, StaticOnMenuItemDeleted );
}

void ComponentMenuPage::CopyUniqueItemsToOtherLayouts()
{
    SaveCurrentLayoutToJSON();

    cJSON* jLayout = 0;
    cJSON* jOtherLayouts[2];

    if( m_CurrentWidth == m_CurrentHeight )
    {
        jLayout = cJSON_GetObjectItem( m_MenuLayouts, "Square" );
        jOtherLayouts[0] = cJSON_GetObjectItem( m_MenuLayouts, "Tall" );
        jOtherLayouts[1] = cJSON_GetObjectItem( m_MenuLayouts, "Wide" );
    }
    else if( m_CurrentWidth < m_CurrentHeight )
    {
        jLayout = cJSON_GetObjectItem( m_MenuLayouts, "Tall" );
        jOtherLayouts[0] = cJSON_GetObjectItem( m_MenuLayouts, "Square" );
        jOtherLayouts[1] = cJSON_GetObjectItem( m_MenuLayouts, "Wide" );
    }
    else
    {
        jLayout = cJSON_GetObjectItem( m_MenuLayouts, "Wide" );
        jOtherLayouts[0] = cJSON_GetObjectItem( m_MenuLayouts, "Square" );
        jOtherLayouts[1] = cJSON_GetObjectItem( m_MenuLayouts, "Tall" );
    }

    unsigned int numitems = cJSON_GetArraySize( jLayout );
    
    // loop through the other 2 layouts.
    for( int otherlayout=0; otherlayout<2; otherlayout++ )
    {
        cJSON* jOtherLayout = jOtherLayouts[otherlayout];

        unsigned int numotheritems = cJSON_GetArraySize( jOtherLayout );

        // loop through all the numitems in the current layout.
        for( unsigned int i=0; i<numitems; i++ )
        {
            cJSON* jMenuItem = cJSON_GetArrayItem( jLayout, i );

            char name[MenuItem::MAX_MENUITEM_NAME_LENGTH];
            cJSONExt_GetString( jMenuItem, "Name", name, MenuItem::MAX_MENUITEM_NAME_LENGTH );

            bool found = false;

            // for each item, check if that item exists in the other layout, based on the items name.
            for( unsigned int otheri=0; otheri<numotheritems; otheri++ )
            {
                cJSON* jOtherMenuItem = cJSON_GetArrayItem( jOtherLayout, otheri );

                char othername[MenuItem::MAX_MENUITEM_NAME_LENGTH];
                cJSONExt_GetString( jOtherMenuItem, "Name", othername, MenuItem::MAX_MENUITEM_NAME_LENGTH );

                if( strcmp( name, othername ) == 0 )
                {
                    found = true;
                    break;
                }
            }

            if( found == false )
            {
                cJSON* jDuplicate = cJSON_Duplicate( jMenuItem, 1 );
                cJSON_AddItemToArray( jOtherLayout, jDuplicate );
            }
        }
    }
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

MYFW_PANELOBJECTLIST_DECLARE_CALLBACK_ONDROP(OnDropMenuItemOnMenuItem, ComponentMenuPage)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_MenuItem )
    {
        MenuItem* pMenuItemDropped = (MenuItem*)g_DragAndDropStruct.m_Value;
        MenuItem* pMenuItemDroppedOn = (MenuItem*)g_pPanelObjectList->GetObject( id );

        // place MenuItem dropped after the one is was dropped on.
        unsigned int iDropped;
        for( iDropped=0; iDropped<m_MenuItemsUsed; iDropped++ )
        {
            if( m_pMenuItems[iDropped] == pMenuItemDropped )
                break;
        }

        unsigned int iDroppedOn;
        for( iDroppedOn=0; iDroppedOn<m_MenuItemsUsed; iDroppedOn++ )
        {
            if( m_pMenuItems[iDroppedOn] == pMenuItemDroppedOn )
                break;
        }

        if( iDropped < iDroppedOn ) // dragged an item from above, so shift all items up.
        {
            for( unsigned int i=iDropped; i<iDroppedOn; i++ )
            {
                m_pMenuItems[i] = m_pMenuItems[i+1];
            }
            m_pMenuItems[iDroppedOn] = pMenuItemDropped;
        }
        else // dragged an item from below, so shift all items down.
        {
            for( unsigned int i=iDropped; i>iDroppedOn+1; i-- )
            {
                m_pMenuItems[i] = m_pMenuItems[i-1];
            }
            m_pMenuItems[iDroppedOn+1] = pMenuItemDropped;
        }

        g_pPanelObjectList->Tree_MoveObject( this, pMenuItemDropped, pMenuItemDroppedOn );
    }
}

MYFW_PANELOBJECTLIST_DECLARE_CALLBACK_ONDROP(OnDropMenuItemOnMenuPage, ComponentMenuPage)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_MenuItem )
    {
        MenuItem* pMenuItemDropped = (MenuItem*)g_DragAndDropStruct.m_Value;

        unsigned int iDropped;
        for( iDropped=0; iDropped<m_MenuItemsUsed; iDropped++ )
        {
            if( m_pMenuItems[iDropped] == pMenuItemDropped )
                break;
        }

        // place MenuItem dropped at start of list.
        for( unsigned int i=iDropped; i>0; i-- )
        {
            m_pMenuItems[i] = m_pMenuItems[i-1];
        }

        m_pMenuItems[0] = pMenuItemDropped;
        g_pPanelObjectList->Tree_MoveObject( this, pMenuItemDropped, 0 );
    }
}

void ComponentMenuPage::OnMenuItemRightClick(wxTreeItemId id)
{
 	wxMenu menu;
    menu.SetClientData( &m_MenuPageEventHandlerForMenuItems );

    m_MenuPageEventHandlerForMenuItems.pMenuPageSelected = this;
    m_MenuPageEventHandlerForMenuItems.pMenuItemSelected = (MenuItem*)g_pPanelObjectList->GetObject( id );

    menu.Append( 1000, "Delete Menu Item" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPageEventHandlerForMenuItems::OnPopupClick );

    menu.Append( 1001, "Copy Menu Item" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPageEventHandlerForMenuItems::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentMenuPageEventHandlerForMenuItems::OnPopupClick(wxEvent &evt)
{
    ComponentMenuPageEventHandlerForMenuItems* pEvtHandler = (ComponentMenuPageEventHandlerForMenuItems*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    ComponentMenuPage* pMenuPage = pEvtHandler->pMenuPageSelected;
    MenuItem* pMenuItem = pEvtHandler->pMenuItemSelected;

    int id = evt.GetId();
    if( id == 1000 )
    {
        MyAssert( pMenuItem != 0 );
        if( pMenuItem && pMenuItem->m_MenuItemDeletedCallbackStruct.pFunc )
            pMenuItem->m_MenuItemDeletedCallbackStruct.pFunc( pMenuItem->m_MenuItemDeletedCallbackStruct.pObj, pMenuItem );
    }

    if( id == 1001 )
    {
        MyAssert( pMenuPage != 0 && pMenuItem != 0 );
        if( pMenuPage != 0 && pMenuItem != 0 )
        {
            MenuItem* pNewMenuItem = 0;

            if( pMenuItem->m_MenuItemType == MIT_Button )
            {
                pNewMenuItem = pMenuPage->AddNewMenuItemToTree( MIT_Button );
                *(MenuButton*)pNewMenuItem = *(MenuButton*)pMenuItem;
                pNewMenuItem->SetPosition( pNewMenuItem->m_Position.x + 20, pNewMenuItem->m_Position.y + 20 );
            }
            if( pMenuItem->m_MenuItemType == MIT_Sprite )
            {
                pNewMenuItem = pMenuPage->AddNewMenuItemToTree( MIT_Sprite );
                *(MenuSprite*)pNewMenuItem = *(MenuSprite*)pMenuItem;
            }
            if( pMenuItem->m_MenuItemType == MIT_Text )
            {
                pNewMenuItem = pMenuPage->AddNewMenuItemToTree( MIT_Text );
                *(MenuText*)pNewMenuItem = *(MenuText*)pMenuItem;
            }
        }
    }
}

#endif //MYFW_USING_WX

cJSON* ComponentMenuPage::ExportAsJSONObject(bool savesceneid)
{
#if MYFW_USING_WX
    SaveMenuPageToDisk();

    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );

    cJSON_AddNumberToObject( jComponent, "Visible", m_Visible );
    cJSON_AddNumberToObject( jComponent, "InputEnabled", m_InputEnabled );
    
    cJSON_AddNumberToObject( jComponent, "Layers", m_LayersThisExistsOn );

    if( m_pMenuLayoutFile )
        cJSON_AddStringToObject( jComponent, "MenuFile", m_pMenuLayoutFile->m_FullPath );
    
    if( m_ButtonActions[0][0] != 0 ) cJSON_AddStringToObject( jComponent, "ActionButtonB", m_ButtonActions[0] );
    if( m_ButtonActions[1][0] != 0 ) cJSON_AddStringToObject( jComponent, "ActionButtonC", m_ButtonActions[1] );
    if( m_ButtonActions[2][0] != 0 ) cJSON_AddStringToObject( jComponent, "ActionButtonD", m_ButtonActions[2] );

    if( m_RelativeCursorSize.x != 0 || m_RelativeCursorSize.y != 0 )
        cJSONExt_AddFloatArrayToObject( jComponent, "RelativeCursorSize", &m_RelativeCursorSize.x, 2 );

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
    cJSONExt_GetBool( jComponent, "InputEnabled", &m_InputEnabled );
    cJSONExt_GetUnsignedInt( jComponent, "Layers", &m_LayersThisExistsOn );

    cJSON* jFilename = cJSON_GetObjectItem( jComponent, "MenuFile" );
    if( jFilename )
    {
        MyFileObject* pFile = g_pFileManager->RequestFile( jFilename->valuestring ); // will add ref.
        SetMenuLayoutFile( pFile ); // will add ref.
        pFile->Release();
    }

    cJSONExt_GetString( jComponent, "ActionButtonB", m_ButtonActions[0], MAX_BUTTON_ACTION_LENGTH );
    cJSONExt_GetString( jComponent, "ActionButtonC", m_ButtonActions[1], MAX_BUTTON_ACTION_LENGTH );
    cJSONExt_GetString( jComponent, "ActionButtonD", m_ButtonActions[2], MAX_BUTTON_ACTION_LENGTH );

    cJSONExt_GetFloatArray( jComponent, "RelativeCursorSize", &m_RelativeCursorSize.x, 2 );
}

void ComponentMenuPage::RegisterCallbacks()
{
    if( m_Enabled )
    {
        MYFW_REGISTER_COMPONENT_CALLBACK( Tick );
        MYFW_REGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        MYFW_REGISTER_COMPONENT_CALLBACK( Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( OnTouch );
        MYFW_REGISTER_COMPONENT_CALLBACK( OnButtons );
        MYFW_REGISTER_COMPONENT_CALLBACK( OnKeys );
        MYFW_REGISTER_COMPONENT_CALLBACK( OnFileRenamed );
    }
}

void ComponentMenuPage::UnregisterCallbacks()
{
    MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
    MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );
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
    ComponentBase::OnLoad();

    // if we don't have a luascript, find the first one attached to this gameobject
    FindLuaScriptComponentPointer();
}

void ComponentMenuPage::OnPlay()
{
    ComponentBase::OnPlay();

    // if we don't have a luascript, find the first one attached to this gameobject
    FindLuaScriptComponentPointer();
}

void ComponentMenuPage::OnGameObjectEnabled()
{
    ComponentBase::OnGameObjectEnabled();

    ShowPage();
}

void ComponentMenuPage::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();

    HidePage();
}

void ComponentMenuPage::SetEnabled(bool enabled)
{
    if( m_Enabled == enabled )
        return;

    ComponentBase::SetEnabled( enabled );

    // if this is newly enabled, trigger the visible callback.
    if( m_Enabled == true )
    {
        m_Visible = false;
        SetVisible( true );
    }
}

// will return true if input is used.
bool ComponentMenuPage::OnTouchCallback(int action, int id, float x, float y, float pressure, float size)
{
    if( m_Visible == false || m_InputEnabled == false )
        return false;

    //ComponentBase::OnTouchCallback( action, id, x, y, pressure, size );

    if( m_MenuPageOnTouchCallbackStruct.pFunc )
    {
        if( m_MenuPageOnTouchCallbackStruct.pFunc( m_MenuPageOnTouchCallbackStruct.pObj, this, action, id, x, y, pressure, size ) )
            return true;
    }

    for( int i=m_MenuItemsUsed-1; i>=0; i-- )
    {
        if( m_pMenuItems[i]->m_MenuItemType == MIT_ScrollBox ||
            m_pMenuItems[i]->m_MenuItemType == MIT_ScrollingText )
        {
            MenuScrollBox* pScrollBox = (MenuScrollBox*)m_pMenuItems[i];
            if( pScrollBox->OnTouch( action, id, x, y, pressure, size ) )
                return true;
        }
    }

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
                        if( m_pMenuItems[i]->ReleaseOnNoCollision( fingerindex, x, y ) )
                        {
                            // finger is further down than when it was on the button.
                            if( y < GetMenuItem(i)->m_Position.y )
                            {
                                if( GetMenuItem(i)->m_MenuItemType == MIT_Button )
                                {
                                    const char* action = GetMenuButton(i)->m_ButtonAction;
                                    if( action != 0 )
                                    {
                                        if( ExecuteAction( "OnSwipeDown", action, m_pMenuItems[i] ) )
                                            return true;
                                    }
                                }
                            }
                        }
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
                        if( ExecuteAction( "OnAction", action, m_pMenuItems[i] ) )
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
bool ComponentMenuPage::OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    if( m_Visible == false || m_InputEnabled == false )
        return false;

    //ComponentBase::OnButtonsCallback( action, id );

    if( m_MenuPageOnButtonsCallbackStruct.pFunc )
    {
        if( m_MenuPageOnButtonsCallbackStruct.pFunc( m_MenuPageOnButtonsCallbackStruct.pObj, this, action, id ) )
            return true;
    }

    // deal with navigation controls.  up/down/left/right
    int dirtocheckx = 0;
    int dirtochecky = 0;

    if( action == GCBA_Down )
    {
        if( id == GCBI_Up )    dirtochecky = 1;
        if( id == GCBI_Down )  dirtochecky = -1;
        if( id == GCBI_Right ) dirtocheckx = 1;
        if( id == GCBI_Left )  dirtocheckx = -1;
    }

    if( dirtocheckx != 0 || dirtochecky != 0 )
    {
        //if( m_Selector_Visible == false ) { m_Selector_Visible = true; /*return true;*/ }

        Vector2 currentposition( -1, 9999 ); // spot at the top left.
        if( m_ItemSelected != -1 )
        {
            MenuItem* pItem = GetMenuItem( m_ItemSelected );
            currentposition = pItem->m_Position;
        }

        int nearestindex = -1;

        float nearestdistance = 999999;

        for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
        {
            MenuItem* pItem = GetMenuItem( i );
            Vector2 position2nditem = pItem->m_Position;

            if( pItem->m_Visible == false || pItem->m_Navigable == false )
                continue;

            float distx = fabs( currentposition.x - position2nditem.x );
            float disty = fabs( currentposition.y - position2nditem.y );
            if( dirtocheckx ) distx /= 2.5f;
            if( dirtochecky ) disty /= 2.5f;

            float distance = sqrt( distx*distx + disty*disty );

            if( distance > 0 && distance < nearestdistance )
            {
                if( dirtochecky != 0 ) // up or down // 1 or -1
                {
                    // if the item is in the correct hemisphere, pick the nearest.
                    if( currentposition.y * dirtochecky < position2nditem.y * dirtochecky )
                    {
                        nearestdistance = distance;
                        nearestindex = i;
                    }
                }
                else //if( dirtocheckx != 0 ) // right or left // 1 or -1
                {
                    // if the item is in the correct hemisphere, pick the nearest.
                    if( currentposition.x * dirtocheckx < position2nditem.x * dirtocheckx )
                    {
                        nearestdistance = distance;
                        nearestindex = i;
                    }
                }
            }
        }

        if( nearestindex != -1 )
        {
            m_ItemSelected = nearestindex;
            //LOGInfo( LOGTag, "SelectedItem: %d\n", m_ItemSelected );
        }

        return true;
    }
    else
    {
        if( action == GCBA_Down )
        {
            if( id == GCBI_ButtonA )
            {
                if( m_ItemSelected != -1 )
                {
                    // set this menu item as the selected input box.
                    if( m_pMenuItems[m_ItemSelected]->m_MenuItemType == MIT_InputBox )
                        m_pInputBoxWithKeyboardFocus = (MenuInputBox*)m_pMenuItems[m_ItemSelected];

                    if( m_pMenuItems[m_ItemSelected]->m_MenuItemType == MIT_Button )
                    {
                        if( ExecuteAction( "OnAction", ((MenuButton*)m_pMenuItems[m_ItemSelected])->m_ButtonAction, m_pMenuItems[m_ItemSelected] ) )
                            return true;
                    }
                }
            }

            if( id == GCBI_ButtonB ) if( ExecuteAction( "OnAction", m_ButtonActions[0], 0 ) ) return true;
            if( id == GCBI_ButtonC ) if( ExecuteAction( "OnAction", m_ButtonActions[1], 0 ) ) return true;
            if( id == GCBI_ButtonD ) if( ExecuteAction( "OnAction", m_ButtonActions[2], 0 ) ) return true;
        }
    }

    return false;
}

// will return true if input is used.
bool ComponentMenuPage::OnKeysCallback(GameCoreButtonActions action, int keycode, int unicodechar)
{
    if( m_Visible == false || m_InputEnabled == false )
        return false;

    //ComponentBase::OnKeysCallback( action, keycode, unicodechar );

    //if( Screen_Base::OnKeyDown( keycode, unicodechar ) )
    //    return true;

    //LOGInfo( LOGTag, "Screen_PlayerCreation::OnKeyDown %p\n", m_pInputBoxWithKeyboardFocus );

    if( m_MenuPageOnKeysCallbackStruct.pFunc )
    {
        m_MenuPageOnKeysCallbackStruct.pFunc( m_MenuPageOnKeysCallbackStruct.pObj, this, action, keycode, unicodechar );
    }

    if( action == GCBA_Down )
    {
        if( m_pInputBoxWithKeyboardFocus )
        {
            return m_pInputBoxWithKeyboardFocus->OnKeyDown( keycode, unicodechar );
        }
    }

    return false;
}

// if any of the materials used for any of the unloaded menu pages has been renamed, then fix the reference.
void ComponentMenuPage::OnFileRenamedCallback(const char* fullpathbefore, const char* fullpathafter)
{
    if( m_MenuLayouts == 0 )
    {
        if( m_pMenuLayoutFile && m_pMenuLayoutFile->m_FileLoadStatus == FileLoadStatus_Success )
        {
            MyAssert( m_MenuLayouts == 0 );
            m_MenuLayouts = cJSON_Parse( m_pMenuLayoutFile->m_pBuffer );
        }
    }

    if( m_MenuLayouts )
    {
        RenameFileInJSONObject( m_MenuLayouts, fullpathbefore, fullpathafter );
    }
}

void ComponentMenuPage::RenameFileInJSONObject(cJSON* jObject, const char* fullpathbefore, const char* fullpathafter)
{
    MyAssert( jObject );

    if( jObject->child )
        RenameFileInJSONObject( jObject->child, fullpathbefore, fullpathafter );

    if( jObject->valuestring && strcmp( jObject->valuestring, fullpathbefore ) == 0 )
    {
        cJSONExt_ReplaceStringInJSONObject( jObject, fullpathafter );
    }

    if( jObject->next )
        RenameFileInJSONObject( jObject->next, fullpathbefore, fullpathafter );
}

void ComponentMenuPage::CreateMenuItems()
{
    if( m_MenuItemsCreated == false )
    {
        if( m_pMenuLayoutFile && m_pMenuLayoutFile->m_FileLoadStatus == FileLoadStatus_Success )
        {
            MyAssert( m_MenuLayouts == 0 );
            m_MenuLayouts = cJSON_Parse( m_pMenuLayoutFile->m_pBuffer );
            if( m_MenuLayouts == 0 )
                m_MenuLayouts = cJSON_CreateObject();

            LoadLayoutBasedOnCurrentAspectRatio();
        }
    }
}

void ComponentMenuPage::TickCallback(double TimePassed)
{
    //ComponentBase::TickCallback( TimePassed );

    if( m_MenuPageTickCallbackStruct.pFunc )
    {
        m_MenuPageTickCallbackStruct.pFunc( m_MenuPageTickCallbackStruct.pObj, this, TimePassed );
    }

    if( m_MenuItemsCreated == false )
    {
        //CreateMenuItems(); // create menu items if they haven't been already.

        //if( m_MenuItemsCreated && m_Enabled && m_Visible )
        if( m_Enabled && m_Visible )
            ShowPage();
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

void ComponentMenuPage::OnSurfaceChangedCallback(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    //ComponentBase::OnSurfaceChangedCallback( TimePassed );

    // if the aspect ratio didn't change, return;
    if( m_CurrentWidth == m_pComponentCamera->m_WindowWidth &&
        m_CurrentHeight == m_pComponentCamera->m_WindowHeight )
    {
        return;
    }

#if MYFW_USING_WX
    // if we have a valid layout, save it.
    if( m_CurrentWidth != 0 && m_CurrentHeight != 0 )
    {
        if( m_MenuItemsCreated == true )
            SaveCurrentLayoutToJSON();
    }
#endif //MYFW_USING_WX

    // if the page isn't visible, don't do anything, items will get created in correct layout when made visible.
    if( m_Visible == false )
        return;

    //m_CurrentWidth = m_pComponentCamera->m_WindowWidth;
    //m_CurrentHeight = m_pComponentCamera->m_WindowHeight;

    // trigger the onvisible callback, this will reload the layout.
    m_Visible = false;
    SetVisible( true );
}

void ComponentMenuPage::LoadLayoutBasedOnCurrentAspectRatio()
{
    if( m_MenuLayouts == 0 )
        return;

    m_CurrentWidth = m_pComponentCamera->m_WindowWidth;
    m_CurrentHeight = m_pComponentCamera->m_WindowHeight;

    MyAssert( m_CurrentWidth != 0 && m_CurrentHeight != 0 );

    cJSON* obj = 0;

    // pick the correct layout, or the first if the correct one doesn't exist.
    //if( m_CurrentWidth == m_CurrentHeight )
    //{
        obj = cJSON_GetObjectItem( m_MenuLayouts, "Square" );
    //}
    
    if( obj == 0 || m_CurrentWidth < m_CurrentHeight )
    {
        obj = cJSON_GetObjectItem( m_MenuLayouts, "Tall" );
    }
    
    if( obj == 0 || m_CurrentHeight < m_CurrentWidth )
    {
        obj = cJSON_GetObjectItem( m_MenuLayouts, "Wide" );
    }

#if MYFW_USING_WX
    if( m_CurrentWidth == m_CurrentHeight )
    {
        g_pPanelObjectList->RenameObject( this, "Menu Page - Square" );
    }
    else if( m_CurrentWidth < m_CurrentHeight )
    {
        g_pPanelObjectList->RenameObject( this, "Menu Page - Tall" );
    }
    else
    {
        g_pPanelObjectList->RenameObject( this, "Menu Page - Wide" );
    }
#endif //MYFW_USING_WX

    if( obj != 0 )
        UpdateLayout( obj );

    // grab the first input box, if there is one and set it as focused.
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        MyAssert( m_pMenuItems[i] );
        if( m_pMenuItems[i] && m_pMenuItems[i]->m_MenuItemType == MIT_InputBox )
            m_pInputBoxWithKeyboardFocus = (MenuInputBox*)m_pMenuItems[i];
    }

    m_MenuItemsCreated = true;

#if MYFW_USING_WX
    if( g_pPanelObjectList->IsObjectSelected( this ) )
    {
        //g_pPanelObjectList->SelectObject( 0 );
        wxTreeItemId id = g_pPanelObjectList->FindObject( this );
        g_pPanelObjectList->m_pTree_Objects->Expand( id );
    }
#endif //MYFW_USING_WX
}

void ComponentMenuPage::UpdateLayout(cJSON* layout)
{
    m_ExtentsBLTRWhenPageLoaded.x = m_pComponentCamera->m_Camera2D.m_OrthoBottom;
    m_ExtentsBLTRWhenPageLoaded.y = m_pComponentCamera->m_Camera2D.m_OrthoLeft;
    m_ExtentsBLTRWhenPageLoaded.z = m_pComponentCamera->m_Camera2D.m_OrthoTop;
    m_ExtentsBLTRWhenPageLoaded.w = m_pComponentCamera->m_Camera2D.m_OrthoRight;
    m_ExtentsSetWhenLoaded = true;

    m_CurrentLayout = layout;

    ClearAllMenuItems();
    m_MenuItemsUsed = Menu_ImportExport::ImportMenuLayout( m_CurrentLayout, m_pMenuItems, MAX_MENU_ITEMS, m_ExtentsBLTRWhenPageLoaded );

    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        m_pMenuItems[i]->m_pMenuPage = this;
    }

#if MYFW_USING_WX
    wxTreeItemId componentID = g_pPanelObjectList->FindObject( this );

    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        m_pMenuItems[i]->m_pMenuPage = this;

        switch( m_pMenuItems[i]->m_MenuItemType )
        {
        case MIT_Sprite:
            {
                AddMenuItemToTree( i, MenuSprite::StaticFillPropertiesWindow, m_pMenuItems[i]->m_Name );
            }
            break;

        case MIT_Text:
            {
                AddMenuItemToTree( i, MenuText::StaticFillPropertiesWindow, m_pMenuItems[i]->m_Name );
            }
            break;

        case MIT_Button:
        case MIT_InputBox:
            {
                AddMenuItemToTree( i, MenuButton::StaticFillPropertiesWindow, m_pMenuItems[i]->m_Name );
            }
            break;

        case MIT_ScrollingText:
            {
                AddMenuItemToTree( i, MenuScrollingText::StaticFillPropertiesWindow, m_pMenuItems[i]->m_Name );
            }
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

void ComponentMenuPage::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    //ComponentBase::DrawCallback( pCamera, pMatViewProj, pShaderOverride );

    if( m_Visible == false || (m_LayersThisExistsOn & pCamera->m_LayersToRender) == 0 )
        return;

    if( m_pComponentCamera == 0 )
        return;

    // deal with cursor
    {
        MenuItem* pCursor = GetMenuItemByName( "Cursor" );
        if( pCursor && pCursor->m_MenuItemType == MIT_Sprite )
        {
            if( m_ItemSelected == -1 )
            {
                pCursor->SetVisible( false );
            }
            else
            {
                pCursor->SetVisible( true );
                
                MenuItem* pItem = m_pMenuItems[m_ItemSelected];

                if( pItem )
                {
                    pCursor->SetPosition( pItem->m_Position.x, pItem->m_Position.y );

                    // make the cursor match the size of the menu item if wanted.
                    if( m_RelativeCursorSize.x != 0 || m_RelativeCursorSize.y != 0 )
                    {
                        Vector2 menuitemsize = pItem->GetSize();
                        menuitemsize += m_RelativeCursorSize;
                        pCursor->SetSize( menuitemsize.x, menuitemsize.y );
                    }
                }
            }
        }
    }

    glDisable( GL_DEPTH_TEST );
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        if( m_pMenuItems[i] )
        {
            m_pMenuItems[i]->Draw( &m_pComponentCamera->m_Camera2D.m_matViewProj );
        }
    }
    glEnable( GL_DEPTH_TEST );

    if( m_MenuPageDrawCallbackStruct.pFunc )
    {
        m_MenuPageDrawCallbackStruct.pFunc( m_MenuPageDrawCallbackStruct.pObj, this, pCamera, pMatViewProj, pShaderOverride );
    }
}

void ComponentMenuPage::ClearAllMenuItems()
{
    // if a menu item is selected, select the menu page.
#if MYFW_USING_WX
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        if( g_pPanelObjectList->IsObjectSelected( m_pMenuItems[i] ) )
        {
            g_pPanelObjectList->SelectObject( 0 );
            g_pPanelObjectList->SelectObject( this );
            wxTreeItemId id = g_pPanelObjectList->FindObject( this );
            g_pPanelObjectList->m_pTree_Objects->Collapse( id );
        }
    }
#endif //MYFW_USING_WX

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

MenuButton* ComponentMenuPage::GetMenuButton(unsigned int index)
{
    MyAssert( m_pMenuItems[index] && m_pMenuItems[index]->m_MenuItemType == MIT_Button );
    
    if( m_pMenuItems[index] && m_pMenuItems[index]->m_MenuItemType == MIT_Button )
        return (MenuButton*)m_pMenuItems[index];

    return 0;
}

MenuButton* ComponentMenuPage::GetMenuButtonByName(const char* name)
{
    for( int i=0; i<MAX_MENU_ITEMS; i++ )
    {
        if( m_pMenuItems[i] && strcmp( m_pMenuItems[i]->m_Name, name ) == 0 )
        {
            MyAssert( m_pMenuItems[i] && m_pMenuItems[i]->m_MenuItemType == MIT_Button );
    
            if( m_pMenuItems[i] && m_pMenuItems[i]->m_MenuItemType == MIT_Button )
                return (MenuButton*)m_pMenuItems[i];
        }
    }

    return 0;
}

void ComponentMenuPage::SetSelectedItemByName(const char* name)
{
    int i;
    for( i=0; i<MAX_MENU_ITEMS; i++ )
    {
        if( m_pMenuItems[i] && strcmp( m_pMenuItems[i]->m_Name, name ) == 0 )
        {
            MyAssert( m_pMenuItems[i] && m_pMenuItems[i]->m_MenuItemType == MIT_Button );
    
            if( m_pMenuItems[i] && m_pMenuItems[i]->m_MenuItemType == MIT_Button )
                break;
        }
    }

    if( i < MAX_MENU_ITEMS )
        m_ItemSelected = i;
}

MenuItem* ComponentMenuPage::GetSelectedItem()
{
    if( m_ItemSelected == -1 )
        return 0;
    
    return m_pMenuItems[m_ItemSelected];
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

void ComponentMenuPage::RegisterMenuPageTickCallback(void* pObj, MenuPageTickCallbackFunc pFunc)
{
    m_MenuPageTickCallbackStruct.pFunc = pFunc;
    m_MenuPageTickCallbackStruct.pObj = pObj;
}

void ComponentMenuPage::RegisterMenuPageDrawCallback(void* pObj, MenuPageDrawCallbackFunc pFunc)
{
    m_MenuPageDrawCallbackStruct.pFunc = pFunc;
    m_MenuPageDrawCallbackStruct.pObj = pObj;
}

void ComponentMenuPage::RegisterMenuPageOnTouchCallback(void* pObj, MenuPageOnTouchCallbackFunc pFunc)
{
    m_MenuPageOnTouchCallbackStruct.pFunc = pFunc;
    m_MenuPageOnTouchCallbackStruct.pObj = pObj;
}

void ComponentMenuPage::RegisterMenuPageOnButtonsCallback(void* pObj, MenuPageOnButtonsCallbackFunc pFunc)
{
    m_MenuPageOnButtonsCallbackStruct.pFunc = pFunc;
    m_MenuPageOnButtonsCallbackStruct.pObj = pObj;
}

void ComponentMenuPage::RegisterMenuPageOnKeysCallback(void* pObj, MenuPageOnKeysCallbackFunc pFunc)
{
    m_MenuPageOnKeysCallbackStruct.pFunc = pFunc;
    m_MenuPageOnKeysCallbackStruct.pObj = pObj;
}

void ComponentMenuPage::SetVisible(bool visible)
{
    if( m_Visible == visible )
        return;

    m_Visible = visible;

    // move all input callbacks for this menu page to the front of the list, to give it top priority.
    if( visible == true )
    {
        ShowPage();
    }
}

bool ComponentMenuPage::IsVisible()
{
    if( m_pGameObject->IsEnabled() == false )
        return false;

    return m_Visible;
}

void ComponentMenuPage::SetInputEnabled(bool inputenabled)
{
    m_InputEnabled = inputenabled;
}

bool ComponentMenuPage::ExecuteAction(const char* function, const char* action, MenuItem* pItem)
{
    if( action != 0 )
    {
        if( m_MenuPageActionCallbackStruct.pFunc )
        {
            if( m_MenuPageActionCallbackStruct.pFunc( m_MenuPageActionCallbackStruct.pObj, this, function, action, pItem ) )
                return true;
        }
#if MYFW_USING_WX
        // in editor, there's a chance the script component was created and not associated with this object.
        FindLuaScriptComponentPointer();
#endif
        if( m_pComponentLuaScript )
        {
            m_pComponentLuaScript->CallFunction( function, action );
            return true;
        }
    }

    return false;
}

void ComponentMenuPage::ShowPage()
{
#if MYFW_USING_WX
    // in editor, there's a chance the script component was created and not associated with this object.
    FindLuaScriptComponentPointer();
#endif

    // don't allow menu page to be shown until the script is loaded.
    if( m_pComponentLuaScript && m_pComponentLuaScript->IsScriptLoaded() == false )
        return;

    bool layoutchanged = false;

    if( m_CurrentWidth != m_pComponentCamera->m_WindowWidth ||
        m_CurrentHeight != m_pComponentCamera->m_WindowHeight )
    {
        m_CurrentWidth = m_pComponentCamera->m_WindowWidth;
        m_CurrentHeight = m_pComponentCamera->m_WindowHeight;
        layoutchanged = true;
    }

    if( m_MenuItemsCreated == false )
    {
        // create menu items if they haven't been already.
        CreateMenuItems();
    }
    else
    {
        // recreate all the menu items if the layout had changed.
        if( m_Visible == true && layoutchanged )
            LoadLayoutBasedOnCurrentAspectRatio();
    }

    g_pComponentSystemManager->MoveInputHandlersToFront( &m_CallbackStruct_OnTouch, &m_CallbackStruct_OnButtons, &m_CallbackStruct_OnKeys );

    if( m_MenuPageVisibleCallbackStruct.pFunc )
    {
        m_MenuPageVisibleCallbackStruct.pFunc( m_MenuPageVisibleCallbackStruct.pObj, this, true );
    }

    // if the currently selected item is disabled, reset the selected item.
    if( m_ItemSelected != -1 && m_pMenuItems[m_ItemSelected] && m_pMenuItems[m_ItemSelected]->m_Enabled == false )
        m_ItemSelected = -1;

    if( m_pComponentLuaScript )
    {
        m_pComponentLuaScript->CallFunction( "OnVisible" );
    }
}

void ComponentMenuPage::HidePage()
{
    if( m_MenuPageVisibleCallbackStruct.pFunc )
    {
        m_MenuPageVisibleCallbackStruct.pFunc( m_MenuPageVisibleCallbackStruct.pObj, this, false );
    }
}
