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
bool ComponentMeshPrimitive::m_PanelWatchBlockVisible = true;
#endif

const char* ComponentMeshPrimitiveTypeStrings[ComponentMeshPrimitive_NumTypes] =
{
    "Plane",
    "Icosphere",
};

ComponentMeshPrimitive::ComponentMeshPrimitive()
: ComponentMesh()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;
}

ComponentMeshPrimitive::~ComponentMeshPrimitive()
{
}

void ComponentMeshPrimitive::Reset()
{
    ComponentMesh::Reset();

    m_MeshPrimitiveType = ComponentMeshPrimitive_NumTypes;

    m_Plane_Size.Set( 10, 10 );
    m_Plane_VertCount.Set( 10, 10 );
    m_Plane_UVStart.Set( 0, 0 );
    m_Plane_UVRange.Set( 1, 1 );

    m_Sphere_Radius = 1;

    //CreatePrimitive();

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
    m_ControlID_MeshPrimitiveType = -1;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentMeshPrimitive::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMeshPrimitive::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "MeshPlane" );
}

void ComponentMeshPrimitive::OnLeftClick(bool clear)
{
    ComponentMesh::OnLeftClick( clear );
}

void ComponentMeshPrimitive::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Mesh Primitive", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentMesh::FillPropertiesWindow( clear );

        m_ControlID_MeshPrimitiveType = g_pPanelWatch->AddEnum( "MPType", (int*)&m_MeshPrimitiveType, ComponentMeshPrimitive_NumTypes, ComponentMeshPrimitiveTypeStrings, this, StaticOnValueChanged );

        if( m_MeshPrimitiveType == ComponentMeshPrimitive_Plane )
        {
            g_pPanelWatch->AddVector2( "Size", &m_Plane_Size, 0.01f, 1000.0f, this, StaticOnValueChanged );

            g_pPanelWatch->AddInt( "VertCount x", &m_Plane_VertCount.x, 2, 1000, this, StaticOnValueChanged );
            g_pPanelWatch->AddInt( "VertCount y", &m_Plane_VertCount.y, 2, 1000, this, StaticOnValueChanged );

            g_pPanelWatch->AddVector2( "UVStart", &m_Plane_UVStart, -1.0f, 1000.0f, this, StaticOnValueChanged );
            g_pPanelWatch->AddVector2( "UVRange", &m_Plane_UVRange, -1.0f, 1000.0f, this, StaticOnValueChanged );
        }

        if( m_MeshPrimitiveType == ComponentMeshPrimitive_Icosphere )
        {
            g_pPanelWatch->AddFloat( "Radius", &m_Sphere_Radius, 0.01f, 100.0f, this, StaticOnValueChanged );
        }
    }
}

void ComponentMeshPrimitive::OnValueChanged(int controlid, bool finishedchanging)
{
    if( finishedchanging )
    {
        //if( controlid == m_ControlID_MeshPrimitiveType )
        {
            CreatePrimitive();
            g_pPanelWatch->m_NeedsRefresh = true;
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentMeshPrimitive::ExportAsJSONObject()
{
    cJSON* component = ComponentMesh::ExportAsJSONObject();

    cJSON_AddNumberToObject( component, "MPType", m_MeshPrimitiveType );

    cJSON_AddNumberToObject( component, "PlaneSizex", m_Plane_Size.x );
    cJSON_AddNumberToObject( component, "PlaneSizey", m_Plane_Size.y );

    cJSON_AddNumberToObject( component, "PlaneVertCountx", m_Plane_VertCount.x );
    cJSON_AddNumberToObject( component, "PlaneVertCounty", m_Plane_VertCount.y );

    cJSON_AddNumberToObject( component, "PlaneUVStartx", m_Plane_UVStart.x );
    cJSON_AddNumberToObject( component, "PlaneUVStarty", m_Plane_UVStart.y );

    cJSON_AddNumberToObject( component, "PlaneUVRangex", m_Plane_UVRange.x );
    cJSON_AddNumberToObject( component, "PlaneUVRangey", m_Plane_UVRange.y );

    cJSON_AddNumberToObject( component, "SphereRadius", m_Sphere_Radius );    

    return component;
}

void ComponentMeshPrimitive::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentMesh::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetInt( jsonobj, "MPType", (int*)&m_MeshPrimitiveType );

    cJSONExt_GetFloat( jsonobj, "PlaneSizex", &m_Plane_Size.x );
    cJSONExt_GetFloat( jsonobj, "PlaneSizey", &m_Plane_Size.y );

    cJSONExt_GetInt( jsonobj, "PlaneVertCountx", &m_Plane_VertCount.x );
    cJSONExt_GetInt( jsonobj, "PlaneVertCounty", &m_Plane_VertCount.y );

    cJSONExt_GetFloat( jsonobj, "PlaneUVStartx", &m_Plane_UVStart.x );
    cJSONExt_GetFloat( jsonobj, "PlaneUVStarty", &m_Plane_UVStart.y );

    cJSONExt_GetFloat( jsonobj, "PlaneUVRangex", &m_Plane_UVRange.x );
    cJSONExt_GetFloat( jsonobj, "PlaneUVRangey", &m_Plane_UVRange.y );

    cJSONExt_GetFloat( jsonobj, "SphereRadius", &m_Sphere_Radius );

    CreatePrimitive();
}

ComponentMeshPrimitive& ComponentMeshPrimitive::operator=(const ComponentMeshPrimitive& other)
{
    MyAssert( &other != this );

    ComponentMesh::operator=( other );

    this->m_MeshPrimitiveType = other.m_MeshPrimitiveType;

    this->m_Plane_Size = other.m_Plane_Size;
    this->m_Plane_VertCount = other.m_Plane_VertCount;
    this->m_Plane_UVStart = other.m_Plane_UVStart;
    this->m_Plane_UVRange = other.m_Plane_UVRange;

    this->m_Sphere_Radius = other.m_Sphere_Radius;

    return *this;
}

void ComponentMeshPrimitive::CreatePrimitive()
{
    if( m_MeshPrimitiveType < 0 || m_MeshPrimitiveType >= ComponentMeshPrimitive_NumTypes )
        return;

    if( m_pMesh == 0 )
        m_pMesh = MyNew MyMesh;

    if( m_pMesh->m_SubmeshList.Count() > 0 )
        m_pMesh->m_SubmeshList[0]->m_PrimitiveType = m_GLPrimitiveType;

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_Plane )
    {
        bool createtriangles = true;
        if( m_GLPrimitiveType == GL_POINTS )
            createtriangles = false;

        m_pMesh->CreatePlane( Vector3(-m_Plane_Size.x/2, 0, -m_Plane_Size.y/2), m_Plane_Size, m_Plane_VertCount, m_Plane_UVStart, m_Plane_UVRange, createtriangles );
    }
    else if( m_MeshPrimitiveType == ComponentMeshPrimitive_Icosphere )
    {
        m_pMesh->CreateIcosphere( m_Sphere_Radius, 0 );
    }
}

void ComponentMeshPrimitive::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentMesh::Draw( pMatViewProj, pShaderOverride, drawcount );
}
