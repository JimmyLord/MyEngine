//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "Component2DCollisionObject.h"
#include "Component2DJointRevolute.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( Component2DJointRevolute ); //_VARIABLE_LIST

Component2DJointRevolute::Component2DJointRevolute(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentBase( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pSecondCollisionObject = 0;
    
    m_AnchorA.Set( 0, 0 );
    m_AnchorB.Set( 0, 0 );

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

Component2DJointRevolute::~Component2DJointRevolute()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void Component2DJointRevolute::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, Component2DJointRevolute* pThis) //_VARIABLE_LIST
{
    AddVar( pList, "SecondCollisionObject", ComponentVariableType::ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, (CVarFunc_DropTarget)&Component2DJointRevolute::OnDrop, 0 );

    AddVar( pList, "AnchorA", ComponentVariableType::Vector2, MyOffsetOf( pThis, &pThis->m_AnchorA ), true, true, 0, (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );
    AddVar( pList, "AnchorB", ComponentVariableType::Vector2, MyOffsetOf( pThis, &pThis->m_AnchorB ), true, true, 0, (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );

    AddVar( pList, "MotorEnabled", ComponentVariableType::Bool, MyOffsetOf( pThis, &pThis->m_MotorEnabled ), true, true, "Motor Enabled", (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorSpeed", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_MotorSpeed ), true, true, "Motor Speed", (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorMaxTorque", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_MotorMaxTorque ), true, true, "Motor Max Torque", (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );

    AddVar( pList, "LimitEnabled", ComponentVariableType::Bool, MyOffsetOf( pThis, &pThis->m_AngleLimitEnabled ), true, true, "Angle Limit Enabled", (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMin", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_AngleLimitMin ), true, true, "Min Angle", (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMax", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_AngleLimitMax ), true, true, "Max Angle", (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );

    AddVar( pList, "BreakForce", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_BreakForce ), true, true, "Break Force", (CVarFunc_ValueChanged)&Component2DJointRevolute::OnValueChanged, 0, 0 );
}

void Component2DJointRevolute::Reset()
{
    ComponentBase::Reset();

    m_pSecondCollisionObject = 0;

    m_AnchorA.Set( 0, 0 );
    m_AnchorB.Set( 0, 0 );

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
void Component2DJointRevolute::LuaRegister(lua_State* luastate)
{
    //luabridge::getGlobalNamespace( luastate )
    //    .beginClass<Component2DJointRevolute>( "Component2DJointRevolute" )
    //        .addData( "density", &Component2DJointRevolute::m_Density )
    //        .addFunction( "GetMass", &Component2DJointRevolute::GetMass )            
    //    .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
#if MYFW_USING_WX
void Component2DJointRevolute::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, Component2DJointRevolute::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "2DJointRevolute", ObjectListIcon_Component );
}

void Component2DJointRevolute::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void Component2DJointRevolute::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "2D Revolute Joint", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}
#endif //MYFW_USING_WX

void* Component2DJointRevolute::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        ComponentBase* pComponent = (ComponentBase*)pDropItem->m_Value;

        if( pComponent->IsA( "2DCollisionObjectComponent" ) )
        {
            oldPointer = m_pSecondCollisionObject;

            m_pSecondCollisionObject = (Component2DCollisionObject*)pComponent;
        }        
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        if( m_pSecondCollisionObject )
            oldPointer = m_pSecondCollisionObject->GetGameObject();

        GameObject* pGameObject = (GameObject*)pDropItem->m_Value;

        m_pSecondCollisionObject = pGameObject->Get2DCollisionObject();
    }

    return oldPointer;
}

void* Component2DJointRevolute::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_pSecondCollisionObject ) )
    {
        if( changedByInterface )
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

    // Sanity check on angle limits.
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

    // The joint will only exist if game is running.
    if( m_pJoint )
    {
        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorEnabled ) )
        {
            if( m_MotorEnabled )
            {
                m_pJoint->EnableMotor( true );
                m_pJoint->SetMotorSpeed( m_MotorSpeed );
                m_pJoint->SetMaxMotorTorque( m_MotorMaxTorque );
            }
            else
            {
                m_pJoint->EnableMotor( false );
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorSpeed ) )
        {
            m_pJoint->SetMotorSpeed( m_MotorSpeed );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorMaxTorque ) )
        {
            m_pJoint->SetMaxMotorTorque( m_MotorMaxTorque );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitEnabled ) )
        {
            if( m_AngleLimitEnabled )
            {
                m_pJoint->EnableLimit( true );
                m_pJoint->SetLimits( m_AngleLimitMin * PI/180, m_AngleLimitMax * PI/180 );
            }
            else
            {
                m_pJoint->EnableLimit( false );
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitMin ) )
        {
            m_pJoint->SetLimits( m_AngleLimitMin * PI/180, m_AngleLimitMax * PI/180 );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_AngleLimitMax ) )
        {
            m_pJoint->SetLimits( m_AngleLimitMin * PI/180, m_AngleLimitMax * PI/180 );
        }
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

Component2DJointRevolute& Component2DJointRevolute::operator=(const Component2DJointRevolute& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pSecondCollisionObject = other.m_pSecondCollisionObject;

    m_AnchorA = other.m_AnchorA;
    m_AnchorB = other.m_AnchorB;

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

void Component2DJointRevolute::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointRevolute, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointRevolute, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointRevolute, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointRevolute, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointRevolute, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointRevolute, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointRevolute, OnFileRenamed );
    }
}

void Component2DJointRevolute::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

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

void Component2DJointRevolute::TickCallback(float deltaTime)
{
    if( m_BreakForce <= 0 )
        return;

    if( m_pJoint )
    {
        b2Vec2 reactionforce = m_pJoint->GetReactionForce( 60.0f );
        float magforce2 = reactionforce.LengthSquared();

        if( magforce2 > m_BreakForce*m_BreakForce )
        {
            Box2DWorld* pBox2DWorld = m_pGameObject->Get2DCollisionObject()->m_pBox2DWorld;
            pBox2DWorld->m_pWorld->DestroyJoint( m_pJoint );
            m_pJoint = 0;
        }
    }
}

void Component2DJointRevolute::OnPlay()
{
    ComponentBase::OnPlay();

    m_pBody = m_pGameObject->Get2DCollisionObject()->m_pBody;
    if( m_pSecondCollisionObject )
        m_pSecondBody = m_pSecondCollisionObject->m_pBody;

    Box2DWorld* pBox2DWorld = m_pGameObject->Get2DCollisionObject()->m_pBox2DWorld;

    if( m_pBody )
    {
        b2RevoluteJointDef jointdef;

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

        if( m_MotorEnabled )
        {
            jointdef.enableMotor = true;
            jointdef.motorSpeed = m_MotorSpeed;
            jointdef.maxMotorTorque = m_MotorMaxTorque;
        }

        if( m_AngleLimitEnabled )
        {
            jointdef.enableLimit = true;
            jointdef.lowerAngle = m_AngleLimitMin * PI/180;
            jointdef.upperAngle = m_AngleLimitMax * PI/180;
        }

        m_pJoint = (b2RevoluteJoint*)pBox2DWorld->m_pWorld->CreateJoint( &jointdef );
    }
}

void Component2DJointRevolute::OnStop()
{
    ComponentBase::OnStop();

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}
