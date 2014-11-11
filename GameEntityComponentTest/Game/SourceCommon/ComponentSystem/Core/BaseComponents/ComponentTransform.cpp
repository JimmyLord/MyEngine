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
    m_BaseType = BaseComponentType_Data;
}

ComponentTransform::ComponentTransform(GameObject* owner)
: ComponentBase( owner )
{
    m_BaseType = BaseComponentType_Data;
}

ComponentTransform::~ComponentTransform()
{
}

#if MYFW_USING_WX
void ComponentTransform::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentTransform::StaticFillPropertiesWindow, 0, gameobjectid, "Transform" );
}

void ComponentTransform::FillPropertiesWindow()
{
    g_pPanelWatch->ClearAllVariables();
    g_pPanelWatch->AddFloat( "x", &m_Position.x, -1.0f, 1.0f );
    g_pPanelWatch->AddFloat( "y", &m_Position.y, -1.0f, 1.0f );
    g_pPanelWatch->AddFloat( "z", &m_Position.z, -1.0f, 1.0f );

    g_pPanelWatch->AddFloat( "scale x", &m_Scale.x, 0.0f, 10.0f );
    g_pPanelWatch->AddFloat( "scale y", &m_Scale.y, 0.0f, 10.0f );
    g_pPanelWatch->AddFloat( "scale z", &m_Scale.z, 0.0f, 10.0f );

    g_pPanelWatch->AddFloat( "rot x", &m_Rotation.x, 0, 360 );
    g_pPanelWatch->AddFloat( "rot y", &m_Rotation.y, 0, 360 );
    g_pPanelWatch->AddFloat( "rot z", &m_Rotation.z, 0, 360 );
}
#endif //MYFW_USING_WX

void ComponentTransform::Reset()
{
    ComponentBase::Reset();

    m_Position.Set( 0,0,0 );
    m_Scale.Set( 1,1,1 );
    m_Rotation.Set( 0,0,0 );
    m_Transform.SetIdentity();
}
