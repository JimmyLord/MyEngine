//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentCollisionObject::ComponentCollisionObject()
: ComponentUpdateable()
{
    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_CollisionObject;

    m_pBody = 0;
}

ComponentCollisionObject::~ComponentCollisionObject()
{
    if( m_pBody )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeRigidBody( m_pBody );
        delete m_pBody;
    }
}

void ComponentCollisionObject::Reset()
{
    ComponentUpdateable::Reset();

    m_Mass = 0;
}

#if MYFW_USING_WX
void ComponentCollisionObject::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentCollisionObject::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "Collision object" );
}

void ComponentCollisionObject::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->AddFloat( "Mass", &m_Mass, 0, 100 );
}
#endif //MYFW_USING_WX

cJSON* ComponentCollisionObject::ExportAsJSONObject()
{
    cJSON* component = ComponentUpdateable::ExportAsJSONObject();

    cJSON_AddNumberToObject( component, "Mass", m_Mass );

    return component;
}

void ComponentCollisionObject::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetFloat( jsonobj, "Mass", &m_Mass );
}

ComponentCollisionObject& ComponentCollisionObject::operator=(const ComponentCollisionObject& other)
{
    assert( &other != this );

    ComponentUpdateable::operator=( other );

    m_Mass = other.m_Mass;

    return *this;
}

void ComponentCollisionObject::OnPlay()
{
    ComponentUpdateable::OnPlay();

    //// set the collision object scale on play, guess this should be set whenever the object is scaled.
    ////   TODO: find a better way to handle the object being scaled in editor.
    //Vector3 localscale = m_pComponentTransform->GetScale();
    //btVector3 scale( localscale.x, localscale.y, localscale.z );
    //m_pBody->getCollisionShape()->setLocalScaling( scale );

    // create a rigidbody on start
    if( m_pBody == 0 )
    {
        btCollisionShape* colShape = new btBoxShape( btVector3(1,1,1) ); // half-extents
        //btCollisionShape* colShape = new btSphereShape( btScalar(1.0f) );
        g_pBulletWorld->m_CollisionShapes.push_back( colShape );

        // Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        //btScalar mass( 1.0f );

        // rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (m_Mass != 0.0f);

        btVector3 localInertia( 0,0,0 );
        if( isDynamic )
            colShape->calculateLocalInertia( m_Mass, localInertia );

        //btVector3 pos(m_pComponentTransform->m_Position.x, m_pComponentTransform->m_Position.y, m_pComponentTransform->m_Position.z );
        //startTransform.setOrigin( pos );
        Vector3 localscale = m_pComponentTransform->GetLocalScale();
        btVector3 scale( localscale.x, localscale.y, localscale.z );
        colShape->setLocalScaling( scale );

        MyMatrix localmat = m_pComponentTransform->GetLocalRotPosMatrix(); //GetLocalTransform();
        startTransform.setFromOpenGLMatrix( &localmat.m11 );

        // using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState( startTransform );
        btRigidBody::btRigidBodyConstructionInfo rbInfo( m_Mass, myMotionState, colShape, localInertia );
        m_pBody = new btRigidBody( rbInfo );
        
        g_pBulletWorld->m_pDynamicsWorld->addRigidBody( m_pBody );
    }
}

void ComponentCollisionObject::OnStop()
{
    ComponentUpdateable::OnStop();

    // shouldn't get hit, all objects are deleted/recreated when gameplay is stopped.
    if( m_pBody )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeRigidBody( m_pBody );
        SAFE_DELETE( m_pBody );
    }
}

void ComponentCollisionObject::Tick(double TimePassed)
{
    //ComponentUpdateable::Tick( TimePassed );

    if( TimePassed == 0 )
    {
        SyncRigidBodyToTransform();
        return;
    }

    MyMatrix* matLocal = m_pComponentTransform->GetLocalTransform();

    btTransform transform;
    m_pBody->getMotionState()->getWorldTransform( transform );
    MyMatrix matRotPos;
    transform.getOpenGLMatrix( &matLocal->m11 );

    // if the collisionshape is scaled, scale our object to match.
    btVector3 scale = m_pBody->getCollisionShape()->getLocalScaling();
    if( scale.x() != 1 || scale.y() != 1 || scale.z() != 1 )
    {
        MyMatrix matScale;
        matScale.CreateScale( scale.x(), scale.y(), scale.z() );
        *matLocal = *matLocal * matScale;
    }

    //btVector3 pos = transform.getOrigin();
    //btQuaternion rot = transform.getRotation();
    //m_pComponentTransform->SetPosition( Vector3( pos.getX(), pos.getY(), pos.getZ() ) );
    //m_pComponentTransform->SetRotation( Vector3( rot.g, pos.getY(), pos.getZ() ) );
}

void ComponentCollisionObject::SyncRigidBodyToTransform()
{
    //m_pComponentTransform->SetPosition( Vector3(2,10,0) );

    btTransform transform;
    //btVector3 pos(m_pComponentTransform->m_Position.x, m_pComponentTransform->m_Position.y, m_pComponentTransform->m_Position.z );
    //transform.setIdentity();
    //transform.setOrigin( pos );
    MyMatrix localmat = m_pComponentTransform->GetLocalRotPosMatrix(); //GetLocalTransform();
    transform.setFromOpenGLMatrix( &localmat.m11 );

    if( m_pBody )
    {
        m_pBody->getMotionState()->setWorldTransform( transform );
        m_pBody->setWorldTransform( transform );

        m_pBody->activate( true );

        g_pBulletWorld->m_pDynamicsWorld->removeRigidBody( m_pBody );
        g_pBulletWorld->m_pDynamicsWorld->addRigidBody( m_pBody );
    }
}
