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

ComponentSystemManager* g_pComponentSystemManager = 0;

ComponentSystemManager::ComponentSystemManager()
{
    g_pComponentSystemManager = this;
}

ComponentSystemManager::~ComponentSystemManager()
{
    while( m_GameObjects.GetHead() )
        delete m_GameObjects.RemHead();

    while( m_ComponentsData.GetHead() )
        delete m_ComponentsData.RemHead();

    while( m_ComponentsInputHandlers.GetHead() )
        delete m_ComponentsInputHandlers.RemHead();    
    
    while( m_ComponentsUpdateable.GetHead() )
        delete m_ComponentsUpdateable.RemHead();

    while( m_ComponentsRenderable.GetHead() )
        delete m_ComponentsRenderable.RemHead();
    
    g_pComponentSystemManager = 0;
}

GameObject* ComponentSystemManager::CreateGameObject()
{
    GameObject* pGameObject = MyNew GameObject;

    m_GameObjects.AddTail( pGameObject );

    return pGameObject;
}

ComponentBase* ComponentSystemManager::AddComponent(ComponentBase* pComponent)
{
    switch( pComponent->m_Type )
    {
    case ComponentType_Data:
        m_ComponentsData.AddTail( pComponent );
        break;

    case ComponentType_InputHandler:
        m_ComponentsInputHandlers.AddTail( pComponent );
        break;

    case ComponentType_Updateable:
        m_ComponentsUpdateable.AddTail( pComponent );
        break;

    case ComponentType_Renderable:
        m_ComponentsRenderable.AddTail( pComponent );
        break;

    case ComponentType_None:
        assert( false ); // shouldn't happen.
        break;
    }

    return pComponent;
}

void ComponentSystemManager::Tick(double TimePassed)
{
}

void ComponentSystemManager::OnDrawFrame()
{
    for( CPPListNode* node = m_ComponentsRenderable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentBase* pComponent = (ComponentBase*)node;

        if( pComponent->m_Type == ComponentType_Renderable )
        {
            ((ComponentRenderable*)pComponent)->Draw();
        }
    }
}

bool ComponentSystemManager::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    for( CPPListNode* node = m_ComponentsInputHandlers.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentBase* pComponent = (ComponentBase*)node;

        if( pComponent->m_Type == ComponentType_InputHandler )
        {
            ((ComponentInputHandler*)pComponent)->OnTouch( action, id, x, y, pressure, size );
        }
    }

    return false;
}

bool ComponentSystemManager::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    for( CPPListNode* node = m_ComponentsInputHandlers.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentBase* pComponent = (ComponentBase*)node;

        if( pComponent->m_Type == ComponentType_InputHandler )
        {
            ((ComponentInputHandler*)pComponent)->OnButtons( action, id );
        }
    }

    return false;
}
