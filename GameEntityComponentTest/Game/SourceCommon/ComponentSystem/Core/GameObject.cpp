//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

GameObject::GameObject(bool managed)
{
    m_SceneID = 0;
    m_ID = 0;
    m_Name = 0;

    m_pComponentTransform = MyNew ComponentTransform();
    m_pComponentTransform->m_pGameObject = this;
    m_pComponentTransform->Reset();

    m_Components.AllocateObjects( MAX_COMPONENTS ); // hard coded nonsense for now, max of 4 components on a game object.

    m_Managed = false;
    if( managed )
        SetManaged( true );
}

GameObject::~GameObject()
{
    if( m_Managed )
        SetManaged( false );

    // if it's in a list, remove it.
    if( this->Prev != 0 )
        Remove();

    SAFE_DELETE( m_pComponentTransform );

    SAFE_DELETE_ARRAY( m_Name );

    if( m_Managed == false )
    {
        while( m_Components.Count() )
        {
            ComponentBase* pComponent = m_Components.RemoveIndex( 0 );
            delete pComponent;
        }
    }
}

void GameObject::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<GameObject>( "GameObject" )
            .addData( "ComponentTransform", &GameObject::m_pComponentTransform )
            .addData( "name", &GameObject::m_Name )
            .addData( "id", &GameObject::m_ID )            
            .addFunction( "SetName", &GameObject::SetName )
            .addFunction( "GetTransform", &GameObject::GetTransform )
            .addFunction( "GetFirstComponentOfBaseType", &GameObject::GetFirstComponentOfBaseType )
            .addFunction( "GetCollisionObject", &GameObject::GetCollisionObject )
        .endClass();
}

#if MYFW_USING_WX
void GameObject::OnLeftClick(bool clear)
{
    // select this GameObject in the editor window.
    ((GameEntityComponentTest*)g_pGameCore)->m_EditorState.m_pSelectedObjects.push_back( this );

    // only show properties of the first selected object.
    if( ((GameEntityComponentTest*)g_pGameCore)->m_EditorState.m_pSelectedObjects.size() > 1 )
        return;

    if( clear )
        g_pPanelWatch->ClearAllVariables();

    m_pComponentTransform->FillPropertiesWindow( false );
    //g_pPanelWatch->AddSpace();
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        m_Components[i]->FillPropertiesWindow( false );
        //g_pPanelWatch->AddSpace();
    }
}

