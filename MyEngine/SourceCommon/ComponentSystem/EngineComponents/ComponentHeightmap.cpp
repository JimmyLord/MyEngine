//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentHeightmap.h"
#include "Core/EngineCore.h"

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentHeightmap ); //_VARIABLE_LIST

ComponentHeightmap::ComponentHeightmap(ComponentSystemManager* pComponentSystemManager)
: ComponentMesh( pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;
}

ComponentHeightmap::~ComponentHeightmap()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Tick );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnSurfaceChanged );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Draw );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnTouch );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnButtons );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnKeys );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnFileRenamed );
}

void ComponentHeightmap::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentHeightmap* pThis) //_VARIABLE_LIST
{
    ComponentMesh::RegisterVariables( pList, pThis );

    AddVar( pList, "Size", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Size ), true, true, "Size", (CVarFunc_ValueChanged)&ComponentHeightmap::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "VertCountx", ComponentVariableType_Vector2Int, MyOffsetOf( pThis, &pThis->m_VertCount ), true, true, "VertCount", (CVarFunc_ValueChanged)&ComponentHeightmap::OnValueChanged, nullptr, nullptr );
}

void ComponentHeightmap::Reset()
{
    ComponentMesh::Reset();

    m_Size.Set( 10.0f, 10.0f );
    m_VertCount.Set( 128, 128 );
}

#if MYFW_USING_LUA
void ComponentHeightmap::LuaRegister(lua_State* luaState)
{
    luabridge::getGlobalNamespace( luaState )
        .beginClass<ComponentHeightmap>( "ComponentHeightmap" )
            //.addData( "m_SampleVector3", &ComponentHeightmap::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentHeightmap::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
void* ComponentHeightmap::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = 0;
    return oldPointer;
}

void* ComponentHeightmap::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_Size ) )
    {
        CreateHeightmap();
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_VertCount ) )
    {
        CreateHeightmap();
    }

    return oldPointer;
}
#endif //MYFW_EDITOR

//cJSON* ComponentHeightmap::ExportAsJSONObject(bool saveSceneID, bool saveID)
//{
//    cJSON* jComponent = ComponentMesh::ExportAsJSONObject( saveSceneID, saveID );
//
//    return jComponent;
//}
//
//void ComponentHeightmap::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
//{
//    ComponentMesh::ImportFromJSONObject( jComponent, sceneID );
//}

void ComponentHeightmap::FinishImportingFromJSONObject(cJSON* jComponent)
{
    // This will be hit on initial load and on quickload.
    // Only create the mesh on initial load.
    // TODO: Also will rebuild if changes are made during runtime inside editor.
    if( m_pMesh == nullptr
//#if MYFW_EDITOR
//        || m_PrimitiveSettingsChangedAtRuntime
//#endif
        )
    {
        CreateHeightmap();
    }
}

ComponentHeightmap& ComponentHeightmap::operator=(const ComponentHeightmap& other)
{
    MyAssert( &other != this );

    ComponentMesh::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_Size = other.m_Size;
    m_VertCount = other.m_VertCount;

    return *this;
}

void ComponentHeightmap::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentHeightmap, OnFileRenamed );
    }
}

void ComponentHeightmap::UnregisterCallbacks()
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

//void ComponentHeightmap::TickCallback(float deltaTime)
//{
//}

void ComponentHeightmap::CreateHeightmap()
{
    if( m_pMesh == nullptr )
    {
        m_pMesh = MyNew MyMesh( m_pComponentSystemManager->GetEngineCore() );
    }
    else
    {
        RemoveFromRenderGraph();
    }

    if( m_pMesh->GetSubmeshListCount() > 0 )
        m_pMesh->GetSubmesh( 0 )->m_PrimitiveType = m_GLPrimitiveType;

    // Create a plane. // TODO: Change to a heightmap.
    {
        bool createTriangles = true;
        if( m_GLPrimitiveType == MyRE::PrimitiveType_Points )
            createTriangles = false;

        m_pMesh->CreatePlane( Vector3(-m_Size.x/2, 0, m_Size.y/2), m_Size, m_VertCount, Vector2(0,0), Vector2(1,1), createTriangles );
    }

    m_GLPrimitiveType = m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;

    // Add the Mesh to the main scene graph.
    AddToRenderGraph();
}
