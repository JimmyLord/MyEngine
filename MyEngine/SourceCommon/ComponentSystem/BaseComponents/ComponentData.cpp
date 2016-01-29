//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentData::m_PanelWatchBlockVisible = true;
#endif

ComponentData::ComponentData()
: ComponentBase()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;
}

ComponentData::~ComponentData()
{
}

#if MYFW_USING_WX
void ComponentData::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentData::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Data component", ObjectListIcon_Component );
}

void ComponentData::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentData::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Data", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );
    }
}
#endif //MYFW_USING_WX

void ComponentData::Reset()
{
    ComponentBase::Reset();

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

ComponentData& ComponentData::operator=(const ComponentData& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    return *this;
}
