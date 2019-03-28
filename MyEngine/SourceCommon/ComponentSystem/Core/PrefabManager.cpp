//
// Copyright (c) 2017-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "PrefabManager.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/EngineEditorCommands.h"
#endif

// ============================================================================================================================
// PrefabReference
// ============================================================================================================================

PrefabReference::PrefabReference()
{
    m_pPrefab = 0;
    m_PrefabID = 0;

    m_ChildID = 0;
    m_pGameObject = 0;

    m_pOriginalPrefabParent = 0;

    m_IsMasterPrefabGameObject = false;
}

PrefabReference::PrefabReference(PrefabObject* pPrefab, uint32 childid, bool setgameobject)
{
    MyAssert( m_pPrefab != 0 );

    m_pPrefab = pPrefab;
    m_PrefabID = 0;

    m_ChildID = childid;
    m_pGameObject = 0;

    m_pOriginalPrefabParent = 0;

#if MYFW_EDITOR
    if( setgameobject )
        m_pGameObject = m_pPrefab->FindChildGameObject( childid );
#endif
}

void PrefabReference::StoreIDsWhileLoading(uint32 prefabid, uint32 childid)
{
    MyAssert( m_pPrefab == 0 );
    MyAssert( m_PrefabID == 0 );
    MyAssert( m_pGameObject == 0 );
    MyAssert( m_ChildID == 0 );

    m_PrefabID = prefabid;
    m_ChildID = childid;
}

void PrefabReference::FinishLoadingPrefab(PrefabFile* pPrefabFile)
{
    MyAssert( m_pPrefab == 0 );
    MyAssert( m_PrefabID != 0 );
    MyAssert( m_pGameObject == 0 );

    // Link to the correct prefab.
    m_pPrefab = pPrefabFile->GetPrefabByID( m_PrefabID );
    MyAssert( m_pPrefab );

#if MYFW_EDITOR
    // Find the correct GameObject from the prefab.
    m_pGameObject = m_pPrefab->GetGameObject( m_ChildID );
    MyAssert( m_pGameObject );
#endif
}

void PrefabReference::SetOriginalParent(GameObject* pGameObject)
{
    MyAssert( m_pOriginalPrefabParent == 0 );
    MyAssert( m_ChildID != 0 );

    m_pOriginalPrefabParent = pGameObject;
}

// ============================================================================================================================
// PrefabObject
// ============================================================================================================================

PrefabObject::PrefabObject()
{
    m_Name[0] = 0;
    m_jPrefab = 0;
    m_pPrefabFile = 0;

    m_PrefabID = 0;

#if MYFW_EDITOR
    m_NextChildPrefabID = 1;

    m_pGameObject = 0;
#endif
}

PrefabObject::~PrefabObject()
{
#if MYFW_EDITOR
    if( m_jPrefab )
    {
        cJSON_Delete( m_jPrefab );
    }

    delete m_pGameObject;
#endif
}

void PrefabObject::Init(PrefabFile* pFile, const char* name, uint32 prefabid)
{
    m_pPrefabFile = pFile;
    SetName( name );
    m_PrefabID = prefabid;
}

void PrefabObject::SetName(const char* name)
{
    strcpy_s( m_Name, MAX_PREFAB_NAME_LENGTH, name );

#if MYFW_USING_WX
    g_pPanelObjectList->RenameObject( this, m_Name );
#endif
}

void PrefabObject::SetPrefabJSONObject(cJSON* jPrefab, bool createmastergameobjects)
{
    if( m_jPrefab )
    {
        cJSON_Delete( m_jPrefab );
#if MYFW_EDITOR
        if( createmastergameobjects )
        {
            delete m_pGameObject;
        }
#endif
    }

    m_jPrefab = jPrefab;

#if MYFW_EDITOR
    // In editor, create a GameObject for this prefab, also creates a GameObject for each child.
    //     Pointers to children are stored in a list inside GameObject.
    // This might cause some undo actions, so wipe them out once the load is complete.
    if( createmastergameobjects )
    {
        unsigned int numItemsInUndoStack = g_pGameCore->GetCommandStack()->GetUndoStackSize();

        m_pGameObject = g_pComponentSystemManager->CreateGameObjectFromPrefab( this, false, SCENEID_Unmanaged );
        m_pGameObject->SetEnabled( false, true );

        g_pGameCore->GetCommandStack()->ClearUndoStack( numItemsInUndoStack );

#if MYFW_USING_WX
        // Add the prefab and all it's children to the object panel.
        AddToObjectList( m_pPrefabFile->m_TreeID, m_jPrefab, m_pGameObject );
#endif
    }
#endif //MYFW_EDITOR
}

