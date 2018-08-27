//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#include "PrefabManager.h"

GameObject::GameObject(bool managed, SceneID sceneid, bool isfolder, bool hastransform, PrefabReference* pPrefabRef)
{
    ClassnameSanityCheck();

    m_pGameObjectThisInheritsFrom = 0;

#if MYFW_EDITOR
    if( pPrefabRef && pPrefabRef->GetPrefab() )
    {
        m_pGameObjectThisInheritsFrom = pPrefabRef->GetGameObject();
    }
#endif

    m_pParentGameObject = 0;

    m_Properties.SetEnabled( false );
    m_Properties.m_pGameObject = this;

    m_Enabled = true;
    if( pPrefabRef != 0 )
        m_PrefabRef = *pPrefabRef;
    m_IsFolder = isfolder;
    m_SceneID = sceneid;
    m_ID = 0;
    m_PhysicsSceneID = sceneid;
    m_Name = 0;
    m_pOriginatingPool = 0;
    
    m_Managed = false;
    if( managed )
        SetManaged( true );

    if( isfolder || hastransform == false )
    {
        m_pComponentTransform = 0;
    }
    else
    {
        m_pComponentTransform = MyNew ComponentTransform();
        m_pComponentTransform->SetSceneID( sceneid );
        m_pComponentTransform->m_pGameObject = this;
        m_pComponentTransform->Reset();
    }

    m_Components.AllocateObjects( MAX_COMPONENTS ); // hard coded nonsense for now, max of 8 components on a game object.
}

GameObject::~GameObject()
{
#if MYFW_USING_WX
    if( g_pPanelWatch->GetObjectBeingWatched() == this )
        g_pPanelWatch->ClearAllVariables();
#endif //MYFW_USING_WX

    NotifyOthersThisWasDeleted();

    MyAssert( m_pOnDeleteCallbacks.GetHead() == 0 );

    // If we still have a parent gameobject, then we're likely still registered in its OnDeleted callback list.
    // Unregister ourselves to stop the parent's gameobject from reporting its deletion.
    if( m_pParentGameObject != 0 )
    {
        m_pParentGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    // If it's in a list, remove it.
    if( this->Prev != 0 )
        Remove();

    // Delete components.
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

    if( m_pComponentTransform )
    {
        m_pComponentTransform->SetEnabled( false );
    }
    SAFE_DELETE( m_pComponentTransform );

    SAFE_DELETE_ARRAY( m_Name );

    // Delete all children.
    while( m_ChildList.GetHead() )
        delete m_ChildList.RemHead();
}

void GameObject::SetGameObjectThisInheritsFrom(GameObject* pObj)
{
    // TODO: Fix prefab when this gets called.
    MyAssert( m_PrefabRef.GetPrefab() == 0 );

    m_pGameObjectThisInheritsFrom = pObj;
}

#if MYFW_USING_LUA
void GameObject::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<GameObject>( "GameObject" )
            .addData( "ComponentTransform", &GameObject::m_pComponentTransform ) // ComponentTransform*
            //.addData( "name", &GameObject::m_Name ) // char*
            .addData( "id", &GameObject::m_ID ) // unsigned int
            .addFunction( "SetEnabled", &GameObject::SetEnabled ) // void GameObject::SetEnabled(bool enabled, bool affectchildren)
            .addFunction( "SetName", &GameObject::SetName ) // void GameObject::SetName(const char* name)
            .addFunction( "GetTransform", &GameObject::GetTransform ) // ComponentTransform* GameObject::GetTransform()
            .addFunction( "GetFirstComponentOfBaseType", &GameObject::GetFirstComponentOfBaseType ) // ComponentBase* GameObject::GetFirstComponentOfBaseType(BaseComponentTypes basetype)
            .addFunction( "GetFirstComponentOfType", &GameObject::GetFirstComponentOfType ) // ComponentBase* GameObject::GetFirstComponentOfType(const char* type)
            .addFunction( "GetAnimationPlayer", &GameObject::GetAnimationPlayer ) // ComponentAnimationPlayer* GameObject::GetAnimationPlayer()    
            .addFunction( "Get3DCollisionObject", &GameObject::Get3DCollisionObject ) // Component3DCollisionObject* GameObject::Get3DCollisionObject()
            .addFunction( "Get2DCollisionObject", &GameObject::Get2DCollisionObject ) // Component2DCollisionObject* GameObject::Get2DCollisionObject()
            .addFunction( "GetParticleEmitter", &GameObject::GetParticleEmitter ) // ComponentParticleEmitter* GameObject::GetParticleEmitter()    
            .addFunction( "GetVoxelWorld", &GameObject::GetVoxelWorld ) // ComponentVoxelWorld* GameObject::GetVoxelWorld()              
            .addFunction( "GetAudioPlayer", &GameObject::GetAudioPlayer ) // ComponentAudioPlayer* GameObject::GetAudioPlayer()            
            .addFunction( "GetObjectPool", &GameObject::GetObjectPool ) // ComponentObjectPool* GameObject::GetObjectPool()            
            .addFunction( "ReturnToPool", &GameObject::ReturnToPool ) // void GameObject::ReturnToPool()            
        .endClass();
}
#endif //MYFW_USING_LUA

cJSON* GameObject::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jGameObject = cJSON_CreateObject();

    cJSON_AddNumberToObject( jGameObject, "ID", m_ID );

    cJSON_AddStringToObject( jGameObject, "Name", m_Name );

    // Inheritance parent can be in a different scene.
    if( m_pGameObjectThisInheritsFrom )
    {
#if MYFW_EDITOR
        // Don't save parentGO if it's the prefab.
        if( m_PrefabRef.GetPrefab() == 0 ||
            (m_pGameObjectThisInheritsFrom != m_PrefabRef.GetGameObject()) )
#endif
        {
            cJSON_AddItemToObject( jGameObject, "ParentGO", m_pGameObjectThisInheritsFrom->ExportReferenceAsJSONObject( m_SceneID ) );
        }
    }

    // Transform/Hierarchy parent must be in the same scene.
    if( m_pParentGameObject )
        cJSON_AddNumberToObject( jGameObject, "ParentGOID", m_pParentGameObject->GetID() );

    if( m_Enabled == false )
        cJSON_AddNumberToObject( jGameObject, "Enabled", m_Enabled );

    if( savesceneid )
        cJSON_AddNumberToObject( jGameObject, "SceneID", m_SceneID );

    if( m_SceneID != m_PhysicsSceneID )
        cJSON_AddNumberToObject( jGameObject, "PhysicsSceneID", m_PhysicsSceneID );

    if( m_PrefabRef.GetPrefab() != 0 )
    {
        cJSON_AddStringToObject( jGameObject, "PrefabFile", m_PrefabRef.GetPrefab()->GetPrefabFile()->GetFile()->GetFullPath() );
        cJSON_AddNumberToObject( jGameObject, "PrefabID", m_PrefabRef.GetPrefab()->GetID() );
        
        if( m_PrefabRef.GetChildID() != 0 )
        {
            cJSON_AddNumberToObject( jGameObject, "PrefabChildID", m_PrefabRef.GetChildID() );
        }
    }
    
    if( m_IsFolder == true )
        cJSON_AddStringToObject( jGameObject, "SubType", "Folder" );
    else if( m_pComponentTransform == false )
        cJSON_AddStringToObject( jGameObject, "SubType", "Logic" );

    cJSON* jProperties = m_Properties.ExportAsJSONObject( false, true );
    // if no properties were saved, don't write it out to disk
    if( jProperties->child == 0 )
    {
        cJSON_Delete( jProperties );
    }
    else
    {
        cJSON_AddItemToObject( jGameObject, "Properties", jProperties );
    }

