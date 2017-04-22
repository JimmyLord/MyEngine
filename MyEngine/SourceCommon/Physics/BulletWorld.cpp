//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#include "BulletWorld.h"
#include "BulletDebugDraw.h"

BulletWorld* g_pBulletWorld = 0;

BulletWorld::BulletWorld(MaterialDefinition* debugdrawmaterial, MyMatrix* matviewproj)
{
    g_pBulletWorld = this;

    CreateWorld( debugdrawmaterial, matviewproj );
}

BulletWorld::~BulletWorld()
{
    if( g_pBulletWorld == this )
        g_pBulletWorld = 0;

    Cleanup();
}

void BulletWorld::CreateWorld(MaterialDefinition* debugdrawmaterial, MyMatrix* matviewproj)
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

    if( debugdrawmaterial != 0 )
    {
        m_pBulletDebugDraw = MyNew BulletDebugDraw( debugdrawmaterial, matviewproj );
        m_pDynamicsWorld->setDebugDrawer( m_pBulletDebugDraw );
    }

    m_pDynamicsWorld->setGravity( btVector3(0,-10,0) );
}

void BulletWorld::PhysicsUpdate(float deltatime)
{
    // Update physics based on how much time passed since last frame
    //    TODO: fix to use substeps properly, at the moment it's causing speed variations.
    m_pDynamicsWorld->stepSimulation( deltatime, 0 );
}

void BulletWorld::PhysicsStep()
{
    // TODO: look into this, it's not updating consistantly
    m_pDynamicsWorld->stepSimulation( 1.0f/60.0f, 10 );

    //// print positions of all objects
    //for( int j=m_pDynamicsWorld->getNumCollisionObjects()-1; j>=0; j-- )
    //{
    //    btCollisionObject* obj = m_pDynamicsWorld->getCollisionObjectArray()[j];
    //    btRigidBody* body = btRigidBody::upcast( obj );
    //    if( body && body->getMotionState() )
    //    {
    //        btTransform trans;
    //        body->getMotionState()->getWorldTransform( trans );
    //        LOGInfo( LOGTag, "world pos = %f,%f,%f\n",
    //            float( trans.getOrigin().getX() ),
    //            float( trans.getOrigin().getY() ),
    //            float( trans.getOrigin().getZ() ) );
    //    }
    //}
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

    delete m_pBulletDebugDraw;

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