void PrefabObject::SetPrefabID(uint32 prefabid)
{
    m_PrefabID = prefabid;
}

const char* PrefabObject::GetName()
{
    return m_Name;
}

cJSON* PrefabObject::GetJSONObject()
{
    return m_jPrefab;
}

uint32 PrefabObject::GetNextChildPrefabIDAndIncrement()
{
    m_NextChildPrefabID++;

    return m_NextChildPrefabID - 1;
}

#if MYFW_EDITOR
GameObject* PrefabObject::GetGameObject(uint32 childid)
{
    return FindChildGameObject( childid );
}

GameObject* PrefabObject::FindChildGameObject(uint32 childid, GameObject* pRootObject)
{
    if( pRootObject == 0 )
        pRootObject = m_pGameObject;

    MyAssert( pRootObject );

    // Return the root prefab gameobject.
    if( childid == 0 )
        return pRootObject;

    // Search through children to find the correct gameobject.
    GameObject* pNextGameObject;
    for( GameObject* pGameObject = pRootObject->GetChildList()->GetHead(); pGameObject != 0; pGameObject = pNextGameObject )
    {
        pNextGameObject = pGameObject->GetNext();

        if( pGameObject->GetPrefabRef()->GetChildID() == childid )
            return pGameObject;

        // Recurse through children.
        GameObject* pFoundGameObject = FindChildGameObject( childid, pGameObject );
        if( pFoundGameObject )
            return pFoundGameObject;
    }

    return 0;
}

#if MYFW_USING_WX
void PrefabObject::AddToObjectList(wxTreeItemId parent, cJSON* jPrefab, GameObject* pGameObject) // Used when prefab created and by undo/redo to add/remove from tree
{
    cJSON* jName = cJSON_GetObjectItem( jPrefab, "Name" );
    MyAssert( jName ); // If this trips, prefab file is likely old, every object should now have a name field.

    // Non-comprehensive assert that this gameobject is the same as the json object we're about to add.
    MyAssert( strcmp( pGameObject->GetName(), jName->valuestring ) == 0 );

    // Add this object to the list.
    wxTreeItemId treeID = g_pPanelObjectList->AddObject( this, PrefabObject::StaticOnLeftClick,
                                                         PrefabObject::StaticOnRightClick, parent,
                                                         jName->valuestring, ObjectListIcon_Prefab );

    // Set the correct GameObject as user data for this tree item.
    g_pPanelObjectList->SetUserData( treeID, pGameObject );

    // If this is the root of the prefab, store the tree id and add drag/drop functionality.
    // TODO: drag/drop children, not sure why, but why not?
    if( parent == m_pPrefabFile->m_TreeID )
    {
        m_TreeID = treeID;
        g_pPanelObjectList->SetDragAndDropFunctions( treeID, PrefabObject::StaticOnDrag, PrefabObject::StaticOnDrop );
    }

    // Add children to the list.
    cJSON* jChildrenArray = cJSON_GetObjectItem( jPrefab, "Children" );
    if( jChildrenArray )
    {
        int childarraysize = cJSON_GetArraySize( jChildrenArray );

        // Get the child list from the GameObject, this should line up with the children in the json struct.
        CPPListHead* pChildGameObjectList = pGameObject->GetChildList();
        GameObject* pChildGameObject = pChildGameObjectList->GetHead();

        for( int i=0; i<childarraysize; i++ )
        {
            cJSON* jChildGameObject = cJSON_GetArrayItem( jChildrenArray, i );
            AddToObjectList( treeID, jChildGameObject, pChildGameObject );

            pChildGameObject = pChildGameObject->GetNext();
        }
    }
}
#endif //MYFW_USING_WX