#if MYFW_EDITOR
    // Export lists of deleted prefab children and components.
    if( m_DeletedPrefabChildIDs.size() > 0 )
    {
        cJSONExt_AddUnsignedIntArrayToObject( jGameObject, "DeletedPrefabChildIDs", &m_DeletedPrefabChildIDs[0], m_DeletedPrefabChildIDs.size() );
    }    
    if( m_DeletedPrefabComponentIDs.size() > 0 )
    {
        cJSONExt_AddUnsignedIntArrayToObject( jGameObject, "DeletedPrefabComponents", &m_DeletedPrefabComponentIDs[0], m_DeletedPrefabComponentIDs.size() );
    }
#endif

    return jGameObject;
}

void GameObject::ImportFromJSONObject(cJSON* jGameObject, SceneID sceneid)
{
    // Load the correct GameObject ID
    cJSONExt_GetUnsignedInt( jGameObject, "ID", &m_ID );

    // Deal with prefabs // only in editor builds, game builds don't much care.
#if MYFW_EDITOR
    cJSON* jPrefabID = cJSON_GetObjectItem( jGameObject, "PrefabID" );
    if( jPrefabID )
    {
        // If we're doing a quick-load of a file, this gameobject should already have its prefab info set up
        if( m_PrefabRef.GetPrefab() )
        {
            // Quick-loading the file, don't load the prefab info.
        }
        else
        {
            cJSON* jPrefabFile = cJSON_GetObjectItem( jGameObject, "PrefabFile" );
            MyAssert( jPrefabFile != 0 );

            if( jPrefabFile )
            {
                // Store the PrefabId and PrefabChildID in the gameobject so they can be used when loading is complete.
                cJSON* jPrefabChildID = cJSON_GetObjectItem( jGameObject, "PrefabChildID" );
                m_PrefabRef.StoreIDsWhileLoading( jPrefabID->valueint, jPrefabChildID ? jPrefabChildID->valueint : 0 );

                PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByFullPath( jPrefabFile->valuestring );
            
                // prefab file load must have been initiated by scene load
                // might want to consider triggering a load here if it's not in the file list.
                MyAssert( pPrefabFile != 0 );

                // if the prefab file isn't loaded yet, store the name and link to the prefab when the file is loaded
                if( pPrefabFile->GetFile()->IsFinishedLoading() == false ) // still loading
                {
                    pPrefabFile->GetFile()->RegisterFileFinishedLoadingCallback( this, StaticOnPrefabFileFinishedLoading );
                }
                else
                {
                    FinishLoadingPrefab( pPrefabFile );
                }
            }
        }
    }
#endif //MYFW_EDITOR

    unsigned int parentgoid = 0;
    cJSONExt_GetUnsignedInt( jGameObject, "ParentGOID", &parentgoid );
    if( parentgoid != 0 )
    {
        GameObject* pParentGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, parentgoid );
        MyAssert( pParentGameObject );

#if MYFW_USING_WX
        // Move as last item in parent.
        GameObject* pLastChild = (GameObject*)pParentGameObject->GetChildList()->GetTail();
        if( pLastChild != 0 )
            g_pPanelObjectList->Tree_MoveObject( this, pLastChild, false );
        else
            g_pPanelObjectList->Tree_MoveObject( this, pParentGameObject, true );
#endif //MYFW_USING_WX

        SetParentGameObject( pParentGameObject );
    }

    // LEGACY: support for old scene files with folders in them
    //    now stored as "SubType", handled in ComponentSystemManager::LoadSceneFromJSON()
    cJSONExt_GetBool( jGameObject, "IsFolder", &m_IsFolder );

    cJSON* jName = cJSON_GetObjectItem( jGameObject, "Name" );
    if( jName )
    {
        SetName( jName->valuestring );
    }
    SetSceneID( sceneid, false ); // set new scene, but don't assign a new GOID.

    m_PhysicsSceneID = m_SceneID;
    cJSONExt_GetUnsignedInt( jGameObject, "PhysicsSceneID", (unsigned int*)&m_PhysicsSceneID );

    bool enabled = true;
    cJSONExt_GetBool( jGameObject, "Enabled", &enabled );
    SetEnabled( enabled, false );

    cJSON* jProperties = cJSON_GetObjectItem( jGameObject, "Properties" );
    if( jProperties )
        m_Properties.ImportFromJSONObject( jProperties, sceneid );

#if MYFW_EDITOR
    // Import lists of deleted prefab children and components.
    {
        cJSON* jArray = cJSON_GetObjectItem( jGameObject, "DeletedPrefabChildIDs" );
        if( jArray )
        {
            int arraysize = cJSON_GetArraySize( jArray );
            for( int i=0; i<arraysize; i++ )
            {
                cJSON* jObj = cJSON_GetArrayItem( jArray, i );
                if( jObj )
                {
                    m_DeletedPrefabChildIDs.push_back( (uint32)jObj->valueint );
                }
            }
        }

        jArray = cJSON_GetObjectItem( jGameObject, "DeletedPrefabComponents" );
        if( jArray )
        {
            int arraysize = cJSON_GetArraySize( jArray );
            for( int i=0; i<arraysize; i++ )
            {
                cJSON* jObj = cJSON_GetArrayItem( jArray, i );
                if( jObj )
                {
                    m_DeletedPrefabComponentIDs.push_back( (uint32)jObj->valueint );
                }
            }
        }
    }
#endif
}

void GameObject::ImportInheritanceInfoFromJSONObject(cJSON* jGameObject)
{
    cJSON* jParentGO = cJSON_GetObjectItem( jGameObject, "ParentGO" );
    if( jParentGO )
    {
        m_pGameObjectThisInheritsFrom = g_pComponentSystemManager->FindGameObjectByJSONRef( jParentGO, m_SceneID, true );

        // If this trips, then the other object might come from another scene that isn't loaded.
        MyAssert( m_pGameObjectThisInheritsFrom != 0 );
    }
}

cJSON* GameObject::ExportReferenceAsJSONObject(SceneID refsceneid)
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

