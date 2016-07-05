//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "VoxelBlock.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"

#if MYFW_USING_WX
bool ComponentVoxelWorld::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentVoxelWorld ); //_VARIABLE_LIST

ComponentVoxelWorld::ComponentVoxelWorld()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pVoxelWorld = MyNew VoxelWorld;
    m_pVoxelWorld->Initialize( Vector3Int( 10, 3, 10 ) );

    m_pVoxelWorld->UpdateVisibility( this );
}

ComponentVoxelWorld::~ComponentVoxelWorld()
{
    delete m_pVoxelWorld;

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentVoxelWorld::RegisterVariables(CPPListHead* pList, ComponentVoxelWorld* pThis) //_VARIABLE_LIST
{
    //AddVar( pList, "SampleFloat", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_SampleVector3 ), true, true, 0, (CVarFunc_ValueChanged)&ComponentVoxelWorld::OnValueChanged, (CVarFunc_DropTarget)&ComponentVoxelWorld::OnDrop, 0 );
}

void ComponentVoxelWorld::Reset()
{
    ComponentBase::Reset();

    //m_SampleVector3.Set( 0, 0, 0 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentVoxelWorld::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSprite>( "ComponentVoxelWorld" )
            //.addData( "m_SampleVector3", &ComponentSprite::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentSprite::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void ComponentVoxelWorld::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentVoxelWorld::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "VoxelWorld", ObjectListIcon_Component );
}

void ComponentVoxelWorld::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentVoxelWorld::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "VoxelWorld", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentVoxelWorld::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        (ComponentBase*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        (GameObject*)g_DragAndDropStruct.m_Value;
    }

    return oldvalue;
}

void* ComponentVoxelWorld::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    //if( pVar->m_Offset == MyOffsetOf( this, &m_SampleVector3 ) )
    //{
    //    MyAssert( pVar->m_ControlID != -1 );
    //}

    return oldpointer;
}
#endif //MYFW_USING_WX

//cJSON* ComponentVoxelWorld::ExportAsJSONObject(bool savesceneid)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );
//
//    ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST
//
//    return jComponent;
//}
//
//void ComponentVoxelWorld::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
//{
//    ComponentBase::ImportFromJSONObject( jComponent, sceneid );
//
//    ImportVariablesFromJSON( jComponent ); //_VARIABLE_LIST
//}

ComponentVoxelWorld& ComponentVoxelWorld::operator=(const ComponentVoxelWorld& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    //m_SampleVector3 = other.m_SampleVector3;

    return *this;
}

void ComponentVoxelWorld::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentVoxelWorld, OnFileRenamed );
    }
}

void ComponentVoxelWorld::UnregisterCallbacks()
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
