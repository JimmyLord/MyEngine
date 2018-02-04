//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentTemplate::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
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

#if MYFW_USING_WX
    AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentTemplate::OnValueChanged, (CVarFunc_DropTarget)&ComponentTemplate::OnDrop, 0 );
#else
    AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentTemplate::OnValueChanged, 0, 0 );
#endif
}

void ComponentTemplate::Reset()
{
    ComponentBase::Reset();

    m_SampleVector3.Set( 0, 0, 0 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentTemplate::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentTemplate>( "ComponentTemplate" )
            //.addData( "m_SampleVector3", &ComponentTemplate::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentTemplate::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void ComponentTemplate::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentTemplate::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Template", ObjectListIcon_Component );
}

void ComponentTemplate::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentTemplate::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Template", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentTemplate::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldpointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        //(ComponentBase*)pDropItem->m_Value;
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        //(GameObject*)pDropItem->m_Value;
    }

    return oldpointer;
}
#endif //MYFW_USING_WX

#if MYFW_EDITOR
void* ComponentTemplate::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_SampleVector3 ) )
    {
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

//cJSON* ComponentTemplate::ExportAsJSONObject(bool savesceneid, bool saveid)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );
//
//    return jComponent;
//}
//
//void ComponentTemplate::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
//{
//    ComponentBase::ImportFromJSONObject( jComponent, sceneid );
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

//void ComponentTemplate::TickCallback(double TimePassed)
//{
//}
