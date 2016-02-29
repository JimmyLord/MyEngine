//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
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

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMeshPrimitive ); //_VARIABLE_LIST

// These strings are used to save, so don't change them.
const char* ComponentMeshPrimitiveTypeStrings[ComponentMeshPrimitive_NumTypes] =
{
    "Plane",
    "Icosphere",
    "2DCircle",
};

ComponentMeshPrimitive::ComponentMeshPrimitive()
: ComponentMesh()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;
}

ComponentMeshPrimitive::~ComponentMeshPrimitive()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentMeshPrimitive::RegisterVariables(CPPListHead* pList, ComponentMeshPrimitive* pThis) //_VARIABLE_LIST
{
    ComponentMesh::RegisterVariables( pList, pThis );

    ComponentVariable* pVars[7];

    pVars[0] = AddVarEnum( pList, "MPType", MyOffsetOf( pThis, &pThis->m_MeshPrimitiveType ), true, true, 0, ComponentMeshPrimitive_NumTypes, ComponentMeshPrimitiveTypeStrings, (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, 0, 0 );
    pVars[1] = AddVar( pList, "PlaneSize", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Plane_Size ), true, true, "Size", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, 0, 0 );
    pVars[2] = AddVar( pList, "PlaneVertCountx", ComponentVariableType_Int, MyOffsetOf( pThis, &pThis->m_Plane_VertCount.x ), true, true, "VertCount X", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, 0, 0 );
    pVars[3] = AddVar( pList, "PlaneVertCounty", ComponentVariableType_Int, MyOffsetOf( pThis, &pThis->m_Plane_VertCount.y ), true, true, "VertCount Y", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, 0, 0 );
    pVars[4] = AddVar( pList, "PlaneUVStart", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Plane_UVStart ), true, true, "UVStart", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, 0, 0 );
    pVars[5] = AddVar( pList, "PlaneUVRange", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Plane_UVRange ), true, true, "UVRange", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, 0, 0 );
    pVars[6] = AddVar( pList, "SphereRadius", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Sphere_Radius ), true, true, "Radius", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, 0, 0 );

#if MYFW_USING_WX
    for( int i=0; i<7; i++ )
        pVars[i]->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentMeshPrimitive::ShouldVariableBeAddedToWatchPanel) );
#endif //MYFW_USING_WX
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
    g_pPanelObjectList->AddObject( this, ComponentMeshPrimitive::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "MeshPrimitive", ObjectListIcon_Component );
}

void ComponentMeshPrimitive::OnLeftClick(unsigned int count, bool clear)
{
    ComponentMesh::OnLeftClick( count, clear );
}

void ComponentMeshPrimitive::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Mesh Primitive", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentMesh::FillPropertiesWindow( clear );

        if( addcomponentvariables )
        {
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
        }
    }
}

bool ComponentMeshPrimitive::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
{
    if( m_MeshPrimitiveType == ComponentMeshPrimitive_Plane )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return true;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return true;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return true;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return true;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return true;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return false;
    }

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_Icosphere )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return false;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return true;
    }

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_2DCircle )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return false;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return true;
    }

    return ComponentMesh::ShouldVariableBeAddedToWatchPanel( pVar );
}

void* ComponentMeshPrimitive::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( finishedchanging )
    {
        //if( controlid == m_ControlID_MeshPrimitiveType )
        {
            CreatePrimitive();
            g_pPanelWatch->m_NeedsRefresh = true;
        }
    }

    return oldpointer;
}
#endif //MYFW_USING_WX

cJSON* ComponentMeshPrimitive::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = ComponentMesh::ExportAsJSONObject( savesceneid );

    return component;
}

void ComponentMeshPrimitive::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentMesh::ImportFromJSONObject( jsonobj, sceneid );

    // for compatibility with old files, now saved as Vector2s
    cJSONExt_GetFloat( jsonobj, "PlaneSizex", &m_Plane_Size.x );
    cJSONExt_GetFloat( jsonobj, "PlaneSizey", &m_Plane_Size.y );
    cJSONExt_GetFloat( jsonobj, "PlaneUVStartx", &m_Plane_UVStart.x );
    cJSONExt_GetFloat( jsonobj, "PlaneUVStarty", &m_Plane_UVStart.y );
    cJSONExt_GetFloat( jsonobj, "PlaneUVRangex", &m_Plane_UVRange.x );
    cJSONExt_GetFloat( jsonobj, "PlaneUVRangey", &m_Plane_UVRange.y );

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

void ComponentMeshPrimitive::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnSurfaceChanged );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnFileRenamed );
    }
}

void ComponentMeshPrimitive::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
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
    else if( m_MeshPrimitiveType == ComponentMeshPrimitive_2DCircle )
    {
        m_pMesh->Create2DCircle( m_Sphere_Radius, 20 );
    }
}

void ComponentMeshPrimitive::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    ComponentMesh::DrawCallback( pCamera, pMatViewProj, pShaderOverride );
}
