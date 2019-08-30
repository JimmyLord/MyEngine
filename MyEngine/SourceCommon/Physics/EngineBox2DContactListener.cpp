//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EngineBox2DContactListener.h"
#include "ComponentSystem/EngineComponents/ComponentLuaScript.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"

EngineBox2DContactListener::EngineBox2DContactListener()
{
}

EngineBox2DContactListener::~EngineBox2DContactListener()
{
}

void EngineBox2DContactListener::BeginContact(b2Contact* contact)
{
    //Box2DContactListener::BeginContact( contact );
    
    b2Fixture* pFixture[2];
    b2Body* pBody[2];
    Component2DCollisionObject* pCollisionComponent[2];

    pFixture[0] = contact->GetFixtureA();
    pFixture[1] = contact->GetFixtureB();

    for( int i=0; i<2; i++ )
    {
        pBody[i] = pFixture[i]->GetBody();
        pCollisionComponent[i] = (Component2DCollisionObject*)pBody[i]->GetUserData();
    }

    for( int i=0; i<2; i++ )
    {
        MyAssert( pCollisionComponent[i] );

        if( pCollisionComponent[i] && pCollisionComponent[i]->m_pComponentLuaScript )
        {
            if( pCollisionComponent[i]->m_pComponentLuaScript )
            {
                b2Manifold* pManifold = contact->GetManifold();

                Component2DCollisionObject* otherComponent = pCollisionComponent[!i];
                GameObject* otherGameObject = otherComponent->GetGameObject();

                if( pManifold->pointCount > 0 )
                {
                    b2Vec2 b2normal = pManifold->localNormal;
                    Vector2 normal( b2normal.x, b2normal.y );
                    if( i == 0 )
                        normal *= -1;

                    pCollisionComponent[i]->m_pComponentLuaScript->CallFunction( "OnCollision", normal, otherGameObject, otherComponent );
                }
                else
                {
                    Vector2 normal( 0, 0 );
                    if( pFixture[i]->IsSensor() )
                        normal = (Vector2&)pBody[!i]->GetLinearVelocity();

                    pCollisionComponent[i]->m_pComponentLuaScript->CallFunction( "OnCollision", normal, otherGameObject, otherComponent );
                }
            }
        }
    }
}

void EngineBox2DContactListener::EndContact(b2Contact* contact)
{
    //Box2DContactListener::EndContact( contact );
}