void PrefabObject::RebuildPrefabJSONObjectFromMasterGameObject()
{
    // Rebuild the JSON objects without changing any childid's
    cJSON* jGameObject = m_pGameObject->ExportAsJSONPrefab( this, false, false );
    SetPrefabJSONObject( jGameObject, false );

    m_pPrefabFile->SetHasAnythingChanged();
}

#if MYFW_USING_WX
void PrefabObject::OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear) // StaticOnLeftClick
{
    g_pPanelWatch->ClearAllVariables();

    // GameObject::OnLeftClick will check which objects in the tree are selected.
    GameObject::OnLeftClick( 1, false );
}

void PrefabObject::OnRightClick(wxTreeItemId treeid) // StaticOnRightClick
{
 	wxMenu menu;
    menu.SetClientData( &m_WxEventHandler );

    m_WxEventHandler.m_pPrefabObject = this;

    MyAssert( treeid.IsOk() );
    wxString itemname = g_pPanelObjectList->m_pTree_Objects->GetItemText( treeid );

    // Count how many prefabs are selected, ignore other selected objects that aren't prefabs.
    int numprefabsselected = 0;
    EditorState* pEditorState = g_pEngineCore->GetEditorState();
    for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
    {
        PrefabObject* pSelectedPrefab = g_pComponentSystemManager->m_pPrefabManager->FindPrefabContainingGameObject( pEditorState->m_pSelectedObjects[i] );
        if( pSelectedPrefab )
        {
            numprefabsselected++;
        }
    }

    if( numprefabsselected > 1 )
    {
        menu.Append( PrefabObjectWxEventHandler::RightClick_DeletePrefab, "Delete prefabs" );
    }
    else
    {
        menu.Append( PrefabObjectWxEventHandler::RightClick_DeletePrefab, "Delete prefab" );
    }

    //wxMenu* templatesmenu = MyNew wxMenu;
    //menu.AppendSubMenu( templatesmenu, "Add Game Object Template" );
    //AddGameObjectTemplatesToMenu( templatesmenu, 0 );

    //menu.Append( RightClick_AddFolder, "Add Folder" );
    //menu.Append( RightClick_AddLogicGameObject, "Add Logical Game Object" );
    //menu.Append( RightClick_UnloadScene, "Unload scene" );

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&PrefabObjectWxEventHandler::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void PrefabObjectWxEventHandler::OnPopupClick(wxEvent &evt)
{
    PrefabObjectWxEventHandler* pEvtHandler = (PrefabObjectWxEventHandler*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    PrefabObject* pPrefabObject = pEvtHandler->m_pPrefabObject;

    int id = evt.GetId();
    if( id == RightClick_DeletePrefab )
    {
        // Create a temp vector to pass into command.
        std::vector<PrefabObject*> prefabs;

        EditorState* pEditorState = g_pEngineCore->GetEditorState();
        if( pEditorState->m_pSelectedObjects.size() > 0 )
        {
            for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
            {
                PrefabObject* pSelectedPrefab = g_pComponentSystemManager->m_pPrefabManager->FindPrefabContainingGameObject( pEditorState->m_pSelectedObjects[i] );
                if( pSelectedPrefab )
                {
                    prefabs.push_back( pSelectedPrefab );
                }
            }
        }
        else
        {
            prefabs.push_back( pPrefabObject );
        }

        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeletePrefabs( prefabs ) );
    }
}

void PrefabObject::OnDrag()
{
    g_DragAndDropStruct.Add( (DragAndDropTypes)DragAndDropTypeEngine_Prefab, this );
}

void PrefabObject::OnDrop(int controlid, int x, int y)
{
}
#endif //MYFW_USING_WX
#endif //MYFW_EDITOR

// ============================================================================================================================
// PrefabFile
// ============================================================================================================================

PrefabFile::PrefabFile(MyFileObject* pFile)
{
    MyAssert( pFile != 0 );

    m_NextPrefabID = 1;
    m_NextPrefabComponentID = 1;
    m_pFile = pFile;

    pFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoading );

#if MYFW_USING_WX
    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    m_TreeID = g_pPanelObjectList->AddObject( this, PrefabFile::StaticOnLeftClick, PrefabFile::StaticOnRightClick, rootid, m_pFile->GetFilenameWithoutExtension(), ObjectListIcon_Scene );
    //g_pPanelObjectList->SetDragAndDropFunctions( treeid, PrefabFile::StaticOnDrag, PrefabFile::StaticOnDrop );
    g_pPanelObjectList->SetIcon( m_TreeID, ObjectListIcon_Prefab );
#endif
}

