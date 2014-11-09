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

GameObject::GameObject()
{
    m_Name = 0;

    m_pComponentTransform = MyNew ComponentTransform( this );
    m_pComponentTransform->Reset();

    m_Components.AllocateObjects( 4 ); // hard coded nonsense for now, max of 4 components on a game object.

#if MYFW_USING_WX
    // Add this game object to the root of the objects tree
    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    wxTreeItemId gameobjectid = g_pPanelObjectList->AddObject( this, GameObject::StaticFillPropertiesWindow, rootid, m_Name );
    m_pComponentTransform->AddToObjectsPanel( gameobjectid );
#endif //MYFW_USING_WX
}

GameObject::~GameObject()
{
    SAFE_DELETE( m_pComponentTransform );

#if MYFW_USING_WX
    if( g_pPanelObjectList )
    {
        g_pPanelObjectList->RemoveObject( this );
    }
#endif //MYFW_USING_WX
}

void GameObject::SetName(char* name)
{
    m_Name = name;

#if MYFW_USING_WX
    if( g_pPanelObjectList )
    {
        g_pPanelObjectList->RenameObject( this, m_Name );
    }
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void GameObject::FillPropertiesWindow()
{
    g_pPanelWatch->ClearAllVariables();
}
#endif //MYFW_USING_WX

ComponentBase* GameObject::AddNewComponent(ComponentBase* pComponent, ComponentSystemManager* pComponentSystemManager)
{
    assert( pComponentSystemManager );
    pComponentSystemManager->AddComponent( pComponent );

    AddExistingComponent( pComponent );

    return pComponent;
}

ComponentBase* GameObject::AddExistingComponent(ComponentBase* pComponent)
{
    pComponent->m_pGameObject = this;
    pComponent->Reset();

    m_Components.Add( pComponent );

#if MYFW_USING_WX
    wxTreeItemId gameobjectid = g_pPanelObjectList->FindObject( this );

    wxTreeItemId id;
    id = g_pPanelObjectList->AddObject( this, GameObject::StaticFillPropertiesWindow, gameobjectid, "component" );
#endif //MYFW_USING_WX

    return pComponent;
}
