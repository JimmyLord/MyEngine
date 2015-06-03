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
    m_pCamera = 0;

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

    // find the first ortho cam component in the scene, settle for a perspective if that's all there is.
    m_pCamera = (ComponentCamera*)g_pComponentSystemManager->GetFirstComponentOfType( "CameraComponent" );
    MyAssert( m_pCamera->IsA( "CameraComponent" ) );
    while( m_pCamera != 0 && m_pCamera->m_Orthographic == false )
    {
        m_pCamera = (ComponentCamera*)g_pComponentSystemManager->GetNextComponentOfType( m_pCamera );
        MyAssert( m_pCamera == 0 || m_pCamera->IsA( "CameraComponent" ) );
    }

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

    if( newfullpath[0] != 0 )
    {
        if( g_pFileManager->DoesFileExist( newfullpath ) )
        {
            // launching this messagebox causes a change focus message on the editbox, causing OnValueChanged to get call again, avoiding issue with this hack.
            h_RenameInProgress = true;

            int answer = wxMessageBox( "File already exists.\nDiscard changes and Load it?", "Menu Load Confirm", wxYES_NO, g_pEngineMainFrame );

            if( answer == wxYES )
            {
                ClearAllMenuItems();
                m_pMenuLayoutFile = g_pFileManager->RequestFile( newfullpath );
            }
        }
        else
        {
            // save as new name
            SaveMenuPageToDisk( newfullpath );
        }

        // request the file, so it's part of the scene.
        m_pMenuLayoutFile = g_pFileManager->RequestFile( newfullpath );
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
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Menu Page", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentBase::FillPropertiesWindow( clear );

        if( m_pMenuLayoutFile )
            m_ControlID_Filename = g_pPanelWatch->AddPointerWithDescription( "Menu file", m_pMenuLayoutFile, m_pMenuLayoutFile->m_FullPath, this, 0, ComponentMenuPage::StaticOnValueChanged );
        else
            m_ControlID_Filename = g_pPanelWatch->AddPointerWithDescription( "Menu file", m_pMenuLayoutFile, "no file", this, 0, ComponentMenuPage::StaticOnValueChanged );
    }
}

void ComponentMenuPage::AppendItemsToRightClickMenu(wxMenu* pMenu)
{
    ComponentBase::AppendItemsToRightClickMenu( pMenu );

    pMenu->Append( 2000, "Add Button" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );

    pMenu->Append( 2001, "Add Sprite" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentMenuPage::OnPopupClick );
}

void ComponentMenuPage::OnPopupClick(wxEvent &evt)
{
    ComponentBase::OnPopupClick( evt );

    ComponentMenuPage* pComponent = (ComponentMenuPage*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    MyAssert( pComponent->IsA( "MenuPageComponent" ) );

    wxTreeItemId componentID = g_pPanelObjectList->FindObject( pComponent );    

    int id = evt.GetId();
    if( id == 2000 )
    {
        MenuButton* pMenuItem = MyNew MenuButton( 100 );
        pMenuItem->SetString( "Test Button" );
        pComponent->m_pMenuItems[pComponent->m_MenuItemsUsed] = pMenuItem;
        pComponent->m_MenuItemsUsed++;
        g_pPanelObjectList->AddObject( pMenuItem, MenuButton::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Button" );
        pMenuItem->RegisterMenuItemDeletedCallback( pComponent, StaticOnMenuItemDeleted );

        //MySprite* pSprite = MyNew MySprite();
        //pSprite->Create( "MenuButton Sprite", 1, 1, 0, 1, 0, 1, Justify_Center, false );
        //pMenuItem->SetSprites( pSprite, pSprite, pSprite, pSprite, pSprite );
        //pSprite->Release();
    }

    if( id == 2001 )
    {
        MenuSprite* pMenuItem = MyNew MenuSprite();
        pComponent->m_pMenuItems[pComponent->m_MenuItemsUsed] = pMenuItem;
        pComponent->m_MenuItemsUsed++;
        g_pPanelObjectList->AddObject( pMenuItem, MenuSprite::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Sprite" );
        pMenuItem->RegisterMenuItemDeletedCallback( pComponent, StaticOnMenuItemDeleted );

        //MySprite* pSprite = MyNew MySprite();
        //pSprite->Create( "MenuSprite Sprite", 1, 1, 0, 1, 0, 1, Justify_Center, false );
        //pMenuItem->SetSprites( pSprite, pSprite );
        //pSprite->Release();
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
#endif //MYFW_USING_WX

cJSON* ComponentMenuPage::ExportAsJSONObject()
{
    // Scene is saving so also save menu file to disk.
    SaveMenuPageToDisk( m_pMenuLayoutFile->m_FullPath );

    cJSON* jComponent = ComponentBase::ExportAsJSONObject();

    if( m_pMenuLayoutFile )
        cJSON_AddStringToObject( jComponent, "MenuFile", m_pMenuLayoutFile->m_FullPath );
    
    return jComponent;
}

void ComponentMenuPage::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jComponent, sceneid );

    cJSON* jFilename = cJSON_GetObjectItem( jComponent, "MenuFile" );
    if( jFilename )
        m_pMenuLayoutFile = g_pFileManager->RequestFile( jFilename->valuestring );
}

ComponentMenuPage& ComponentMenuPage::operator=(const ComponentMenuPage& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    return *this;
}

// will return true if input is used.
bool ComponentMenuPage::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
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
                g_pPanelObjectList->AddObject( m_pMenuItems[i], MenuButton::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Button" );
            }

            m_MenuItemsCreated = true;
        }
    }
#endif

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
    if( m_pCamera == 0 )
        return;

    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        if( m_pMenuItems[i] )
        {
            m_pMenuItems[i]->Draw( &m_pCamera->m_Camera2D.m_matViewProj );
        }
    }
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