PrefabFile::~PrefabFile()
{
    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_Prefabs.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        PrefabObject* pPrefab = (PrefabObject*)pNode;

        // delete the cJSON* jPrefab object, it should have been detached from any cJSON branch when loaded/saved
        cJSON_Delete( pPrefab->m_jPrefab );
        pPrefab->m_jPrefab = 0;

#if MYFW_EDITOR
        RemovePrefab( pPrefab );
#endif

        delete pPrefab;
    }

    m_pFile->Release();

#if MYFW_USING_WX
    g_pPanelObjectList->RemoveObject( this );
#endif
}

uint32 PrefabFile::GetNextPrefabIDAndIncrement()
{
    m_NextPrefabID++;

    return m_NextPrefabID - 1;
}

uint32 PrefabFile::GetNextPrefabComponentIDAndIncrement()
{
    m_NextPrefabComponentID++;

    return m_NextPrefabComponentID - 1;
}

PrefabObject* PrefabFile::GetFirstPrefabByName(const char* name)
{
    for( CPPListNode* pNode = m_Prefabs.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        PrefabObject* pPrefab = (PrefabObject*)pNode;

        if( strcmp( pPrefab->GetName(), name ) == 0 )
        {
            return pPrefab;
        }
    }

    return 0;
}

PrefabObject* PrefabFile::GetPrefabByID(uint32 prefabid)
{
    for( CPPListNode* pNode = m_Prefabs.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        PrefabObject* pPrefab = (PrefabObject*)pNode;

        if( pPrefab->GetID() == prefabid )
        {
            return pPrefab;
        }
    }

    return 0;
}

uint32 FindHighestPrefabComponentID(GameObject* pGameObject)
{
    uint32 highestID = 0;

#if MYFW_EDITOR
    // Check each component.
    for( unsigned int i=0; i<pGameObject->GetComponentCountIncludingCore(); i++ )
    {
        uint32 ID = pGameObject->GetComponentByIndexIncludingCore( i )->GetPrefabComponentID();
        if( ID > highestID )
            highestID = ID;
    }

    // Recurse through children.
    GameObject* pChild = pGameObject->GetFirstChild();
    while( pChild )
    {
        uint32 ID = FindHighestPrefabComponentID( pChild );
        if( ID > highestID )
            highestID = ID;

        pChild = pChild->GetNext();
    }
#endif //MYFW_EDITOR

    return highestID;
}

void PrefabFile::OnFileFinishedLoading(MyFileObject* pFile)
{
#if MYFW_EDITOR
    pFile->UnregisterFileFinishedLoadingCallback( this );

    cJSON* jRoot = cJSON_Parse( pFile->GetBuffer() );

    cJSON* jPrefab = jRoot->child;
    while( jPrefab )
    {
        cJSON* jNextPrefab = jPrefab->next;

        // Add the prefab to our array
        PrefabObject* pPrefab = new PrefabObject();
        m_Prefabs.AddTail( pPrefab );

        // Deal with the prefab id, increment out counter if the one found in the file is bigger
        uint32 prefabid = 0;
        cJSONExt_GetUnsignedInt( jPrefab, "ID", &prefabid );

        if( prefabid >= m_NextPrefabID )
            m_NextPrefabID = prefabid + 1;

        // if PrefabID is zero or a duplicate of other objects id, things will break
        // TODO? check for duplicates as well and pop up warning in editor
        MyAssert( prefabid != 0 );

        // Initialize the actual PrefabObject
        cJSON* jPrefabObject = cJSON_GetObjectItem( jPrefab, "Object" );

        pPrefab->Init( this, jPrefab->string, prefabid );
        pPrefab->SetPrefabJSONObject( jPrefabObject, true );

        // Find highest PrefabComponentID in this GameObject.
        {
            GameObject* pGameObject = pPrefab->GetGameObject();
            uint32 highestID = FindHighestPrefabComponentID( pGameObject );
            
            if( highestID >= m_NextPrefabComponentID )
                m_NextPrefabComponentID = highestID + 1;
        }

        // TODO: Once prefab editing is a thing, make sure m_NextChildPrefabID is one higher than highest found. 
        //if( childprefabid >= pPrefab->m_NextChildPrefabID )
        //    pPrefab->m_NextChildPrefabID = childprefabid + 1;

        // Detach the object from the json branch.  We don't want it deleted since it's stored in the PrefabObject
        cJSON_DetachItemFromObject( jPrefab, "Object" );

        jPrefab = jNextPrefab;
    }

    cJSON_Delete( jRoot );
#else
    MyAssert( false );
#endif //MYFW_EDITOR
}

