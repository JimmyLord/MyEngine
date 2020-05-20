//
// Copyright (c) 2014-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "PrefabManager.h"
#include "ComponentSystem/BaseComponents/ComponentRenderable.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/EngineComponents/ComponentLuaScript.h"
#include "ComponentSystem/EngineComponents/ComponentObjectPool.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#endif

GameObject::GameObject(EngineCore* pEngineCore, bool managed, SceneID sceneID, bool isFolder, bool hasTransform, PrefabReference* pPrefabRef)
: m_Properties( pEngineCore, pEngineCore->GetComponentSystemManager() )
{
    ClassnameSanityCheck();

    m_pEngineCore = pEngineCore;

    m_pGameObjectThisInheritsFrom = nullptr;

#if MYFW_EDITOR
    if( pPrefabRef && pPrefabRef->GetPrefab() )
    {
        m_pGameObjectThisInheritsFrom = pPrefabRef->GetGameObject();
    }
#endif

    m_pParentGameObject = nullptr;

    m_Properties.SetEnabled( false );
    m_Properties.SetGameObject( this );

    m_Enabled = true;
    if( pPrefabRef != nullptr )
        m_PrefabRef = *pPrefabRef;
    m_IsFolder = isFolder;
    m_SceneID = sceneID;
    m_ID = 0;
    m_PhysicsSceneID = sceneID;
    m_Name = nullptr;
    m_pOriginatingPool = nullptr;
    
    m_Managed = false;
    if( managed )
        SetManaged( true );

    if( isFolder || hasTransform == false )
    {
        m_pComponentTransform = nullptr;
    }
    else
    {
        m_pComponentTransform = MyNew ComponentTransform( pEngineCore, pEngineCore->GetComponentSystemManager() );
        m_pComponentTransform->SetType( ComponentType_Transform );
        m_pComponentTransform->SetSceneID( sceneID );
        m_pComponentTransform->SetGameObject( this );
        m_pComponentTransform->Reset();
    }

    m_Components.AllocateObjects( MAX_COMPONENTS ); // Hard coded nonsense for now, max of 8 components on a game object.
}

