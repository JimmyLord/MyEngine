//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

GameObject::GameObject(bool managed, int sceneid)
{
    ClassnameSanityCheck();

    m_pGameObjectThisInheritsFrom = 0;

    m_Enabled = true;
    m_SceneID = sceneid;
    m_ID = 0;
    m_PhysicsSceneID = 0;
    m_Name = 0;

    m_pComponentTransform = MyNew ComponentTransform();
    m_pComponentTransform->SetSceneID( sceneid );
    m_pComponentTransform->m_pGameObject = this;
    m_pComponentTransform->Reset();

    m_Components.AllocateObjects( MAX_COMPONENTS ); // hard coded nonsense for now, max of 4 components on a game object.

    m_Managed = false;
    if( managed )
        SetManaged( true );
}

GameObject::~GameObject()
{
#if MYFW_USING_WX
    if( g_pPanelWatch->GetObjectBeingWatched() == this )
        g_pPanelWatch->ClearAllVariables();
#endif //MYFW_USING_WX

    NotifyOthersThisWasDeleted();

    MyAssert( m_pOnDeleteCallbacks.GetHead() == 0 );

    // if it's in a list, remove it.
    if( this->Prev != 0 )
        Remove();

    // if this object is managed, the ComponentSystemManager will delete the components.
    if( m_Managed == false )
    {
        while( m_Components.Count() )
        {
            ComponentBase* pComponent = m_Components.RemoveIndex( 0 );
            pComponent->SetEnabled( false );
            delete pComponent;
        }
    }

    if( m_Managed )
        SetManaged( false );

    m_pComponentTransform->SetEnabled( false );
    SAFE_DELETE( m_pComponentTransform );

    SAFE_DELETE_ARRAY( m_Name );

    // delete all children.
    while( m_ChildList.GetHead() )
        delete m_ChildList.RemHead();
}

#if MYFW_USING_LUA
void GameObject::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<GameObject>( "GameObject" )
            .addData( "ComponentTransform", &GameObject::m_pComponentTransform )
            .addData( "name", &GameObject::m_Name )
            .addData( "id", &GameObject::m_ID )            
            .addFunction( "SetEnabled", &GameObject::SetEnabled )
            .addFunction( "SetName", &GameObject::SetName )
            .addFunction( "GetTransform", &GameObject::GetTransform )
            .addFunction( "GetFirstComponentOfBaseType", &GameObject::GetFirstComponentOfBaseType )
            .addFunction( "GetFirstComponentOfType", &GameObject::GetFirstComponentOfType )
            .addFunction( "GetAnimationPlayer", &GameObject::GetAnimationPlayer )
            .addFunction( "GetCollisionObject", &GameObject::GetCollisionObject )
            .addFunction( "Get2DCollisionObject", &GameObject::Get2DCollisionObject )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void GameObject::OnTitleLabelClicked(int controlid, bool finishedchanging)
{
    SetEnabled( !m_Enabled );
    g_pPanelWatch->m_NeedsRefresh = true;
}

void GameObject::OnLeftClick(unsigned int count, bool clear)
{
    // select this GameObject in the editor window.
    if( g_pEngineCore->m_pEditorState == 0 )
        return;

    if( g_pEngineCore->m_pEditorState->IsGameObjectSelected( this ) == false )
        g_pEngineCore->m_pEditorState->m_pSelectedObjects.push_back( this );

    //LOGInfo( LOGTag, "Selected objects: %d\n", g_pEngineCore->m_pEditorState->m_pSelectedObjects.size() );

    // only show properties of the first selected object.
    if( g_pEngineCore->m_pEditorState->m_pSelectedObjects.size() > 1 )
        return;

    if( clear )
        g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->SetObjectBeingWatched( this );

    // show the gameobject name and an enabled checkbox.
    char tempname[100];
    if( m_Enabled )
    {
        if( m_pGameObjectThisInheritsFrom == 0 )
            snprintf_s( tempname, 100, "%s", m_Name );
        else
            snprintf_s( tempname, 100, "%s (%s)", m_Name, m_pGameObjectThisInheritsFrom->m_Name );
    }
    else
    {
        if( m_pGameObjectThisInheritsFrom == 0 )
            snprintf_s( tempname, 100, "** DISABLED ** %s ** DISABLED **", m_Name );
        else
            snprintf_s( tempname, 100, "** DISABLED ** %s (%s) ** DISABLED **", m_Name, m_pGameObjectThisInheritsFrom->m_Name );
    }
    g_pPanelWatch->AddSpace( tempname, this, GameObject::StaticOnTitleLabelClicked );

    m_pComponentTransform->FillPropertiesWindow( false, true );
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        m_Components[i]->FillPropertiesWindow( false, true );
    }
}

