//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

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

    m_MenuItemsUsed = 0;
    for( unsigned int i=0; i<MAX_MENU_ITEMS; i++ )
    {
        m_pMenuItems[i] = 0;
    }
    m_pMenuItemHeld = 0;

    m_pMenuLayoutFile = 0;

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
}

void ComponentMenuPage::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;

    m_Visible = true;
    m_LayersThisExistsOn = Layer_HUD;

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

#if MYFW_USING_WX
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

    cJSON* jMenuItems = Menu_ImportExport::ExportMenuLayout( m_pMenuItems, m_MenuItemsUsed );
    
    char* string = cJSON_Print( jMenuItems );
    cJSON_Delete( jMenuItems );

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
    // Scene is saving so also save menu file to disk.
    SaveMenuPageToDisk( m_pMenuLayoutFile->m_FullPath );

    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );

    cJSON_AddNumberToObject( jComponent, "Visible", m_Visible );
    cJSON_AddNumberToObject( jComponent, "Layers", m_LayersThisExistsOn );

    if( m_pMenuLayoutFile )
        cJSON_AddStringToObject( jComponent, "MenuFile", m_pMenuLayoutFile->m_FullPath );
    
    return jComponent;
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
bool ComponentMenuPage::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
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

                    if( action != 0 )//&& OnMenuAction( action ) )
                    {
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
bool ComponentMenuPage::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    return false;
}

void ComponentMenuPage::Tick(double TimePassed)
{
#if MYFW_USING_WX
    if( m_MenuItemsCreated == false )
    {
        if( m_pMenuLayoutFile && m_pMenuLayoutFile->m_FileLoadStatus == FileLoadStatus_Success )
        {
            ClearAllMenuItems();
            m_MenuItemsUsed = Menu_ImportExport::ImportMenuLayout( m_pMenuLayoutFile->m_pBuffer, m_pMenuItems, MAX_MENU_ITEMS );

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

                case MIT_Base:
                case MIT_ScrollBox:
                case MIT_ScrollingText:
                case MIT_InputBox:
                case MIT_CheckBox:
                case MIT_NumMenuItemTypes:
                default:
                    MyAssert( false );
                }
            }

            m_MenuItemsCreated = true;
        }
    }
#endif

    // Tick all the menu items.
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        if( m_pMenuItems[i] )
        {
            m_pMenuItems[i]->Tick( TimePassed );
        }
    }
}

void ComponentMenuPage::Draw()
{
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
        g_pPanelObjectList->RemoveObject( m_pMenuItems[i] );
        SAFE_DELETE( m_pMenuItems[i] );
    }
    m_MenuItemsUsed = 0;
}

void ComponentMenuPage::SetMenuLayoutFile(MyFileObject* pFile)
{
    if( pFile )
        pFile->AddRef();
    SAFE_RELEASE( m_pMenuLayoutFile );
    m_pMenuLayoutFile = pFile;
}