cJSON* GameObject::ExportAsJSONPrefab(PrefabObject* pPrefab, bool assignNewChildIDs, bool assignNewComponentIDs)
{
    cJSON* jGameObject = cJSON_CreateObject();

    // Back-up sceneid.
    // Set gameobject to scene zero, so any gameobject references will store the sceneid when serialized (since they will differ)
    // Set it back later, without changing gameobject id.
    SceneID sceneidbackup = GetSceneID();
    SetSceneID( SCENEID_Unmanaged, false );

    // Don't export the ParentGOID, we'll ignore that it was parented at all.
    //// Transform/Hierarchy parent must be in the same scene.
    //if( m_pParentGameObject )
    //    cJSON_AddNumberToObject( jGameObject, "ParentGOID", m_pParentGameObject->GetID() );

    cJSON_AddStringToObject( jGameObject, "Name", GetName() );

    if( m_IsFolder == true )
        cJSON_AddStringToObject( jGameObject, "SubType", "Folder" );
    else if( m_pComponentTransform == false )
        cJSON_AddStringToObject( jGameObject, "SubType", "Logic" );

    // Export the prefab this object is an instance of.
    if( m_PrefabRef.GetPrefab() != 0 )
    {
        // Nested Prefabs must come from the same file.
        // TODO: Replace this assert with an error message.
        MyAssert( m_PrefabRef.GetPrefab()->GetPrefabFile() == pPrefab->GetPrefabFile() );

        // If this prefab inherits from another prefab, save the prefabID.
        if( m_pGameObjectThisInheritsFrom != 0 )
        {
            PrefabReference* pInheritedPrefabRef = m_pGameObjectThisInheritsFrom->GetPrefabRef();
            //PrefabReference* pInheritedPrefabRef = &m_PrefabRef;
            cJSON_AddNumberToObject( jGameObject, "PrefabID", pInheritedPrefabRef->GetPrefab()->GetID() );

            if( pInheritedPrefabRef->GetChildID() != 0 )
            {
                cJSON_AddNumberToObject( jGameObject, "PrefabChildID", pInheritedPrefabRef->GetChildID() );
            }
        }
    }

    // Export properties, if none were saved, don't add the block to jGameObject
    cJSON* jProperties = m_Properties.ExportAsJSONObject( false, false );
    if( jProperties->child == 0 )
    {
        cJSON_Delete( jProperties );
    }
    else
    {
        cJSON_AddItemToObject( jGameObject, "Properties", jProperties );
    }

    // Export components
    if( m_Components.Count() > 0 )
    {
        cJSON* jComponentArray = cJSON_CreateArray();
        cJSON_AddItemToObject( jGameObject, "Components", jComponentArray );
        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
#if MYFW_EDITOR
            // Assign PrefabComponentIDs if they weren't previously assigned.
            if( assignNewComponentIDs && m_Components[i]->GetPrefabComponentID() == 0 )
            {
                // If this component is from an instance of a Prefab, don't assign new component IDs for it.
                if( assignNewComponentIDs && m_Components[i]->GetPrefabComponentID() == 0 )
                {
                    // Get a prefab component id from the file. It should be unique within that prefab file.
                    uint32 ID = pPrefab->GetPrefabFile()->GetNextPrefabComponentIDAndIncrement();
                    m_Components[i]->SetPrefabComponentID( ID );
                }
            }
#endif //MYFW_EDITOR

            cJSON* jComponent = m_Components[i]->ExportAsJSONObject( false, false );

            cJSON_AddItemToArray( jComponentArray, jComponent );
        }
    }

    // Loop through children and add them to jGameObject.
    if( m_ChildList.GetHead() != 0 )
    {
        cJSON* jChildrenArray = cJSON_CreateArray();
        cJSON_AddItemToObject( jGameObject, "Children", jChildrenArray );
        
        for( CPPListNode* pNode = m_ChildList.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            GameObject* pChildGameObject = (GameObject*)pNode;

            cJSON* jChildObject = pChildGameObject->ExportAsJSONPrefab( pPrefab, assignNewChildIDs, assignNewComponentIDs );
            MyAssert( jChildObject );
            cJSON_AddItemToArray( jChildrenArray, jChildObject );

            // Add ChildID.
            {
                uint32 childid = 0;

                if( assignNewChildIDs )
                    childid = pPrefab->GetNextChildPrefabIDAndIncrement();
                else
                    childid = pChildGameObject->GetPrefabRef()->GetChildID();

                MyAssert( childid != 0 );
                cJSON_AddNumberToObject( jChildObject, "ChildID", childid );
            }

            // Add the child's offset from the parent.
            ComponentTransform* pTransform = pChildGameObject->GetTransform();
            if( pTransform )
            {
                cJSON* jTransform = pTransform->ExportLocalTransformAsJSONObject();
                cJSON_AddItemToObject( jChildObject, "LocalTransform", jTransform );
            }
        }
    }

    // Reset scene id to original value, don't change the gameobjectid.
    SetSceneID( sceneidbackup, false );

    return jGameObject;
}

// Exposed to Lua, change elsewhere if function signature changes.
void GameObject::SetEnabled(bool enabled, bool affectchildren)
{
    if( m_Enabled == enabled )
        return;

    m_Enabled = enabled;

    // Un/register all component callbacks.
    if( m_Enabled )
        RegisterAllComponentCallbacks( false );
    else
        UnregisterAllComponentCallbacks( false );

    // Loop through all components and call OnGameObjectEnabled/OnGameObjectDisabled.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Enabled )
            m_Components[i]->OnGameObjectEnabled();
        else
            m_Components[i]->OnGameObjectDisabled();
    }

    // Recurse through children.
    if( affectchildren )
    {
        GameObject* pChild = GetFirstChild();

        while( pChild )
        {
            pChild->SetEnabled( enabled, true );
            pChild = (GameObject*)pChild->GetNext();
        }
    }
}

void GameObject::RegisterAllComponentCallbacks(bool ignoreenabledflag)
{
    // Loop through all components and register/unregister their callbacks.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->IsEnabled() || ignoreenabledflag )
            m_Components[i]->RegisterCallbacks();
    }
}

void GameObject::UnregisterAllComponentCallbacks(bool ignoreenabledflag)
{
    // Loop through all components and register/unregister their callbacks.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->IsEnabled() || ignoreenabledflag )
            m_Components[i]->UnregisterCallbacks();
    }
}

void GameObject::SetSceneID(SceneID sceneid, bool assignnewgoid)
{
    if( m_SceneID == sceneid )
        return;

    m_SceneID = sceneid;

    // Loop through components and change the sceneid in each.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        m_Components[i]->SetSceneID( sceneid );
    }
   
    if( assignnewgoid )
    {
        m_ID = g_pComponentSystemManager->GetNextGameObjectIDAndIncrement( sceneid );
    }
}

void GameObject::SetID(unsigned int id)
{
    m_ID = id;
}

// Exposed to Lua, change elsewhere if function signature changes.
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

void GameObject::SetOriginatingPool(ComponentObjectPool* pPool)
{
    MyAssert( m_pOriginatingPool == 0 );

    m_pOriginatingPool = pPool;
}

