//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#include "PrefabManager.h"

GameObject::GameObject(bool managed, int sceneid, bool isfolder, bool hastransform, PrefabObject* pPrefab)
{
    ClassnameSanityCheck();

    m_pGameObjectThisInheritsFrom = 0;

    m_pParentGameObject = 0;

    m_Properties.SetEnabled( false );
    m_Properties.m_pGameObject = this;

    m_Enabled = true;
    m_pPrefab = pPrefab;
    m_IsFolder = isfolder;
    m_SceneID = sceneid;
    m_ID = 0;
    m_PhysicsSceneID = sceneid;
    m_Name = 0;

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

    // Delete components
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
            .addFunction( "GetParticleEmitter", &GameObject::GetParticleEmitter )
            .addFunction( "GetVoxelWorld", &GameObject::GetVoxelWorld )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void GameObject::OnTitleLabelClicked(int controlid, bool finishedchanging)
{
    g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_EnableObject( this, !m_Enabled ) );
    g_pPanelWatch->SetNeedsRefresh();
}

void GameObject::OnLeftClick(unsigned int count, bool clear)
{
    // select this GameObject in the editor window.
    if( g_pEngineCore->m_pEditorState == 0 )
        return;

    if( g_pEngineCore->m_pEditorState->IsGameObjectSelected( this ) == false )
        g_pEngineCore->m_pEditorState->m_pSelectedObjects.push_back( this );

    // if this is a folder, select all objects inside
    if( m_IsFolder )
    {
        for( CPPListNode* pNode = m_ChildList.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            // TODO: recurse through children
            g_pEngineCore->m_pEditorState->m_pSelectedObjects.push_back( (GameObject*)pNode );
        }
    }

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

    m_Properties.FillPropertiesWindow( false );

    if( m_pComponentTransform )
    {
        if( count <= 1 )
            m_pComponentTransform->m_MultiSelectedComponents.clear();

        m_pComponentTransform->FillPropertiesWindow( false, true );
    }

    for( unsigned int i=0; i<m_Components.Count(); i++ )
    {
        if( count <= 1 )
            m_Components[i]->m_MultiSelectedComponents.clear();

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
    
#if _DEBUG // TODO: enable this in release mode once it's further along.
        wxMenu* prefabmenu = MyNew wxMenu;
        menu.AppendSubMenu( prefabmenu, "Create Prefab in" );

        unsigned int numprefabfiles = g_pComponentSystemManager->m_pPrefabManager->GetNumberOfFiles();
        for( unsigned int i=0; i<numprefabfiles; i++ )
        {
            PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByIndex( i );
            MyFileObject* pFile = pPrefabFile->GetFile();
            MyAssert( pFile != 0 );

            prefabmenu->Append( RightClick_CreatePrefab + i, pFile->m_FilenameWithoutExtension );
        }

        prefabmenu->Append( RightClick_CreatePrefab + numprefabfiles, "New/Load Prefab file..." );
#endif

        menu.Append( RightClick_DeleteGameObject, "Delete GameObject" );
    }
    else
    {
        menu.Append( RightClick_DuplicateFolder, "Duplicate Folder and all contents" );
        menu.Append( RightClick_DeleteFolder, "Delete Folder and all contents" );
    }

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

        ComponentBase* pComponent = 0;
        if( g_pEngineCore->m_EditorMode )
            pComponent = pGameObject->AddNewComponent( type, pGameObject->GetSceneID() );
        else
            pComponent = pGameObject->AddNewComponent( type, 0 );

        pComponent->OnLoad();
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
            g_pComponentSystemManager->m_pPrefabManager->CreatePrefabInFile( fileindex, pGameObject->GetName(), pGameObject );
        }
    }
    else if( id == RightClick_DeleteGameObject )
    {
        EditorState* pEditorState = g_pEngineCore->m_pEditorState;

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
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
        }
    }
    else if( id == RightClick_DeleteFolder )
    {
        // delete all gameobjects in the folder, along with the folder itself.
        std::vector<GameObject*> gameobjects;

        gameobjects.push_back( pGameObject ); // the folder itself.
        for( CPPListNode* pNode = pGameObject->m_ChildList.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            // TODO: recurse through children
            gameobjects.push_back( (GameObject*)pNode );
        }

        g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
    }
    else if( id == RightClick_DuplicateFolder )
    {
        if( g_pEngineCore->m_EditorMode )
            g_pComponentSystemManager->EditorCopyGameObject( pGameObject, false );
        else
            g_pComponentSystemManager->CopyGameObject( pGameObject, "runtime duplicate" );
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

        // if we're dropping this object on itself, kick out.
        if( pGameObject == this )
            return;

        // Change the dropped gameobject's sceneid to match this one.
        pGameObject->SetSceneID( this->GetSceneID() );

        // if you drop a game object on another, parent them or move above/below depending on the "y"
        wxTreeItemId treeid = g_pPanelObjectList->FindObject( this );
        wxRect rect;
        g_pPanelObjectList->m_pTree_Objects->GetBoundingRect( treeid, rect, false );

        // range must match code in PanelObjectListDropTarget::OnDragOver // TODO: fix this
        if( y > rect.GetBottom() - 10 )
        {
            // move below the selected item
            g_pPanelObjectList->Tree_MoveObject( pGameObject, this, false );
            pGameObject->MoveAfter( this );
            GameObject* thisparent = this->GetParentGameObject();
            pGameObject->SetParentGameObject( thisparent );
        }
        else
        {
            // Parent the object dropped to this.
            pGameObject->SetParentGameObject( this );

            // move as first item in parent
            g_pPanelObjectList->Tree_MoveObject( pGameObject, this, true );
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

void GameObject::UpdateObjectListIcon()
{
    // Set the icon for the gameobject in the objectlist panel tree.
    int iconindex = ObjectListIcon_GameObject;
    if( m_pPrefab != 0 )
        iconindex = ObjectListIcon_Prefab;
    else if( m_IsFolder )
        iconindex = ObjectListIcon_Folder;
    else if( m_pComponentTransform == 0 )
        iconindex = ObjectListIcon_LogicObject;

    wxTreeItemId gameobjectid = g_pPanelObjectList->FindObject( this );
    if( gameobjectid.IsOk() )
        g_pPanelObjectList->SetIcon( gameobjectid, iconindex );
}

void GameObject::FinishLoadingPrefab(PrefabFile* pPrefabFile, uint32 prefabid)
{
    // link to the correct prefab
    m_pPrefab = pPrefabFile->GetPrefabByID( prefabid );

    // TODO: check the if the gameobect(s) in the prefab are completely different and deal with it

    // otherwise, importing same prefab, so update all undivorced variables to match prefab file
    {
        GameObject* pPrefabGameObject = m_pPrefab->GetGameObject();
        MyAssert( pPrefabGameObject );

        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
            ComponentBase* pComponent = m_Components[i];
            ComponentBase* pPrefabComponent = pPrefabGameObject->m_Components[i];

            pComponent->SyncUndivorcedVariables( pPrefabComponent );
        }
    }

    UpdateObjectListIcon();
}

void GameObject::OnPrefabFileFinishedLoading(MyFileObject* pFile)
{
    PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetPrefabFileForFileObject( pFile->m_FullPath );
    FinishLoadingPrefab( pPrefabFile, m_PrefabID );

    pFile->UnregisterFileFinishedLoadingCallback( this );
}
#endif //MYFW_USING_WX

cJSON* GameObject::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jGameObject = cJSON_CreateObject();

    cJSON_AddNumberToObject( jGameObject, "ID", m_ID );

    cJSON_AddStringToObject( jGameObject, "Name", m_Name );

    // Inheritance parent can be in a different scene.
    if( m_pGameObjectThisInheritsFrom )
        cJSON_AddItemToObject( jGameObject, "ParentGO", m_pGameObjectThisInheritsFrom->ExportReferenceAsJSONObject( m_SceneID ) );

    // Transform/Heirarchy parent must be in the same scene.
    if( m_pParentGameObject )
        cJSON_AddNumberToObject( jGameObject, "ParentGOID", m_pParentGameObject->GetID() );

    if( m_Enabled == false )
        cJSON_AddNumberToObject( jGameObject, "Enabled", m_Enabled );

    if( savesceneid )
        cJSON_AddNumberToObject( jGameObject, "SceneID", m_SceneID );

    if( m_SceneID != m_PhysicsSceneID )
        cJSON_AddNumberToObject( jGameObject, "PhysicsSceneID", m_PhysicsSceneID );

    if( m_pPrefab != 0 )
    {
        cJSON_AddStringToObject( jGameObject, "PrefabFile", m_pPrefab->GetPrefabFile()->GetFile()->m_FullPath );
        cJSON_AddNumberToObject( jGameObject, "PrefabID", m_pPrefab->GetID() );
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

    return jGameObject;
}

void GameObject::ImportFromJSONObject(cJSON* jGameObject, unsigned int sceneid)
{
    // Deal with prefabs // only in editor builds, game builds don't much care.
#if MYFW_USING_WX
    cJSON* jPrefabID = cJSON_GetObjectItem( jGameObject, "PrefabID" );
    if( jPrefabID )
    {
        cJSON* jPrefabFile = cJSON_GetObjectItem( jGameObject, "PrefabFile" );
        MyAssert( jPrefabFile != 0 );

        if( jPrefabFile )
        {
            m_PrefabID = jPrefabID->valueint;

            PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetPrefabFileForFileObject( jPrefabFile->valuestring );
            
            // prefab file load must have been initiated by scene load
            // might want to consider triggering a load here if it's not in the file list.
            MyAssert( pPrefabFile != 0 );

            // if the prefab file isn't loaded yet, store the name and link to the prefab when the file is loaded
            if( true )
            {
                pPrefabFile->GetFile()->RegisterFileFinishedLoadingCallback( this, StaticOnPrefabFileFinishedLoading );
            }
            else
            {
                FinishLoadingPrefab( pPrefabFile, m_PrefabID );
            }
        }
    }
#endif // MYFW_USING_WX

    cJSON* jParentGO = cJSON_GetObjectItem( jGameObject, "ParentGO" );
    if( jParentGO )
    {
        m_pGameObjectThisInheritsFrom = g_pComponentSystemManager->FindGameObjectByJSONRef( jParentGO, m_SceneID );

        // if this trips, then other object might be loaded after this or come from another scene that isn't loaded.
        MyAssert( m_pGameObjectThisInheritsFrom != 0 );
    }

    unsigned int parentgoid = 0;
    cJSONExt_GetUnsignedInt( jGameObject, "ParentGOID", &parentgoid );
    if( parentgoid != 0 )
    {
        GameObject* pParentGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, parentgoid );
        SetParentGameObject( pParentGameObject );
    }

    // LEGACY: support for old scene files with folders in them
    //    now stored as "SubType", handled in ComponentSystemManager::LoadSceneFromJSON()
    cJSONExt_GetBool( jGameObject, "IsFolder", &m_IsFolder );

    cJSONExt_GetUnsignedInt( jGameObject, "ID", &m_ID );

    cJSON* jName = cJSON_GetObjectItem( jGameObject, "Name" );
    if( jName )
    {
        SetName( jName->valuestring );
    }
    SetSceneID( sceneid, false ); // set new scene, but don't assign a new GOID.

    m_PhysicsSceneID = m_SceneID;
    cJSONExt_GetUnsignedInt( jGameObject, "PhysicsSceneID", &m_PhysicsSceneID );

    bool enabled = true;
    cJSONExt_GetBool( jGameObject, "Enabled", &enabled );
    SetEnabled( enabled );

    cJSON* jProperties = cJSON_GetObjectItem( jGameObject, "Properties" );
    if( jProperties )
        m_Properties.ImportFromJSONObject( jProperties, sceneid );
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

cJSON* GameObject::ExportAsJSONPrefab()
{
    cJSON* jGameObject = cJSON_CreateObject();

    // Back-up sceneid.
    // Set gameobject to scene zero, so any gameobject references will store the sceneid when serialized (since they will differ)
    // Set it back later, without changing gameobject id.
    unsigned int sceneidbackup = GetSceneID();
    SetSceneID( 0, false );

    // Transform/Heirarchy parent must be in the same scene.
    if( m_pParentGameObject )
        cJSON_AddNumberToObject( jGameObject, "ParentGOID", m_pParentGameObject->GetID() );

    if( m_IsFolder == true )
        cJSON_AddStringToObject( jGameObject, "SubType", "Folder" );
    else if( m_pComponentTransform == false )
        cJSON_AddStringToObject( jGameObject, "SubType", "Logic" );

    cJSON* jProperties = m_Properties.ExportAsJSONObject( false, false );
    // if no properties were saved, don't write it out to disk
    if( jProperties->child == 0 )
    {
        cJSON_Delete( jProperties );
    }
    else
    {
        cJSON_AddItemToObject( jGameObject, "Properties", jProperties );
    }

    // export components
    if( m_Components.Count() > 0 )
    {
        cJSON* jComponentArray = cJSON_CreateArray();
        cJSON_AddItemToObject( jGameObject, "Components", jComponentArray );
        for( unsigned int i=0; i<m_Components.Count(); i++ )
        {
            cJSON* jComponent = m_Components[i]->ExportAsJSONObject( false, false );

            cJSON_AddItemToArray( jComponentArray, jComponent );
        }
    }

    // Reset scene id to original value, don't change the gameobjectid.
    SetSceneID( sceneidbackup, false );

    return jGameObject;
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

void GameObject::SetSceneID(unsigned int sceneid, bool assignnewgoid)
{
    if( m_SceneID == sceneid )
        return;

    m_SceneID = sceneid;

    // Loop through components and change the sceneid in each
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
    // if the old parent is the same as the new one, kick out.
    if( m_pParentGameObject == pParentGameObject )
        return;

    // if we had an old parent:
    if( m_pParentGameObject != 0 )
    {
        // stop it's gameobject from reporting it's deletion
        m_pParentGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    m_pParentGameObject = pParentGameObject;

    // if we have a new parent
    if( m_pParentGameObject != 0 )
    {
        // register the gameobject of the parent to notify us of it's deletion.
        pParentGameObject->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        pParentGameObject->m_ChildList.MoveTail( this );
    }
    else
    {
        g_pComponentSystemManager->GetSceneInfo( m_SceneID )->m_GameObjects.MoveTail( this );
    }

    // parent one transform to another, if there are transforms.
    if( m_pComponentTransform )
    {
        ComponentTransform* pNewParentTransform = 0;
        if( m_pParentGameObject != 0 )
            pNewParentTransform = pParentGameObject->m_pComponentTransform;

        m_pComponentTransform->SetParentTransform( pNewParentTransform );
    }
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
            UpdateObjectListIcon();
            
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

    if( componenttype == ComponentType_Transform )
    {
        // Special handling of ComponentType_Transform, only offer option if GameObject doesn't have a transform
        //     m_pComponentTransform will be set in AddExistingComponent() below.
#if MYFW_USING_WX
        // Update the icon
        UpdateObjectListIcon();
        pComponent->m_Type = -1; // hack, all transforms have -1 as type, setting this to be consistent.
#endif //MYFW_USING_WX
    }
    else
    {
        MyAssert( pComponentSystemManager );
        if( m_Managed )
        {
            pComponentSystemManager->AddComponent( pComponent );
        }
    }

    if( sceneid != 0 )
    {
        unsigned int id = pComponentSystemManager->GetNextComponentIDAndIncrement( sceneid );
        pComponent->SetID( id );
    }

    MyAssert( sceneid == 0 || m_SceneID == sceneid );
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
            GameObject* pChildObject = (GameObject*)pNode;

            if( pChildObject->m_pComponentTransform )
            {
                pChildObject->m_pComponentTransform->SetParentTransform( m_pComponentTransform );
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

    MyAssert( pComponent->GetSceneID() == 0 || m_SceneID == pComponent->GetSceneID() );

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
            GameObject* pChildObject = (GameObject*)pNode;

            if( pChildObject->m_pComponentTransform )
            {
                pChildObject->m_pComponentTransform->SetParentTransform( 0 );
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
    int bp = 1;
}
