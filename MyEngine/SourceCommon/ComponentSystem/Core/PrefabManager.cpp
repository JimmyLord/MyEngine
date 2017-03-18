//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "PrefabManager.h"

// ============================================================================================================================
// PrefabObject
// ============================================================================================================================

PrefabObject::PrefabObject()
{
    m_Name[0] = 0;
    m_jPrefab = 0;
    m_pPrefabFile = 0;

#if MYFW_USING_WX
    m_pGameObject = 0;
#endif
}

PrefabObject::~PrefabObject()
{
    delete m_pGameObject;
}

void PrefabObject::Init(PrefabFile* pFile, const char* name)
{
#if MYFW_USING_WX
    wxTreeItemId rootid = pFile->m_TreeID;
    m_TreeID = g_pPanelObjectList->AddObject( this, PrefabObject::StaticOnLeftClick, PrefabObject::StaticOnRightClick, rootid, m_Name, ObjectListIcon_GameObject );
    g_pPanelObjectList->SetDragAndDropFunctions( m_TreeID, PrefabObject::StaticOnDrag, PrefabObject::StaticOnDrop );

    m_pPrefabFile = pFile;
    SetName( name );
#endif
}

void PrefabObject::SetName(const char* name)
{
    strcpy_s( m_Name, MAX_PREFAB_NAME_LENGTH, name );

#if MYFW_USING_WX
    g_pPanelObjectList->RenameObject( this, m_Name );
#endif
}

void PrefabObject::SetPrefabJSONString(cJSON* jPrefab)
{
    if( m_jPrefab )
    {
        cJSON_Delete( m_jPrefab );
#if MYFW_USING_WX
        delete m_pGameObject;
#endif
    }

    m_jPrefab = jPrefab;
#if MYFW_USING_WX
    m_pGameObject = g_pComponentSystemManager->CreateGameObjectFromPrefab( this, false, 0 );
    m_pGameObject->SetEnabled( false );
#endif
}

const char* PrefabObject::GetName()
{
    return m_Name;
}

cJSON* PrefabObject::GetJSONObject()
{
    return m_jPrefab;
}

#if MYFW_USING_WX
void PrefabObject::OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear)
{
}

void PrefabObject::OnRightClick(wxTreeItemId treeid)
{
 	wxMenu menu;
    menu.SetClientData( this );

    MyAssert( treeid.IsOk() );
    wxString itemname = g_pPanelObjectList->m_pTree_Objects->GetItemText( treeid );
    
    //m_SceneIDBeingAffected = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );
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

void PrefabObject::OnDrag()
{
    g_DragAndDropStruct.m_Type = (DragAndDropTypes)DragAndDropTypeEngine_Prefab;
    g_DragAndDropStruct.m_Value = this;
}

void PrefabObject::OnDrop(int controlid, wxCoord x, wxCoord y)
{
}
#endif

// ============================================================================================================================
// PrefabFile
// ============================================================================================================================

PrefabFile::PrefabFile(MyFileObject* pFile)
{
    MyAssert( pFile != 0 );

    m_pFile = pFile;

    pFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoading );

#if MYFW_USING_WX
    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    m_TreeID = g_pPanelObjectList->AddObject( this, PrefabFile::StaticOnLeftClick, PrefabFile::StaticOnRightClick, rootid, m_pFile->m_FilenameWithoutExtension, ObjectListIcon_Scene );
    //g_pPanelObjectList->SetDragAndDropFunctions( treeid, PrefabFile::StaticOnDrag, PrefabFile::StaticOnDrop );
#endif
}

PrefabFile::~PrefabFile()
{
#if MYFW_USING_WX
    for( unsigned int prefabindex=0; prefabindex<m_Prefabs.size(); prefabindex++ )
#else
    for( unsigned int prefabindex=0; prefabindex<m_Prefabs.Count(); prefabindex++ )
#endif
    {
        cJSON_Delete( m_Prefabs[prefabindex]->m_jPrefab );

        delete m_Prefabs[prefabindex];
    }

    m_pFile->Release();
}

PrefabObject* PrefabFile::GetPrefabByName(const char* name)
{
#if MYFW_USING_WX
    for( unsigned int prefabindex=0; prefabindex<m_Prefabs.size(); prefabindex++ )
#else
    for( unsigned int prefabindex=0; prefabindex<m_Prefabs.Count(); prefabindex++ )
#endif
    {
        if( strcmp( m_Prefabs[prefabindex]->GetName(), name ) == 0 )
        {
            return m_Prefabs[prefabindex];
        }
    }

    return 0;
}

void PrefabFile::OnFileFinishedLoading(MyFileObject* pFile)
{
    pFile->UnregisterFileFinishedLoadingCallback( this );

    cJSON* jRoot = cJSON_Parse( pFile->m_pBuffer );

    cJSON* jPrefab = jRoot->child;
    while( jPrefab )
    {
        cJSON* jNextPrefab = jPrefab->next;

#if MYFW_USING_WX
        PrefabObject* pPrefab = new PrefabObject();
        m_Prefabs.push_back( pPrefab );
#else
        PrefabObject* pPrefab = new PrefabObject();
        m_Prefabs.Add( pPrefab );
#endif
        pPrefab->Init( this, jPrefab->string );
        pPrefab->SetPrefabJSONString( jPrefab );

        cJSON_DetachItemFromObject( jRoot, jPrefab->string );

        jPrefab = jNextPrefab;
    }

    cJSON_Delete( jRoot );
}

