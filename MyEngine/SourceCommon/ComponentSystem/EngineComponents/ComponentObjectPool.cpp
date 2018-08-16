//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentObjectPool::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentObjectPool ); //_VARIABLE_LIST

ComponentObjectPool::ComponentObjectPool()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pGameObjectInPool = 0;
    m_PoolSize = 100;
    m_LogWarningsWhenEmpty = true;
}

ComponentObjectPool::~ComponentObjectPool()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentObjectPool::RegisterVariables(CPPListHead* pList, ComponentObjectPool* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    //MyAssert( offsetof( ComponentObjectPool, m_SampleVector3 ) == MyOffsetOf( pThis, &pThis->m_SampleVector3 ) );
#if __GNUC__
#pragma GCC diagnostic pop
#endif

#if MYFW_USING_WX
    //AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentObjectPool::OnValueChanged, (CVarFunc_DropTarget)&ComponentObjectPool::OnDrop, 0 );
#else
    //AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentObjectPool::OnValueChanged, 0, 0 );
#endif
}

void ComponentObjectPool::Reset()
{
    ComponentBase::Reset();

    m_pGameObjectInPool = 0;
    m_PoolSize = 100;
    m_LogWarningsWhenEmpty = true;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentObjectPool::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentObjectPool>( "ComponentObjectPool" )
            //.addData( "m_SampleVector3", &ComponentObjectPool::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentObjectPool::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
#if MYFW_USING_WX
void ComponentObjectPool::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentObjectPool::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "ObjectPool", ObjectListIcon_Component );
}

void ComponentObjectPool::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentObjectPool::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "ObjectPool", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}
#endif //MYFW_USING_WX

void* ComponentObjectPool::OnDrop(ComponentVariable* pVar, int x, int y)
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

void* ComponentObjectPool::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    //if( pVar->m_Offset == MyOffsetOf( this, &m_SampleVector3 ) )
    //{
    //}

    return oldpointer;
}
#endif //MYFW_EDITOR

//cJSON* ComponentObjectPool::ExportAsJSONObject(bool savesceneid, bool saveid)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );
//
//    return jComponent;
//}
//
//void ComponentObjectPool::ImportFromJSONObject(cJSON* jComponent, SceneID sceneid)
//{
//    ComponentBase::ImportFromJSONObject( jComponent, sceneid );
//}

ComponentObjectPool& ComponentObjectPool::operator=(const ComponentObjectPool& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pGameObjectInPool = other.m_pGameObjectInPool;
    m_PoolSize = other.m_PoolSize;
    m_LogWarningsWhenEmpty = other.m_LogWarningsWhenEmpty;

    return *this;
}

void ComponentObjectPool::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentObjectPool, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentObjectPool, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentObjectPool, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentObjectPool, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentObjectPool, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentObjectPool, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentObjectPool, OnFileRenamed );
    }
}

void ComponentObjectPool::UnregisterCallbacks()
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

//void ComponentObjectPool::TickCallback(float deltaTime)
//{
//}