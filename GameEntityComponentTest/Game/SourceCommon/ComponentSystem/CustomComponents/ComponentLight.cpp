//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

#if MYFW_USING_WX
bool ComponentLight::m_PanelWatchBlockVisible = true;
#endif

ComponentLight::ComponentLight()
: ComponentData()
{
    m_BaseType = BaseComponentType_Data;

    m_pLight = 0;
}

ComponentLight::~ComponentLight()
{
    if( m_pLight )
        g_pLightManager->DestroyLight( m_pLight );
}

void ComponentLight::Reset()
{
    ComponentData::Reset();

    if( m_pLight == 0 )
    {
        m_pLight = g_pLightManager->CreateLight();
        m_pGameObject->m_pComponentTransform->RegisterPositionChangedCallback( this, StaticOnTransformPositionChanged );
    }

    m_pLight->m_Position = m_pGameObject->m_pComponentTransform->GetPosition();
    m_pLight->m_Color.Set( 0,0,0,0 );
    m_pLight->m_Attenuation.Set( 0,0,0 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentLight::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentLight::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Light" );
}

void ComponentLight::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentLight::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Light", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentData::FillPropertiesWindow( clear );

        g_pPanelWatch->AddColorFloat( "color", &m_pLight->m_Color, 0, 1 );
        g_pPanelWatch->AddVector3( "atten", &m_pLight->m_Attenuation, 0, 1 );
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentLight::ExportAsJSONObject()
{
    cJSON* component = ComponentData::ExportAsJSONObject();

    cJSONExt_AddFloatArrayToObject( component, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_AddFloatArrayToObject( component, "Atten", &m_pLight->m_Attenuation.x, 3 );

    return component;
}

void ComponentLight::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentData::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetFloatArray( jsonobj, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_GetFloatArray( jsonobj, "Atten", &m_pLight->m_Attenuation.x, 3 );
}

void ComponentLight::OnTransformPositionChanged(Vector3& newpos)
{
    assert( m_pLight );

    m_pLight->m_Position = newpos;
}

ComponentLight& ComponentLight::operator=(const ComponentLight& other)
{
    assert( &other != this );

    ComponentData::operator=( other );

    this->m_pLight->m_Color = other.m_pLight->m_Color;
    this->m_pLight->m_Attenuation = other.m_pLight->m_Attenuation;

    return *this;
}
