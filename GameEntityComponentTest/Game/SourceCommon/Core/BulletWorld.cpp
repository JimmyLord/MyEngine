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

#include "BulletWorld.h"

BulletWorld* g_pBulletWorld = 0;

BulletWorld::BulletWorld()
{
    g_pBulletWorld = this;

    CreateWorld();
}

BulletWorld::~BulletWorld()
{
    if( g_pBulletWorld == this )
        g_pBulletWorld = 0;

    Cleanup();
}

void BulletWorld::CreateWorld()
{
    // collision configuration contains default setup for memory, collision setup.
    //   Advanced users can create their own configuration.
    m_pCollisionConfiguration = MyNew btDefaultCollisionConfiguration();

    // use the default collision m_pDispatcher.
    //   For parallel processing you can use a diffent m_pDispatcher (see Extras/BulletMultiThreaded)
    m_pDispatcher = MyNew btCollisionDispatcher( m_pCollisionConfiguration );

    // btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    m_pOverlappingPairCache = MyNew btDbvtBroadphase();

    // the default constraint m_pSolver.
    //   For parallel processing you can use a different m_pSolver (see Extras/BulletMultiThreaded)
    m_pSolver = new btSequentialImpulseConstraintSolver();

    m_pDynamicsWorld = new btDiscreteDynamicsWorld( m_pDispatcher, m_pOverlappingPairCache, m_pSolver, m_pCollisionConfiguration );

    m_pDynamicsWorld->setGravity( btVector3(0,-10,0) );




    // create a few basic rigid bodies
    btCollisionShape* groundShape = new btBoxShape( btVector3(btScalar(50.),btScalar(50.),btScalar(50.)) );

    m_CollisionShapes.push_back(groundShape);

    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin( btVector3(0,-56,0) );

    {
        btScalar mass(0.);

        // rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia( 0,0,0 );
        if( isDynamic )
            groundShape->calculateLocalInertia( mass, localInertia );

        // using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState( groundTransform );
        btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, groundShape, localInertia );
        btRigidBody* body = new btRigidBody( rbInfo );

        // add the body to the dynamics world
        m_pDynamicsWorld->addRigidBody( body );
    }


    //{
    //    // create a dynamic rigidbody

    //    //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
    //    btCollisionShape* colShape = new btSphereShape( btScalar(1.) );
    //    m_CollisionShapes.push_back( colShape );

    //    // Create Dynamic Objects
    //    btTransform startTransform;
    //    startTransform.setIdentity();

    //    btScalar mass( 1.f );

    //    // rigidbody is dynamic if and only if mass is non zero, otherwise static
    //    bool isDynamic = (mass != 0.f);

    //    btVector3 localInertia( 0,0,0 );
    //    if( isDynamic )
    //        colShape->calculateLocalInertia( mass, localInertia );

    //    startTransform.setOrigin( btVector3(2,10,0) );

    //    // using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    //    btDefaultMotionState* myMotionState = new btDefaultMotionState( startTransform );
    //    btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, colShape, localInertia );
    //    btRigidBody* body = new btRigidBody( rbInfo );

    //    m_pDynamicsWorld->addRigidBody( body );
    //}
}

void BulletWorld::PhysicsStep()
{
    m_pDynamicsWorld->stepSimulation( 1.0f/60.0f, 10 );

    // print positions of all objects
    for( int j=m_pDynamicsWorld->getNumCollisionObjects()-1; j>=0; j-- )
    {
        btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast( obj );
        if( body && body->getMotionState() )
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform( trans );
            LOGInfo( LOGTag, "world pos = %f,%f,%f\n",
                float( trans.getOrigin().getX() ),
                float( trans.getOrigin().getY() ),
                float( trans.getOrigin().getZ() ) );
        }
    }
}

void BulletWorld::Cleanup()
{
    int i;

    // remove the rigidbodies from the dynamics world and delete them
    for( i=m_pDynamicsWorld->getNumCollisionObjects()-1; i>=0; i-- )
    {
        btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast( obj );
        if( body && body->getMotionState() )
        {
            delete body->getMotionState();
        }
        m_pDynamicsWorld->removeCollisionObject( obj );
        delete obj;
    }

    // delete collision shapes
    for( int j=0; j<m_CollisionShapes.size(); j++ )
    {
        btCollisionShape* shape = m_CollisionShapes[j];
        m_CollisionShapes[j] = 0;
        delete shape;
    }

    // delete dynamics world
    delete m_pDynamicsWorld;

    // delete m_pSolver
    delete m_pSolver;

    // delete broadphase
    delete m_pOverlappingPairCache;

    // delete m_pDispatcher
    delete m_pDispatcher;

    delete m_pCollisionConfiguration;

    // next line is optional: it will be cleared by the destructor when the array goes out of scope
    m_CollisionShapes.clear();
}
