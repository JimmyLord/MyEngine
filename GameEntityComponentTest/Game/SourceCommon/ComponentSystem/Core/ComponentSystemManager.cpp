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

ComponentSystemManager::ComponentSystemManager(ComponentTypeManager* typemanager)
{
    g_pComponentSystemManager = this;

    m_pComponentTypeManager = typemanager;

#if MYFW_USING_WX
    // Add click callbacks to the root of the objects tree
    g_pPanelObjectList->SetTreeRootData( this, ComponentSystemManager::StaticOnLeftClick, ComponentSystemManager::StaticOnRightClick );
#endif //MYFW_USING_WX
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

    SAFE_DELETE( m_pComponentTypeManager );
    
    g_pComponentSystemManager = 0;
}

#if MYFW_USING_WX
void ComponentSystemManager::OnLeftClick()
{
    g_pPanelWatch->ClearAllVariables();
}

void ComponentSystemManager::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    menu.Append( 0, "Add Game Object" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentSystemManager::OnPopupClick );

    // blocking call. // should delete all categorymenu's new'd above when done.
 	g_pPanelWatch->PopupMenu( &menu );
}

void ComponentSystemManager::OnPopupClick(wxEvent &evt)
{
    int id = evt.GetId();
    if( id == 0 )
    {
        g_pComponentSystemManager->CreateGameObject();
    }
}
#endif //MYFW_USING_WX

GameObject* ComponentSystemManager::CreateGameObject()
{
    GameObject* pGameObject = MyNew GameObject;

    m_GameObjects.AddTail( pGameObject );

    return pGameObject;
}

void ComponentSystemManager::DeleteGameObject(GameObject* pObject)
{
    assert( false ); // not done yet.
}

ComponentBase* ComponentSystemManager::AddComponent(ComponentBase* pComponent)
{
    switch( pComponent->m_BaseType )
    {
    case BaseComponentType_Data:
        m_ComponentsData.AddTail( pComponent );
        break;

    case BaseComponentType_InputHandler:
        m_ComponentsInputHandlers.AddTail( pComponent );
        break;

    case BaseComponentType_Updateable:
        m_ComponentsUpdateable.AddTail( pComponent );
        break;

    case BaseComponentType_Renderable:
        m_ComponentsRenderable.AddTail( pComponent );
        break;

    case BaseComponentType_None:
        assert( false ); // shouldn't happen.
        break;
    }

    return pComponent;
}

void ComponentSystemManager::DeleteComponent(ComponentBase* pComponent)
{
    if( pComponent->m_pGameObject )
    {
        pComponent = pComponent->m_pGameObject->RemoveComponent( pComponent );
        if( pComponent == 0 )
            return;
    }

#if MYFW_USING_WX
    g_pPanelObjectList->RemoveObject( pComponent );
#endif
    pComponent->Remove();
    SAFE_DELETE( pComponent );
}

void ComponentSystemManager::Tick(double TimePassed)
{
    for( CPPListNode* node = m_ComponentsUpdateable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentUpdateable* pComponent = (ComponentUpdateable*)node;

        if( pComponent->m_BaseType == BaseComponentType_Updateable )
        {
            pComponent->Tick( TimePassed );
        }
    }
}

void ComponentSystemManager::OnDrawFrame()
{
    for( CPPListNode* node = m_ComponentsRenderable.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentRenderable* pComponent = (ComponentRenderable*)node;

        if( pComponent->m_BaseType == BaseComponentType_Renderable )
        {
            pComponent->Draw();
        }
    }
}

bool ComponentSystemManager::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    for( CPPListNode* node = m_ComponentsInputHandlers.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)node;

        if( pComponent->m_BaseType == BaseComponentType_InputHandler )
        {
            if( pComponent->OnTouch( action, id, x, y, pressure, size ) == true )
                return true;
        }
    }

    return false;
}

bool ComponentSystemManager::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    for( CPPListNode* node = m_ComponentsInputHandlers.GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)node;

        if( pComponent->m_BaseType == BaseComponentType_InputHandler )
        {
            if( pComponent->OnButtons( action, id ) == true )
                return true;
        }
    }

    return false;
}
