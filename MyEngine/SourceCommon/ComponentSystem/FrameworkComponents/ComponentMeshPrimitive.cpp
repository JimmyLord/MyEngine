//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentMeshPrimitive.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../../../SourceEditor/Commands/EngineEditorCommands.h"
#endif

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMeshPrimitive ); //_VARIABLE_LIST

// These strings are used to save/load, so don't change them.
const char* ComponentMeshPrimitiveTypeStrings[ComponentMeshPrimitive_NumTypes] =
{
    "Plane",       // ComponentMeshPrimitive_Plane
    "Icosphere",   // ComponentMeshPrimitive_Icosphere
    "2DCircle",    // ComponentMeshPrimitive_2DCircle
    "Grass",       // ComponentMeshPrimitive_Grass
    "Copy",        // ComponentMeshPrimitive_ReferenceToAnotherMeshPrimitive
};

ComponentMeshPrimitive::ComponentMeshPrimitive(ComponentSystemManager* pComponentSystemManager)
: ComponentMesh( pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;
}

ComponentMeshPrimitive::~ComponentMeshPrimitive()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentMeshPrimitive::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentMeshPrimitive* pThis) //_VARIABLE_LIST
{
    ComponentMesh::RegisterVariables( pList, pThis );

    ComponentVariable* pVars[8];

    pVars[0] = AddVarEnum( pList, "MPType", MyOffsetOf( pThis, &pThis->m_MeshPrimitiveType ), true, true, nullptr, ComponentMeshPrimitive_NumTypesAccessibleFromInterface, ComponentMeshPrimitiveTypeStrings, (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );
    pVars[1] = AddVar( pList, "PlaneSize", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Plane_Size ), true, true, "Size", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );
    pVars[2] = AddVar( pList, "PlaneVertCountx", ComponentVariableType_Int, MyOffsetOf( pThis, &pThis->m_Plane_VertCount.x ), true, true, "VertCount X", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );
    pVars[3] = AddVar( pList, "PlaneVertCounty", ComponentVariableType_Int, MyOffsetOf( pThis, &pThis->m_Plane_VertCount.y ), true, true, "VertCount Y", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );
    pVars[4] = AddVar( pList, "UVs per Quad", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_Plane_UVsPerQuad ), true, true, "UVs per Quad", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );
    pVars[5] = AddVar( pList, "PlaneUVStart", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Plane_UVStart ), true, true, "UVStart", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );
    pVars[6] = AddVar( pList, "PlaneUVRange", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Plane_UVRange ), true, true, "UVRange", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );
    pVars[7] = AddVar( pList, "SphereRadius", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Sphere_Radius ), true, true, "Radius", (CVarFunc_ValueChanged)&ComponentMeshPrimitive::OnValueChanged, nullptr, nullptr );

#if MYFW_EDITOR
    for( int i=0; i<8; i++ )
    {
        pVars[i]->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentMeshPrimitive::ShouldVariableBeAddedToWatchPanel) );
        //pVars[i]->AddCallback_VariableAddedToInterface( (CVarFunc_VariableAddedToInterface)(&ComponentMeshPrimitive::VariableAddedToWatchPanel) );
    }
#endif
}

void ComponentMeshPrimitive::Reset()
{
    ComponentMesh::Reset();

    m_MeshPrimitiveType = ComponentMeshPrimitive_NumTypes;

    m_Plane_Size.Set( 10, 10 );
    m_Plane_VertCount.Set( 10, 10 );
    m_Plane_UVsPerQuad = false;
    m_Plane_UVStart.Set( 0, 0 );
    m_Plane_UVRange.Set( 1, 1 );

    m_Sphere_Radius = 1;

    m_pOtherMeshPrimitive = nullptr;

#if MYFW_EDITOR
    m_PrimitiveSettingsChangedAtRuntime = false;
#endif //MYFW_EDITOR
}

