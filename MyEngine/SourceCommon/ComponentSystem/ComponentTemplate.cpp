//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentTemplate.h"

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentTemplate ); //_VARIABLE_LIST

ComponentTemplate::ComponentTemplate()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_SampleVector3.Set( 0, 0, 0 );
}

ComponentTemplate::~ComponentTemplate()
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

void ComponentTemplate::RegisterVariables(CPPListHead* pList, ComponentTemplate* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    MyAssert( offsetof( ComponentTemplate, m_SampleVector3 ) == MyOffsetOf( pThis, &pThis->m_SampleVector3 ) );
#if __GNUC__
#pragma GCC diagnostic pop
#endif

    AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentTemplate::OnValueChanged, 0, 0 );
}

void ComponentTemplate::Reset()
{
    ComponentBase::Reset();

    m_SampleVector3.Set( 0, 0, 0 );
}

#if MYFW_USING_LUA
void ComponentTemplate::LuaRegister(lua_State* luaState)
{
    luabridge::getGlobalNamespace( luaState )
        .beginClass<ComponentTemplate>( "ComponentTemplate" )
            //.addData( "m_SampleVector3", &ComponentTemplate::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentTemplate::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
void* ComponentTemplate::OnDrop(ComponentVariable* pVar, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        //oldPointer = old component;
        //m_Value = (ComponentBase*)pDropItem->m_Value;
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        //oldPointer = old gameobject;
        //m_Value = (GameObject*)pDropItem->m_Value;
    }

    return oldPointer;
}

void* ComponentTemplate::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_SampleVector3 ) )
    {
    }

    return oldPointer;
}
#endif //MYFW_EDITOR

//cJSON* ComponentTemplate::ExportAsJSONObject(bool saveSceneID, bool saveID)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( saveSceneID, saveID );
//
//    return jComponent;
//}
//
//void ComponentTemplate::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
//{
//    ComponentBase::ImportFromJSONObject( jComponent, sceneID );
//}

ComponentTemplate& ComponentTemplate::operator=(const ComponentTemplate& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_SampleVector3 = other.m_SampleVector3;

    return *this;
}

void ComponentTemplate::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTemplate, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTemplate, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTemplate, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTemplate, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTemplate, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTemplate, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentTemplate, OnFileRenamed );
    }
}

void ComponentTemplate::UnregisterCallbacks()
{
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

//void ComponentTemplate::TickCallback(float deltaTime)
//{
//}
