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
bool Component2DJointPrismatic::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( Component2DJointPrismatic ); //_VARIABLE_LIST

Component2DJointPrismatic::Component2DJointPrismatic()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pSecondCollisionObject = 0;
    
    m_Up.Set( 0, 1 );
    m_AnchorA.Set( 0, 0 );
    m_AnchorB.Set( 0, 0 );

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

Component2DJointPrismatic::~Component2DJointPrismatic()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void Component2DJointPrismatic::RegisterVariables(CPPListHead* pList, Component2DJointPrismatic* pThis) //_VARIABLE_LIST
{
    AddVar( pList, "SecondCollisionObject", ComponentVariableType_ComponentPtr,
        MyOffsetOf( pThis, &pThis->m_pSecondCollisionObject ), true, true, 0,
        (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, (CVarFunc_DropTarget)&Component2DJointPrismatic::OnDrop, 0 );

    AddVar( pList, "Up", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Up ), true, true, 0, (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );

    AddVar( pList, "AnchorA", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_AnchorA ), true, true, 0, (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );
    AddVar( pList, "AnchorB", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_AnchorB ), true, true, 0, (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );

    AddVar( pList, "MotorEnabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_MotorEnabled ), true, true, "Motor Enabled", (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorSpeed", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_MotorSpeed ), true, true, "Motor Speed", (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );
    AddVar( pList, "MotorMaxForce", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_MotorMaxForce ), true, true, "Motor Max Force", (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );

    AddVar( pList, "LimitEnabled", ComponentVariableType_Bool, MyOffsetOf( pThis, &pThis->m_TranslationLimitEnabled ), true, true, "Limit Enabled", (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMin", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_TranslationLimitMin ), true, true, "Min Translation", (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );
    AddVar( pList, "LimitMax", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_TranslationLimitMax ), true, true, "Max Translation", (CVarFunc_ValueChanged)&Component2DJointPrismatic::OnValueChanged, 0, 0 );
}

void Component2DJointPrismatic::Reset()
{
    ComponentBase::Reset();

    m_pSecondCollisionObject = 0;

    m_Up.Set( 0, 1 );

    m_AnchorA.Set( 0, 0 );
    m_AnchorB.Set( 0, 0 );

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
void Component2DJointPrismatic::LuaRegister(lua_State* luastate)
{
    //luabridge::getGlobalNamespace( luastate )
    //    .beginClass<Component2DJointPrismatic>( "Component2DJointPrismatic" )
    //        .addData( "density", &Component2DJointPrismatic::m_Density )
    //        .addFunction( "GetMass", &Component2DJointPrismatic::GetMass )            
    //    .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
#if MYFW_USING_WX
void Component2DJointPrismatic::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, Component2DJointPrismatic::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "2DJointPrismatic", ObjectListIcon_Component );
}

void Component2DJointPrismatic::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void Component2DJointPrismatic::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "2D Prismatic Joint", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}
#endif //MYFW_USING_WX

void* Component2DJointPrismatic::OnDrop(ComponentVariable* pVar, int x, int y)
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

void* Component2DJointPrismatic::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
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
                m_pJoint->EnableMotor( true );
                m_pJoint->SetMotorSpeed( m_MotorSpeed );
                m_pJoint->SetMaxMotorForce( m_MotorMaxForce );
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

        if( pVar->m_Offset == MyOffsetOf( this, &m_MotorMaxForce ) )
        {
            m_pJoint->SetMaxMotorForce( m_MotorMaxForce );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitEnabled ) )
        {
            if( fequal( m_MotorSpeed, 0 ) == false )
            {
                m_pJoint->EnableLimit( true );
                m_pJoint->SetLimits( m_TranslationLimitMin, m_TranslationLimitMax );
            }
            else
            {
                m_pJoint->EnableMotor( false );
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitMin ) )
        {
            m_pJoint->SetLimits( m_TranslationLimitMin, m_TranslationLimitMax );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_TranslationLimitMax ) )
        {
            m_pJoint->SetLimits( m_TranslationLimitMin, m_TranslationLimitMax );
        }
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

Component2DJointPrismatic& Component2DJointPrismatic::operator=(const Component2DJointPrismatic& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_pSecondCollisionObject = other.m_pSecondCollisionObject;

    m_Up = other.m_Up;

    m_AnchorA = other.m_AnchorA;
    m_AnchorB = other.m_AnchorB;

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

void Component2DJointPrismatic::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointPrismatic, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointPrismatic, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointPrismatic, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointPrismatic, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointPrismatic, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointPrismatic, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DJointPrismatic, OnFileRenamed );
    }
}

void Component2DJointPrismatic::UnregisterCallbacks()
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

void Component2DJointPrismatic::OnPlay()
{
    ComponentBase::OnPlay();

    m_pBody = m_pGameObject->Get2DCollisionObject()->m_pBody;
    if( m_pSecondCollisionObject )
        m_pSecondBody = m_pSecondCollisionObject->m_pBody;

    Box2DWorld* pBox2DWorld = m_pGameObject->Get2DCollisionObject()->m_pBox2DWorld;

    if( m_pBody )
    {
        b2PrismaticJointDef jointdef;

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

            MyMatrix transform = *m_pGameObject->GetTransform()->GetWorldTransform();
            Vector4 up = transform * Vector4( m_Up, 0, 0 ).GetNormalized();
            up.Normalize();
            jointdef.Initialize( m_pBody, pBox2DWorld->m_pGround, anchorpos, b2Vec2( up.x, up.y ) );
        }

        if( m_MotorEnabled )
        {
            jointdef.enableMotor = true;
            jointdef.motorSpeed = m_MotorSpeed;
            jointdef.maxMotorForce = m_MotorMaxForce;
        }

        if( m_TranslationLimitEnabled )
        {
            jointdef.enableLimit = true;
            jointdef.lowerTranslation = m_TranslationLimitMin;
            jointdef.upperTranslation = m_TranslationLimitMax;
        }

        m_pJoint = (b2PrismaticJoint*)pBox2DWorld->m_pWorld->CreateJoint( &jointdef );
    }
}

void Component2DJointPrismatic::OnStop()
{
    ComponentBase::OnStop();

    m_pJoint = 0;
    m_pBody = 0;
    m_pSecondBody = 0;
}
