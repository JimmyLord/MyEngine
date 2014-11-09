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

ComponentTransform::ComponentTransform()
: ComponentBase()
{
    m_Type = ComponentType_Data;
}

ComponentTransform::ComponentTransform(GameObject* owner)
: ComponentBase( owner )
{
    m_Type = ComponentType_Data;
}

ComponentTransform::~ComponentTransform()
{
}

#if MYFW_USING_WX
void ComponentTransform::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentTransform::StaticFillPropertiesWindow, gameobjectid, "Transform" );
}

void ComponentTransform::FillPropertiesWindow()
{
    g_pPanelWatch->ClearAllVariables();
    g_pPanelWatch->AddFloat( "x", &m_Transform.m41, -1.0f, 1.0f );
    g_pPanelWatch->AddFloat( "y", &m_Transform.m42, -1.0f, 1.0f );
    g_pPanelWatch->AddFloat( "z", &m_Transform.m43, -1.0f, 1.0f );
}
#endif //MYFW_USING_WX

void ComponentTransform::Reset()
{
    ComponentBase::Reset();

    m_Transform.SetIdentity();
}