GameObject::~GameObject()
{
    NotifyOthersThisWasDeleted();

    MyAssert( m_pOnDeleteCallbacks.GetHead() == nullptr );

    // If we still have a parent gameobject, then we're likely still registered in its OnDeleted callback list.
    // Unregister ourselves to stop the parent's gameobject from reporting its deletion.
    if( m_pParentGameObject != nullptr )
    {
        m_pParentGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    // If it's in a list, remove it.
    if( this->Prev != nullptr )
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

    m_Properties.SetEnabled( false );

    SAFE_DELETE_ARRAY( m_Name );

    // Delete all children.
    while( m_ChildList.GetHead() )
    {
        delete m_ChildList.RemHead();
    }
}

void GameObject::SetGameObjectThisInheritsFrom(GameObject* pObj)
{
    // TODO: Fix prefab when this gets called.
    MyAssert( m_PrefabRef.GetPrefab() == nullptr );

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
            .addFunction( "Editor_AddNewComponent", (ComponentBase* (GameObject::*)(const char* componentName)) &GameObject::AddNewComponent ) // ComponentBase* GameObject::AddNewComponent(const char* componentName)
            .addFunction( "IsEnabled", &GameObject::IsEnabled ) // void GameObject::IsEnabled()
            .addFunction( "SetEnabled", &GameObject::SetEnabledViaEvent ) // void GameObject::SetEnabled(bool enabled, bool affectchildren)
            .addFunction( "SetName", &GameObject::SetName ) // void GameObject::SetName(const char* name)
            .addFunction( "SetParentGameObject", &GameObject::SetParentGameObject ) // void SetParentGameObject(GameObject* pNewParentGameObject);
            .addFunction( "GetTransform", &GameObject::GetTransform ) // ComponentTransform* GameObject::GetTransform()
            .addFunction( "GetName", &GameObject::GetName ) // const char* GameObject::GetName()
            .addFunction( "GetComponentByIndex", &GameObject::GetComponentByIndex_Friendly ) // GetComponentByIndex_Friendly(unsigned int index)
            .addFunction( "GetFirstComponentOfBaseType", &GameObject::GetFirstComponentOfBaseType ) // ComponentBase* GameObject::GetFirstComponentOfBaseType(BaseComponentTypes baseType)
            .addFunction( "GetFirstComponentOfType", &GameObject::GetFirstComponentOfType ) // ComponentBase* GameObject::GetFirstComponentOfType(const char* type)
            .addFunction( "GetFirstChild", &GameObject::GetFirstChild ) // GameObject* GetFirstChild()
            .addFunction( "GetNextGameObjectInList", &GameObject::GetNextGameObjectInList ) // GameObject* GetNextGameObjectInList()
            .addFunction( "ReturnToPool", &GameObject::ReturnToPool ) // void GameObject::ReturnToPool()
            .addFunction( "GetFlagStringIfSet", &GameObject::GetFlagStringIfSet ) // const char* GameObject::GetFlagStringIfSet(unsigned int bit)

            .addFunction( "GetSprite", &GameObject::GetSprite ) // ComponentSprite* GameObject::GetSprite()
            .addFunction( "GetVoxelWorld", &GameObject::GetVoxelWorld ) // ComponentVoxelWorld* GameObject::GetVoxelWorld()
            .addFunction( "Get3DCollisionObject", &GameObject::Get3DCollisionObject ) // Component3DCollisionObject* GameObject::Get3DCollisionObject()
            .addFunction( "Get2DCollisionObject", &GameObject::Get2DCollisionObject ) // Component2DCollisionObject* GameObject::Get2DCollisionObject()
            .addFunction( "GetLuaScript", &GameObject::GetLuaScript ) // ComponentLuaScript* GameObject::GetLuaScript()
            .addFunction( "GetParticleEmitter", &GameObject::GetParticleEmitter ) // ComponentParticleEmitter* GameObject::GetParticleEmitter()
            .addFunction( "GetAnimationPlayer", &GameObject::GetAnimationPlayer ) // ComponentAnimationPlayer* GameObject::GetAnimationPlayer()
            .addFunction( "Get2DAnimationPlayer", &GameObject::Get2DAnimationPlayer ) // ComponentAnimationPlayer2D* GameObject::Get2DAnimationPlayer()
            .addFunction( "GetAudioPlayer", &GameObject::GetAudioPlayer ) // ComponentAudioPlayer* GameObject::GetAudioPlayer()
            .addFunction( "GetObjectPool", &GameObject::GetObjectPool ) // ComponentObjectPool* GameObject::GetObjectPool()
        .endClass();
}
#endif //MYFW_USING_LUA

cJSON* GameObject::ExportAsJSONObject(bool saveSceneID)
{
    cJSON* jGameObject = cJSON_CreateObject();

    cJSON_AddNumberToObject( jGameObject, "ID", m_ID );

    cJSON_AddStringToObject( jGameObject, "Name", m_Name );

    // Inheritance parent can be in a different scene.
    if( m_pGameObjectThisInheritsFrom )
    {
#if MYFW_EDITOR
        // Don't save parentGO if it's the prefab.
        if( m_PrefabRef.GetPrefab() == nullptr ||
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

    if( saveSceneID )
        cJSON_AddNumberToObject( jGameObject, "SceneID", m_SceneID );

    if( m_SceneID != m_PhysicsSceneID )
        cJSON_AddNumberToObject( jGameObject, "PhysicsSceneID", m_PhysicsSceneID );

    if( m_PrefabRef.GetPrefab() != nullptr )
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
    else if( m_pComponentTransform == nullptr )
        cJSON_AddStringToObject( jGameObject, "SubType", "Logic" );

    cJSON* jProperties = m_Properties.ExportAsJSONObject( false, true );
    // If no properties were saved, don't write it out to disk.
    if( jProperties->child == nullptr )
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
        cJSONExt_AddUnsignedIntArrayToObject( jGameObject, "DeletedPrefabChildIDs", &m_DeletedPrefabChildIDs[0], (int)m_DeletedPrefabChildIDs.size() );
    }    
    if( m_DeletedPrefabComponentIDs.size() > 0 )
    {
        cJSONExt_AddUnsignedIntArrayToObject( jGameObject, "DeletedPrefabComponents", &m_DeletedPrefabComponentIDs[0], (int)m_DeletedPrefabComponentIDs.size() );
    }
#endif

    return jGameObject;
}

void GameObject::ImportFromJSONObject(cJSON* jGameObject, SceneID sceneID)
{
    // Load the correct GameObject ID.
    cJSONExt_GetUnsignedInt( jGameObject, "ID", &m_ID );

    // Deal with prefabs. // Only in editor builds, game builds don't much care.
#if MYFW_EDITOR
    cJSON* jPrefabID = cJSON_GetObjectItem( jGameObject, "PrefabID" );
    if( jPrefabID )
    {
        // If we're doing a quick-load of a file, this gameobject should already have its prefab info set up.
        if( m_PrefabRef.GetPrefab() )
        {
            // Quick-loading the file, don't load the prefab info.
        }
        else
        {
            cJSON* jPrefabFile = cJSON_GetObjectItem( jGameObject, "PrefabFile" );
            MyAssert( jPrefabFile != nullptr );

            if( jPrefabFile )
            {
                // Store the PrefabId and PrefabChildID in the gameobject so they can be used when loading is complete.
                cJSON* jPrefabChildID = cJSON_GetObjectItem( jGameObject, "PrefabChildID" );
                m_PrefabRef.StoreIDsWhileLoading( jPrefabID->valueint, jPrefabChildID ? jPrefabChildID->valueint : 0 );

                PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByFullPath( jPrefabFile->valuestring );
            
                // Prefab file load must have been initiated by scene load.
                // Might want to consider triggering a load here if it's not in the file list.
                MyAssert( pPrefabFile != nullptr );

                // If the prefab file isn't loaded yet, store the name and link to the prefab when the file is loaded.
                if( pPrefabFile->GetFile()->IsFinishedLoading() == false ) // Still loading.
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

    unsigned int parentGOID = 0;
    cJSONExt_GetUnsignedInt( jGameObject, "ParentGOID", &parentGOID );
    if( parentGOID != 0 )
    {
        GameObject* pParentGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneID, parentGOID );
        MyAssert( pParentGameObject );

        SetParentGameObject( pParentGameObject );
    }

    // LEGACY: Support for old scene files with folders in them.
    //    Now stored as "SubType", handled in ComponentSystemManager::LoadSceneFromJSON().
    cJSONExt_GetBool( jGameObject, "IsFolder", &m_IsFolder );

    cJSON* jName = cJSON_GetObjectItem( jGameObject, "Name" );
    if( jName )
    {
        SetName( jName->valuestring );
    }
    SetSceneID( sceneID, false ); // Set new scene, but don't assign a new GOID.

    m_PhysicsSceneID = m_SceneID;
    cJSONExt_GetUnsignedInt( jGameObject, "PhysicsSceneID", (unsigned int*)&m_PhysicsSceneID );

    bool enabled = true;
    cJSONExt_GetBool( jGameObject, "Enabled", &enabled );
    SetEnabled( enabled, false );

    cJSON* jProperties = cJSON_GetObjectItem( jGameObject, "Properties" );
    if( jProperties )
        m_Properties.ImportFromJSONObject( jProperties, sceneID );

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
        MyAssert( m_pGameObjectThisInheritsFrom != nullptr );
    }
}

cJSON* GameObject::ExportReferenceAsJSONObject(SceneID refSceneID)
{
    // See ComponentSystemManager::FindGameObjectByJSONRef.

    cJSON* gameobjectref = cJSON_CreateObject();

    if( refSceneID != m_SceneID )
    {
        cJSON_AddStringToObject( gameobjectref, "Scene", GetSceneInfo()->m_FullPath );
    }

    cJSON_AddNumberToObject( gameobjectref, "GOID", m_ID );

    return gameobjectref;
}

cJSON* GameObject::ExportAsJSONPrefab(PrefabObject* pPrefab, bool assignNewChildIDs, bool assignNewComponentIDs)
{
    cJSON* jGameObject = cJSON_CreateObject();

    // Back-up sceneID.
    // Set gameobject to scene zero, so any gameobject references will store the sceneID when serialized (since they will differ).
    // Set it back later, without changing gameobject id.
    SceneID sceneIDBackup = GetSceneID();
    SetSceneID( SCENEID_Unmanaged, false );

    // Don't export the ParentGOID, we'll ignore that it was parented at all.
    //// Transform/Hierarchy parent must be in the same scene.
    //if( m_pParentGameObject )
    //    cJSON_AddNumberToObject( jGameObject, "ParentGOID", m_pParentGameObject->GetID() );

    cJSON_AddStringToObject( jGameObject, "Name", GetName() );

    if( m_IsFolder == true )
        cJSON_AddStringToObject( jGameObject, "SubType", "Folder" );
    else if( m_pComponentTransform == nullptr )
        cJSON_AddStringToObject( jGameObject, "SubType", "Logic" );

    // Export the prefab this object is an instance of.
    if( m_PrefabRef.GetPrefab() != nullptr )
    {
        // Nested Prefabs must come from the same file.
        // TODO: Replace this assert with an error message.
        MyAssert( m_PrefabRef.GetPrefab()->GetPrefabFile() == pPrefab->GetPrefabFile() );

        // If this prefab inherits from another prefab, save the prefabID.
        if( m_pGameObjectThisInheritsFrom != nullptr )
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

    // Export properties, if none were saved, don't add the block to jGameObject.
    cJSON* jProperties = m_Properties.ExportAsJSONObject( false, false );
    if( jProperties->child == nullptr )
    {
        cJSON_Delete( jProperties );
    }
    else
    {
        cJSON_AddItemToObject( jGameObject, "Properties", jProperties );
    }

    // Export components.
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
    if( m_ChildList.GetHead() != nullptr )
    {
        cJSON* jChildrenArray = cJSON_CreateArray();
        cJSON_AddItemToObject( jGameObject, "Children", jChildrenArray );
        
        for( GameObject* pChildGameObject = m_ChildList.GetHead(); pChildGameObject; pChildGameObject = pChildGameObject->GetNext() )
        {
            cJSON* jChildObject = pChildGameObject->ExportAsJSONPrefab( pPrefab, assignNewChildIDs, assignNewComponentIDs );
            MyAssert( jChildObject );
            cJSON_AddItemToArray( jChildrenArray, jChildObject );

            // Add ChildID.
            {
                uint32 childID = 0;

                if( assignNewChildIDs )
                    childID = pPrefab->GetNextChildPrefabIDAndIncrement();
                else
                    childID = pChildGameObject->GetPrefabRef()->GetChildID();

                MyAssert( childID != 0 );
                cJSON_AddNumberToObject( jChildObject, "ChildID", childID );
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
    SetSceneID( sceneIDBackup, false );

    return jGameObject;
}

const char* GameObject::GetFlagStringIfSet(unsigned int bit)
{
    if( m_Properties.GetFlags() & 1 << bit )
    {
        return g_pEngineCore->GetGameObjectFlagString( bit );
    }

    return nullptr;
}

// Exposed to Lua, change elsewhere if function signature changes.
void GameObject::SetEnabledViaEvent(bool enabled, bool affectChildren)
{
    if( m_Enabled == enabled )
        return;

    // If game is running and we want to enable/disable an object, then send a message and do it at the start of the next frame.
    EventManager* pEventManager = m_pEngineCore->GetManagers()->GetEventManager();
    MyEvent* pEvent = pEventManager->CreateNewEvent( "GameObjectEnable" );
    pEvent->AttachPointer( pEventManager, "GameObject", this );
    pEvent->AttachBool( pEventManager, "Enable", enabled );
    pEvent->AttachBool( pEventManager, "AffectChildren", affectChildren );
    pEventManager->QueueEvent( pEvent );
}

void GameObject::SetEnabled(bool enabled, bool affectChildren)
{
    if( m_Enabled == enabled )
        return;

    m_Enabled = enabled;

    // Loop through all components and call OnGameObjectEnabled/OnGameObjectDisabled.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Enabled )
            m_Components[i]->OnGameObjectEnabled();
        else
            m_Components[i]->OnGameObjectDisabled();
    }

    //// Un/register all component callbacks.
    //if( m_Enabled )
    //    RegisterAllComponentCallbacks( false );
    //else
    //    UnregisterAllComponentCallbacks( false );

    // Recurse through children.
    if( affectChildren )
    {
        GameObject* pChild = GetFirstChild();

        while( pChild )
        {
            pChild->SetEnabled( enabled, true );
            pChild = pChild->GetNext();
        }
    }
}

//void GameObject::RegisterAllComponentCallbacks(bool ignoreEnabledFlag)
//{
//    // Loop through all components and register/unregister their callbacks.
//    for( unsigned int i=0; i<m_Components.Count(); i++ )
//    {
//        if( m_Components[i]->IsEnabled() || ignoreEnabledFlag )
//            m_Components[i]->RegisterCallbacks();
//    }
//}
//
//void GameObject::UnregisterAllComponentCallbacks(bool ignoreEnabledFlag)
//{
//    // Loop through all components and register/unregister their callbacks.
//    for( unsigned int i=0; i<m_Components.Count(); i++ )
//    {
//        if( m_Components[i]->IsEnabled() || ignoreEnabledFlag )
//            m_Components[i]->UnregisterCallbacks();
//    }
//}

void GameObject::SetSceneID(SceneID sceneID, bool assignNewGOID)
{
    if( m_SceneID == sceneID )
        return;

    m_SceneID = sceneID;

    // Loop through components and change the sceneID in each.
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        m_Components[i]->SetSceneID( sceneID );
    }
   
    if( assignNewGOID )
    {
        m_ID = g_pComponentSystemManager->GetNextGameObjectIDAndIncrement( sceneID );
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
        if( strcmp( m_Name, name ) == 0 ) // Name hasn't changed.
            return;

        delete[] m_Name;
    }
    
    size_t len = strlen( name );
    
    m_Name = MyNew char[len+1];
    strcpy_s( m_Name, len+1, name );
}

void GameObject::SetOriginatingPool(ComponentObjectPool* pPool)
{
    MyAssert( m_pOriginatingPool == nullptr );

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
    if( pOldParentGameObject != nullptr )
    {
        // Stop the parent's gameobject from reporting its deletion.
        pOldParentGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    // If we have a new parent:
    if( pNewParentGameObject != nullptr )
    {
        // Register with the parent's gameobject to notify us of its deletion.
        pNewParentGameObject->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        // The prefab's m_pGameObject will be null when the scene is loading but the prefab file isn't loaded.
        // Skip the check in this case since the childID should already be in the "deleted child" list.
        if( m_PrefabRef.GetGameObject() != nullptr && m_PrefabRef.IsMasterPrefabGameObject() == false )
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
        ComponentTransform* pNewParentTransform = nullptr;
        if( m_pParentGameObject != nullptr )
            pNewParentTransform = pNewParentGameObject->m_pComponentTransform;

        m_pComponentTransform->SetParentTransform( pNewParentTransform );
    }
}

bool GameObject::IsParentedTo(GameObject* pPotentialParent, bool onlyCheckDirectParent)
{
    GameObject* pParent = GetParentGameObject();

    if( pParent == pPotentialParent )
        return true;

    if( pParent == nullptr || onlyCheckDirectParent )
        return false;

    return pParent->IsParentedTo( pPotentialParent, false );
}

void GameObject::SetManaged(bool managed)
{
    MyAssert( m_Managed != managed );
    if( m_Managed == managed )
        return;

    m_Managed = managed;
}

SceneInfo* GameObject::GetSceneInfo()
{
    return g_pComponentSystemManager->GetSceneInfo( m_SceneID );
}

unsigned int GameObject::GetComponentCount()
{
    return m_Components.Count();
}

ComponentBase* GameObject::GetComponentByIndex_Friendly(unsigned int index)
{
    if( index < m_Components.Count() )
    {
        return (ComponentBase*)m_Components[index];
    }

    return nullptr; // 'index' was out of bounds.
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
        return m_Components.Count() + 2; // + properties + transform.
    }
    else
    {
        return m_Components.Count() + 1; // + properties.
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

ComponentBase* GameObject::AddNewComponent(const char* componentType)
{
    int type = m_pEngineCore->GetComponentSystemManager()->GetComponentTypeManager()->GetTypeByName( componentType );
    if( type == -1 )
    {
        LOGError( LOGTag, "AddNewComponent: Type not found: %s\n", componentType );
        return nullptr;
    }
    
    return AddNewComponent( type, m_SceneID, g_pComponentSystemManager );
}

ComponentBase* GameObject::AddNewComponent(int componentType, SceneID sceneID, ComponentSystemManager* pComponentSystemManager)
{
    MyAssert( componentType != -1 );

    if( m_Components.Count() >= m_Components.Length() )
        return nullptr;

    ComponentBase* pComponent = pComponentSystemManager->GetComponentTypeManager()->CreateComponent( componentType );
    //pComponent->SetComponentSystemManager( pComponentSystemManager );

    if( componentType == ComponentType_Transform )
    {
        // Special handling of ComponentType_Transform, only offer option if GameObject doesn't have a transform.
        //     m_pComponentTransform will be set in AddExistingComponent() below.
    }
    else
    {
        MyAssert( pComponentSystemManager );
        if( m_Managed )
        {
            pComponentSystemManager->AddComponent( pComponent );
        }
    }

    if( sceneID != SCENEID_Unmanaged )
    {
        unsigned int id = pComponentSystemManager->GetNextComponentIDAndIncrement( sceneID );
        pComponent->SetID( id );
    }

    MyAssert( sceneID == SCENEID_Unmanaged || m_SceneID == sceneID );
    pComponent->SetSceneID( sceneID );

    AddExistingComponent( pComponent, true );

    return pComponent;
}

ComponentBase* GameObject::AddExistingComponent(ComponentBase* pComponent, bool resetComponent)
{
    // Special handling for adding transform component.
    if( pComponent->IsA( "TransformComponent" ) )
    {
        m_pComponentTransform = (ComponentTransform*)pComponent;

        pComponent->SetGameObject( this );
        if( resetComponent )
            pComponent->Reset();

        // Re-parent all child transforms, if they have one.
        for( GameObject* pChildGameObject = m_ChildList.GetHead(); pChildGameObject; pChildGameObject = pChildGameObject->GetNext() )
        {
            // TODO: Recurse through children.
            if( pChildGameObject->m_pComponentTransform )
            {
                pChildGameObject->m_pComponentTransform->SetParentTransform( m_pComponentTransform );
            }
        }

        // Re-enable all renderable components.
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
            return nullptr;

        pComponent->SetGameObject( this );
        if( resetComponent )
            pComponent->Reset();

        m_Components.Add( pComponent );

        // If the component isn't already in the system managers component list, add it, whether gameobject is managed or not.
        if( pComponent->Prev == 0 )
            g_pComponentSystemManager->AddComponent( pComponent );
    }

    //// Register this components callbacks.
    //pComponent->RegisterCallbacks();

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

    return pComponent;
}

ComponentBase* GameObject::RemoveComponent(ComponentBase* pComponent)
{
    bool found = false;

    // Special handling for removing transform component.
    if( pComponent->IsA( "TransformComponent" ) )
    {
        found = true;

        // Unparent all child transforms, if they have one.
        for( GameObject* pChildGameObject = m_ChildList.GetHead(); pChildGameObject; pChildGameObject = pChildGameObject->GetNext() )
        {
            // TODO: Recurse through children.
            if( pChildGameObject->m_pComponentTransform )
            {
                pChildGameObject->m_pComponentTransform->SetParentTransform( nullptr );
            }
        }

        // Disable all renderable components.
        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
            if( m_Components[i]->IsA( "RenderableComponent" ) )
            {
                m_Components[i]->SetEnabled( false );
            }
        }

        m_pComponentTransform = nullptr;
    }
    else
    {
        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
            if( m_Components[i] == pComponent )
            {
                found = true;
                m_Components.RemoveIndex_MaintainOrder( i );

                // Remove from system managers component list.
                pComponent->Remove();
                pComponent->Prev = nullptr;
                pComponent->Next = nullptr;
            }
        }
    }

    if( found )
    {
        // Disable and unregister all this components callbacks.
        pComponent->SetEnabled( false );

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

        return pComponent;
    }

    return nullptr; // Component not found.
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
#endif //MYFW_EDITOR

    return nullptr;
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

    return nullptr;
}

// Gets the first material found.
MaterialDefinition* GameObject::GetMaterial()
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->GetBaseType() == BaseComponentType_Renderable )
        {
            MyAssert( m_Components[i]->IsA( "RenderableComponent" ) );
            return ((ComponentRenderable*)m_Components[i])->GetMaterial( 0 );
        }
    }

    return nullptr;
}

// Set the material on all renderable components attached to this object.
void GameObject::SetMaterial(MaterialDefinition* pMaterial)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->GetBaseType() == BaseComponentType_Renderable )
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
        if( m_Components[i]->GetBaseType() == BaseComponentType_Updateable )
        {
#if MYFW_USING_LUA
            ComponentLuaScript* pLuaComponent = m_Components[i]->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)m_Components[i] : nullptr;
            if( pLuaComponent )
                pLuaComponent->SetScriptFile( pFile );
#endif //MYFW_USING_LUA
        }
    }
}