void GameObject::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    wxMenu* categorymenu = 0;
    char* lastcategory = 0;

    menu.Append( 1000, "Duplicate GameObject" ); // matches 1000 in OnPopupClick()

    for( unsigned int i=0; i<g_pComponentTypeManager->GetNumberOfComponentTypes(); i++ )
    {
        if( lastcategory != g_pComponentTypeManager->GetTypeCategory( i ) )
        {
            categorymenu = MyNew wxMenu;
            menu.AppendSubMenu( categorymenu, g_pComponentTypeManager->GetTypeCategory( i ) );
        }

        categorymenu->Append( i, g_pComponentTypeManager->GetTypeName( i ) );

        lastcategory = g_pComponentTypeManager->GetTypeCategory( i );
    }
    
    menu.Append( 1001, "Delete GameObject" ); // matches 1001 in OnPopupClick()

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&GameObject::OnPopupClick );
    
    // blocking call. // should delete all categorymenu's new'd above when done.
 	g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void GameObject::OnPopupClick(wxEvent &evt)
{
    GameObject* pGameObject = (GameObject*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    if( pGameObject->m_Components.Count() >= pGameObject->m_Components.Length() )
        return;

    unsigned int id = evt.GetId();

    if( id < g_pComponentTypeManager->GetNumberOfComponentTypes() )
    {
        ComponentTypes type = (ComponentTypes)id;

        if( ((GameEntityComponentTest*)g_pGameCore)->m_EditorMode )
            pGameObject->AddNewComponent( type, pGameObject->GetSceneID() );
        else
            pGameObject->AddNewComponent( type, 0 );
    }
    else if( id == 1000 ) // "Duplicate GameObject"
    {
        g_pComponentSystemManager->EditorCopyGameObject( pGameObject );
    }
    else if( id == 1001 ) // "Delete GameObject"
    {
        EditorState* pEditorState = &((GameEntityComponentTest*)g_pGameCore)->m_EditorState;

        // if the object isn't selected, delete just the one object, otherwise delete all selected objects.
        if( pEditorState->IsObjectSelected( pGameObject ) )
        {
            g_pGameMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteObjects( pEditorState->m_pSelectedObjects ) );
        }
        else
        {
            // create a temp vector to pass into command.
            std::vector<GameObject*> gameobjects;
            gameobjects.push_back( pGameObject );
            g_pGameMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
        }
    }
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

void GameObject::OnLabelEdit()
{
    wxString newname = g_pPanelObjectList->GetObjectName( this );

    size_t len = newname.length();
    if( len > 0 )
    {
        SetName( newname );
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

void GameObject::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    cJSONExt_GetUnsignedInt( jsonobj, "ID", &m_ID );

    cJSON* obj = cJSON_GetObjectItem( jsonobj, "Name" );
    if( obj )
    {
        SetName( obj->valuestring );
    }
    SetSceneID( sceneid );
}

void GameObject::SetSceneID(unsigned int sceneid)
{
    m_SceneID = sceneid;
}

void GameObject::SetID(unsigned int id)
{
    m_ID = id;
}

void GameObject::SetName(const char* name)
{
    assert( name );

    if( m_Name )
    {
        if( strcmp( m_Name, name ) == 0 ) // name hasn't changed.
            return;

        delete[] m_Name;
    }
    
    size_t len = strlen( name );
    
    m_Name = MyNew char[len+1];
    strcpy_s( m_Name, len+1, name );

#if MYFW_USING_WX
    if( g_pPanelObjectList )
    {
        g_pPanelObjectList->RenameObject( this, m_Name );
    }
#endif //MYFW_USING_WX
}

void GameObject::SetManaged(bool managed)
{
    if( m_Managed == false && managed == true )
    {
        m_Managed = true;
#if MYFW_USING_WX
        if( g_pPanelObjectList )
        {
            // Add this game object to the root of the objects tree
            wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
            wxTreeItemId gameobjectid = g_pPanelObjectList->AddObject( this, GameObject::StaticOnLeftClick, GameObject::StaticOnRightClick, rootid, m_Name );
            g_pPanelObjectList->SetDragAndDropFunctions( this, GameObject::StaticOnDrag, GameObject::StaticOnDrop );
            g_pPanelObjectList->SetLabelEditFunction( this, GameObject::StaticOnLabelEdit );
            m_pComponentTransform->AddToObjectsPanel( gameobjectid );
            for( unsigned int i=0; i<m_Components.Count(); i++ )
            {
                m_Components[i]->AddToObjectsPanel( gameobjectid );
            }
        }
#endif //MYFW_USING_WX
        return;
    }
    
    if( m_Managed == true && managed == false )
    {
        m_Managed = false;
#if MYFW_USING_WX
        if( g_pPanelObjectList )
        {
            g_pPanelObjectList->RemoveObject( m_pComponentTransform );
            g_pPanelObjectList->RemoveObject( this );
        }
#endif //MYFW_USING_WX
        return;
    }

    // one of the two conditions above should be true.
    assert( false );
}

ComponentBase* GameObject::AddNewComponent(int componenttype, unsigned int sceneid, ComponentSystemManager* pComponentSystemManager)
{
    assert( componenttype != -1 );

    if( m_Components.Count() >= m_Components.Length() )
        return 0;

    ComponentBase* pComponent = g_pComponentTypeManager->CreateComponent( componenttype );

    assert( pComponentSystemManager );
    if( m_Managed )
    {
        pComponentSystemManager->AddComponent( pComponent );
    }
    pComponent->SetID( pComponentSystemManager->m_NextComponentID );
    pComponentSystemManager->m_NextComponentID++;

    assert( sceneid == 0 || m_SceneID == sceneid );
    pComponent->SetSceneID( sceneid );

    AddExistingComponent( pComponent, true );

    return pComponent;
}

ComponentBase* GameObject::AddExistingComponent(ComponentBase* pComponent, bool resetcomponent)
{
    if( m_Components.Count() >= m_Components.Length() )
        return 0;

    pComponent->m_pGameObject = this;
    if( resetcomponent )
        pComponent->Reset();

    m_Components.Add( pComponent );

    assert( pComponent->GetSceneID() == 0 || m_SceneID == pComponent->GetSceneID() );

    // add this to the system managers component list.
    if( pComponent->Prev == 0 )
        g_pComponentSystemManager->AddComponent( pComponent );

#if MYFW_USING_WX
    if( m_Managed )
    {
        wxTreeItemId gameobjectid = g_pPanelObjectList->FindObject( this );
        pComponent->AddToObjectsPanel( gameobjectid );
    }
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
            
            // remove from system managers component list.
            pComponent->Remove();
            pComponent->Prev = 0;
            pComponent->Next = 0;

#if MYFW_USING_WX
            // remove the component from the object list.
            if( g_pPanelObjectList )
            {
                g_pPanelObjectList->RemoveObject( pComponent );
            }
#endif //MYFW_USING_WX

            return pComponent;
        }
    }

    return 0; // component not found.
}

ComponentBase* GameObject::GetFirstComponentOfBaseType(BaseComponentTypes basetype)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->m_BaseType == basetype )
        {
            return m_Components[i];
        }
    }

    return 0; // component not found.
}

ComponentCollisionObject* GameObject::GetCollisionObject()
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        //if( m_Components[i]->m_Type == ComponentType_CollisionObject )
        {
            if( dynamic_cast<ComponentCollisionObject*>(m_Components[i]) != 0 )
                return (ComponentCollisionObject*)m_Components[i];
        }
    }

    return 0;
}
