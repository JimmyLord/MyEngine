//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool Component2DJointWeld::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( Component2DJointWeld ); //_VARIABLE_LIST

Component2DJointWeld::Component2DJointWeld()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pSecondCollisionObject = 0;
    
    m_AnchorA.Set( 0, 0 );
    m_AnchorB.Set( 0, 0 );

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}

Component2DJointWeld::~Component2DJointWeld()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void Component2DJointWeld::RegisterVariables(CPPListHead* pList, Component2DJointWeld* pThis) //_VARIABLE_LIST
{
#if MYFW_USING_WX
    AddVar( pList, "SecondCollisionObject", ComponentVariableType_ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component2DJointWeld::OnValueChanged, (CVarFunc_DropTarget)&Component2DJointWeld::OnDrop, 0 );
#else
    AddVar( pList, "SecondCollisionObject", ComponentVariableType_ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component2DJointWeld::OnValueChanged, 0, 0 );
#endif

    AddVar( pList, "AnchorA", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_AnchorA ), true, true, 0, (CVarFunc_ValueChanged)&Component2DJointWeld::OnValueChanged, 0, 0 );
    AddVar( pList, "AnchorB", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_AnchorB ), true, true, 0, (CVarFunc_ValueChanged)&Component2DJointWeld::OnValueChanged, 0, 0 );
}

void Component2DJointWeld::Reset()
{
    ComponentBase::Reset();

    m_pSecondCollisionObject = 0;

    m_AnchorA.Set( 0, 0 );
    m_AnchorB.Set( 0, 0 );

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void Component2DJointWeld::LuaRegister(lua_State* luastate)
{
    //luabridge::getGlobalNamespace( luastate )
    //    .beginClass<Component2DJointWeld>( "Component2DJointWeld" )
    //        .addData( "density", &Component2DJointWeld::m_Density )
    //        .addFunction( "GetMass", &Component2DJointWeld::GetMass )            
    //    .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void Component2DJointWeld::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, Component2DJointWeld::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "2DJointWeld", ObjectListIcon_Component );
}

void Component2DJointWeld::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void Component2DJointWeld::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "2D Weld Joint", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* Component2DJointWeld::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = m_pSecondCollisionObject;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        ComponentBase* pComponent = (ComponentBase*)pDropItem->m_Value;

        if( pComponent->IsA( "2DCollisionObjectComponent" ) )
        {
            m_pSecondCollisionObject = (Component2DCollisionObject*)pComponent;
        }        
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)pDropItem->m_Value;

        m_pSecondCollisionObject = pGameObject->Get2DCollisionObject();
    }

    return oldvalue;
}
#endif //MYFW_USING_WX

#if MYFW_EDITOR
void* Component2DJointWeld::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_pSecondCollisionObject ) )
    {
        if( changedbyinterface )
        {
#if MYFW_USING_WX
            wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
            if( text == "" || text == "none" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );
                oldpointer = this->m_pSecondCollisionObject;
                m_pSecondCollisionObject = 0;

                g_pPanelWatch->SetNeedsRefresh();
            }
#endif //MYFW_USING_WX
        }
        else if( pNewValue->GetComponentPtr() != 0 )
        {
            MyAssert( false );
            // TODO: implement this block
        }
    }

    // the joint will only exist if game is running.
    if( m_pJoint )
    {
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

Component2DJointWeld& Component2DJointWeld::operator=(const Component2DJointWeld& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pSecondCollisionObject = other.m_pSecondCollisionObject;

    m_AnchorA = other.m_AnchorA;
    m_AnchorB = other.m_AnchorB;

    m_pJoint = other.m_pJoint;
    m_pBody = other.m_pBody;
    m_pSecondBody = other.m_pSecondBody;

    return *this;
}

void Component2DJointWeld::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointWeld, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointWeld, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointWeld, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointWeld, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointWeld, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointWeld, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointWeld, OnFileRenamed );
    }
}

void Component2DJointWeld::UnregisterCallbacks()
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

void Component2DJointWeld::OnPlay()
{
    ComponentBase::OnPlay();

    m_pBody = m_pGameObject->Get2DCollisionObject()->m_pBody;
    if( m_pSecondCollisionObject )
        m_pSecondBody = m_pSecondCollisionObject->m_pBody;

    Box2DWorld* pBox2DWorld = m_pGameObject->Get2DCollisionObject()->m_pBox2DWorld;

    if( m_pBody )
    {
        b2WeldJointDef jointdef;

        if( m_pSecondBody )
        {
            //Vector3 posA = m_pGameObject->GetTransform()->GetWorldPosition();
            //Vector3 posB = m_pSecondCollisionObject->m_pGameObject->GetTransform()->GetWorldPosition();
            //b2Vec2 anchorpos( posB.x - posA.x + m_AnchorA.x, posB.y - posA.y + m_AnchorA.y );
            //b2Vec2 anchorpos( (posB.x - m_AnchorB.x) - posA.x, (posB.y - m_AnchorB.y) - posA.y );

            jointdef.bodyA = m_pBody;
            jointdef.bodyB = m_pSecondBody;
            jointdef.collideConnected = false;
            //jointdef.localAnchorA = anchorpos;//m_AnchorA.x, m_AnchorA.y );
            jointdef.localAnchorA.Set( m_AnchorA.x, m_AnchorA.y );
            jointdef.localAnchorB.Set( m_AnchorB.x, m_AnchorB.y );
        }
        else
        {
            Vector3 pos = m_pGameObject->GetTransform()->GetWorldPosition();
            b2Vec2 anchorpos( pos.x + m_AnchorA.x, pos.y + m_AnchorA.y );
            //b2Vec2 anchorpos( pos.x + m_AnchorB.x, pos.y + m_AnchorB.y );

            jointdef.Initialize( m_pBody, pBox2DWorld->m_pGround, anchorpos );
        }

        m_pJoint = (b2WeldJoint*)pBox2DWorld->m_pWorld->CreateJoint( &jointdef );
    }
}

void Component2DJointWeld::OnStop()
{
    ComponentBase::OnStop();

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}