#if MYFW_EDITOR
void PrefabFile::Save()
{
    cJSON* jRoot = cJSON_CreateObject();

    for( CPPListNode* pNode = m_Prefabs.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        PrefabObject* pPrefab = (PrefabObject*)pNode;

        cJSON* jPrefabObject = cJSON_CreateObject();
        cJSON_AddItemToObject( jRoot, pPrefab->m_Name, jPrefabObject );
        cJSON_AddNumberToObject( jPrefabObject, "ID", pPrefab->m_PrefabID );
        cJSON_AddItemToObject( jPrefabObject, "Object", pPrefab->m_jPrefab );
    }

    char* jsonstring = cJSON_Print( jRoot );

    FILE* pFile;
#if MYFW_WINDOWS
    fopen_s( &pFile, m_pFile->GetFullPath(), "wb" );
#else
    pFile = fopen( m_pFile->GetFullPath(), "wb" );
#endif
    fprintf( pFile, "%s", jsonstring );
    fclose( pFile );
    
    cJSONExt_free( jsonstring );
    
    // Detach "Object" from the json branch since we're storing it in m_Prefabs[i]->m_jPrefab and will delete elsewhere
    for( CPPListNode* pNode = m_Prefabs.GetHead(); pNode != 0; pNode = pNode->GetNext() )
    {
        PrefabObject* pPrefab = (PrefabObject*)pNode;

        cJSON* jPrefabObject = cJSON_GetObjectItem( jRoot, pPrefab->m_Name );
        cJSON_DetachItemFromObject( jPrefabObject, "Object" );
    }

    cJSON_Delete( jRoot );
}

void PrefabFile::RemovePrefab(PrefabObject* pPrefab)
{
    // Remove prefab from m_Prefabs list
    pPrefab->Remove();

#if MYFW_USING_WX
    // Remove prefab from Object List tree
    g_pPanelObjectList->RemoveObject( pPrefab );
#endif
}

void PrefabFile::AddExistingPrefab(PrefabObject* pPrefab, PrefabObject* pPreviousPrefab) // used to undo delete in editor
{
    // Add prefab to m_Prefabs list
    if( pPreviousPrefab )
    {
        pPrefab->AddAfter( pPreviousPrefab );
    }
    else
    {
        m_Prefabs.AddHead( pPrefab );
    }

#if MYFW_USING_WX
    // Add prefab to Object List tree inside TreeID of this file.
    pPrefab->AddToObjectList( this->m_TreeID, pPrefab->m_jPrefab, pPrefab->m_pGameObject );
    if( pPreviousPrefab != 0 )
    {
        g_pPanelObjectList->Tree_MoveObject( pPrefab, pPreviousPrefab, false );
    }
    else
    {
        g_pPanelObjectList->Tree_MoveObject( pPrefab->m_TreeID, m_TreeID, true );
    }
#endif
}
#endif //MYFW_EDITOR

#if MYFW_USING_WX
void PrefabFile::OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear) // StaticOnLeftClick
{
}