void GameObject::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    wxMenu* categorymenu = 0;
    const char* lastcategory = 0;

    // if there are ever more than 1000 component types?!? increase the RightClick_* initial value in header.
    MyAssert( g_pComponentTypeManager->GetNumberOfComponentTypes() < RightClick_DuplicateGameObject );

    menu.Append( RightClick_DuplicateGameObject, "Duplicate GameObject" );
    menu.Append( RightClick_CreateChild, "Create Child GameObject" );
    if( m_pGameObjectThisInheritsFrom )
    {
        menu.Append( RightClick_ClearParent, "Clear Parent" );
    }

    unsigned int numtypes = g_pComponentTypeManager->GetNumberOfComponentTypes();
    for( unsigned int i=0; i<numtypes; i++ )
    {
        if( lastcategory != g_pComponentTypeManager->GetTypeCategory( i ) )
        {
            categorymenu = MyNew wxMenu;
            menu.AppendSubMenu( categorymenu, g_pComponentTypeManager->GetTypeCategory( i ) );
        }

        categorymenu->Append( i, g_pComponentTypeManager->GetTypeName( i ) );

        lastcategory = g_pComponentTypeManager->GetTypeCategory( i );
    }
    
    menu.Append( RightClick_DeleteGameObject, "Delete GameObject" );

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&GameObject::OnPopupClick );
    
    // blocking call. // should delete all categorymenu's new'd above when done.
 	g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void GameObject::OnPopupClick(wxEvent &evt)
{
    GameObject* pGameObject = (GameObject*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    unsigned int id = evt.GetId();

    if( id < g_pComponentTypeManager->GetNumberOfComponentTypes() )
    {
        if( pGameObject->m_Components.Count() >= pGameObject->m_Components.Length() )
            return;

        int type = id; // could be EngineComponentTypes or GameComponentTypes type.

        if( g_pEngineCore->m_EditorMode )
            pGameObject->AddNewComponent( type, pGameObject->GetSceneID() );
        else
            pGameObject->AddNewComponent( type, 0 );
    }
    else if( id == RightClick_DuplicateGameObject )
    {
        if( g_pEngineCore->m_EditorMode )
            g_pComponentSystemManager->EditorCopyGameObject( pGameObject, false );
        else
            g_pComponentSystemManager->CopyGameObject( pGameObject, "runtime duplicate" );
    }
    else if( id == RightClick_CreateChild )
    {
        GameObject* pNewObject = g_pComponentSystemManager->EditorCopyGameObject( pGameObject, true );
        pNewObject->m_pGameObjectThisInheritsFrom = pGameObject;
    }
    else if( id == RightClick_ClearParent )
    {
        m_pGameObjectThisInheritsFrom = 0;
    }
    else if( id == RightClick_DeleteGameObject )
    {
        EditorState* pEditorState = g_pEngineCore->m_pEditorState;

        // if the object isn't selected, delete just the one object, otherwise delete all selected objects.
        if( pEditorState->IsGameObjectSelected( pGameObject ) )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteObjects( pEditorState->m_pSelectedObjects ) );

            pGameObject->NotifyOthersThisWasDeleted();
        }
        else
        {
            // create a temp vector to pass into command.
            std::vector<GameObject*> gameobjects;
            gameobjects.push_back( pGameObject );
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
        }
    }
}

void GameObject::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_GameObjectPointer;
    g_DragAndDropStruct.m_Value = this;
}

void GameObject::OnDrop(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)g_DragAndDropStruct.m_Value;

        // if you drop a game object on another, parent them or move above/below depending on the "y"
        wxTreeItemId treeid = g_pPanelObjectList->FindObject( this );
        wxRect rect;
        g_pPanelObjectList->m_pTree_Objects->GetBoundingRect( treeid, rect, false );

        if( false ) //y < rect.GetTop() + 5 ) // move above the selected item
        {
            //g_pPanelObjectList->Tree_MoveObjectBefore( pGameObject, this, false );
            //pGameObject->MoveBefore( this );
        }
        else if( y > rect.GetBottom() - 10 ) // move below the selected item
        {
            g_pPanelObjectList->Tree_MoveObject( pGameObject, this, false );
            pGameObject->MoveAfter( this );
        }
        else // Parent the object dropped to this.
        {
            pGameObject->SetParentGameObject( this );
        }
    }
}