#if MYFW_USING_WX
void PrefabFile::Save()
{
    cJSON* jRoot = cJSON_CreateObject();

    for( unsigned int i=0; i<m_Prefabs.size(); i++ )
    {
        cJSON_AddItemToObject( jRoot, m_Prefabs[i]->m_Name, m_Prefabs[i]->m_jPrefab );
    }

    char* jsonstring = cJSON_Print( jRoot );

    FILE* pFile;
#if MYFW_WINDOWS
    fopen_s( &pFile, m_pFile->m_FullPath, "wb" );
#else
    pFile = fopen( m_pFile->m_FullPath, "wb" );
#endif
    fprintf( pFile, "%s", jsonstring );
    fclose( pFile );
    
    cJSONExt_free( jsonstring );
    
    for( unsigned int i=0; i<m_Prefabs.size(); i++ )
    {
        cJSON_DetachItemFromObject( jRoot, m_Prefabs[i]->m_Name );
    }

    cJSON_Delete( jRoot );
}

void PrefabFile::OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear)
{
}

void PrefabFile::OnRightClick(wxTreeItemId treeid)
{
 	wxMenu menu;
    menu.SetClientData( this );

    MyAssert( treeid.IsOk() );
    wxString itemname = g_pPanelObjectList->m_pTree_Objects->GetItemText( treeid );
    
    //m_SceneIDBeingAffected = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );
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
#endif

// ============================================================================================================================
// PrefabManager
// ============================================================================================================================

PrefabManager::PrefabManager()
{
}

PrefabManager::~PrefabManager()
{
#if MYFW_USING_WX
    for( unsigned int fileindex=0; fileindex<m_pPrefabFiles.size(); fileindex++ )
#else
    for( unsigned int fileindex=0; fileindex<m_pPrefabFiles.Count(); fileindex++ )
#endif
    {
        delete m_pPrefabFiles[fileindex];
    }
}

unsigned int PrefabManager::GetNumberOfFiles()
{
#if MYFW_USING_WX
    return m_pPrefabFiles.size();
#else
    return m_pPrefabFiles.Count();
#endif
}

void PrefabManager::SetNumberOfFiles(unsigned int numfiles)
{
#if !MYFW_USING_WX
    m_pPrefabFiles.AllocateObjects( numfiles );
#endif
}

PrefabFile* PrefabManager::GetLoadedPrefabFileByIndex(unsigned int fileindex)
{
#if MYFW_USING_WX
    MyAssert( fileindex < m_pPrefabFiles.size() );
#else
    MyAssert( fileindex < m_pPrefabFiles.Count() );
#endif

    return m_pPrefabFiles[fileindex];
}

PrefabFile* PrefabManager::RequestFile(const char* prefabfilename)
{
    MyFileObject* pFile = g_pFileManager->RequestFile( prefabfilename );
    MyAssert( pFile );

    PrefabFile* pPrefabFile = MyNew PrefabFile( pFile );

#if MYFW_USING_WX
    m_pPrefabFiles.push_back( pPrefabFile );
#else
    MyAssert( m_pPrefabFiles.Count() < m_pPrefabFiles.Length() );
    m_pPrefabFiles.Add( pPrefabFile );
#endif

    return pPrefabFile;
}

PrefabFile* PrefabManager::GetPrefabFileForFileObject(const char* prefabfilename)
{
#if MYFW_USING_WX
    unsigned int numprefabfiles = m_pPrefabFiles.size();
#else
    unsigned int numprefabfiles = m_pPrefabFiles.Count();
#endif

    for( unsigned int i=0; i<numprefabfiles; i++ )
    {
        if( strcmp( m_pPrefabFiles[i]->GetFile()->m_FullPath, prefabfilename ) == 0 )
        {
            return m_pPrefabFiles[i];
        }
    }

    return 0;
}

#if MYFW_USING_WX
void PrefabManager::LoadFileNow(const char* prefabfilename)
{
    MyFileObject* pFile = g_pFileManager->LoadFileNow( prefabfilename );
    MyAssert( pFile );

    PrefabFile* pPrefabFile = MyNew PrefabFile( pFile );
    pPrefabFile->OnFileFinishedLoading( pFile );

    m_pPrefabFiles.push_back( pPrefabFile );
}

void PrefabManager::CreatePrefabInFile(unsigned int fileindex, const char* prefabname, GameObject* pGameObject)
{
    cJSON* jGameObject = pGameObject->ExportAsJSONPrefab();

    PrefabObject* pPrefab = new PrefabObject();
    m_pPrefabFiles[fileindex]->m_Prefabs.push_back( pPrefab );
    pPrefab->SetName( prefabname );
    pPrefab->SetPrefabJSONString( jGameObject );

    // Kick off immediate save of prefab file.
    m_pPrefabFiles[fileindex]->Save();

    // TODO: show this prefab in the object or file list... or both?
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
        RequestFile( relativepath );
    }
}

bool PrefabManager::CreateOrLoadFile()
{
    // Pick an existing file to load or create a new file
    {
        wxFileDialog FileDialog( g_pEngineMainFrame, _("Load or Create Prefab file"), "./Data/Prefabs", "", "Prefab files (*.myprefabs)|*.myprefabs", wxFD_OPEN );
    
        if( FileDialog.ShowModal() != wxID_CANCEL )
        {
            wxString wxpath = FileDialog.GetPath();
            char fullpath[MAX_PATH];
            sprintf_s( fullpath, MAX_PATH, "%s", (const char*)wxpath );
            const char* relativepath = GetRelativePath( fullpath );

            if( relativepath != 0 )
            {
                if( g_pFileManager->DoesFileExist( relativepath ) )
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
    }

    return false;
}
#endif
