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
    m_ID = 0;
    m_Name = 0;

    m_pComponentTransform = MyNew ComponentTransform( this );
    m_pComponentTransform->Reset();

    m_Components.AllocateObjects( 4 ); // hard coded nonsense for now, max of 4 components on a game object.

#if MYFW_USING_WX
    // Add this game object to the root of the objects tree
    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    wxTreeItemId gameobjectid = g_pPanelObjectList->AddObject( this, GameObject::StaticOnLeftClick, GameObject::StaticOnRightClick, rootid, m_Name );
    g_pPanelObjectList->SetDragAndDropFunctions( this, GameObject::StaticOnDrag, GameObject::StaticOnDrop );
    m_pComponentTransform->AddToObjectsPanel( gameobjectid );
#endif //MYFW_USING_WX
}

GameObject::~GameObject()
{
#if MYFW_USING_WX
    if( g_pPanelObjectList )
    {
        g_pPanelObjectList->RemoveObject( m_pComponentTransform );
        g_pPanelObjectList->RemoveObject( this );
    }

    SAFE_DELETE( m_pComponentTransform );
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void GameObject::OnLeftClick()
{
    g_pPanelWatch->ClearAllVariables();
}

void GameObject::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    wxMenu* categorymenu = 0;
    char* lastcategory = 0;

    for( unsigned int i=0; i<g_pComponentTypeManager->GetNumberOfComponentTypes(); i++ )
    {
        if( lastcategory != g_pComponentTypeManager->GetTypeCategory( i ) )
        {
            categorymenu = MyNew wxMenu;
            menu.AppendSubMenu( categorymenu, g_pComponentTypeManager->GetTypeCategory( i ) );
        }

        categorymenu->Append( i, g_pComponentTypeManager->GetTypeName( i ) );
    }
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&GameObject::OnPopupClick );

    // blocking call. // should delete all categorymenu's new'd above when done.
 	g_pPanelWatch->PopupMenu( &menu );
}

void GameObject::OnPopupClick(wxEvent &evt)
{
    GameObject* pGameObject = (GameObject*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    if( pGameObject->m_Components.Count() >= pGameObject->m_Components.Length() )
        return;

    int id = evt.GetId();
    ComponentTypes type = (ComponentTypes)id;

    pGameObject->AddNewComponent( type );
}

void GameObject::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_GameObjectPointer;
    g_DragAndDropStruct.m_Value = this;
}

void GameObject::OnDrop()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)g_DragAndDropStruct.m_Value;

        // for testing, if we drag a game object onto another one, copy the transform component values
        //*this->m_pComponentTransform = *pGameObject->m_pComponentTransform;

        // parent one transform to another.
        this->m_pComponentTransform->SetParent( pGameObject->m_pComponentTransform );
    }
}
#endif //MYFW_USING_WX

cJSON* GameObject::ExportAsJSONObject()
{
    cJSON* gameobject = cJSON_CreateObject();

    cJSON_AddNumberToObject( gameobject, "ID", m_ID );
    cJSON_AddStringToObject( gameobject, "Name", m_Name );

    return gameobject;
}

void GameObject::ImportFromJSONObject()
{
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

ComponentBase* GameObject::AddNewComponent(int componenttype, ComponentSystemManager* pComponentSystemManager)
{
    if( m_Components.Count() >= m_Components.Length() )
        return 0;

    ComponentBase* pComponent = g_pComponentTypeManager->CreateComponent( componenttype );

    assert( pComponentSystemManager );
    pComponentSystemManager->AddComponent( pComponent );
    pComponent->m_ID = pComponentSystemManager->m_NextComponentID;
    pComponentSystemManager->m_NextComponentID++;

    AddExistingComponent( pComponent );

    return pComponent;
}

ComponentBase* GameObject::AddExistingComponent(ComponentBase* pComponent)
{
    if( m_Components.Count() >= m_Components.Length() )
        return 0;

    pComponent->m_pGameObject = this;
    pComponent->Reset();

    m_Components.Add( pComponent );

#if MYFW_USING_WX
    wxTreeItemId gameobjectid = g_pPanelObjectList->FindObject( this );
    pComponent->AddToObjectsPanel( gameobjectid );
#endif //MYFW_USING_WX

    return pComponent;
}

ComponentBase* GameObject::RemoveComponent(ComponentBase* pComponent)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i] == pComponent )
        {
            m_Components.RemoveIndex_MaintainOrder( i );
            return pComponent;
        }
    }

    return 0; // component not found.
}