void GameObject::OnLabelEdit(wxString newlabel)
{
    size_t len = newlabel.length();
    if( len > 0 )
    {
        SetName( newlabel );
    }
}
#endif //MYFW_USING_WX

cJSON* GameObject::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jGameObject = cJSON_CreateObject();

    if( m_pGameObjectThisInheritsFrom )
        cJSON_AddItemToObject( jGameObject, "ParentGO", m_pGameObjectThisInheritsFrom->ExportReferenceAsJSONObject( m_SceneID ) );

    if( m_Enabled == false )
        cJSON_AddNumberToObject( jGameObject, "Enabled", m_Enabled );

    if( savesceneid )
        cJSON_AddNumberToObject( jGameObject, "SceneID", m_SceneID );

    cJSON_AddNumberToObject( jGameObject, "ID", m_ID );
    if( m_SceneID != m_PhysicsSceneID )
        cJSON_AddNumberToObject( jGameObject, "PhysicsSceneID", m_PhysicsSceneID );

    cJSON_AddStringToObject( jGameObject, "Name", m_Name );

    return jGameObject;
}

void GameObject::ImportFromJSONObject(cJSON* jGameObject, unsigned int sceneid)
{
    cJSON* obj;

    obj = cJSON_GetObjectItem( jGameObject, "ParentGO" );
    if( obj )
    {
        m_pGameObjectThisInheritsFrom = g_pComponentSystemManager->FindGameObjectByJSONRef( obj, m_SceneID );

        // if this trips, then other object might be loaded after this or come from another scene that isn't loaded.
        MyAssert( m_pGameObjectThisInheritsFrom != 0 );
    }

    cJSONExt_GetUnsignedInt( jGameObject, "ID", &m_ID );
    m_PhysicsSceneID = m_SceneID;
    cJSONExt_GetUnsignedInt( jGameObject, "PhysicsSceneID", &m_PhysicsSceneID );

    obj = cJSON_GetObjectItem( jGameObject, "Name" );
    if( obj )
    {
        SetName( obj->valuestring );
    }
    SetSceneID( sceneid );

    bool enabled = true;
    cJSONExt_GetBool( jGameObject, "Enabled", &enabled );
    SetEnabled( enabled );
}

cJSON* GameObject::ExportReferenceAsJSONObject(unsigned int refsceneid)
{
    // see ComponentSystemManager::FindGameObjectByJSONRef

    cJSON* gameobjectref = cJSON_CreateObject();

    if( refsceneid != m_SceneID )
    {
        cJSON_AddStringToObject( gameobjectref, "Scene", GetSceneInfo()->m_FullPath );
    }

    cJSON_AddNumberToObject( gameobjectref, "GOID", m_ID );

    return gameobjectref;
}

void GameObject::SetEnabled(bool enabled)
{
    if( m_Enabled == enabled )
        return;

    m_Enabled = enabled;

    // un/register all component callbacks
    if( m_Enabled )
        RegisterAllComponentCallbacks( false );
    else
        UnregisterAllComponentCallbacks( false );

    // loop through all components and call OnGameObjectEnabled/OnGameObjectDisabled
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Enabled )
            m_Components[i]->OnGameObjectEnabled();
        else
            m_Components[i]->OnGameObjectDisabled();
    }
}

void GameObject::RegisterAllComponentCallbacks(bool ignoreenabledflag)
{
    // loop through all components and register/unregister their callbacks.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Enabled || ignoreenabledflag )
            m_Components[i]->RegisterCallbacks();
    }
}

void GameObject::UnregisterAllComponentCallbacks(bool ignoreenabledflag)
{
    // loop through all components and register/unregister their callbacks.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Enabled || ignoreenabledflag )
            m_Components[i]->UnregisterCallbacks();
    }
}

void GameObject::SetSceneID(unsigned int sceneid)
{
    if( m_SceneID == sceneid )
        return;

    m_SceneID = sceneid;
    m_ID = g_pComponentSystemManager->GetNextGameObjectIDAndIncrement( sceneid );
}

void GameObject::SetID(unsigned int id)
{
    m_ID = id;
}

