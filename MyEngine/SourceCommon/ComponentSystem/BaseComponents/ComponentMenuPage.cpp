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

    m_MenuItemsUsed = 0;
    for( unsigned int i=0; i<MAX_MENUITEMS; i++ )
    {
        m_pMenuItems[i] = 0;
    }
    m_pMenuItemHeld = 0;
}

ComponentMenuPage::~ComponentMenuPage()
{
    for( unsigned int i=0; i<m_MenuItemsUsed; i++ )
    {
        SAFE_DELETE( m_pMenuItems[i] );
    }
    for( unsigned int i=m_MenuItemsUsed; i<MAX_MENUITEMS; i++ )
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

        MySprite* pSprite = MyNew MySprite();
        pSprite->Create( "MenuButton Sprite", 1, 1, 0, 1, 0, 1, Justify_Center, false );
        pMenuItem->SetSprites( pSprite, pSprite, pSprite, 0, 0 );
        pSprite->Release();
    }

    if( id == 2001 )
    {
        MenuSprite* pMenuItem = MyNew MenuSprite();
        pComponent->m_pMenuItems[pComponent->m_MenuItemsUsed] = pMenuItem;
        pComponent->m_MenuItemsUsed++;
        g_pPanelObjectList->AddObject( pMenuItem, MenuButton::StaticFillPropertiesWindow, MenuItem::StaticOnRightClick, componentID, "Sprite" );
        pMenuItem->RegisterMenuItemDeletedCallback( pComponent, StaticOnMenuItemDeleted );

        MySprite* pSprite = MyNew MySprite();
        pSprite->Create( "MenuSprite Sprite", 1, 1, 0, 1, 0, 1, Justify_Center, false );
        pMenuItem->SetSprites( pSprite, pSprite );
        pSprite->Release();
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
