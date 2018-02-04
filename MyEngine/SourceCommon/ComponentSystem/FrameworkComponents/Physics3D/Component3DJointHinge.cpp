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
bool Component3DJointHinge::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( Component3DJointHinge ); //_VARIABLE_LIST

Component3DJointHinge::Component3DJointHinge()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pSecondCollisionObject = 0;
    
    m_PivotA.Set( 0, 0, 0 );
    m_PivotB.Set( 0, 0, 0 );

    m_MotorEnabled = false;
    m_MotorSpeed = 0;
    m_MotorMaxTorque = 0;

    m_AngleLimitEnabled = false;
    m_AngleLimitMin = 0;
    m_AngleLimitMax = 0;

    m_BreakForce = 0;

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}

Component3DJointHinge::~Component3DJointHinge()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void Component3DJointHinge::RegisterVariables(CPPListHead* pList, Component3DJointHinge* pThis) //_VARIABLE_LIST
{
#if MYFW_USING_WX
    AddVar( pList, "SecondCollisionObject", ComponentVariableType_ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, (CVarFunc_DropTarget)&Component3DJointHinge::OnDrop, 0 );
#else
    AddVar( pList, "SecondCollisionObject", ComponentVariableType_ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );
#endif

    AddVar( pList, "PivotA", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_PivotA ), true, true, 0, (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );
    AddVar( pList, "PivotB", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_PivotB ), true, true, 0, (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );

    AddVar( pList, "MotorEnabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_MotorEnabled ), true, true, "Motor Enabled", (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorSpeed", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_MotorSpeed ), true, true, "Motor Speed", (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorMaxTorque", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_MotorMaxTorque ), true, true, "Motor Max Torque", (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );

    AddVar( pList, "LimitEnabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_AngleLimitEnabled ), true, true, "Angle Limit Enabled", (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMin", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_AngleLimitMin ), true, true, "Min Angle", (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMax", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_AngleLimitMax ), true, true, "Max Angle", (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );

    AddVar( pList, "BreakForce", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_BreakForce ), true, true, "Break Force", (CVarFunc_ValueChanged)&Component3DJointHinge::OnValueChanged, 0, 0 );
}

void Component3DJointHinge::Reset()
{
    ComponentBase::Reset();

    m_pSecondCollisionObject = 0;

    m_PivotA.Set( 0, 0, 0 );
    m_PivotB.Set( 0, 0, 0 );

    m_MotorEnabled = false;
    m_MotorSpeed = 0;
    m_MotorMaxTorque = 0;

    m_AngleLimitEnabled = false;
    m_AngleLimitMin = 0;
    m_AngleLimitMax = 0;

    m_BreakForce = 0;

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void Component3DJointHinge::LuaRegister(lua_State* luastate)
{
    //luabridge::getGlobalNamespace( luastate )
    //    .beginClass<Component3DJointHinge>( "Component3DJointHinge" )
    //        .addData( "density", &Component3DJointHinge::m_Density )
    //        .addFunction( "GetMass", &Component3DJointHinge::GetMass )            
    //    .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void Component3DJointHinge::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, Component3DJointHinge::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "3DJointHinge", ObjectListIcon_Component );
}

void Component3DJointHinge::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void Component3DJointHinge::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "3D Hinge Joint", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* Component3DJointHinge::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = m_pSecondCollisionObject;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        ComponentBase* pComponent = (ComponentBase*)pDropItem->m_Value;

        if( pComponent->IsA( "3DCollisionObjectComponent" ) )
        {
            m_pSecondCollisionObject = (Component3DCollisionObject*)pComponent;
        }        
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)pDropItem->m_Value;

        m_pSecondCollisionObject = pGameObject->Get3DCollisionObject();
    }

    return oldvalue;
}
#endif //MYFW_USING_WX

#if MYFW_EDITOR
void* Component3DJointHinge::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
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

    // sanity check on angle limits.
    if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitMin ) )
    {
        if( m_AngleLimitMin > m_AngleLimitMax )
            m_AngleLimitMin = m_AngleLimitMax;
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitMax ) )
    {
        if( m_AngleLimitMax < m_AngleLimitMin )
            m_AngleLimitMax = m_AngleLimitMin;
    }

    // the joint will only exist if game is running.
    if( m_pJoint )
    {
        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorEnabled ) )
        {
            if( fequal( m_MotorSpeed, 0 ) == false )
            {
                m_pJoint->enableAngularMotor( true, m_MotorSpeed, m_MotorMaxTorque );
            }
            else
            {
                m_pJoint->enableAngularMotor( false, 0, 0 );
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorSpeed ) )
        {
            m_pJoint->setMotorTargetVelocity( m_MotorSpeed );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorMaxTorque ) )
        {
            m_pJoint->setMaxMotorImpulse( m_MotorMaxTorque );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitEnabled ) )
        {
            if( fequal( m_MotorSpeed, 0 ) == false )
            {
                m_pJoint->setLimit( m_AngleLimitMin * PI/180, m_AngleLimitMax * PI/180 );
            }
            else
            {
                m_pJoint->setLimit( 0, 0 );
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitMin ) )
        {
            m_pJoint->setLimit( m_AngleLimitMin * PI/180, m_AngleLimitMax * PI/180 );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitMax ) )
        {
            m_pJoint->setLimit( m_AngleLimitMin * PI/180, m_AngleLimitMax * PI/180 );
        }
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

