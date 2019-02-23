//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "Component3DJointPoint2Point.h"
#include "Component3DCollisionObject.h"
#include "ComponentSystem/Core/GameObject.h"

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( Component3DJointPoint2Point ); //_VARIABLE_LIST

Component3DJointPoint2Point::Component3DJointPoint2Point()
: Component3DJointBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pSecondCollisionObject = 0;
    
    m_PivotA.Set( 0, 0, 0 );
    m_PivotB.Set( 0, 0, 0 );

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}

Component3DJointPoint2Point::~Component3DJointPoint2Point()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void Component3DJointPoint2Point::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, Component3DJointPoint2Point* pThis) //_VARIABLE_LIST
{
    AddVar( pList, "SecondCollisionObject", ComponentVariableType_ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component3DJointPoint2Point::OnValueChanged, (CVarFunc_DropTarget)&Component3DJointPoint2Point::OnDrop, 0 );

    AddVar( pList, "PivotA", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_PivotA ), true, true, 0, (CVarFunc_ValueChanged)&Component3DJointPoint2Point::OnValueChanged, 0, 0 );
    AddVar( pList, "PivotB", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_PivotB ), true, true, 0, (CVarFunc_ValueChanged)&Component3DJointPoint2Point::OnValueChanged, 0, 0 );
}

void Component3DJointPoint2Point::Reset()
{
    ComponentBase::Reset();

    m_pSecondCollisionObject = 0;

    m_PivotA.Set( 0, 0, 0 );
    m_PivotB.Set( 0, 0, 0 );

    if( m_pJoint )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeConstraint( m_pJoint );
        delete m_pJoint;
    }

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void Component3DJointPoint2Point::LuaRegister(lua_State* luastate)
{
    //luabridge::getGlobalNamespace( luastate )
    //    .beginClass<Component3DJointPoint2Point>( "Component3DJointPoint2Point" )
    //        .addData( "density", &Component3DJointPoint2Point::m_Density )
    //        .addFunction( "GetMass", &Component3DJointPoint2Point::GetMass )            
    //    .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
#if MYFW_USING_WX
void Component3DJointPoint2Point::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, Component3DJointPoint2Point::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "3DJointPoint2Point", ObjectListIcon_Component );
}

void Component3DJointPoint2Point::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void Component3DJointPoint2Point::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "3D Joint - Point2Point", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}
#endif //MYFW_USING_WX

void* Component3DJointPoint2Point::OnDrop(ComponentVariable* pVar, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        ComponentBase* pComponent = (ComponentBase*)pDropItem->m_Value;

        if( pComponent->IsA( "3DCollisionObjectComponent" ) )
        {
            oldPointer = m_pSecondCollisionObject;

            m_pSecondCollisionObject = (Component3DCollisionObject*)pComponent;
        }        
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        if( m_pSecondCollisionObject )
            oldPointer = m_pSecondCollisionObject;

        GameObject* pGameObject = (GameObject*)pDropItem->m_Value;

        m_pSecondCollisionObject = pGameObject->Get3DCollisionObject();
    }

    return oldPointer;
}

void* Component3DJointPoint2Point::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
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
            // TODO: Implement this block.
        }
    }

    // The joint will only exist if game is running.
    if( m_pJoint )
    {
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

Component3DJointPoint2Point& Component3DJointPoint2Point::operator=(const Component3DJointPoint2Point& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: Replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pSecondCollisionObject = other.m_pSecondCollisionObject;

    m_PivotA = other.m_PivotA;
    m_PivotB = other.m_PivotB;

    m_pJoint = other.m_pJoint;
    m_pBody = other.m_pBody;
    m_pSecondBody = other.m_pSecondBody;

    return *this;
}

void Component3DJointPoint2Point::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointPoint2Point, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointPoint2Point, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointPoint2Point, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointPoint2Point, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointPoint2Point, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointPoint2Point, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointPoint2Point, OnFileRenamed );
    }
}

void Component3DJointPoint2Point::UnregisterCallbacks()
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

void Component3DJointPoint2Point::OnPlay()
{
    ComponentBase::OnPlay();

    m_pBody = m_pGameObject->Get3DCollisionObject()->GetBody();
    if( m_pSecondCollisionObject )
        m_pSecondBody = m_pSecondCollisionObject->GetBody();

    if( m_pBody )
    {
        //b2WeldJointDef jointdef;

        //if( m_pSecondBody )
        //{
        //    //Vector3 posA = m_pGameObject->GetTransform()->GetWorldPosition();
        //    //Vector3 posB = m_pSecondCollisionObject->m_pGameObject->GetTransform()->GetWorldPosition();
        //    //b2Vec2 anchorpos( posB.x - posA.x + m_AnchorA.x, posB.y - posA.y + m_AnchorA.y );
        //    //b2Vec2 anchorpos( (posB.x - m_AnchorB.x) - posA.x, (posB.y - m_AnchorB.y) - posA.y );

        //    jointdef.bodyA = m_pBody;
        //    jointdef.bodyB = m_pSecondBody;
        //    jointdef.collideConnected = false;
        //    //jointdef.localAnchorA = anchorpos;//m_AnchorA.x, m_AnchorA.y );
        //    jointdef.localAnchorA.Set( m_AnchorA.x, m_AnchorA.y );
        //    jointdef.localAnchorB.Set( m_AnchorB.x, m_AnchorB.y );
        //}
        //else
        //{
        //    Vector3 pos = m_pGameObject->GetTransform()->GetWorldPosition();
        //    b2Vec2 anchorpos( pos.x + m_AnchorA.x, pos.y + m_AnchorA.y );
        //    //b2Vec2 anchorpos( pos.x + m_AnchorB.x, pos.y + m_AnchorB.y );

        //    jointdef.Initialize( m_pBody, pBox3DWorld->m_pGround, anchorpos );
        //}

        if( m_pSecondBody )
        {
            btVector3 pivota( m_PivotA.x, m_PivotA.y, m_PivotA.z );
            btVector3 pivotb( m_PivotB.x, m_PivotB.y, m_PivotB.z );
            m_pJoint = new btPoint2PointConstraint( *m_pBody, *m_pSecondBody, pivota, pivotb );
            g_pBulletWorld->m_pDynamicsWorld->addConstraint( m_pJoint, true );
        }
        else
        {
            btVector3 pivota( m_PivotA.x, m_PivotA.y, m_PivotA.z );
            m_pJoint = new btPoint2PointConstraint( *m_pBody, pivota );
            g_pBulletWorld->m_pDynamicsWorld->addConstraint( m_pJoint, true );
        }
    }
}

void Component3DJointPoint2Point::OnStop()
{
    ComponentBase::OnStop();

    RemoveJointFromWorld();
    delete m_pJoint;
    m_pJoint = 0;

    m_pBody = 0;
    m_pSecondBody = 0;
}

void Component3DJointPoint2Point::RemoveJointFromWorld()
{
    if( m_pJoint )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeConstraint( m_pJoint );
    }
}
