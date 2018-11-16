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
bool Component3DJointSlider::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( Component3DJointSlider ); //_VARIABLE_LIST

Component3DJointSlider::Component3DJointSlider()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pSecondCollisionObject = 0;
    
    m_AxisA.Set( 0, 0, 0 );
    m_AxisB.Set( 0, 0, 0 );

    m_MotorEnabled = false;
    m_MotorSpeed = 0;
    m_MotorMaxForce = 0;

    m_TranslationLimitEnabled = false;
    m_TranslationLimitMin = 0;
    m_TranslationLimitMax = 0;

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}

Component3DJointSlider::~Component3DJointSlider()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void Component3DJointSlider::RegisterVariables(CPPListHead* pList, Component3DJointSlider* pThis) //_VARIABLE_LIST
{
    AddVar( pList, "SecondCollisionObject", ComponentVariableType_ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, (CVarFunc_DropTarget)&Component3DJointSlider::OnDrop, 0 );

    AddVar( pList, "AxisA", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_AxisA ), true, true, 0, (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );
    AddVar( pList, "AxisB", ComponentVariableType_Vector3, MyOffsetOf( pThis, &pThis->m_AxisB ), true, true, 0, (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );

    AddVar( pList, "MotorEnabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_MotorEnabled ), true, true, "Motor Enabled", (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorSpeed", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_MotorSpeed ), true, true, "Motor Speed", (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorMaxForce", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_MotorMaxForce ), true, true, "Motor Max Force", (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );

    AddVar( pList, "LimitEnabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_TranslationLimitEnabled ), true, true, "Limit Enabled", (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMin", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_TranslationLimitMin ), true, true, "Min Translation", (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMax", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_TranslationLimitMax ), true, true, "Max Translation", (CVarFunc_ValueChanged)&Component3DJointSlider::OnValueChanged, 0, 0 );
}

void Component3DJointSlider::Reset()
{
    ComponentBase::Reset();

    m_pSecondCollisionObject = 0;

    m_AxisA.Set( 0, 0, 0 );
    m_AxisB.Set( 0, 0, 0 );

    m_MotorEnabled = false;
    m_MotorSpeed = 0;
    m_MotorMaxForce = 0;

    m_TranslationLimitEnabled = false;
    m_TranslationLimitMin = 0;
    m_TranslationLimitMax = 0;

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void Component3DJointSlider::LuaRegister(lua_State* luastate)
{
    //luabridge::getGlobalNamespace( luastate )
    //    .beginClass<Component3DJointSlider>( "Component3DJointSlider" )
    //        .addData( "density", &Component3DJointSlider::m_Density )
    //        .addFunction( "GetMass", &Component3DJointSlider::GetMass )            
    //    .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
#if MYFW_USING_WX
void Component3DJointSlider::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, Component3DJointSlider::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "3DJointSlider", ObjectListIcon_Component );
}

void Component3DJointSlider::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void Component3DJointSlider::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "3D Slider Joint", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}
#endif //MYFW_USING_WX

void* Component3DJointSlider::OnDrop(ComponentVariable* pVar, int x, int y)
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
            oldPointer = m_pSecondCollisionObject->GetGameObject();

        GameObject* pGameObject = (GameObject*)pDropItem->m_Value;

        m_pSecondCollisionObject = pGameObject->Get3DCollisionObject();
    }

    return oldPointer;
}

void* Component3DJointSlider::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
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

    // sanity check on Translation limits.
    if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitMin ) )
    {
        if( m_TranslationLimitMin > m_TranslationLimitMax )
            m_TranslationLimitMin = m_TranslationLimitMax;
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitMax ) )
    {
        if( m_TranslationLimitMax < m_TranslationLimitMin )
            m_TranslationLimitMax = m_TranslationLimitMin;
    }

    // the joint will only exist if game is running.
    if( m_pJoint )
    {
        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorEnabled ) )
        {
            if( fequal( m_MotorSpeed, 0 ) == false )
            {
                //m_pJoint->EnableMotor( true );
                //m_pJoint->SetMotorSpeed( m_MotorSpeed );
                //m_pJoint->SetMaxMotorForce( m_MotorMaxForce );
            }
            else
            {
                //m_pJoint->EnableMotor( false );
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorSpeed ) )
        {
            //m_pJoint->SetMotorSpeed( m_MotorSpeed );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorMaxForce ) )
        {
            //m_pJoint->SetMaxMotorForce( m_MotorMaxForce );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitEnabled ) )
        {
            if( fequal( m_MotorSpeed, 0 ) == false )
            {
                //m_pJoint->EnableLimit( true );
                //m_pJoint->SetLimits( m_TranslationLimitMin, m_TranslationLimitMax );
            }
            else
            {
                //m_pJoint->EnableMotor( false );
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitMin ) )
        {
            //m_pJoint->SetLimits( m_TranslationLimitMin, m_TranslationLimitMax );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitMax ) )
        {
            //m_pJoint->SetLimits( m_TranslationLimitMin, m_TranslationLimitMax );
        }
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

Component3DJointSlider& Component3DJointSlider::operator=(const Component3DJointSlider& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pSecondCollisionObject = other.m_pSecondCollisionObject;

    m_AxisA = other.m_AxisA;
    m_AxisB = other.m_AxisB;

    m_MotorEnabled = other.m_MotorEnabled;
    m_MotorSpeed = other.m_MotorSpeed;
    m_MotorMaxForce = other.m_MotorMaxForce;

    m_TranslationLimitEnabled = other.m_TranslationLimitEnabled;
    m_TranslationLimitMin = other.m_TranslationLimitMin;
    m_TranslationLimitMax = other.m_TranslationLimitMax;

    m_pJoint = other.m_pJoint;
    m_pBody = other.m_pBody;
    m_pSecondBody = other.m_pSecondBody;

    return *this;
}

void Component3DJointSlider::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointSlider, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointSlider, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointSlider, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointSlider, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointSlider, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointSlider, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component3DJointSlider, OnFileRenamed );
    }
}