Component3DJointHinge& Component3DJointHinge::operator=(const Component3DJointHinge& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pSecondCollisionObject = other.m_pSecondCollisionObject;

    m_PivotA = other.m_PivotA;
    m_PivotB = other.m_PivotB;

    m_MotorEnabled = other.m_MotorEnabled;
    m_MotorSpeed = other.m_MotorSpeed;
    m_MotorMaxTorque = other.m_MotorMaxTorque;

    m_AngleLimitEnabled = other.m_AngleLimitEnabled;
    m_AngleLimitMin = other.m_AngleLimitMin;
    m_AngleLimitMax = other.m_AngleLimitMax;

    m_BreakForce = other.m_BreakForce;

    m_pJoint = other.m_pJoint;
    m_pBody = other.m_pBody;
    m_pSecondBody = other.m_pSecondBody;

    return *this;
}

void Component3DJointHinge::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointHinge, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointHinge, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointHinge, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointHinge, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointHinge, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointHinge, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointHinge, OnFileRenamed );
    }
}

void Component3DJointHinge::UnregisterCallbacks()
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

void Component3DJointHinge::TickCallback(double TimePassed)
{
    if( m_BreakForce <= 0 )
        return;

    //if( m_pJoint )
    //{
    //    b2Vec2 reactionforce = m_pJoint->GetReactionForce( 60.0f );
    //    float magforce2 = reactionforce.LengthSquared();

    //    if( magforce2 > m_BreakForce*m_BreakForce )
    //    {
    //        Box3DWorld* pBox3DWorld = m_pGameObject->Get3DCollisionObject()->m_pBox3DWorld;
    //        pBox3DWorld->m_pWorld->DestroyJoint( m_pJoint );
    //        m_pJoint = 0;
    //    }
    //}
}

void Component3DJointHinge::OnPlay()
{
    ComponentBase::OnPlay();

    m_pBody = m_pGameObject->Get3DCollisionObject()->GetBody();
    if( m_pSecondCollisionObject )
        m_pSecondBody = m_pSecondCollisionObject->GetBody();

    if( m_pBody )
    {
    //    b2RevoluteJointDef jointdef;

    //    if( m_pSecondBody )
    //    {
    //        //Vector3 posA = m_pGameObject->GetTransform()->GetWorldPosition();
    //        //Vector3 posB = m_pSecondCollisionObject->m_pGameObject->GetTransform()->GetWorldPosition();
    //        //b2Vec2 anchorpos( posB.x - posA.x + m_AnchorA.x, posB.y - posA.y + m_AnchorA.y );
    //        //b2Vec2 anchorpos( (posB.x - m_AnchorB.x) - posA.x, (posB.y - m_AnchorB.y) - posA.y );

    //        jointdef.bodyA = m_pBody;
    //        jointdef.bodyB = m_pSecondBody;
    //        jointdef.collideConnected = false;
    //        //jointdef.localAnchorA = anchorpos;//m_AnchorA.x, m_AnchorA.y );
    //        jointdef.localAnchorA.Set( m_AnchorA.x, m_AnchorA.y );
    //        jointdef.localAnchorB.Set( m_AnchorB.x, m_AnchorB.y );
    //    }
    //    else
    //    {
    //        Vector3 pos = m_pGameObject->GetTransform()->GetWorldPosition();
    //        b2Vec2 anchorpos( pos.x + m_AnchorA.x, pos.y + m_AnchorA.y );
    //        //b2Vec2 anchorpos( pos.x + m_AnchorB.x, pos.y + m_AnchorB.y );

    //        jointdef.Initialize( m_pBody, pBox3DWorld->m_pGround, anchorpos );
    //    }

    //    if( m_MotorEnabled )
    //    {
    //        jointdef.enableMotor = true;
    //        jointdef.motorSpeed = m_MotorSpeed;
    //        jointdef.maxMotorTorque = m_MotorMaxTorque;
    //    }

    //    if( m_AngleLimitEnabled )
    //    {
    //        jointdef.enableLimit = true;
    //        jointdef.lowerAngle = m_AngleLimitMin * PI/180;
    //        jointdef.upperAngle = m_AngleLimitMax * PI/180;
    //    }

    //    m_pJoint = (b2RevoluteJoint*)pBox3DWorld->m_pWorld->CreateJoint( &jointdef );

        if( m_pSecondBody )
        {
            btVector3 pivotInA( m_PivotA.x, m_PivotA.y, m_PivotA.z );
            btVector3 pivotInB( m_PivotB.x, m_PivotB.y, m_PivotB.z );
            btVector3 axisInA( 0, 1, 0 );
            btVector3 axisInB( 0, 1, 0 );
            m_pJoint = new btHingeConstraint( *m_pBody, *m_pSecondBody, pivotInA, pivotInB, axisInA, axisInB );
            g_pBulletWorld->m_pDynamicsWorld->addConstraint( m_pJoint, true );
        }
        else
        {
            btVector3 pivotInA( m_PivotA.x, m_PivotA.y, m_PivotA.z );
            btVector3 axisInA( 0, 1, 0 );
            m_pJoint = new btHingeConstraint( *m_pBody, pivotInA, axisInA );
            g_pBulletWorld->m_pDynamicsWorld->addConstraint( m_pJoint, true );
        }

        if( m_pJoint )
        {
            if( m_MotorEnabled )
                m_pJoint->enableAngularMotor( true, m_MotorSpeed, m_MotorMaxTorque );
            else
                m_pJoint->enableAngularMotor( false, 0, 0 );
        }
    }
}

void Component3DJointHinge::OnStop()
{
    ComponentBase::OnStop();

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}

void Component3DJointHinge::RemoveJointFromWorld()
{
    if( m_pJoint )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeConstraint( m_pJoint );
    }
}