void GameObject::ReturnToPool()
{
    MyAssert( m_pOriginatingPool != nullptr );

    m_pOriginatingPool->ReturnObjectToPool( this );
}

// Exposed to Lua, change elsewhere if function signature changes.
ComponentBase* GameObject::GetFirstComponentOfBaseType(BaseComponentTypes baseType)
{
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( m_Components[i]->GetBaseType() == baseType )
        {
            return m_Components[i];
        }
    }

    return nullptr; // Component not found.
}

ComponentBase* GameObject::GetNextComponentOfBaseType(ComponentBase* pLastComponent)
{
    MyAssert( pLastComponent != nullptr );

    bool foundlast = false;
    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( pLastComponent == m_Components[i] )
        {
            foundlast = true;
        }
        else if( foundlast && m_Components[i]->GetBaseType() == pLastComponent->GetBaseType() )
        {
            return m_Components[i];
        }
    }

    return nullptr; // Component not found.
}

// Exposed to Lua, change elsewhere if function signature changes.
ComponentBase* GameObject::GetFirstComponentOfType(const char* type)
{
    for( unsigned int i=0; i<GetComponentCountIncludingCore(); i++ )
    {
        ComponentBase* pComponent = GetComponentByIndexIncludingCore( i );
        if( pComponent->IsA( type ) )
            return pComponent;
    }

    return nullptr; // Component not found.
}