void Component3DJointSlider::UnregisterCallbacks()
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

void Component3DJointSlider::OnPlay()
{
    ComponentBase::OnPlay();

    m_pBody = m_pGameObject->Get3DCollisionObject()->GetBody();
    if( m_pSecondCollisionObject )
        m_pSecondBody = m_pSecondCollisionObject->GetBody();

    if( m_pBody )
    {
    //    b2SliderJointDef jointdef;

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

    //        MyMatrix transform = *m_pGameObject->GetTransform()->GetWorldTransform();
    //        Vector4 up = transform * Vector4( m_Up, 0, 0 ).GetNormalized();
    //        up.Normalize();
    //        jointdef.Initialize( m_pBody, pBox3DWorld->m_pGround, anchorpos, b2Vec2( up.x, up.y ) );
    //    }

    //    if( m_MotorEnabled )
    //    {
    //        jointdef.enableMotor = true;
    //        jointdef.motorSpeed = m_MotorSpeed;
    //        jointdef.maxMotorForce = m_MotorMaxForce;
    //    }

    //    if( m_TranslationLimitEnabled )
    //    {
    //        jointdef.enableLimit = true;
    //        jointdef.lowerTranslation = m_TranslationLimitMin;
    //        jointdef.upperTranslation = m_TranslationLimitMax;
    //    }

    //    m_pJoint = (b2SliderJoint*)pBox3DWorld->m_pWorld->CreateJoint( &jointdef );

        btTransform frameInA;

        // Convert axisA (direction vector) into rotation matrix
        {
            // If axis is (0,0,0), change direction to up
            Vector3 axisA = m_AxisA;
            if( axisA.LengthSquared() == 0 )
                axisA.y = 1;

            // Convert direction vector into yaw and pitch (no roll).
            float roty = atan2( axisA.z, axisA.x ) * -1;
            float rotz = atan2( axisA.y, axisA.x );

            //LOGInfo( LOGTag, "Axis: (%0.2f, %0.2f, %0.2f)\n", axisA.x, axisA.y, axisA.z );
            //LOGInfo( LOGTag, "Angle:(0, %0.2f, %0.2f)\n", roty, rotz );

            frameInA.setIdentity();
            frameInA.getBasis().setEulerZYX( 0, roty, rotz );
        }

        if( m_pSecondBody )
        {
            btTransform frameInB;

            // Convert axisB (direction vector) into rotation matrix
            {
                // If axis is (0,0,0), change direction to up
                Vector3 axisB = m_AxisB;
                if( axisB.LengthSquared() == 0 )
                    axisB.y = 1;

                // Convert direction vector into yaw and pitch (no roll).
                float roty = atan2( axisB.z, axisB.x ) * -1;
                float rotz = atan2( axisB.y, axisB.x );

                frameInB.setIdentity();
                frameInB.getBasis().setEulerZYX( 0, roty, rotz );
            }

            m_pJoint = new btSliderConstraint( *m_pBody, *m_pSecondBody, frameInA, frameInB, false );
            g_pBulletWorld->m_pDynamicsWorld->addConstraint( m_pJoint, true );
        }
        else
        {
            m_pJoint = new btSliderConstraint( *m_pBody, frameInA, false );
            g_pBulletWorld->m_pDynamicsWorld->addConstraint( m_pJoint, true );
        }

        if( m_pJoint )
        {
            // Angular motor not working.
            if( m_MotorEnabled )
            {
                m_pJoint->setPoweredAngMotor( true );
                m_pJoint->setMaxAngMotorForce( m_MotorMaxForce );
                m_pJoint->setTargetAngMotorVelocity( m_MotorSpeed );
            }
            else
            {
                m_pJoint->setPoweredAngMotor( false );
            }
        }
    }
}

void Component3DJointSlider::OnStop()
{
    ComponentBase::OnStop();

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}

void Component3DJointSlider::RemoveJointFromWorld()
{
    if( m_pJoint )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeConstraint( m_pJoint );
    }
}
