//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentMeshPlane::ComponentMeshPlane()
: ComponentMesh()
{
    m_BaseType = BaseComponentType_Renderable;
}

ComponentMeshPlane::~ComponentMeshPlane()
{
}

void ComponentMeshPlane::Reset()
{
    ComponentMesh::Reset();

    m_Size.Set( 10, 10 );
    m_VertCount.Set( 10, 10 );
    m_UVStart.Set( 0, 0 );
    m_UVRange.Set( 1, 1 );

    CreatePlane();
}

#if MYFW_USING_WX
void ComponentMeshPlane::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentMeshPlane::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "MeshPlane" );
}

void ComponentMeshPlane::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

    ComponentMesh::FillPropertiesWindow( clear );

    g_pPanelWatch->AddFloat( "Size x", &m_Size.x, 0.01f, 1000.0f, this, StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "Size y", &m_Size.y, 0.01f, 1000.0f, this, StaticOnValueChanged );

    g_pPanelWatch->AddInt( "VertCount x", &m_VertCount.x, 2, 1000, this, StaticOnValueChanged );
    g_pPanelWatch->AddInt( "VertCount y", &m_VertCount.y, 2, 1000, this, StaticOnValueChanged );

    g_pPanelWatch->AddFloat( "UVStart x", &m_UVStart.x, -1.0f, 1000.0f, this, StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "UVStart y", &m_UVStart.y, -1.0f, 1000.0f, this, StaticOnValueChanged );

    g_pPanelWatch->AddFloat( "UVRange x", &m_UVRange.x, -1.0f, 1000.0f, this, StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "UVRange y", &m_UVRange.y, -1.0f, 1000.0f, this, StaticOnValueChanged );
}

void ComponentMeshPlane::OnValueChanged(int id)
{
    CreatePlane();
}
#endif //MYFW_USING_WX

cJSON* ComponentMeshPlane::ExportAsJSONObject()
{
    cJSON* component = ComponentMesh::ExportAsJSONObject();

    cJSON_AddNumberToObject( component, "m_Sizex", m_Size.x );
    cJSON_AddNumberToObject( component, "m_Sizey", m_Size.y );

    cJSON_AddNumberToObject( component, "VertCountx", m_VertCount.x );
    cJSON_AddNumberToObject( component, "VertCounty", m_VertCount.y );

    cJSON_AddNumberToObject( component, "UVStartx", m_UVStart.x );
    cJSON_AddNumberToObject( component, "UVStarty", m_UVStart.y );

    cJSON_AddNumberToObject( component, "UVRangex", m_UVRange.x );
    cJSON_AddNumberToObject( component, "UVRangey", m_UVRange.y );

    return component;
}

void ComponentMeshPlane::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentMesh::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetFloat( jsonobj, "m_Sizex", &m_Size.x );
    cJSONExt_GetFloat( jsonobj, "m_Sizey", &m_Size.y );

    cJSONExt_GetInt( jsonobj, "VertCountx", &m_VertCount.x );
    cJSONExt_GetInt( jsonobj, "VertCounty", &m_VertCount.y );

    cJSONExt_GetFloat( jsonobj, "UVStartx", &m_UVStart.x );
    cJSONExt_GetFloat( jsonobj, "UVStarty", &m_UVStart.y );

    cJSONExt_GetFloat( jsonobj, "UVRangex", &m_UVRange.x );
    cJSONExt_GetFloat( jsonobj, "UVRangey", &m_UVRange.y );

    CreatePlane();
}

ComponentMeshPlane& ComponentMeshPlane::operator=(const ComponentMeshPlane& other)
{
    assert( &other != this );

    ComponentMesh::operator=( other );

    return *this;
}

void ComponentMeshPlane::CreatePlane()
{
    if( m_pMesh == 0 )
        m_pMesh = MyNew MyMesh;

    m_pMesh->m_PrimitiveType = m_PrimitiveType;

    bool createtriangles = true;
    if( m_PrimitiveType == GL_POINTS )
        createtriangles = false;

    m_pMesh->CreatePlane( Vector3(-m_Size.x/2, 0, -m_Size.y/2), m_Size, m_VertCount, m_UVStart, m_UVRange, createtriangles );
}

void ComponentMeshPlane::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentMesh::Draw( pMatViewProj, pShaderOverride, drawcount );
}