void GameObject::SetParentGameObject(GameObject* pNewParentGameObject)
{
    // If the old parent is the same as the new one, kick out.
    if( m_pParentGameObject == pNewParentGameObject )
        return;

    GameObject* pOldParentGameObject = m_pParentGameObject;
    m_pParentGameObject = pNewParentGameObject;

    // If we had an old parent:
    if( pOldParentGameObject != 0 )
    {
        // Stop the parent's gameobject from reporting its deletion.
        pOldParentGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    // If we have a new parent:
    if( pNewParentGameObject != 0 )
    {
        // Register with the parent's gameobject to notify us of its deletion.
        pNewParentGameObject->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        // The prefab's m_pGameObject will be null when the scene is loading but the prefab file isn't loaded.
        // Skip the check in this case since the childID should already be in the "deleted child" list.
        if( m_PrefabRef.GetGameObject() != 0 && m_PrefabRef.IsMasterPrefabGameObject() == false )
        {
            // If this object is part of the same prefab as the new parent, check if the new parent is missing this child.
            if( m_PrefabRef.GetPrefab() && m_PrefabRef.GetPrefab() == pNewParentGameObject->m_PrefabRef.GetPrefab() )
            {
                uint32 childID = m_PrefabRef.GetChildID();
#if MYFW_EDITOR
                if( childID != 0 && pNewParentGameObject->IsMissingPrefabChild( childID ) )
                {
                    // If the new parent is missing this child, remove it from its list of "deleted" children since it's about to get it back.
                    pNewParentGameObject->RemovePrefabChildIDFromListOfDeletedPrefabChildIDs( childID );
                }
#endif //MYFW_EDITOR
            }
        }

        // Move the child object to the new parent.
        pNewParentGameObject->m_ChildList.MoveTail( this );
    }
    else
    {
        g_pComponentSystemManager->GetSceneInfo( m_SceneID )->m_GameObjects.MoveTail( this );
    }

    // Now that this object is unparented from the old parent, check if it's missing this child since there could have been 2+ of them attached.
    if( pOldParentGameObject )
    {
        // If this object is part of the same prefab as the old parent, check if the old parent is now missing this child.
        if( m_PrefabRef.GetPrefab() == pOldParentGameObject->m_PrefabRef.GetPrefab() )
        {
            uint32 childID = m_PrefabRef.GetChildID();
#if MYFW_EDITOR
            if( childID != 0 && pOldParentGameObject->IsMissingPrefabChild( childID ) )
            {
                // If the old parent is now missing this child, add it to its list of "deleted" children.
                pOldParentGameObject->AddPrefabChildIDToListOfDeletedPrefabChildIDs( childID );
            }
#endif //MYFW_EDITOR
        }
    }

    // Parent one transform to another, if there are transforms.
    if( m_pComponentTransform )
    {
        ComponentTransform* pNewParentTransform = 0;
        if( m_pParentGameObject != 0 )
            pNewParentTransform = pNewParentGameObject->m_pComponentTransform;

        m_pComponentTransform->SetParentTransform( pNewParentTransform );
    }
}

bool GameObject::IsParentedTo(GameObject* pPotentialParent, bool onlycheckdirectparent)
{
    GameObject* pParent = GetParentGameObject();

    if( pParent == pPotentialParent )
        return true;

    if( pParent == 0 || onlycheckdirectparent )
        return false;

    return pParent->IsParentedTo( pPotentialParent, false );
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
            // Add this game object to the root of the objects tree.
            wxTreeItemId rootid = g_pComponentSystemManager->GetTreeIDForScene( m_SceneID );
            MyAssert( rootid.IsOk() );

            wxTreeItemId gameobjectid = g_pPanelObjectList->AddObject( this, GameObject::StaticOnLeftClick, GameObject::StaticOnRightClick, rootid, m_Name );
            g_pPanelObjectList->SetDragAndDropFunctions( gameobjectid, GameObject::StaticOnDrag, GameObject::StaticOnDrop );
            g_pPanelObjectList->SetLabelEditFunction( gameobjectid, GameObject::StaticOnLabelEdit );
            UpdateObjectListIcon();
            
            // Place the child under the parent in the object list.
            if( m_pParentGameObject )
            {
                GameObject* pPrevChild = (GameObject*)GetPrev();

                if( pPrevChild != 0 )
                    gameobjectid = g_pPanelObjectList->Tree_MoveObject( this, pPrevChild, false );
                else
                    gameobjectid = g_pPanelObjectList->Tree_MoveObject( this, m_pParentGameObject, true );                
            }

            if( m_pComponentTransform )
            {
                m_pComponentTransform->AddToObjectsPanel( gameobjectid );
            }

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
            // Remove transform component from object list.
            if( m_pComponentTransform )
            {
                g_pPanelObjectList->RemoveObject( m_pComponentTransform );
            }

            // Remove other components from object list.
            for( unsigned int i=0; i<m_Components.Count(); i++ )
            {
                g_pPanelObjectList->RemoveObject( m_Components[i] );
            }

            // Remove the gameobject itself from the object list.
            g_pPanelObjectList->RemoveObject( this );
        }
        return;
    }
#endif //MYFW_USING_WX
}

unsigned int GameObject::GetComponentCount()
{
    return m_Components.Count();
}

ComponentBase* GameObject::GetComponentByIndex(unsigned int index)
{
    MyAssert( index < m_Components.Count() );

    return m_Components[index];
}

unsigned int GameObject::GetComponentCountIncludingCore()
{
    if( m_pComponentTransform )
    {
        return m_Components.Count() + 2; // + properties + transform
    }
    else
    {
        return m_Components.Count() + 1; // + properties
    }
}

ComponentBase* GameObject::GetComponentByIndexIncludingCore(unsigned int index)
{
    if( m_pComponentTransform )
    {
        MyAssert( index < GetComponentCount() + 2 );

        if( index == 0 )
            return &m_Properties;
        else if( index == 1 )
            return m_pComponentTransform;
        else
            return m_Components[index-2];
    }
    else
    {
        MyAssert( index < GetComponentCount() + 1 );

        if( index == 0 )
            return &m_Properties;
        else
            return m_Components[index-1];
    }
}

ComponentBase* GameObject::AddNewComponent(int componenttype, SceneID sceneid, ComponentSystemManager* pComponentSystemManager)
{
    MyAssert( componenttype != -1 );

    if( m_Components.Count() >= m_Components.Length() )
        return 0;

    ComponentBase* pComponent = g_pComponentTypeManager->CreateComponent( componenttype );

    if( componenttype == ComponentType_Transform )
    {
        // Special handling of ComponentType_Transform, only offer option if GameObject doesn't have a transform
        //     m_pComponentTransform will be set in AddExistingComponent() below.
#if MYFW_EDITOR
#if MYFW_USING_WX
        // Update the icon
        UpdateObjectListIcon();
#endif //MYFW_USING_WX
        pComponent->m_Type = -1; // hack, all transforms have -1 as type, setting this to be consistent.
#endif //MYFW_EDITOR
    }
    else
    {
        MyAssert( pComponentSystemManager );
        if( m_Managed )
        {
            pComponentSystemManager->AddComponent( pComponent );
        }
    }

    if( sceneid != SCENEID_Unmanaged )
    {
        unsigned int id = pComponentSystemManager->GetNextComponentIDAndIncrement( sceneid );
        pComponent->SetID( id );
    }

    MyAssert( sceneid == SCENEID_Unmanaged || m_SceneID == sceneid );
    pComponent->SetSceneID( sceneid );

    AddExistingComponent( pComponent, true );

    return pComponent;
}

