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
}

ComponentMenuPage::~ComponentMenuPage()
{
}

void ComponentMenuPage::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;

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
}

void ComponentMenuPage::Draw()
{
}