#if MYFW_USING_LUA
void ComponentMeshPrimitive::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentMeshPrimitive>( "ComponentMeshPrimitive" )
            //.addData( "density", &ComponentMeshPrimitive::m_Density ) // float
            //.addFunction( "ClearVelocity", &ComponentMeshPrimitive::ClearVelocity ) // void Component2DCollisionObject::ClearVelocity()
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
bool ComponentMeshPrimitive::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
{
    if( m_MeshPrimitiveType == ComponentMeshPrimitive_Plane )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return true;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return true;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return true;
        if( strcmp( pVar->m_Label, "UVs per Quad" ) == 0 )       return true;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return true;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return true;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return false;
    }

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_Icosphere )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return false;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "UVs per Quad" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return true;
    }

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_2DCircle )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return false;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "UVs per Quad" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return true;
    }

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_Grass )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return true;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return true;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return true;
        if( strcmp( pVar->m_Label, "UVs per Quad" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return true;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return false;
    }

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_ReferenceToAnotherMeshPrimitive ||
        m_MeshPrimitiveType == ComponentMeshPrimitive_NumTypes )
    {
        if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )          return false;
        if( strcmp( pVar->m_Label, "PlaneVertCountx" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "PlaneVertCounty" ) == 0 )    return false;
        if( strcmp( pVar->m_Label, "UVs per Quad" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVStart" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "PlaneUVRange" ) == 0 )       return false;
        if( strcmp( pVar->m_Label, "SphereRadius" ) == 0 )       return false;
    }

    return ComponentMesh::ShouldVariableBeAddedToWatchPanel( pVar );
}

void* ComponentMeshPrimitive::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = nullptr;

    if( finishedChanging )
    {
        if( g_pEngineCore->IsInEditorMode() )
        {
            if( strcmp( pVar->m_Label, "MPType" ) == 0 )
            {
                if( oldValue == ComponentMeshPrimitive_ReferenceToAnotherMeshPrimitive )
                {
                    if( changedByInterface )
                    {
                        // Changing from a copy of another mesh primitive to a new object, so create a new mesh.
                        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh( this, m_pMesh, m_MeshPrimitiveType ) );
                    }

                    // If switching away from a reference to another mesh primitive (Do or Redo),
                    //   let EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh call CreatePrimitive().
                    return oldPointer;
                }
            }
        }

        if( g_pEngineCore->IsInEditorMode() == false )
        {
            if( m_pOtherMeshPrimitive )
            {
                m_pOtherMeshPrimitive->m_PrimitiveSettingsChangedAtRuntime = true;
            }
            else
            {
                m_PrimitiveSettingsChangedAtRuntime = true;
            }
        }

        MyClampMin( m_Plane_VertCount.x, 2 );
        MyClampMin( m_Plane_VertCount.y, 2 );

        CreatePrimitive();

        PushChangesToRenderGraphObjects();
    }

    return oldPointer;
}
#endif //MYFW_EDITOR

cJSON* ComponentMeshPrimitive::ExportAsJSONObject(bool saveSceneID, bool saveID)
{
    cJSON* jComponent = ComponentMesh::ExportAsJSONObject( saveSceneID, saveID );

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_ReferenceToAnotherMeshPrimitive )
    {
        cJSON_AddItemToObject( jComponent, "OtherMeshPrimitive", m_pOtherMeshPrimitive->ExportReferenceAsJSONObject() );
    }

    return jComponent;
}

void ComponentMeshPrimitive::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
{
    ComponentMesh::ImportFromJSONObject( jComponent, sceneID );

    // For compatibility with old files, now saved as Vector2s.
    cJSONExt_GetFloat( jComponent, "PlaneSizex", &m_Plane_Size.x );
    cJSONExt_GetFloat( jComponent, "PlaneSizey", &m_Plane_Size.y );
    cJSONExt_GetFloat( jComponent, "PlaneUVStartx", &m_Plane_UVStart.x );
    cJSONExt_GetFloat( jComponent, "PlaneUVStarty", &m_Plane_UVStart.y );
    cJSONExt_GetFloat( jComponent, "PlaneUVRangex", &m_Plane_UVRange.x );
    cJSONExt_GetFloat( jComponent, "PlaneUVRangey", &m_Plane_UVRange.y );

    // This will be hit on initial load and on quickload.
    // Only create the mesh on initial load.
    // Also will rebuild if changes are made during runtime inside editor.
    if( m_pMesh == nullptr
#if MYFW_EDITOR
        || m_PrimitiveSettingsChangedAtRuntime
#endif
      )
    {
        CreatePrimitive();
    }
}

