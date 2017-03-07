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
// PrefabFile
// ============================================================================================================================

PrefabFile::PrefabFile(MyFileObject* pFile)
{
    MyAssert( pFile != 0 );

    m_pFile = pFile;

    pFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoading );
}

PrefabFile::~PrefabFile()
{
#if MYFW_USING_WX
    for( unsigned int prefabindex=0; prefabindex<m_pPrefabs.size(); prefabindex++ )
#else
    for( unsigned int prefabindex=0; prefabindex<m_pPrefabs.Count(); prefabindex++ )
#endif
    {
        cJSON_Delete( m_pPrefabs[prefabindex].m_jPrefab );

        // TODO: delete/release m_pGameObject
        MyAssert( m_pPrefabs[prefabindex].m_pGameObject == 0 );
    }

    m_pFile->Release();
}

void PrefabFile::OnFileFinishedLoading(MyFileObject* pFile)
{
    pFile->UnregisterFileFinishedLoadingCallback( this );

    cJSON* jRoot = cJSON_Parse( pFile->m_pBuffer );

    cJSON* jPrefab = jRoot->child;
    while( jPrefab )
    {
        cJSON* jNextPrefab = jPrefab->next;

        PrefabObject temp;
        temp.SetName( jPrefab->string );
        temp.m_jPrefab = jPrefab;
#if MYFW_USING_WX
        m_pPrefabs.push_back( temp );
#else
        m_pPrefabs.Add( temp );
#endif

        cJSON_DetachItemFromObject( jRoot, jPrefab->string );

        jPrefab = jNextPrefab;
    }

    cJSON_Delete( jRoot );
}

#if MYFW_USING_WX
void PrefabFile::Save()
{
    cJSON* jRoot = cJSON_CreateObject();

    for( unsigned int i=0; i<m_pPrefabs.size(); i++ )
    {
        cJSON_AddItemToObject( jRoot, m_pPrefabs[i].m_Name, m_pPrefabs[i].m_jPrefab );
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
    
    for( unsigned int i=0; i<m_pPrefabs.size(); i++ )
    {
        cJSON_DetachItemFromObject( jRoot, m_pPrefabs[i].m_Name );
    }

    cJSON_Delete( jRoot );
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

    PrefabObject temp;
    temp.SetName( prefabname );
    temp.m_jPrefab = jGameObject;
    m_pPrefabFiles[fileindex]->m_pPrefabs.push_back( temp );

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