void GameObject::SetName(const char* name)
{
    MyAssert( name );

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

void GameObject::SetParentGameObject(GameObject* pParentGameObject)
{
    //GameObject* pOldParentGameObject = m_pComponentTransform->m_pParentGameObject;
    //if( pOldParentGameObject )
    //    pOldParentGameObject->m_pComponentTransform->UnregisterPositionChangedCallback();

    // parent one transform to another.
    this->m_pComponentTransform->SetParentTransform( pParentGameObject->m_pComponentTransform );

    // If the parent is in another scene, move the game object to that scene.
    unsigned int sceneid = pParentGameObject->GetSceneID();
    SetSceneID( sceneid );

    pParentGameObject->m_ChildList.MoveTail( this );
#if MYFW_USING_WX
    if( GetPrev() == 0 )
        g_pPanelObjectList->Tree_MoveObject( this, pParentGameObject, true );
    else
        g_pPanelObjectList->Tree_MoveObject( this, GetPrev(), false );
#endif
}

void GameObject::SetManaged(bool managed)
{
    MyAssert( m_Managed != managed );
    if( m_Managed == managed )
        return;

    m_Managed = managed;

#if MYFW_USING_WX
    if( m_Managed == true )
    {
        if( g_pPanelObjectList )
        {
            // Add this game object to the root of the objects tree
            //wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
            wxTreeItemId rootid = g_pComponentSystemManager->GetTreeIDForScene( m_SceneID );
            MyAssert( rootid.IsOk() );
            wxTreeItemId gameobjectid = g_pPanelObjectList->AddObject( this, GameObject::StaticOnLeftClick, GameObject::StaticOnRightClick, rootid, m_Name );
            g_pPanelObjectList->SetDragAndDropFunctions( gameobjectid, GameObject::StaticOnDrag, GameObject::StaticOnDrop );
            g_pPanelObjectList->SetLabelEditFunction( gameobjectid, GameObject::StaticOnLabelEdit );
            m_pComponentTransform->AddToObjectsPanel( gameobjectid );
            for( unsigned int i=0; i<m_Components.Count(); i++ )
            {
                m_Components[i]->AddToObjectsPanel( gameobjectid );
            }

        }
        return;
    }
    else
    {
        if( g_pPanelObjectList )
        {
            g_pPanelObjectList->RemoveObject( m_pComponentTransform );
            g_pPanelObjectList->RemoveObject( this );
        }
        return;
    }
#endif //MYFW_USING_WX
}

ComponentBase* GameObject::AddNewComponent(int componenttype, unsigned int sceneid, ComponentSystemManager* pComponentSystemManager)
{
    MyAssert( componenttype != -1 );

    if( m_Components.Count() >= m_Components.Length() )
        return 0;

    ComponentBase* pComponent = g_pComponentTypeManager->CreateComponent( componenttype );

    MyAssert( pComponentSystemManager );
    if( m_Managed )
    {
        pComponentSystemManager->AddComponent( pComponent );
    }
    unsigned int id = pComponentSystemManager->GetNextComponentIDAndIncrement( sceneid );
    pComponent->SetID( id );

    MyAssert( sceneid == 0 || m_SceneID == sceneid );
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

    // register this components callbacks.
    pComponent->RegisterCallbacks();

    MyAssert( pComponent->GetSceneID() == 0 || m_SceneID == pComponent->GetSceneID() );

    // add this to the system managers component list.
    if( pComponent->Prev == 0 )
        g_pComponentSystemManager->AddComponent( pComponent );

#if MYFW_USING_WX
    if( m_Managed )
    {
        wxTreeItemId gameobjectid = g_pPanelObjectList->FindObject( this );
        if( gameobjectid.IsOk() )
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

            // unregister all this components callbacks.
            pComponent->UnregisterCallbacks();
            
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

ComponentBase* GameObject::FindComponentByID(unsigned int componentid)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->GetID() == componentid )
        {
            return m_Components[i];
        }
    }

    return 0;
}

ComponentAnimationPlayer* GameObject::GetAnimationPlayer()
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        ComponentAnimationPlayer* pPlayer = ((ComponentBase*)m_Components[i])->IsA( "AnimPlayerComponent" ) ? (ComponentAnimationPlayer*)m_Components[i] : 0;
        if( pPlayer != 0 )
        {
            return pPlayer;
        }
    }

    return 0; // component not found.
}

// Gets the first material found.
MaterialDefinition* GameObject::GetMaterial()
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->m_BaseType == BaseComponentType_Renderable )
        {
            MyAssert( m_Components[i]->IsA( "RenderableComponent" ) );
            return ((ComponentRenderable*)m_Components[i])->GetMaterial( 0 );
        }
    }

    return 0;
}