void PrefabFile::OnRightClick(wxTreeItemId treeid) // StaticOnRightClick
{
 	wxMenu menu;
    menu.SetClientData( this );

    MyAssert( treeid.IsOk() );
    wxString itemname = g_pPanelObjectList->m_pTree_Objects->GetItemText( treeid );
    
    //m_SceneIDBeingAffected = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );
    //if( m_SceneIDBeingAffected != SCENEID_NotFound...
    //
    //menu.Append( RightClick_AddGameObject, "Add Game Object" );

    //wxMenu* templatesmenu = MyNew wxMenu;
    //menu.AppendSubMenu( templatesmenu, "Add Game Object Template" );
    //AddGameObjectTemplatesToMenu( templatesmenu, 0 );

    //menu.Append( RightClick_AddFolder, "Add Folder" );
    //menu.Append( RightClick_AddLogicGameObject, "Add Logical Game Object" );
    //menu.Append( RightClick_UnloadScene, "Unload scene" );

    //menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SceneHandler::OnPopupClick );

    // blocking call.
    //g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}
#endif //MYFW_USING_WX

// ============================================================================================================================
// PrefabManager
// ============================================================================================================================

PrefabManager::PrefabManager(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;
}

PrefabManager::~PrefabManager()
{
    for( unsigned int fileindex=0; fileindex<m_pPrefabFiles.size(); fileindex++ )
    {
        delete m_pPrefabFiles[fileindex];
    }
}

unsigned int PrefabManager::GetNumberOfFiles()
{
    return (unsigned int)m_pPrefabFiles.size();
}

void PrefabManager::SetNumberOfFiles(unsigned int numfiles)
{
#if !MYFW_EDITOR
    m_pPrefabFiles.AllocateObjects( numfiles );
#endif
}

PrefabFile* PrefabManager::GetLoadedPrefabFileByIndex(unsigned int fileindex)
{
    MyAssert( fileindex < m_pPrefabFiles.size() );

    return m_pPrefabFiles[fileindex];
}

PrefabFile* PrefabManager::GetLoadedPrefabFileByFullPath(const char* fullpath)
{
    unsigned int numprefabfiles = (unsigned int)m_pPrefabFiles.size();

    for( unsigned int i=0; i<numprefabfiles; i++ )
    {
        if( strcmp( m_pPrefabFiles[i]->GetFile()->GetFullPath(), fullpath ) == 0 )
        {
            return m_pPrefabFiles[i];
        }
    }

    return 0;
}

PrefabFile* PrefabManager::RequestFile(const char* prefabfilename)
{
    FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
    MyFileObject* pFile = pFileManager->RequestFile( prefabfilename );
    MyAssert( pFile );

    PrefabFile* pPrefabFile = MyNew PrefabFile( pFile );

#if MYFW_EDITOR
    m_pPrefabFiles.push_back( pPrefabFile );
#else
    MyAssert( m_pPrefabFiles.Count() < m_pPrefabFiles.Length() );
    m_pPrefabFiles.Add( pPrefabFile );
#endif

    return pPrefabFile;
}

void PrefabManager::UnloadAllPrefabFiles()
{
    while( m_pPrefabFiles.size() )
    {
        delete m_pPrefabFiles[0];
#if MYFW_EDITOR
        m_pPrefabFiles[0] = m_pPrefabFiles.back();
        m_pPrefabFiles.pop_back();
#else
        m_pPrefabFiles.RemoveIndex( 0 );
#endif
    }
}

#if MYFW_EDITOR
void PrefabManager::LoadFileNow(const char* prefabfilename)
{
    FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
    MyFileObject* pFile = pFileManager->LoadFileNow( prefabfilename );
    MyAssert( pFile );

    PrefabFile* pPrefabFile = MyNew PrefabFile( pFile );
    pPrefabFile->OnFileFinishedLoading( pFile );

    m_pPrefabFiles.push_back( pPrefabFile );
}

PrefabObject* PrefabManager::CreatePrefabInFile(unsigned int fileindex, const char* prefabname, GameObject* pGameObject)
{
    PrefabFile* pFile = m_pPrefabFiles[fileindex];

    // Check if a prefab with this name already exists and fail if it does
    if( pFile->GetFirstPrefabByName( prefabname ) )
    {
#if MYFW_USING_WX
        wxMessageBox( "A prefab with this name already exists, rename it and create it again", "Failed to create prefab" );
#else
        LOGError( LOGTag, "A prefab with this name already exists, rename it and create it again" );
#endif
        return 0;
    }

    // Create a PrefabObject and stick it in the PrefabFile
    PrefabObject* pPrefab = new PrefabObject();
    pFile->m_Prefabs.AddTail( pPrefab );

    // Initialize its values
    pPrefab->Init( pFile, prefabname, pFile->GetNextPrefabIDAndIncrement() );
    cJSON* jGameObject = pGameObject->ExportAsJSONPrefab( pPrefab, true, true );
    pPrefab->SetPrefabJSONObject( jGameObject, true );

    // Kick off immediate save of prefab file.
    pFile->Save();

    return pPrefab;
}

void PrefabManager::CreateFile(const char* relativepath)
{
    // create an empty stub of a file so it can be properly requested by our code
    FILE* pFile = 0;
#if MYFW_WINDOWS
    fopen_s( &pFile, relativepath, "wb" );
#else
    pFile = fopen( relativepath, "wb" );
#endif
    if( pFile )
    {
        fclose( pFile );
    }

    // if the file managed to save, request it.
    if( pFile != 0 )
    {
        FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
        pFileManager->RequestFile( relativepath );
    }
}

bool PrefabManager::CreateOrLoadFile()
{
    // Pick an existing file to load or create a new file
#if MYFW_USING_WX
    wxFileDialog FileDialog( g_pEngineMainFrame, _("Load or Create Prefab file"), "./Data/Prefabs", "", "Prefab files (*.myprefabs)|*.myprefabs", wxFD_OPEN );
    
    if( FileDialog.ShowModal() != wxID_CANCEL )
    {
        wxString wxpath = FileDialog.GetPath();
        char fullpath[MAX_PATH];
        sprintf_s( fullpath, MAX_PATH, "%s", (const char*)wxpath );
        const char* relativepath = GetRelativePath( fullpath );

        if( relativepath != 0 )
        {
            if( FileManager::DoesFileExist( relativepath ) )
            {
                LoadFileNow( relativepath );
                return true;
            }
            else
            {
                CreateFile( relativepath );
                return true;
            }
        }
    }
#else
    // TODO: Implement this in ImGui builds.
    MyAssert( false );
#endif

    return false;
}

void PrefabManager::SaveAllPrefabs(bool saveunchanged)
{
    unsigned int numprefabfiles = (unsigned int)m_pPrefabFiles.size();

    for( unsigned int i=0; i<numprefabfiles; i++ )
    {
        if( saveunchanged || m_pPrefabFiles[i]->HasAnythingChanged() )
            m_pPrefabFiles[i]->Save();
    }
}

PrefabObject* PrefabManager::FindPrefabContainingGameObject(GameObject* pGameObject)
{
    for( unsigned int i=0; i<m_pPrefabFiles.size(); i++ )
    {
        for( CPPListNode* pNode = m_pPrefabFiles[i]->m_Prefabs.GetHead(); pNode != 0; pNode = pNode->GetNext() )
        {
            PrefabObject* pPrefab = (PrefabObject*)pNode;
            
            if( pPrefab->m_pGameObject == pGameObject )
            {
                return pPrefab;
            }
        }
    }

    return 0;
}

void PrefabManager::DeleteSelectedPrefabs()
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();
    MyAssert( pEditorState->m_pSelectedObjects.size() > 0 );

    if( pEditorState->m_pSelectedObjects.size() > 0 )
    {
        // Create a temp vector to pass into command.
        std::vector<PrefabObject*> prefabs;

        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
        {
            PrefabObject* pSelectedPrefab = g_pComponentSystemManager->m_pPrefabManager->FindPrefabContainingGameObject( pEditorState->m_pSelectedObjects[i] );
            if( pSelectedPrefab )
            {
                prefabs.push_back( pSelectedPrefab );
            }
        }

        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeletePrefabs( prefabs ) );
    }
}
#endif //MYFW_EDITOR
