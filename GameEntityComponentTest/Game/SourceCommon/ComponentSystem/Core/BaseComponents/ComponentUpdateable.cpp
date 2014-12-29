//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentUpdateable::ComponentUpdateable()
: ComponentBase()
{
    m_BaseType = BaseComponentType_Updateable;
}

ComponentUpdateable::~ComponentUpdateable()
{
}

void ComponentUpdateable::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;
}

#if MYFW_USING_WX
void ComponentUpdateable::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentUpdateable::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "Updateable" );
}

void ComponentUpdateable::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();
}
#endif //MYFW_USING_WX

ComponentUpdateable& ComponentUpdateable::operator=(const ComponentUpdateable& other)
{
    assert( &other != this );

    ComponentBase::operator=( other );

    return *this;
}

//void ComponentUpdateable::Tick(double TimePassed)
//{
//}