ComponentBase* GameObject::AddExistingComponent(ComponentBase* pComponent, bool resetcomponent)
{
    // special handling for adding transform component
    if( pComponent->IsA( "TransformComponent" ) )
    {
        m_pComponentTransform = (ComponentTransform*)pComponent;

#if MYFW_USING_WX
        // Update the icon
        UpdateObjectListIcon();
#endif //MYFW_USING_WX

        pComponent->m_pGameObject = this;
        if( resetcomponent )
            pComponent->Reset();

        // re-parent all child transforms, if they have one
        for( CPPListNode* pNode = m_ChildList.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            // TODO: recurse through children
            GameObject* pChildGameObject = (GameObject*)pNode;

            if( pChildGameObject->m_pComponentTransform )
            {
                pChildGameObject->m_pComponentTransform->SetParentTransform( m_pComponentTransform );
            }
        }

        // Re-enable all renderable components
        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
            if( m_Components[i]->IsA( "RenderableComponent" ) )
            {
                m_Components[i]->SetEnabled( true );
            }
        }
    }
    else
    {
        if( m_Components.Count() >= m_Components.Length() )
            return 0;

        pComponent->m_pGameObject = this;
        if( resetcomponent )
            pComponent->Reset();

        m_Components.Add( pComponent );

        // if the component isn't already in the system managers component list, add it, whether gameobject is managed or not
        if( pComponent->Prev == 0 )
            g_pComponentSystemManager->AddComponent( pComponent );
    }

    // register this components callbacks.
    pComponent->RegisterCallbacks();

    // pComponent->GetSceneID() == SCENEID_Unmanaged || 
    MyAssert( m_SceneID == pComponent->GetSceneID() );

#if MYFW_EDITOR
    {
        // Remove this prefab component id from the "deleted" list.
        uint32 id = pComponent->GetPrefabComponentID();
        if( id != 0 )
        {
            // Make sure it's in the list.
            MyAssert( std::find( m_DeletedPrefabComponentIDs.begin(), m_DeletedPrefabComponentIDs.end(), id ) != m_DeletedPrefabComponentIDs.end() );

            std::vector<uint32>::iterator lastremoved = std::remove( m_DeletedPrefabComponentIDs.begin(), m_DeletedPrefabComponentIDs.end(), id );
            m_DeletedPrefabComponentIDs.erase( lastremoved, m_DeletedPrefabComponentIDs.end() );
        }
    }
#endif

#if MYFW_USING_WX
    if( m_Managed )
    {
        wxTreeItemId gameobjectid = g_pPanelObjectList->FindObject( this );
        if( gameobjectid.IsOk() )
            pComponent->AddToObjectsPanel( gameobjectid );
    }

    g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX

    return pComponent;
}

ComponentBase* GameObject::RemoveComponent(ComponentBase* pComponent)
{
    bool found = false;

    // special handling for removing transform component
    if( pComponent->IsA( "TransformComponent" ) )
    {
        found = true;

        // Unparent all child transforms, if they have one
        for( CPPListNode* pNode = m_ChildList.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            // TODO: recurse through children
            GameObject* pChildGameObject = (GameObject*)pNode;

            if( pChildGameObject->m_pComponentTransform )
            {
                pChildGameObject->m_pComponentTransform->SetParentTransform( 0 );
            }
        }

        // Disable all renderable components
        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
            if( m_Components[i]->IsA( "RenderableComponent" ) )
            {
                m_Components[i]->SetEnabled( false );
            }
        }

        m_pComponentTransform = 0;

#if MYFW_USING_WX
        // Update the icon
        UpdateObjectListIcon();
#endif //MYFW_USING_WX
    }
    else
    {
        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
            if( m_Components[i] == pComponent )
            {
                found = true;
                m_Components.RemoveIndex_MaintainOrder( i );

                // remove from system managers component list.
                pComponent->Remove();
                pComponent->Prev = 0;
                pComponent->Next = 0;
            }
        }
    }

    if( found )
    {
        // unregister all this components callbacks.
        pComponent->UnregisterCallbacks();

#if MYFW_EDITOR
        {
            // Add this prefab component id to the "deleted" list.
            uint32 id = pComponent->GetPrefabComponentID();
            if( id != 0 )
            {
                // Make sure it's not already in the list.
                MyAssert( std::find( m_DeletedPrefabComponentIDs.begin(), m_DeletedPrefabComponentIDs.end(), id ) == m_DeletedPrefabComponentIDs.end() );

                m_DeletedPrefabComponentIDs.push_back( id );
            }
        }
#endif

#if MYFW_USING_WX
        // remove the component from the object list.
        if( g_pPanelObjectList )
        {
            g_pPanelObjectList->RemoveObject( pComponent );
        }
#endif //MYFW_USING_WX

        return pComponent;
    }

    return 0; // component not found.
}

ComponentBase* GameObject::FindComponentByPrefabComponentID(unsigned int prefabComponentID)
{
#if MYFW_EDITOR
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->GetPrefabComponentID() == prefabComponentID )
        {
            return m_Components[i];
        }
    }
#endif MYFW_EDITOR

    return 0;
}

ComponentBase* GameObject::FindComponentByID(unsigned int componentID)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->GetID() == componentID )
        {
            return m_Components[i];
        }
    }

    return 0;
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

void GameObject::ReturnToPool()
{
    MyAssert( m_pOriginatingPool != 0 );

    m_pOriginatingPool->ReturnObjectToPool( this );
}

// Exposed to Lua, change elsewhere if function signature changes.
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

// Exposed to Lua, change elsewhere if function signature changes.
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
}

void GameObject::NotifyOthersThisWasDeleted()
{
    for( CPPListNode* pNode = m_pOnDeleteCallbacks.GetHead(); pNode; )
    {
        CPPListNode* pNextNode = pNode->GetNext();

        GameObjectDeletedCallbackStruct* pCallbackStruct = (GameObjectDeletedCallbackStruct*)pNode;

        // Remove the callback struct from the list before calling the function
        //     since the callback function might try to unregister (and delete) the callback struct
        pCallbackStruct->Remove();

        // Call the onGameObjectDeleted callback function
        pCallbackStruct->pFunc( pCallbackStruct->pObj, this );

        // Delete the struct
        delete pCallbackStruct;

        pNode = pNextNode;
    }
}

void GameObject::OnGameObjectDeleted(GameObject* pGameObject)
{
    // if our parent was deleted, clear the pointer.
    MyAssert( m_pParentGameObject == pGameObject ); // the callback should have only been registered if needed.
    if( m_pParentGameObject == pGameObject )
    {
        // we're in the callback, so don't unregister the callback.
        if( m_pComponentTransform )
        {
            m_pComponentTransform->SetParentTransform( 0 );
        }
    }
}

void GameObject::OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor)
{
    //int bp = 1;
}

#if MYFW_EDITOR
bool GameObject::IsMissingPrefabChild(uint32 childID)
{
    MyAssert( m_pGameObjectThisInheritsFrom != 0 );

    // If this doesn't inherit from it's prefab, kick out, we're likely using direct (single scene) inheritance.
    if( m_pGameObjectThisInheritsFrom->GetPrefabRef()->GetPrefab() != this->m_PrefabRef.GetPrefab() )
        return false;

    // Check if prefab has this childID as a child.
    bool prefabHasThisChild = false;
    GameObject* pChild = m_pGameObjectThisInheritsFrom->GetFirstChild();
    while( pChild )
    {
        if( pChild->GetPrefabRef()->GetChildID() == childID )
        {
            MyAssert( pChild->GetPrefabRef()->GetPrefab() == m_pGameObjectThisInheritsFrom->m_PrefabRef.GetPrefab() );

            prefabHasThisChild = true;
            break;
        }

        pChild = (GameObject*)pChild->GetNext();
    }

    // If the prefab doesn't expect this child to be attached, the child isn't missing.
    if( prefabHasThisChild == false )
        return false;

    // Check if this object already has this childID as a child.
    pChild = GetFirstChild();
    while( pChild )
    {
        if( pChild->GetPrefabRef()->GetPrefab() == m_PrefabRef.GetPrefab() &&
            pChild->GetPrefabRef()->GetChildID() == childID )
        {
            // We already have this child, so it's not missing.
            return false;
        }

        pChild = (GameObject*)pChild->GetNext();
    }

    // The prefab expects this child and we don't have one, so it's missing.
    return true;
}