ComponentBase* GameObject::GetNextComponentOfType(ComponentBase* pLastComponent)
{
    MyAssert( pLastComponent != nullptr );

    bool foundLast = false;
    for( unsigned int i=0; i<GetComponentCountIncludingCore(); i++ )
    {
        ComponentBase* pComponent = GetComponentByIndexIncludingCore( i );

        if( pLastComponent == pComponent )
            foundLast = true;
        else if( foundLast && ((ComponentBase*)pComponent)->IsA( pLastComponent->GetClassname() ) )
            return (ComponentBase*)pComponent;
    }

    return nullptr; // Component not found.
}

void GameObject::RegisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc* pCallback)
{
    MyAssert( pCallback != nullptr );

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

    // TODO: Pool callback structures.
    GameObjectDeletedCallbackStruct* pCallbackStruct = MyNew GameObjectDeletedCallbackStruct;
    pCallbackStruct->pObj = pObj;
    pCallbackStruct->pFunc = pCallback;

    m_pOnDeleteCallbacks.AddTail( pCallbackStruct );
}

void GameObject::UnregisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc* pCallback)
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
        //     since the callback function might try to unregister (and delete) the callback struct.
        pCallbackStruct->Remove();

        // Call the onGameObjectDeleted callback function.
        pCallbackStruct->pFunc( pCallbackStruct->pObj, this );

        // Delete the struct.
        delete pCallbackStruct;

        pNode = pNextNode;
    }
}