void ComponentMeshPrimitive::FinishImportingFromJSONObject(cJSON* jComponent)
{
    // This will be hit on initial load and on quickload.
    // Only create the mesh on initial load.
    // Also will rebuild if changes are made during runtime inside editor.
    if( m_pMesh == nullptr
#if MYFW_EDITOR
        || m_PrimitiveSettingsChangedAtRuntime
#endif
      )
    {
        // If referencing another mesh primitive component, grab it's mesh pointer.
        if( m_MeshPrimitiveType == ComponentMeshPrimitive_ReferenceToAnotherMeshPrimitive )
        {
            cJSON* jComponentRef = cJSON_GetObjectItem( jComponent, "OtherMeshPrimitive" );

            ComponentMeshPrimitive* pComponent = (ComponentMeshPrimitive*)g_pComponentSystemManager->FindComponentByJSONRef( jComponentRef, m_SceneIDLoadedFrom );
            if( pComponent == nullptr )
            {
                LOGError( LOGTag, "A referenced mesh primitive wasn't found, changing type to 'Plane'. GameObject: %s\n", m_pGameObject->GetName() );
                m_MeshPrimitiveType = ComponentMeshPrimitive_Plane;
                CreatePrimitive();
            }
            else
            {
                MyAssert( pComponent->IsA( "MeshPrimitiveComponent" ) );
                m_pOtherMeshPrimitive = pComponent;

                MyAssert( m_pOtherMeshPrimitive->m_pMesh != nullptr );
                SetMesh( m_pOtherMeshPrimitive->m_pMesh );
            }
        }
    }

#if MYFW_EDITOR
    m_PrimitiveSettingsChangedAtRuntime = false;
#endif
}

ComponentMeshPrimitive& ComponentMeshPrimitive::operator=(ComponentMeshPrimitive& other)
{
    MyAssert( &other != this );

    this->m_MeshPrimitiveType = ComponentMeshPrimitive_ReferenceToAnotherMeshPrimitive;
    if( other.m_pOtherMeshPrimitive )
        this->m_pOtherMeshPrimitive = other.m_pOtherMeshPrimitive;
    else
        this->m_pOtherMeshPrimitive = &other;

    this->m_Plane_Size = other.m_Plane_Size;
    this->m_Plane_VertCount = other.m_Plane_VertCount;
    this->m_Plane_UVsPerQuad = other.m_Plane_UVsPerQuad;
    this->m_Plane_UVStart = other.m_Plane_UVStart;
    this->m_Plane_UVRange = other.m_Plane_UVRange;

    this->m_Sphere_Radius = other.m_Sphere_Radius;

    ComponentMesh::operator=( other );

    return *this;
}

void ComponentMeshPrimitive::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnSurfaceChanged );
        MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentMeshPrimitive, Draw ); //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMeshPrimitive, OnFileRenamed );
    }
}

void ComponentMeshPrimitive::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentMeshPrimitive::CreatePrimitive()
{
    if( m_MeshPrimitiveType >= ComponentMeshPrimitive_NumTypesAccessibleFromInterface )
        return;

    if( m_pMesh == nullptr )
    {
        m_pMesh = MyNew MyMesh( m_pComponentSystemManager->GetEngineCore() );
    }
    else
        RemoveFromRenderGraph();

    if( m_pMesh->GetSubmeshListCount() > 0 )
        m_pMesh->GetSubmesh( 0 )->m_PrimitiveType = m_GLPrimitiveType;

    if( m_MeshPrimitiveType == ComponentMeshPrimitive_Plane )
    {
        bool createTriangles = true;
        if( m_GLPrimitiveType == MyRE::PrimitiveType_Points )
            createTriangles = false;

        if( m_Plane_UVsPerQuad )
            m_pMesh->CreatePlaneUVsNotShared( Vector3(-m_Plane_Size.x/2, 0, m_Plane_Size.y/2), m_Plane_Size, m_Plane_VertCount, m_Plane_UVStart, m_Plane_UVRange, createTriangles );
        else
            m_pMesh->CreatePlane( Vector3(-m_Plane_Size.x/2, 0, m_Plane_Size.y/2), m_Plane_Size, m_Plane_VertCount, m_Plane_UVStart, m_Plane_UVRange, createTriangles );
    }
    else if( m_MeshPrimitiveType == ComponentMeshPrimitive_Icosphere )
    {
        m_pMesh->CreateIcosphere( m_Sphere_Radius, 0 );
    }
    else if( m_MeshPrimitiveType == ComponentMeshPrimitive_2DCircle )
    {
        m_pMesh->Create2DCircle( m_Sphere_Radius, 20 );
    }
    else if( m_MeshPrimitiveType == ComponentMeshPrimitive_Grass )
    {
        m_pMesh->CreateGrass( Vector3(-m_Plane_Size.x/2, 0, -m_Plane_Size.y/2), m_Plane_Size, m_Plane_VertCount, m_Plane_UVStart );
    }

    m_GLPrimitiveType = m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;

    // Add the Mesh to the main scene graph.
    AddToRenderGraph();
}

void ComponentMeshPrimitive::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    ComponentMesh::DrawCallback( pCamera, pMatProj, pMatView, pShaderOverride );
}