void GameObject::AddPrefabChildIDToListOfDeletedPrefabChildIDs(uint32 childID)
{
    MyAssert( childID != 0 );

    std::vector<uint32>* pList = &m_DeletedPrefabChildIDs;

    // Make sure it's not already in the list.
    MyAssert( std::find( pList->begin(), pList->end(), childID ) == pList->end() );

    pList->push_back( childID );
}

void GameObject::RemovePrefabChildIDFromListOfDeletedPrefabChildIDs(uint32 childID)
{
    MyAssert( childID != 0 );

    std::vector<uint32>* pList = &m_DeletedPrefabChildIDs;

    // Make sure it's already in the list.
    MyAssert( std::find( pList->begin(), pList->end(), childID ) != pList->end() );

    std::vector<uint32>::iterator lastremoved = std::remove( pList->begin(), pList->end(), childID );
    pList->erase( lastremoved, pList->end() );
}

void GameObject::AddPrefabComponentIDToListOfDeletedPrefabComponentIDs(uint32 componentID)
{
}

void GameObject::RemovePrefabComponentIDFromListOfDeletedPrefabComponentIDs(uint32 componentID)
{
}

void GameObject::OnPopupClick(GameObject* pGameObject, unsigned int id)
{
    if( id < g_pComponentTypeManager->GetNumberOfComponentTypes() )
    {
        if( pGameObject->m_Components.Count() >= pGameObject->m_Components.Length() )
            return;

        int type = id; // could be EngineComponentTypes or GameComponentTypes type.

        ComponentBase* pComponent = 0;
        if( g_pEngineCore->IsInEditorMode() )
            pComponent = pGameObject->AddNewComponent( type, pGameObject->GetSceneID() );
        else
            pComponent = pGameObject->AddNewComponent( type, SCENEID_Unmanaged );

        pComponent->OnLoad();
    }
    else if( id == RightClick_DuplicateGameObject )
    {
        if( g_pEngineCore->IsInEditorMode() )
            g_pComponentSystemManager->EditorCopyGameObject( pGameObject, false );
        else
            g_pComponentSystemManager->CopyGameObject( pGameObject, "runtime duplicate", false );
    }
    else if( id == RightClick_CreateChild )
    {
        GameObject* pNewObject = g_pComponentSystemManager->EditorCopyGameObject( pGameObject, true );
        pNewObject->m_pGameObjectThisInheritsFrom = pGameObject;
    }
    else if( id == RightClick_ClearParent )
    {
        EditorState* pEditorState = g_pEngineCore->GetEditorState();

        // if the object isn't selected, delete just the one object, otherwise delete all selected objects.
        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ClearParentOfGameObjects( &pEditorState->m_pSelectedObjects ) );
    }
    else if( id >= RightClick_CreatePrefab && id < RightClick_CreatePrefab + 10000 )
    {
        unsigned int numprefabfiles = g_pComponentSystemManager->m_pPrefabManager->GetNumberOfFiles();
        if( id == RightClick_CreatePrefab + numprefabfiles )
        {
            // Load or create a new file.
            if( g_pComponentSystemManager->m_pPrefabManager->CreateOrLoadFile() )
            {
                // Create a prefab based on selected object.
                unsigned int fileindex = numprefabfiles;
                g_pComponentSystemManager->m_pPrefabManager->CreatePrefabInFile( fileindex, pGameObject->GetName(), pGameObject );
            }
        }
        else
        {
            // Create a prefab based on selected object.
            unsigned int fileindex = id - RightClick_CreatePrefab;
            PrefabObject* pPrefab = g_pComponentSystemManager->m_pPrefabManager->CreatePrefabInFile( fileindex, pGameObject->GetName(), pGameObject );

            // Set existed object to inherit from this prefab.
            if( pPrefab )
            {
                Editor_SetGameObjectAndAllChildrenToInheritFromPrefab( pPrefab, 0 );
                //PrefabReference prefabRef( pPrefab, prefabchildid, true );
                //Editor_SetPrefab( &prefabRef );
            }
        }
    }
    else if( id == RightClick_DeleteGameObject )
    {
        EditorState* pEditorState = g_pEngineCore->GetEditorState();

        // if the object isn't selected, delete just the one object, otherwise delete all selected objects.
        if( pEditorState->IsGameObjectSelected( pGameObject ) )
        {
            pEditorState->DeleteSelectedObjects();
        }
        else
        {
            // create a temp vector to pass into command.
            std::vector<GameObject*> gameobjects;
            gameobjects.push_back( pGameObject );
            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
        }
    }
    else if( id == RightClick_DeleteFolder )
    {
        // delete all gameobjects in the folder, along with the folder itself.
        std::vector<GameObject*> gameobjects;

        pGameObject->AddToList( &gameobjects );

        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
    }
    else if( id == RightClick_DuplicateFolder )
    {
        if( g_pEngineCore->IsInEditorMode() )
            g_pComponentSystemManager->EditorCopyGameObject( pGameObject, false );
        else
            g_pComponentSystemManager->CopyGameObject( pGameObject, "runtime duplicate", false );
    }
    else if( id >= RightClick_AdditionalSceneHandlerOptions && id < RightClick_EndOfAdditionalSceneHandlerOptions )
    {
#if MYFW_USING_WX
        int idForSceneHandler = id - RightClick_AdditionalSceneHandlerOptions;
        
        SceneHandler* pSceneHandler = g_pComponentSystemManager->m_pSceneHandler;
        pSceneHandler->HandleRightClickCommand( idForSceneHandler, pGameObject );
#endif //MYFW_USING_WX
    }
}

#if MYFW_USING_WX
void GameObject::OnTitleLabelClicked(int controlid, bool finishedchanging) // StaticOnTitleLabelClicked
{
    g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_EnableObject( this, !m_Enabled, true ) );
    g_pPanelWatch->SetNeedsRefresh();
}

void GameObject::OnLeftClick(unsigned int count, bool clear)
{
    g_pEngineCore->OnObjectListTreeMultipleSelection( false );
    return;
}