void GameObject::OnGameObjectDeleted(GameObject* pGameObject)
{
    // If our parent was deleted, clear the pointer.
    MyAssert( m_pParentGameObject == pGameObject ); // The callback should have only been registered if needed.
    if( m_pParentGameObject == pGameObject )
    {
        // We're in the callback, so don't unregister the callback.
        if( m_pComponentTransform )
        {
            m_pComponentTransform->SetParentTransform( nullptr );
        }
    }
}

void GameObject::OnTransformChanged(const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor)
{
    //int bp = 1;
}

#if MYFW_EDITOR
bool GameObject::IsMissingPrefabChild(uint32 childID)
{
    MyAssert( m_pGameObjectThisInheritsFrom != nullptr );

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

        pChild = pChild->GetNext();
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

        pChild = pChild->GetNext();
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
    if( id < m_pEngineCore->GetComponentSystemManager()->GetComponentTypeManager()->GetNumberOfComponentTypes() )
    {
        if( pGameObject->m_Components.Count() >= pGameObject->m_Components.Length() )
            return;

        int type = id; // Could be EngineComponentTypes or GameComponentTypes type.

        ComponentBase* pComponent = nullptr;
        if( g_pEngineCore->IsInEditorMode() )
            pComponent = pGameObject->AddNewComponent( type, pGameObject->GetSceneID(), g_pComponentSystemManager );
        else
            pComponent = pGameObject->AddNewComponent( type, SCENEID_Unmanaged, g_pComponentSystemManager );

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

        // If the object isn't selected, delete just the one object, otherwise delete all selected objects.
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

        // If the object isn't selected, delete just the one object, otherwise delete all selected objects.
        if( pEditorState->IsGameObjectSelected( pGameObject ) )
        {
            pEditorState->DeleteSelectedObjects();
        }
        else
        {
            // Create a temp vector to pass into command.
            std::vector<GameObject*> gameobjects;
            gameobjects.push_back( pGameObject );
            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
        }
    }
    else if( id == RightClick_DeleteFolder )
    {
        // Delete all gameobjects in the folder, along with the folder itself.
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
}

void GameObject::OnDrop(int controlid, int x, int y, GameObjectOnDropActions action)
{
    std::vector<GameObject*> selectedObjects;

    // Range must match code in PanelObjectListDropTarget::OnDragOver. // TODO: fix this.
    bool setAsChild = true;

    if( action == GameObjectOnDropAction_Reorder )
    {
        setAsChild = false;
    }

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

            // Don't allow a parent to be attached to or become a sibling of this object.
            if( this->IsParentedTo( pGameObject, false ) )
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

void GameObject::FinishLoadingPrefab(PrefabFile* pPrefabFile)
{
    // Link the PrefabRef to the correct prefab and GameObject now that the file is finished loading.
    m_PrefabRef.FinishLoadingPrefab( pPrefabFile );
    if( m_pParentGameObject != nullptr )
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
    MyAssert( m_PrefabRef.GetPrefab() != nullptr );

    if( m_PrefabRef.GetChildID() == 0 )
        return this;

    return m_pParentGameObject->FindRootGameObjectOfPrefabInstance();
}

// Used when deleting prefabs.
void GameObject::Editor_SetPrefab(PrefabReference* pPrefabRef)
{
    m_PrefabRef = *pPrefabRef;
    m_pGameObjectThisInheritsFrom = m_PrefabRef.GetGameObject();
}

void GameObject::Editor_SetGameObjectAndAllChildrenToInheritFromPrefab(PrefabObject* pPrefab, uint32 prefabChildID)
{
    PrefabReference ref( pPrefab, prefabChildID, true );
    m_PrefabRef = ref;
    m_pGameObjectThisInheritsFrom = m_PrefabRef.GetGameObject();

    // Set children.
    GameObject* pChildGO = GetChildList()->GetHead();
    GameObject* pPrefabChildGO = pPrefab->GetGameObject()->GetChildList()->GetHead();
    while( pChildGO )
    {
        // Temp assert, test nested prefabs and replace with an 'if'.
        MyAssert( pPrefabChildGO->GetPrefabRef()->GetPrefab() == pPrefab );

        uint32 prefabChildChildID = pPrefabChildGO->GetPrefabRef()->GetChildID();
        pChildGO->Editor_SetGameObjectAndAllChildrenToInheritFromPrefab( pPrefab, prefabChildChildID );

        pChildGO = pChildGO->GetNext();
        pPrefabChildGO = pPrefabChildGO->GetNext();
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
        if( m_Components[i]->GetBaseType() == BaseComponentType_Renderable )
        {
            MyAssert( m_Components[i]->IsA( "RenderableComponent" ) );

            ComponentRenderable* pRenderable = (ComponentRenderable*)m_Components[i];

            if( pRenderable )
            {
                // TODO: Deal with more than just the first submeshes material.
                int submeshIndex = 0;

                // Go through same code to drop a material on the component, so inheritance and undo/redo will be handled.
                ComponentVariable* pVar = pRenderable->GetComponentVariableForMaterial( submeshIndex );

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
        for( GameObject* pGameObject = GetChildList()->GetHead(); pGameObject; pGameObject = pGameObject->GetNext() )
        {
            pGameObject->AddToList( pList );
        }
    }
}
#endif //MYFW_EDITOR