// Set the material on all renderable components attached to this object.
void GameObject::SetMaterial(MaterialDefinition* pMaterial)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->m_BaseType == BaseComponentType_Renderable )
        {
            MyAssert( m_Components[i]->IsA( "RenderableComponent" ) );
            ((ComponentRenderable*)m_Components[i])->SetMaterial( pMaterial, 0 );
        }
    }
}

void GameObject::SetScriptFile(MyFileObject* pFile)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->m_BaseType == BaseComponentType_Updateable )
        {
#if MYFW_USING_LUA
            ComponentLuaScript* pLuaComponent = m_Components[i]->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)m_Components[i] : 0;
            if( pLuaComponent )
                pLuaComponent->SetScriptFile( pFile );
#endif //MYFW_USING_LUA
        }
    }
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

ComponentBase* GameObject::GetNextComponentOfBaseType(ComponentBase* pLastComponent)
{
    MyAssert( pLastComponent != 0 );

    bool foundlast = false;
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( pLastComponent == m_Components[i] )
        {
            foundlast = true;
        }
        else if( foundlast && m_Components[i]->m_BaseType == pLastComponent->m_BaseType )
        {
            return m_Components[i];
        }
    }

    return 0; // component not found.
}

ComponentBase* GameObject::GetFirstComponentOfType(const char* type)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( ((ComponentBase*)m_Components[i])->IsA( type ) )
            return (ComponentBase*)m_Components[i];
    }

    return 0; // component not found.
}

ComponentBase* GameObject::GetNextComponentOfType(ComponentBase* pLastComponent)
{
    MyAssert( pLastComponent != 0 );

    bool foundlast = false;
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( pLastComponent == m_Components[i] )
            foundlast = true;
        else if( foundlast && ((ComponentBase*)m_Components[i])->IsA( pLastComponent->GetClassname() ) )
            return (ComponentBase*)m_Components[i];
    }

    return 0; // component not found.
}

ComponentCollisionObject* GameObject::GetCollisionObject()
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->IsA( "CollisionObjectComponent" ) )
            return (ComponentCollisionObject*)m_Components[i];
    }

    return 0;
}

Component2DCollisionObject* GameObject::Get2DCollisionObject()
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->IsA( "2DCollisionObjectComponent" ) )
            return (Component2DCollisionObject*)m_Components[i];
    }

    return 0;
}

void GameObject::RegisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );

//#if _DEBUG
    // Make sure the same callback isn't being registered.
    for( CPPListNode* pNode = m_pOnDeleteCallbacks.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        GameObjectDeletedCallbackStruct* pCallbackStruct = (GameObjectDeletedCallbackStruct*)pNode;
        //MyAssert( pCallbackStruct->pObj != pObj && pCallbackStruct->pFunc != pCallback );
        if( pCallbackStruct->pObj == pObj && pCallbackStruct->pFunc == pCallback )
            return;
    }
//#endif

    // TODO: pool callback structures.
    GameObjectDeletedCallbackStruct* pCallbackStruct = MyNew GameObjectDeletedCallbackStruct;
    pCallbackStruct->pObj = pObj;
    pCallbackStruct->pFunc = pCallback;

    m_pOnDeleteCallbacks.AddTail( pCallbackStruct );
}

void GameObject::UnregisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback)
{
    for( CPPListNode* pNode = m_pOnDeleteCallbacks.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        GameObjectDeletedCallbackStruct* pCallbackStruct = (GameObjectDeletedCallbackStruct*)pNode;
        if( pCallbackStruct->pObj == pObj && pCallbackStruct->pFunc == pCallback )
        {
            pCallbackStruct->Remove();
            delete pCallbackStruct;
            return;
        }
    }

    MyAssert( false );
}

void GameObject::NotifyOthersThisWasDeleted()
{
    for( CPPListNode* pNode = m_pOnDeleteCallbacks.GetHead(); pNode; )
    {
        CPPListNode* pNextNode = pNode->GetNext();

        GameObjectDeletedCallbackStruct* pCallbackStruct = (GameObjectDeletedCallbackStruct*)pNode;
        pCallbackStruct->pFunc( pCallbackStruct->pObj, this );

        pCallbackStruct->Remove();
        delete pCallbackStruct;

        pNode = pNextNode;
    }
}

void GameObject::OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor)
{
    int bp = 1;
}