void GameObject::ShowInWatchPanel(bool isprefab)
{
    if( g_pEngineCore->GetEditorState() == 0 )
        return;

    if( m_IsFolder )
        return;

    g_pPanelWatch->ClearAllVariables();
    g_pEngineCore->OnObjectListTreeSelectionChanged();

    // Select this GameObject in the editor window if it's not a prefab.
    if( g_pEngineCore->GetEditorState()->IsGameObjectSelected( this ) == false && isprefab == false )
        g_pEngineCore->GetEditorState()->m_pSelectedObjects.push_back( this );

    g_pPanelWatch->SetObjectBeingWatched( this );

    // Show the gameobject name and an enabled checkbox.
    char tempname[100];
    if( m_Enabled )
    {
        if( m_pGameObjectThisInheritsFrom == 0 )
            sprintf_s( tempname, 100, "%s", m_Name );
        else
            sprintf_s( tempname, 100, "%s (%s)", m_Name, m_pGameObjectThisInheritsFrom->m_Name );
    }
    else
    {
        if( m_pGameObjectThisInheritsFrom == 0 )
            sprintf_s( tempname, 100, "** DISABLED ** %s ** DISABLED **", m_Name );
        else
            sprintf_s( tempname, 100, "** DISABLED ** %s (%s) ** DISABLED **", m_Name, m_pGameObjectThisInheritsFrom->m_Name );
    }

    // Only allow enable/disable click on regular non-prefab objects.
    if( isprefab )
    {
        g_pPanelWatch->AddSpace( tempname, this, 0 );
    }
    else
    {
        g_pPanelWatch->AddSpace( tempname, this, GameObject::StaticOnTitleLabelClicked );
    }

    // Add variables from ComponentGameObjectProperties.
    m_Properties.m_MultiSelectedComponents.clear();
    m_Properties.FillPropertiesWindow( false );

    // Add variables from ComponentTransform.
    if( m_pComponentTransform )
    {
        m_pComponentTransform->m_MultiSelectedComponents.clear();
        m_pComponentTransform->FillPropertiesWindow( false, true );
    }

    // Add variables from all other components.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        m_Components[i]->m_MultiSelectedComponents.clear();
        m_Components[i]->FillPropertiesWindow( false, true );
    }
}

void GameObject::OnRightClick() // StaticOnRightClick
{
 	wxMenu menu;
    menu.SetClientData( this );

    wxMenu* categorymenu = 0;
    const char* lastcategory = 0;

    // if there are ever more than 1000 component types?!? increase the RightClick_* initial value in header.
    MyAssert( g_pComponentTypeManager->GetNumberOfComponentTypes() < RightClick_DuplicateGameObject );

    if( m_IsFolder == false )
    {
        menu.Append( RightClick_DuplicateGameObject, "Duplicate GameObject" );
        menu.Append( RightClick_CreateChild, "Create Child GameObject" );
        if( m_pGameObjectThisInheritsFrom )
        {
            menu.Append( RightClick_ClearParent, "Clear Parent" );
        }

        // Special handling of ComponentType_Transform, only offer option if GameObject doesn't have a transform
        int first = 0;
        if( m_pComponentTransform != 0 )
            first = 1;

        unsigned int numtypes = g_pComponentTypeManager->GetNumberOfComponentTypes();
        for( unsigned int i=first; i<numtypes; i++ )
        {
            if( lastcategory != g_pComponentTypeManager->GetTypeCategory( i ) )
            {
                categorymenu = MyNew wxMenu;
                menu.AppendSubMenu( categorymenu, g_pComponentTypeManager->GetTypeCategory( i ) );

#if MYFW_OSX
                // Not needed on Windows build, but seems OSX doesn't call OnPopupClick callback for submenus without this.
                categorymenu->SetClientData( this );
                categorymenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&GameObject::OnPopupClick );
#endif
            }

            if( i == ComponentType_Mesh )
            {
                // don't include ComponentType_Mesh in the right-click menu.
                // TODO: if more exceptions are made, improve this system.
            }
            else
            {
                categorymenu->Append( i, g_pComponentTypeManager->GetTypeName( i ) );
            }

            lastcategory = g_pComponentTypeManager->GetTypeCategory( i );
        }

        // Create prefab menu and submenus.
        AddPrefabSubmenusToMenu( &menu, RightClick_CreatePrefab );

        menu.Append( RightClick_DeleteGameObject, "Delete GameObject" );
    }
    else
    {
        // Add folder specific options to menu.
        menu.Append( RightClick_DuplicateFolder, "Duplicate Folder and all contents" );
        menu.Append( RightClick_DeleteFolder, "Delete Folder and all contents" );

        // Have SceneHandler add all of it's options to the menu (Create GameObject, etc...).
        wxTreeItemId treeid = this->GetSceneInfo()->m_TreeID;
        SceneID sceneid = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );

        if( sceneid != SCENEID_NotFound )
        {
            SceneHandler* pSceneHandler = g_pComponentSystemManager->m_pSceneHandler;
            pSceneHandler->AddGameObjectMenuOptionsToMenu( &menu, RightClick_AdditionalSceneHandlerOptions, sceneid );

            // Create prefab menu and submenus.
            AddPrefabSubmenusToMenu( &menu, RightClick_CreatePrefab );
        }
    }

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&GameObject::OnPopupClick );
    
    // blocking call. // should delete all categorymenu's new'd above when done.
 	g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void GameObject::AddPrefabSubmenusToMenu(wxMenu* menu, int itemidoffset)
{
    // Create prefab menu and submenus for each file.
    wxMenu* prefabmenu = MyNew wxMenu;
    menu->AppendSubMenu( prefabmenu, "Create Prefab in" );

#if MYFW_OSX
    // Not needed on Windows build, but seems OSX doesn't call OnPopupClick callback for submenus without this.
    menu->SetClientData( this );
    menu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&GameObject::OnPopupClick );
#endif

    unsigned int numprefabfiles = g_pComponentSystemManager->m_pPrefabManager->GetNumberOfFiles();
    for( unsigned int i=0; i<numprefabfiles; i++ )
    {
        PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByIndex( i );
        MyFileObject* pFile = pPrefabFile->GetFile();
        MyAssert( pFile != 0 );

        prefabmenu->Append( itemidoffset + i, pFile->GetFilenameWithoutExtension() );
    }

    prefabmenu->Append( itemidoffset + numprefabfiles, "New/Load Prefab file..." );
}

void GameObject::OnPopupClick(wxEvent &evt)
{
    GameObject* pGameObject = (GameObject*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    unsigned int id = evt.GetId();

    OnPopupClick( pGameObject, id );
}

void GameObject::OnDrag()
{
    g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, this );
}
#endif //MYFW_USING_WX

void GameObject::OnDrop(int controlid, int x, int y, GameObjectOnDropActions action)
{
#if MYFW_USING_WX
    // If you drop a game object on another, parent them or move above/below depending on the "y".
    // The bounding rect will change once the first item is moved, so get the rect once before moving things.
    wxTreeItemId treeid = g_pPanelObjectList->FindObject( this );
    wxRect rect;
    g_pPanelObjectList->m_pTree_Objects->GetBoundingRect( treeid, rect, false );
#endif //MYFW_USING_WX

    std::vector<GameObject*> selectedObjects;

    // Range must match code in PanelObjectListDropTarget::OnDragOver. // TODO: fix this
    bool setAsChild = true;
#if MYFW_USING_WX
    if( y > rect.GetBottom() - 10 )
    {
        // Move below the selected item.
        setAsChild = false;
    }
#else
    if( action == GameObjectOnDropAction_Reorder )
    {
        setAsChild = false;
    }
#endif //MYFW_USING_WX

    // Move/Reparent all of the selected items.
    for( unsigned int i=0; i<g_DragAndDropStruct.GetItemCount(); i++ )
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( i );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
        {
            GameObject* pGameObject = (GameObject*)pDropItem->m_Value;

            // If we're dropping this object on itself, kick out.
            if( pGameObject == this )
                continue;

            // If we're attempting to set dragged objects as children,
            //   don't allow folders to be children of non-folder gameobjects.
            if( setAsChild )
            {
                if( m_IsFolder == false && pGameObject->IsFolder() )
                    continue;
            }

            // If this object's parent is in the list, don't add it to the selected list.
            {
                bool hasParentInList = false;
                for( unsigned int j=0; j<g_DragAndDropStruct.GetItemCount(); j++ )
                {
                    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( j );

                    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
                    {
                        GameObject* pPotentialParentGameObject = (GameObject*)pDropItem->m_Value;

                        if( pGameObject->IsParentedTo( pPotentialParentGameObject, true ) )
                        {
                            hasParentInList = true;
                            break;
                        }
                    }
                }

                if( hasParentInList )
                    continue;
            }

            selectedObjects.push_back( pGameObject );
        }
    }

    if( selectedObjects.size() > 0 )
    {
        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ReorderOrReparentGameObjects( selectedObjects, this, GetSceneID(), setAsChild ) );
    }
}

