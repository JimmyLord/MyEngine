//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentAnimationPlayer2D::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentAnimationPlayer2D ); //_VARIABLE_LIST

ComponentAnimationPlayer2D::ComponentAnimationPlayer2D()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_TimeBetweenFrames = 0;
}

ComponentAnimationPlayer2D::~ComponentAnimationPlayer2D()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentAnimationPlayer2D::RegisterVariables(CPPListHead* pList, ComponentAnimationPlayer2D* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if MYFW_IOS || MYFW_OSX || MYFW_NACL
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    MyAssert( offsetof( ComponentAnimationPlayer2D, m_TimeBetweenFrames ) == MyOffsetOf( pThis, &pThis->m_TimeBetweenFrames ) );
#if MYFW_IOS || MYFW_OSX
#pragma GCC diagnostic default "-Winvalid-offsetof"
#endif

    AddVar( pList, "Time between frames", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_TimeBetweenFrames ), true, true, 0, (CVarFunc_ValueChanged)&ComponentAnimationPlayer2D::OnValueChanged, (CVarFunc_DropTarget)&ComponentAnimationPlayer2D::OnDrop, 0 );
}

void ComponentAnimationPlayer2D::Reset()
{
    ComponentBase::Reset();

    m_pSpriteComponent = 0;

    m_TimeBetweenFrames = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentAnimationPlayer2D::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentAnimationPlayer2D>( "ComponentAnimationPlayer2D" )
            //.addData( "m_TimeBetweenFrames", &ComponentAnimationPlayer2D::m_TimeBetweenFrames )
            //.addFunction( "GetTimeBetweenFrames", &ComponentAnimationPlayer2D::GetTimeBetweenFrames )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void ComponentAnimationPlayer2D::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentAnimationPlayer2D::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Animation 2D", ObjectListIcon_Component );
}

void ComponentAnimationPlayer2D::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentAnimationPlayer2D::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Template", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentAnimationPlayer2D::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldpointer = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        (ComponentBase*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        (GameObject*)g_DragAndDropStruct.m_Value;
    }

    return oldpointer;
}

void* ComponentAnimationPlayer2D::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_TimeBetweenFrames ) )
    {
        MyAssert( pVar->m_ControlID != -1 );
    }

    return oldpointer;
}
#endif //MYFW_USING_WX

//cJSON* ComponentAnimationPlayer2D::ExportAsJSONObject(bool savesceneid)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );
//
//    return jComponent;
//}
//
//void ComponentAnimationPlayer2D::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
//{
//    ComponentBase::ImportFromJSONObject( jComponent, sceneid );
//}

ComponentAnimationPlayer2D& ComponentAnimationPlayer2D::operator=(const ComponentAnimationPlayer2D& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_TimeBetweenFrames = other.m_TimeBetweenFrames;

    return *this;
}

void ComponentAnimationPlayer2D::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnFileRenamed );
    }
}

void ComponentAnimationPlayer2D::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentAnimationPlayer2D::TickCallback(double TimePassed)
{
    if( m_pSpriteComponent == 0 )
    {
        ComponentBase* pComponent = m_pGameObject->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        if( pComponent )
            m_pSpriteComponent = pComponent->IsA( "SpriteComponent" ) ? (ComponentSprite*)pComponent : 0;
    }

    if( m_pSpriteComponent == 0 )
        return;
}