#if MYFW_USING_WX
void GameObject::OnLabelEdit(wxString newlabel)
{
    size_t len = newlabel.length();
    if( len > 0 )
    {
        SetName( newlabel );
    }
}

void GameObject::UpdateObjectListIcon()
{
    // Set the icon for the gameobject in the objectlist panel tree.
    int iconindex = ObjectListIcon_GameObject;
    if( m_PrefabRef.GetPrefab() != 0 )
        iconindex = ObjectListIcon_Prefab;
    else if( m_IsFolder )
        iconindex = ObjectListIcon_Folder;
    else if( m_pComponentTransform == 0 )
        iconindex = ObjectListIcon_LogicObject;

    wxTreeItemId gameobjectid = g_pPanelObjectList->FindObject( this );
    if( gameobjectid.IsOk() )
        g_pPanelObjectList->SetIcon( gameobjectid, iconindex );
}
#endif //MYFW_USING_WX

void GameObject::FinishLoadingPrefab(PrefabFile* pPrefabFile)
{
    // Link the PrefabRef to the correct prefab and GameObject now that the file is finished loading.
    m_PrefabRef.FinishLoadingPrefab( pPrefabFile );
    if( m_pParentGameObject != 0 )
    {
        m_PrefabRef.SetOriginalParent( m_pParentGameObject );
    }

#if MYFW_EDITOR
    m_pGameObjectThisInheritsFrom = m_PrefabRef.GetGameObject();
#endif

    // TODO: Check the if the gameobect(s) in the prefab are completely different and deal with it.

    // Otherwise, importing same prefab, so update all undivorced variables to match prefab file.
    {
        GameObject* pPrefabGameObject = m_PrefabRef.GetGameObject();
        MyAssert( pPrefabGameObject );

        for( unsigned int i=0; i<pPrefabGameObject->m_Components.Count(); i++ )
        {
            ComponentBase* pPrefabComponent = pPrefabGameObject->m_Components[i];
            ComponentBase* pComponent = FindComponentByPrefabComponentID( pPrefabComponent->GetPrefabComponentID() );

            if( pComponent )
            {
                pComponent->SyncUndivorcedVariables( pPrefabComponent );
            }
        }
    }

#if MYFW_USING_WX
    UpdateObjectListIcon();
#endif //MYFW_USING_WX
}

void GameObject::OnPrefabFileFinishedLoading(MyFileObject* pFile) // StaticOnPrefabFileFinishedLoading
{
    PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByFullPath( pFile->GetFullPath() );
    FinishLoadingPrefab( pPrefabFile );

    pFile->UnregisterFileFinishedLoadingCallback( this );
}

// Returns the gameobject in the scene that lines up with the root of the prefab.
// Useful for prefab subobjects(children) to quickly find the starting point of the prefab instance.
GameObject* GameObject::FindRootGameObjectOfPrefabInstance()
{
    MyAssert( m_PrefabRef.GetPrefab() != 0 );

    if( m_PrefabRef.GetChildID() == 0 )
        return this;

    return m_pParentGameObject->FindRootGameObjectOfPrefabInstance();
}

// Used when deleting prefabs.
void GameObject::Editor_SetPrefab(PrefabReference* pPrefabRef)
{
    m_PrefabRef = *pPrefabRef;
    m_pGameObjectThisInheritsFrom = m_PrefabRef.GetGameObject();

#if MYFW_USING_WX
    UpdateObjectListIcon();
#endif //MYFW_USING_WX
}

void GameObject::Editor_SetGameObjectAndAllChildrenToInheritFromPrefab(PrefabObject* pPrefab, uint32 prefabChildID)
{
    PrefabReference ref( pPrefab, prefabChildID, true );
    m_PrefabRef = ref;
    m_pGameObjectThisInheritsFrom = m_PrefabRef.GetGameObject();

    // Set children.
    GameObject* pChildGO = (GameObject*)GetChildList()->GetHead();
    GameObject* pPrefabChildGO = (GameObject*)pPrefab->GetGameObject()->GetChildList()->GetHead();
    while( pChildGO )
    {
        // Temp assert, test nested prefabs and replace with an 'if'
        MyAssert( pPrefabChildGO->GetPrefabRef()->GetPrefab() == pPrefab );

        uint32 prefabChildChildID = pPrefabChildGO->GetPrefabRef()->GetChildID();
        pChildGO->Editor_SetGameObjectAndAllChildrenToInheritFromPrefab( pPrefab, prefabChildChildID );

        pChildGO = (GameObject*)pChildGO->GetNext();
        pPrefabChildGO = (GameObject*)pPrefabChildGO->GetNext();
    }
}

void GameObject::Editor_SetGameObjectThisInheritsFromIgnoringPrefabRef(GameObject* pObj)
{
    m_pGameObjectThisInheritsFrom = pObj;
}

// Set the material on all renderable components attached to this object.
void GameObject::Editor_SetMaterial(MaterialDefinition* pMaterial)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->m_BaseType == BaseComponentType_Renderable )
        {
            MyAssert( m_Components[i]->IsA( "RenderableComponent" ) );

            ComponentRenderable* pRenderable = (ComponentRenderable*)m_Components[i];

            if( pRenderable )
            {
                // TODO: Deal with more than just the first submeshes material.
                int submeshindex = 0;

                // Go through same code to drop a material on the component, so inheritance and undo/redo will be handled
                ComponentVariable* pVar = pRenderable->GetComponentVariableForMaterial( submeshindex );

                g_DragAndDropStruct.Clear();
                g_DragAndDropStruct.SetControlID( pVar->m_ControlID );
                g_DragAndDropStruct.Add( DragAndDropType_MaterialDefinitionPointer, pMaterial );

                pRenderable->OnDropVariable( pVar, 0, -1, -1, true );
            }
        }
    }
}

void GameObject::AddToList(std::vector<GameObject*>* pList)
{
    // Don't allow same object to be in the list twice.
    // This can happen if a folder is selected along with an item inside.
    if( std::find( pList->begin(), pList->end(), this ) != pList->end() )
        return;

    // Select the object.
    pList->push_back( this );

    // If this is a folder, select all objects inside.
    if( IsFolder() )
    {
        for( CPPListNode* pNode = GetChildList()->GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ((GameObject*)pNode)->AddToList( pList );
        }
    }
}
#endif //MYFW_EDITOR
