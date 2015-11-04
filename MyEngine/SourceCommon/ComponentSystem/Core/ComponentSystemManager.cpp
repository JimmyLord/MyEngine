//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

ComponentSystemManager* g_pComponentSystemManager = 0;

ComponentSystemManager::ComponentSystemManager(ComponentTypeManager* typemanager)
{
    g_pComponentSystemManager = this;

    m_pComponentTypeManager = typemanager;
#if MYFW_USING_WX
    m_pSceneHandler = MyNew SceneHandler();
#endif

#if MYFW_USING_WX
    g_pMaterialManager->RegisterMaterialCreatedCallback( this, StaticOnMaterialCreated );
#endif

    m_NextSceneID = 1;

    m_WaitingForFilesToFinishLoading = false;
    m_StartGamePlayWhenDoneLoading = false;

#if MYFW_USING_WX
    // Add click callbacks to the root of the objects tree
    g_pPanelObjectList->SetTreeRootData( this, ComponentSystemManager::StaticOnLeftClick, ComponentSystemManager::StaticOnRightClick );

    // Create a scene for "Unmanaged" objects.
    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    wxTreeItemId treeid = g_pPanelObjectList->AddObject( m_pSceneHandler, SceneHandler::StaticOnLeftClick, SceneHandler::StaticOnRightClick, rootid, "Unmanaged" );
    g_pPanelObjectList->SetDragAndDropFunctions( treeid, SceneHandler::StaticOnDrag, SceneHandler::StaticOnDrop );
    m_pSceneInfoMap[0].treeid = treeid;
    m_pSceneInfoMap[0].fullpath[0] = 0;
#endif //MYFW_USING_WX
}

ComponentSystemManager::~ComponentSystemManager()
{
    while( m_GameObjects.GetHead() )
        delete m_GameObjects.RemHead();

    while( m_Files.GetHead() )
        delete m_Files.RemHead();

    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        while( m_Components[i].GetHead() )
        {
            ComponentBase* pComponent = (ComponentBase*)m_Components[i].GetHead();
            pComponent->SetEnabled( false );

            delete m_Components[i].RemHead();
        }
    }

#if MYFW_USING_WX
    SAFE_DELETE( m_pSceneHandler );
#endif
    SAFE_DELETE( m_pComponentTypeManager );

    // if a component didn't unregister its callbacks, assert.
    MyAssert( m_pComponentCallbackList_Tick.GetHead() == 0 );
    MyAssert( m_pComponentCallbackList_OnSurfaceChanged.GetHead() == 0 );
    MyAssert( m_pComponentCallbackList_Draw.GetHead() == 0 );
    MyAssert( m_pComponentCallbackList_OnTouch.GetHead() == 0 );
    MyAssert( m_pComponentCallbackList_OnButtons.GetHead() == 0 );
    MyAssert( m_pComponentCallbackList_OnKeys.GetHead() == 0 );
    MyAssert( m_pComponentCallbackList_OnFileRenamed.GetHead() == 0 );
    
    g_pComponentSystemManager = 0;
}

// TODO: put in some sort of priority/sorting system for callbacks.
#define MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS(CallbackType) \
    void ComponentSystemManager::RegisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct) \
    { \
        MyAssert( pCallbackStruct->pFunc != 0 && pCallbackStruct->pObj != 0 ); \
        m_pComponentCallbackList_##CallbackType.AddTail( pCallbackStruct ); \
    } \
    void ComponentSystemManager::UnregisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct) \
    { \
        if( pCallbackStruct->Prev ) \
            pCallbackStruct->Remove(); \
    }

MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( Tick );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnSurfaceChanged );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( Draw );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnTouch );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnButtons );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnKeys );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnFileRenamed );

void ComponentSystemManager::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSystemManager>( "ComponentSystemManager" )
            .addFunction( "CreateGameObject", &ComponentSystemManager::CreateGameObject )
            .addFunction( "DeleteGameObject", &ComponentSystemManager::DeleteGameObject )
            .addFunction( "CopyGameObject", &ComponentSystemManager::CopyGameObject )
            .addFunction( "FindGameObjectByName", &ComponentSystemManager::FindGameObjectByName )
        .endClass();
}

#if MYFW_USING_WX
void ComponentSystemManager::CheckForUpdatedDataSourceFiles(bool initialcheck)
{
    for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

        if( (initialcheck == false || pFileInfo->m_DidInitialCheckIfSourceFileWasUpdated == false) && // haven't done initial check
            pFileInfo->m_pFile->m_FileLastWriteTime.dwHighDateTime != 0 && // converted file has been loaded
            pFileInfo->m_SourceFileFullPath[0] != 0 )                      // we have a source file
        {
#if MYFW_WINDOWS
            WIN32_FIND_DATAA data;
            memset( &data, 0, sizeof( data ) );

            HANDLE handle = FindFirstFileA( pFileInfo->m_SourceFileFullPath, &data );
            if( handle != INVALID_HANDLE_VALUE )
                FindClose( handle );

            // if the source file is newer than the data file, reimport it.
            if( data.ftLastWriteTime.dwHighDateTime > pFileInfo->m_pFile->m_FileLastWriteTime.dwHighDateTime ||
                ( data.ftLastWriteTime.dwHighDateTime == pFileInfo->m_pFile->m_FileLastWriteTime.dwHighDateTime &&
                  data.ftLastWriteTime.dwLowDateTime > pFileInfo->m_pFile->m_FileLastWriteTime.dwLowDateTime ) )
            {
                ImportDataFile( pFileInfo->m_SceneID, pFileInfo->m_SourceFileFullPath );
                bool updated = true;
            }

            pFileInfo->m_DidInitialCheckIfSourceFileWasUpdated = true;
        }
#endif
    }
}

void ComponentSystemManager::OnFileUpdated(MyFileObject* pFile)
{
    for( unsigned int i=0; i<m_pFileUpdatedCallbackList.size(); i++ )
    {
        m_pFileUpdatedCallbackList[i].pFunc( m_pFileUpdatedCallbackList[i].pObj, pFile );
    }
}

void ComponentSystemManager::Editor_RegisterFileUpdatedCallback(FileUpdatedCallbackFunction pFunc, void* pObj)
{
    MyAssert( pFunc != 0 );
    MyAssert( pObj != 0 );

    FileUpdatedCallbackStruct callbackstruct;
    callbackstruct.pFunc = pFunc;
    callbackstruct.pObj = pObj;

    m_pFileUpdatedCallbackList.push_back( callbackstruct );
}

void ComponentSystemManager::AddAllMaterialsToFilesList()
{
    for( CPPListNode* pNode = g_pMaterialManager->m_Materials.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pNode;

        if( pMaterial->m_pFile )
        {
            if( GetFileObjectIfUsedByScene( pMaterial->m_pFile->m_FullPath, 1 ) == 0 )
            {
                MyFileInfo* pFileInfo = MyNew MyFileInfo();
                
                pFileInfo->m_pFile = pMaterial->m_pFile;
                pMaterial->m_pFile->AddRef();

                pFileInfo->m_pMaterial = pMaterial;
                pMaterial->AddRef();

                pFileInfo->m_SceneID = 1;
                m_Files.AddTail( pFileInfo );
            }
        }
    }
}

void ComponentSystemManager::OnLeftClick(unsigned int count, bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();
}

void ComponentSystemManager::OnRightClick()
{
    //wxMenu menu;
    //menu.SetClientData( this );

    //menu.Append( 1000, "Add Game Object" );
    //menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentSystemManager::OnPopupClick );

    //// blocking call. // should delete all categorymenu's new'd above when done.
    //g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentSystemManager::OnPopupClick(wxEvent &evt)
{
    //int id = evt.GetId();
    //if( id == 1000 )
    //{
    //    GameObject* pGameObject = g_pComponentSystemManager->CreateGameObject();
    //    pGameObject->SetName( "New Game Object" );
    //}
}

void ComponentSystemManager::OnMemoryPanelFileSelectedLeftClick()
{
    // not sure why I put this in anymore... might be handy later.
    //int bp = 1;
}

void ComponentSystemManager::OnMaterialCreated(MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial );

    // if this material doesn't have a file and it has a name, then save it.
    if( pMaterial && pMaterial->m_pFile == 0 && pMaterial->m_Name[0] != 0 )
    {
        int oldrefcount = pMaterial->GetRefCount();

        g_pMaterialManager->SaveAllMaterials();
        AddAllMaterialsToFilesList();
        pMaterial->Release(); // ref should have been added by AddAllMaterialsToFilesList().
    }
}
#endif //MYFW_USING_WX

void ComponentSystemManager::MoveAllFilesNeededForLoadingScreenToStartOfFileList()
{
    for( CPPListNode* pNode = m_GameObjects.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)pNode;

        if( strncmp( pGameObject->GetName(), "Load", 4 ) == 0 )
        {
            for( unsigned int i=0; i<pGameObject->m_Components.Count(); i++ )
            {
                ComponentBase* pComponent = pGameObject->m_Components[i];

                // move sprite material files to front of list.
                if( pComponent->IsA( "SpriteComponent" ) )
                {
                    MySprite* pSprite = ((ComponentSprite*)pComponent)->m_pSprite;
                    if( pSprite && pSprite->GetMaterial() )
                    {
                        pSprite->GetMaterial()->MoveAssociatedFilesToFrontOfFileList();
                    }
                }

                // move lua scripts to front of list.
                if( pComponent->IsA( "LuaScriptComponent" ) )
                {
                    MyFileObject* pScriptFile = ((ComponentLuaScript*)pComponent)->GetScriptFile();
                    if( pScriptFile )
                    {
                        g_pFileManager->MoveFileToFrontOfFileLoadedList( pScriptFile );
                    }
                }
            }
        }
    }
}

void ComponentSystemManager::AddListOfFilesUsedToJSONObject(unsigned int sceneid, cJSON* filearray)
{
    // TODO: there are currently many ways a file can be loaded into a secondary scene without being in this list.
    //       need to adjust code in various components to account for this.

    for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyFileInfo* pFileInfo = (MyFileInfo*)pNode;
        
        MyAssert( pFileInfo );
        if( pFileInfo == 0 )
            continue;

        if( pFileInfo->m_SceneID != sceneid )
            continue;
        
        MyFileObject* pFile = pFileInfo->m_pFile;
        if( pFile != 0 )
        {
            // skip over shader include files.
            if( pFile->IsA( "MyFileShader" ) )
            {
                MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;
                if( pShaderFile && pShaderFile->m_IsAnIncludeFile )
                {
                    MyAssert( false ); // shader include files shouldn't be in the file list.
                    continue;
                }
            }

            cJSON* jFile = cJSON_CreateObject();
            cJSON_AddItemToObject( jFile, "Path", cJSON_CreateString( pFile->m_FullPath ) );
            cJSON_AddItemToArray( filearray, jFile );

            // Save the source path if there is one.
            if( pFileInfo->m_SourceFileFullPath[0] != 0 )
            {
                cJSON_AddItemToObject( jFile, "SourcePath", cJSON_CreateString( pFileInfo->m_SourceFileFullPath ) );
            }
        }
    }

    // Only save files used by this scene.
    //MyFileObject* pFile = g_pFileManager->GetFirstFileLoaded();
    //while( pFile != 0 )
    //{
    //    // skip over shader include files.
    //    if( pFile->IsA( "MyFileShader" ) )
    //    {
    //        MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;
    //        if( pShaderFile && pShaderFile->m_IsAnIncludeFile )
    //        {
    //            pFile = (MyFileObject*)pFile->GetNext();
    //            continue;
    //        }
    //    }

    //    cJSON* jFile = cJSON_CreateObject();
    //    cJSON_AddItemToObject( jFile, "Path", cJSON_CreateString( pFile->m_FullPath ) );
    //    cJSON_AddItemToArray( filearray, jFile );

    //    // Find the MyFileInfo object and save the source path if there is one.
    //    // TODO: this may fail if the file is loaded by multiple scenes.
    //    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( pFile->m_FullPath, -1 );
    //    if( pFileInfo && pFileInfo->m_SourceFileFullPath[0] != 0 )
    //    {
    //        cJSON_AddItemToObject( jFile, "SourcePath", cJSON_CreateString( pFileInfo->m_SourceFileFullPath ) );
    //    }

    //    pFile = (MyFileObject*)pFile->GetNext();
    //}

    //// don't save files still loading, they can be readded if needed.
    ////pFile = g_pFileManager->GetFirstFileStillLoading();
    ////while( pFile != 0 )
    ////{
    ////    cJSON_AddItemToArray( filearray, cJSON_CreateString( pFile->m_FullPath ) );
    ////    pFile = (MyFileObject*)pFile->GetNext();
    ////}
}

char* ComponentSystemManager::SaveSceneToJSON(unsigned int sceneid)
{
    cJSON* root = cJSON_CreateObject();
    cJSON* filearray = cJSON_CreateArray();
    cJSON* gameobjectarray = cJSON_CreateArray();
    cJSON* transformarray = cJSON_CreateArray();
    cJSON* componentarray = cJSON_CreateArray();

    cJSON_AddItemToObject( root, "Files", filearray );
    cJSON_AddItemToObject( root, "GameObjects", gameobjectarray );
    cJSON_AddItemToObject( root, "Transforms", transformarray );
    cJSON_AddItemToObject( root, "Components", componentarray );

    bool savingallscenes = (sceneid == UINT_MAX);

    // move files used by gameobjects that start with "Load" to front of file list.
    {
        MoveAllFilesNeededForLoadingScreenToStartOfFileList();
    }

    // add the files used.
    AddListOfFilesUsedToJSONObject( sceneid, filearray );

    // add the game objects and their transform components.
    {
        for( CPPListNode* pNode = m_GameObjects.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            GameObject* pGameObject = (GameObject*)pNode;
            if( pGameObject->IsManaged() &&
                ( pGameObject->GetSceneID() == sceneid || savingallscenes )
              )
            {
                cJSON_AddItemToArray( gameobjectarray, pGameObject->ExportAsJSONObject( savingallscenes ) );
            }
        }

        for( CPPListNode* pNode = m_GameObjects.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            GameObject* pGameObject = (GameObject*)pNode;
            if( pGameObject->IsManaged() &&
                ( pGameObject->GetSceneID() == sceneid || savingallscenes )
              )
            {
                ComponentBase* pComponent = pGameObject->m_pComponentTransform;
                cJSON_AddItemToArray( transformarray, pComponent->ExportAsJSONObject( savingallscenes ) );
            }
        }
    }

    // Add each of the component types, don't save components of unmanaged objects
    {
        for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
        {
            for( CPPListNode* pNode = m_Components[i].GetHead(); pNode; pNode = pNode->GetNext() )
            {
                ComponentBase* pComponent = (ComponentBase*)pNode;
                if( pComponent->m_pGameObject->IsManaged() &&
                    ( pComponent->m_pGameObject->GetSceneID() == sceneid || savingallscenes )
                  )
                {
                    cJSON_AddItemToArray( componentarray, pComponent->ExportAsJSONObject( savingallscenes ) );
                }
            }
        }
    }

    char* savestring = cJSON_Print( root );
    cJSON_Delete(root);

    return savestring;
}

MyFileInfo* ComponentSystemManager::GetFileInfoIfUsedByScene(const char* fullpath, unsigned int sceneid)
{
    for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

        if( strcmp( pFileInfo->m_pFile->m_FullPath, fullpath ) == 0 && (sceneid == -1 || pFileInfo->m_SceneID == sceneid) )
            return pFileInfo;
    }

    return 0;
}

MyFileObject* ComponentSystemManager::GetFileObjectIfUsedByScene(const char* fullpath, unsigned int sceneid)
{
    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( fullpath, sceneid );

    if( pFileInfo )
        return pFileInfo->m_pFile;

    return 0;
}

MyFileObject* ComponentSystemManager::LoadDataFile(const char* relativepath, unsigned int sceneid, const char* fullsourcefilepath, bool convertifrequired)
{
    MyAssert( relativepath );

    // each scene loaded will add ref's to the file.
    MyFileObject* pFile = GetFileObjectIfUsedByScene( relativepath, sceneid );

    // if the file is already tagged as being used by this scene, don't request/addref it.
    if( pFile != 0 )
    {
        LOGInfo( LOGTag, "%s already in scene, reloading\n", relativepath );
        g_pFileManager->ReloadFile( pFile );
        OnFileUpdated_CallbackFunction( pFile );
    }
    else
    {
        size_t fulllen = 0;
        if( fullsourcefilepath )
            fulllen = strlen( fullsourcefilepath );
        size_t rellen = strlen( relativepath );

#if MYFW_USING_WX
        if( convertifrequired && fullsourcefilepath )
        {
            MyFileObject* pFile = ImportDataFile( sceneid, fullsourcefilepath );

            if( pFile )
                return pFile;
        }
#endif
        
        TextureDefinition* pTexture = 0;

        // load textures differently than other files.
        if( rellen > 4 && strcmp( &relativepath[rellen-4], ".png" ) == 0 )
        {
            pTexture = g_pTextureManager->CreateTexture( relativepath );
            pFile = pTexture->m_pFile;
            pFile->AddRef();
        }
        else
        {
            pFile = g_pFileManager->RequestFile( relativepath );
#if MYFW_USING_WX
            pFile->SetCustomLeftClickCallback( StaticOnMemoryPanelFileSelectedLeftClick, this );
#endif
        }

        // store pFile so we can free it afterwards.
        MyFileInfo* pFileInfo = MyNew MyFileInfo();
        pFileInfo->m_pFile = pFile;
        pFileInfo->m_pTexture = pTexture;
        pFileInfo->m_SceneID = sceneid;
        m_Files.AddTail( pFileInfo );

        // if the extension of the source file is different than that of the file we're loading,
        //  then store the source file path in the fileobject, so we can detect/reconvert if that file changes.
        if( rellen > 4 && fulllen > 4 &&
            ( strcmp( &relativepath[rellen-4], &fullsourcefilepath[fulllen-4] ) != 0 ) )
        {
            const char* relativepath = GetRelativePath( fullsourcefilepath );

            // store the relative path if the file is relative... otherwise store the full path.
            if( relativepath == 0 )
                strcpy_s( pFileInfo->m_SourceFileFullPath, MAX_PATH, fullsourcefilepath );
            else
                strcpy_s( pFileInfo->m_SourceFileFullPath, MAX_PATH, relativepath );

            for( unsigned int i=0; i<strlen(pFileInfo->m_SourceFileFullPath); i++ )
            {
                if( pFileInfo->m_SourceFileFullPath[i] == '\\' )
                    pFileInfo->m_SourceFileFullPath[i] = '/';
            }
        }

        // if we're loading a mesh file type, create a mesh.
        {
            if( strcmp( pFile->m_ExtensionWithDot, ".obj" ) == 0 )
            {
                pFileInfo->m_pMesh = MyNew MyMesh();
                pFileInfo->m_pMesh->CreateFromOBJFile( pFile );
            }
            if( strcmp( pFile->m_ExtensionWithDot, ".mymesh" ) == 0 )
            {
                pFileInfo->m_pMesh = MyNew MyMesh();
                pFileInfo->m_pMesh->CreateFromMyMeshFile( pFile );
            }
        }

        // if we're loading an .glsl file, create a ShaderGroup.
        if( strcmp( pFile->m_ExtensionWithDot, ".glsl" ) == 0 )
        {
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFile( pFile );
            if( pShaderGroup )
            {
                pShaderGroup->AddRef();
                pFileInfo->m_pShaderGroup = pShaderGroup;
            }
            else
            {
                pFileInfo->m_pShaderGroup = MyNew ShaderGroup( pFile );
            }
        }

        // if we're loading an .mymaterial file, create a Material.
        if( strcmp( pFile->m_ExtensionWithDot, ".mymaterial" ) == 0 )
        {
            pFileInfo->m_pMaterial = g_pMaterialManager->LoadMaterial( pFile->m_FullPath );
        }
    }

    return pFile;
}

#if MYFW_USING_WX
MyFileObject* ComponentSystemManager::ImportDataFile(unsigned int sceneid, const char* fullsourcefilepath)
{
    MyAssert( fullsourcefilepath );
    if( fullsourcefilepath == 0 )
        return 0;

    //const char* relativepath = GetRelativePath( fullsourcefilepath );
    //size_t rellen = strlen( relativepath );

    size_t fulllen = 0;
    fulllen = strlen( fullsourcefilepath );

    // convert any .fbx files into a mymesh and load that.
    if( ( fulllen > 4 && strcmp( &fullsourcefilepath[fulllen-4], ".fbx" ) == 0 ) ||
        ( fulllen > 4 && strcmp( &fullsourcefilepath[fulllen-4], ".obj" ) == 0 ) ||
        ( fulllen > 6 && strcmp( &fullsourcefilepath[fulllen-6], ".blend" ) == 0 )
      )
    {
        // run MeshTool to convert the mesh and put the result in "Data/Meshes"
        const int paramsbuffersize = MAX_PATH * 2 + 50;
        char params[paramsbuffersize]; // 2 full paths plus a few extra chars

        char workingdir[MAX_PATH];
#if MYFW_WINDOWS
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
        getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif

        char filename[MAX_PATH];
        for( int i=(int)strlen(fullsourcefilepath)-1; i>=0; i-- )
        {
            if( fullsourcefilepath[i] == '\\' || fullsourcefilepath[i] == '/' || i == 0 )
            {
                strcpy_s( filename, MAX_PATH, &fullsourcefilepath[i+1] );
                break;
            }
        }

        sprintf_s( params, paramsbuffersize, "-s %s -o Data/Meshes/%s -m Data/Materials", fullsourcefilepath, filename );

        LOGInfo( LOGTag, "Converting %s to mymesh %s\n", fullsourcefilepath, params );

#if MYFW_WINDOWS
        SHELLEXECUTEINFOA info = {0};
        info.cbSize = sizeof( SHELLEXECUTEINFOA );
        info.fMask = SEE_MASK_NOASYNC; //SEE_MASK_NOCLOSEPROCESS;
        info.hwnd = 0;
        info.lpVerb = 0;
        info.lpFile = "Tools\\MeshTool.exe";
        info.lpParameters = params;
        info.lpDirectory = workingdir;
        info.nShow = SW_SHOWNOACTIVATE;
        info.hInstApp = 0;

        if( ShellExecuteExA( &info ) == 0 )
        //if( ShellExecuteA( 0, "open", "Tools/MeshTool.exe", params, workingdir, SW_SHOWNOACTIVATE ) != 0 )
        {
            LOGError( LOGTag, "Something went wrong with conversion\n" );
        }
        else
        {
            LOGInfo( LOGTag, "Conversion complete\n" );

            // file isn't found by loading code if this sleep isn't here. TODO: find better solution.
            Sleep( 500 );

            // create a new relative path.
            char newrelativepath[MAX_PATH];
            sprintf_s( newrelativepath, MAX_PATH, "Data/Meshes/%s.mymesh", filename );

            return LoadDataFile( newrelativepath, sceneid, fullsourcefilepath, false );
        }
#else
        LOGError( LOGTag, "Mesh conversion only works on Windows ATM\n" );
#endif
    }

    return 0;
}
#endif //MYFW_USING_WX

void ComponentSystemManager::FreeAllDataFiles(unsigned int sceneidtoclear)
{
    for( CPPListNode* pNode = m_Files.GetHead(); pNode;  )
    {
        MyFileInfo* pFile = (MyFileInfo*)pNode;
        pNode = pNode->GetNext();

        if( sceneidtoclear == UINT_MAX || pFile->m_SceneID == sceneidtoclear )
            delete pFile;
    }
}

void ComponentSystemManager::LoadSceneFromJSON(const char* scenename, const char* jsonstr, unsigned int sceneid)
{
    cJSON* root = cJSON_Parse( jsonstr );

    if( root == 0 )
        return;

    cJSON* filearray = cJSON_GetObjectItem( root, "Files" );
    cJSON* gameobjectarray = cJSON_GetObjectItem( root, "GameObjects" );
    cJSON* transformarray = cJSON_GetObjectItem( root, "Transforms" );
    cJSON* componentarray = cJSON_GetObjectItem( root, "Components" );

#if MYFW_USING_WX
    if( sceneid != UINT_MAX )
    {
        CreateNewScene( scenename, sceneid );
    }
#endif //MYFW_USING_WX

    // request all files used by scene.
    if( filearray && sceneid != UINT_MAX )
    {
        for( int i=0; i<cJSON_GetArraySize( filearray ); i++ )
        {
            cJSON* jFile = cJSON_GetArrayItem( filearray, i );

            if( jFile->valuestring != 0 )
            {
                LoadDataFile( jFile->valuestring, sceneid, 0 );
            }
            else
            {
                cJSON* jPath = cJSON_GetObjectItem( jFile, "Path" );
                if( jPath )
                {
                    LoadDataFile( jPath->valuestring, sceneid, 0 );

                    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( jPath->valuestring, sceneid );
                    MyAssert( pFileInfo );
                    if( pFileInfo )
                    {
                        cJSON* jSourcePath = cJSON_GetObjectItem( jFile, "SourcePath" );
                        if( jSourcePath )
                        {
                            strcpy_s( pFileInfo->m_SourceFileFullPath, MAX_PATH, jSourcePath->valuestring );
                        }
                    }
                }
            }
        }
    }

    bool getsceneidfromeachobject = (sceneid == UINT_MAX);

    // create/init all the game objects
    for( int i=0; i<cJSON_GetArraySize( gameobjectarray ); i++ )
    {
        cJSON* gameobj = cJSON_GetArrayItem( gameobjectarray, i );

        if( getsceneidfromeachobject )
            cJSONExt_GetUnsignedInt( gameobj, "SceneID", &sceneid );

        unsigned int id;
        cJSONExt_GetUnsignedInt( gameobj, "ID", &id );

        // find an existing game object with the same id or create a new one.
        GameObject* pGameObject = FindGameObjectByID( sceneid, id );
        if( pGameObject ) { MyAssert( pGameObject->GetSceneID() == sceneid ); }

        if( pGameObject == 0 )        
            pGameObject = CreateGameObject( true, sceneid );

        pGameObject->ImportFromJSONObject( gameobj, sceneid );
        
        unsigned int gameobjectid = pGameObject->GetID();
        if( gameobjectid > m_pSceneInfoMap[sceneid].m_NextGameObjectID )
            m_pSceneInfoMap[sceneid].m_NextGameObjectID = gameobjectid + 1;
    }

    // setup all the game object transforms
    for( int i=0; i<cJSON_GetArraySize( transformarray ); i++ )
    {
        cJSON* transformobj = cJSON_GetArrayItem( transformarray, i );
        
        if( getsceneidfromeachobject )
            cJSONExt_GetUnsignedInt( transformobj, "SceneID", &sceneid );

        unsigned int goid = 0;
        cJSONExt_GetUnsignedInt( transformobj, "GOID", &goid );
        MyAssert( goid > 0 );

        GameObject* pGameObject = FindGameObjectByID( sceneid, goid );
        MyAssert( pGameObject );

        if( pGameObject )
        {
            pGameObject->SetID( goid );
            pGameObject->m_pComponentTransform->ImportFromJSONObject( transformobj, sceneid );

            if( goid >= m_pSceneInfoMap[sceneid].m_NextGameObjectID )
                m_pSceneInfoMap[sceneid].m_NextGameObjectID = goid + 1;
        }
    }

    // create/init all the other components
    for( int i=0; i<cJSON_GetArraySize( componentarray ); i++ )
    {
        cJSON* componentobj = cJSON_GetArrayItem( componentarray, i );
        
        if( getsceneidfromeachobject )
            cJSONExt_GetUnsignedInt( componentobj, "SceneID", &sceneid );

        unsigned int id = 0;
        cJSONExt_GetUnsignedInt( componentobj, "GOID", &id );
        MyAssert( id > 0 );
        GameObject* pGameObject = FindGameObjectByID( sceneid, id );
        MyAssert( pGameObject );

        cJSON* typeobj = cJSON_GetObjectItem( componentobj, "Type" );
        MyAssert( typeobj );
        int type = -1;
        if( typeobj )
            type = g_pComponentTypeManager->GetTypeByName( typeobj->valuestring );

        if( type == -1 )
        {
            LOGError( LOGTag, "Unknown component in scene file: %s\n", typeobj->valuestring );
            MyAssert( false );
        }
        else
        {
            unsigned int id;
            cJSONExt_GetUnsignedInt( componentobj, "ID", &id );

            ComponentBase* pComponent = FindComponentByID( id, sceneid );
            if( pComponent ) { MyAssert( pComponent->GetSceneID() == sceneid ); }
    
            if( pComponent == 0 )
                pComponent = pGameObject->AddNewComponent( type, sceneid );

            pComponent->ImportFromJSONObject( componentobj, sceneid );
            
            pComponent->SetID( id );
            if( id >= m_pSceneInfoMap[sceneid].m_NextComponentID )
                m_pSceneInfoMap[sceneid].m_NextComponentID = id + 1;
        }
    }

    cJSON_Delete( root );

    SyncAllRigidBodiesToObjectTransforms();
}

void ComponentSystemManager::FinishLoading(bool lockwhileloading, bool playwhenfinishedloading)
{
    m_StartGamePlayWhenDoneLoading = playwhenfinishedloading;
    m_WaitingForFilesToFinishLoading = lockwhileloading;

    if( lockwhileloading )
    {
        for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            MyAssert( pFileInfo && pFileInfo->m_pFile );
            if( pFileInfo->m_pFile->m_FileLoadStatus == FileLoadStatus_Loading )
                return;
        }

        m_WaitingForFilesToFinishLoading = false;
    }

    OnLoad();

    if( playwhenfinishedloading )
    {
        g_pEngineCore->RegisterGameplayButtons();
        OnPlay();
        m_StartGamePlayWhenDoneLoading = false;
    }
}

void ComponentSystemManager::SyncAllRigidBodiesToObjectTransforms()
{
    for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode; pNode = pNode->GetNext() )
    {
        if( ((ComponentBase*)pNode)->m_Type == ComponentType_CollisionObject )
        {
            ComponentCollisionObject* pComponent = (ComponentCollisionObject*)pNode;

            pComponent->SyncRigidBodyToTransform();
        }
    }
}

void ComponentSystemManager::UnloadScene(unsigned int sceneidtoclear, bool clearunmanagedcomponents)
{
    // Remove all components, except ones attached to unmanaged game objects(if wanted)
    {
        for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
        {
            for( CPPListNode* pNode = m_Components[i].GetHead(); pNode;  )
            {
                ComponentBase* pComponent = (ComponentBase*)pNode;
                
                pNode = pNode->GetNext();

                unsigned int sceneid = pComponent->GetSceneID();

                if( (pComponent->m_pGameObject->IsManaged() || clearunmanagedcomponents) &&
                    (sceneidtoclear == UINT_MAX || sceneid == sceneidtoclear) )
                {
                    DeleteComponent( pComponent );
                }
                else if( pComponent->GetID() > m_pSceneInfoMap[sceneid].m_NextComponentID )
                {
                    // not sure how this could happen.
                    MyAssert( false );
                    m_pSceneInfoMap[sceneid].m_NextComponentID = pComponent->GetID() + 1;
                }
            }
        }
    }

    // delete all game objects.
    {
        for( CPPListNode* pNode = m_GameObjects.GetHead(); pNode;  )
        {
            GameObject* pGameObject = (GameObject*)pNode;

            pNode = pNode->GetNext();

            unsigned int sceneid = pGameObject->GetSceneID();
            unsigned int gameobjectid = pGameObject->GetID();

            if( (pGameObject->IsManaged() || clearunmanagedcomponents) &&
                (sceneidtoclear == UINT_MAX || sceneid == sceneidtoclear) )
            {
                DeleteGameObject( pGameObject, true );
            }
            else if( sceneid == sceneidtoclear && gameobjectid > m_pSceneInfoMap[sceneid].m_NextGameObjectID )
            {
                m_pSceneInfoMap[sceneid].m_NextGameObjectID = gameobjectid + 1;
            }
        }
    }

    // release any file ref's added by this scene.
    FreeAllDataFiles( sceneidtoclear );

#if MYFW_USING_WX
    // erase the scene node from the object list tree.
    if( sceneidtoclear == UINT_MAX )
    {
        typedef std::map<int, SceneInfo>::iterator it_type;
        for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); )
        {
            unsigned int sceneid = iterator->first;
            SceneInfo* pSceneInfo = &iterator->second;

            if( sceneid != 0 && sceneid != EngineCore::ENGINE_SCENE_ID ) // don't clear the "Unmanaged" or ENGINE_SCENE_ID scenes
            {
                MyAssert( pSceneInfo->treeid.IsOk() );
                if( pSceneInfo->treeid.IsOk() )
                    g_pPanelObjectList->m_pTree_Objects->Delete( pSceneInfo->treeid );
                m_pSceneInfoMap.erase( iterator++ );
            }
            else
            {
                iterator++;
            }
        }
    }
    else if( sceneidtoclear != 0 ) // don't clear the "Unmanaged" label.
    {
        SceneInfo* pSceneInfo = GetSceneInfo( sceneidtoclear );
        if( pSceneInfo && pSceneInfo->treeid.IsOk() )
            g_pPanelObjectList->m_pTree_Objects->Delete( pSceneInfo->treeid );
        m_pSceneInfoMap.erase( sceneidtoclear );
    }
#endif
}

GameObject* ComponentSystemManager::CreateGameObject(bool manageobject, int sceneid)
{
    GameObject* pGameObject = MyNew GameObject( manageobject, sceneid );
    unsigned int id = GetNextGameObjectIDAndIncrement( sceneid );
    pGameObject->SetID( id );

    //if( manageobject )
        m_GameObjects.AddTail( pGameObject );

    return pGameObject;
}

void ComponentSystemManager::UnmanageGameObject(GameObject* pObject)
{
    MyAssert( pObject && pObject->IsManaged() == true );

    // remove all components from their respective component lists.
    for( unsigned int i=0; i<pObject->m_Components.Count(); i++ )
    {
        // remove from list and clear CPPListNode prev/next
        pObject->m_Components[i]->Remove();
        pObject->m_Components[i]->Prev = 0;
        pObject->m_Components[i]->Next = 0;
    }

    pObject->SetManaged( false );
}

void ComponentSystemManager::ManageGameObject(GameObject* pObject)
{
    MyAssert( pObject && pObject->IsManaged() == false );

    // add all the gameobject's components back into the component lists.
    for( unsigned int i=0; i<pObject->m_Components.Count(); i++ )
    {
        AddComponent( pObject->m_Components[i] );
    }

    pObject->SetManaged( true );
}

void ComponentSystemManager::DeleteGameObject(GameObject* pObject, bool deletecomponents)
{
#if MYFW_USING_WX
    if( g_pEngineCore->m_pEditorState->IsGameObjectSelected( pObject ) )
    {
        g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();
    }
#endif

    if( deletecomponents )
    {
        while( pObject->m_Components.Count() )
        {
            ComponentBase* pComponent = pObject->m_Components.RemoveIndex( 0 );

#if MYFW_USING_WX
            if( g_pEngineCore->m_pEditorState->IsComponentSelected( pComponent ) )
            {
                g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();
            }
#endif

            pComponent->SetEnabled( false );
            delete pComponent;
        }
    }

    SAFE_DELETE( pObject );
}

#if MYFW_USING_WX
GameObject* ComponentSystemManager::EditorCopyGameObject(GameObject* pObject, bool NewObjectInheritsFromOld)
{
    EditorCommand_CopyGameObject* pCommand = MyNew EditorCommand_CopyGameObject( pObject, NewObjectInheritsFromOld );
    g_pEngineMainFrame->m_pCommandStack->Do( pCommand );

    return pCommand->GetCreatedObject();
}
#endif

GameObject* ComponentSystemManager::CopyGameObject(GameObject* pObject, const char* newname)
{
    GameObject* pNewObject = CreateGameObject( true, pObject->GetSceneID() );
    pNewObject->SetName( newname );

    pNewObject->SetGameObjectThisInheritsFrom( pObject->GetGameObjectThisInheritsFrom() );

    *pNewObject->m_pComponentTransform = *pObject->m_pComponentTransform;

    for( unsigned int i=0; i<pObject->m_Components.Count(); i++ )
    {
        ComponentBase* pComponent = 0;

        if( g_pEngineCore->m_EditorMode )
            pComponent = pNewObject->AddNewComponent( pObject->m_Components[i]->m_Type, pNewObject->GetSceneID() );
        else
            pComponent = pNewObject->AddNewComponent( pObject->m_Components[i]->m_Type, 0 );

        pComponent->CopyFromSameType_Dangerous( pObject->m_Components[i] );

        pComponent->OnLoad();
    }

    return pNewObject;
}

unsigned int ComponentSystemManager::GetNextGameObjectIDAndIncrement(unsigned int sceneid)
{
    SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( sceneid );
    
    MyAssert( pSceneInfo );
    if( pSceneInfo == 0 )
        return 0; // we have problems if this happens.

    pSceneInfo->m_NextGameObjectID++;
    return pSceneInfo->m_NextGameObjectID-1;
}

unsigned int ComponentSystemManager::GetNextComponentIDAndIncrement(unsigned int sceneid)
{
    SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( sceneid );
    
    MyAssert( pSceneInfo );
    if( pSceneInfo == 0 )
        return 0; // we have problems if this happens.

    pSceneInfo->m_NextComponentID++;
    return pSceneInfo->m_NextComponentID-1;
}

GameObject* ComponentSystemManager::FindGameObjectByID(unsigned int sceneid, unsigned int goid)
{
    for( CPPListNode* node = m_GameObjects.GetHead(); node != 0; node = node->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)node;

        if( pGameObject->IsManaged() && pGameObject->GetSceneID() == sceneid && pGameObject->GetID() == goid )
            return pGameObject;
    }

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByName(const char* name)
{
    for( CPPListNode* node = m_GameObjects.GetHead(); node != 0; node = node->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)node;

        if( strcmp( pGameObject->GetName(), name ) == 0 )
            return pGameObject;
    }

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByJSONRef(cJSON* pJSONGameObjectRef, unsigned int defaultsceneid)
{
    // see GameObject::ExportReferenceAsJSONObject

    cJSON* jScenePath = cJSON_GetObjectItem( pJSONGameObjectRef, "Scene" );
    unsigned int sceneid = defaultsceneid;
    if( jScenePath )
    {
        sceneid = GetSceneIDFromFullpath( jScenePath->valuestring );
        if( sceneid == -1 )
            return 0; // scene isn't loaded, so object can't be found.
        //TODO: saving will throw all reference info away and piss people off :)
    }

    unsigned int goid = -1;
    cJSONExt_GetUnsignedInt( pJSONGameObjectRef, "GOID", &goid );
    MyAssert( goid != -1 );

    for( CPPListNode* node = m_GameObjects.GetHead(); node != 0; node = node->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)node;

        if( pGameObject->GetSceneID() == sceneid && pGameObject->GetID() == goid )
            return pGameObject;
    }

    return 0;
}

ComponentCamera* ComponentSystemManager::GetFirstCamera()
{
    for( CPPListNode* node = m_Components[BaseComponentType_Camera].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pCamera = (ComponentCamera*)node;

        // skip unmanaged cameras. (editor cam)
        if( pCamera->m_pGameObject->IsManaged() == true )
        {
            MyAssert( pCamera->m_Type == ComponentType_Camera );

            return pCamera;
        }
    }

    return 0;
}

ComponentBase* ComponentSystemManager::GetFirstComponentOfType(const char* type)
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            if( ((ComponentBase*)node)->IsA( type ) )
                return (ComponentBase*)node;
        }
    }

    return 0; // component not found.
}

ComponentBase* ComponentSystemManager::GetNextComponentOfType(ComponentBase* pLastComponent)
{
    MyAssert( pLastComponent != 0 );

    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        bool foundlast = false;
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            if( pLastComponent == node )
                foundlast = true;
            else if( foundlast && ((ComponentBase*)node)->IsA( pLastComponent->GetClassname() ) )
                return (ComponentBase*)node;
        }
    }

    return 0; // component not found.
}

ComponentBase* ComponentSystemManager::AddComponent(ComponentBase* pComponent)
{
    MyAssert( pComponent->m_BaseType >= 0 && pComponent->m_BaseType < BaseComponentType_NumTypes ); // shouldn't happen.

    m_Components[pComponent->m_BaseType].AddTail( pComponent );

    return pComponent;
}

void ComponentSystemManager::DeleteComponent(ComponentBase* pComponent)
{
    if( pComponent->m_pGameObject )
    {
        pComponent->m_pGameObject->RemoveComponent( pComponent );
    }

#if MYFW_USING_WX
    g_pPanelObjectList->RemoveObject( pComponent );
#endif

    pComponent->SetEnabled( false );
    SAFE_DELETE( pComponent );
}

ComponentBase* ComponentSystemManager::FindComponentByID(unsigned int id, unsigned int sceneid)
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode;  )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            pNode = pNode->GetNext();
            if( pComponent->GetID() == id && pComponent->GetSceneID() == sceneid )
                return pComponent;
        }
    }

    return 0;
}

void ComponentSystemManager::Tick(double TimePassed)
{
    if( m_WaitingForFilesToFinishLoading )
    {
        FinishLoading( true, m_StartGamePlayWhenDoneLoading );
    }

#if MYFW_USING_WX
    CheckForUpdatedDataSourceFiles( true );
#endif

    // update all Components:

    // all scripts.
    //for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    //{
    //    ComponentUpdateable* pComponent = (ComponentUpdateable*)node;

    //    if( pComponent->m_BaseType == BaseComponentType_Updateable && pComponent->m_Type == ComponentType_LuaScript )
    //    {
    //        pComponent->Tick( TimePassed );
    //    }
    //}

    // then all other "Updateables".
    for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentUpdateable* pComponent = (ComponentUpdateable*)node;

        if( pComponent->m_BaseType == BaseComponentType_Updateable ) //&& pComponent->m_Type != ComponentType_LuaScript )
        {
            pComponent->Tick( TimePassed );
        }
    }

    // update all components that registered a tick callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_Tick.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_Tick* pCallbackStruct = (ComponentCallbackStruct_Tick*)pNode;
        MyAssert( pCallbackStruct->pFunc != 0 );

        (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( TimePassed );
    }

    // update all cameras after game objects are updated.
    for( CPPListNode* node = m_Components[BaseComponentType_Camera].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pComponent = (ComponentCamera*)node;

        if( pComponent->m_BaseType == BaseComponentType_Camera )
        {
            pComponent->Tick( TimePassed );
        }
    }
}

void ComponentSystemManager::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    for( CPPListNode* node = m_Components[BaseComponentType_Camera].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pCamera = (ComponentCamera*)node;

        if( pCamera->m_BaseType == BaseComponentType_Camera )
        {
            // TODO: fix this hack, don't resize unmanaged cams (a.k.a. editor camera)
            if( pCamera->m_pGameObject->IsManaged() == true )
            {
                pCamera->OnSurfaceChanged( startx, starty, width, height, desiredaspectwidth, desiredaspectheight );
            }
        }
    }

    //// For now, tell menu pages that the aspect ratio changed.
    //for( CPPListNode* node = m_Components[BaseComponentType_MenuPage].GetHead(); node != 0; node = node->GetNext() )
    //{
    //    ComponentMenuPage* pComponent = (ComponentMenuPage*)node;

    //    pComponent->OnSurfaceChanged( startx, starty, width, height, desiredaspectwidth, desiredaspectheight );
    //}

    // notify all components that registered a callback of a change to the surfaces/aspect ratio.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnSurfaceChanged.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnSurfaceChanged* pCallbackStruct = (ComponentCallbackStruct_OnSurfaceChanged*)pNode;

        (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( startx, starty, width, height, desiredaspectwidth, desiredaspectheight );
    }
}

void ComponentSystemManager::OnDrawFrame()
{
    for( CPPListNode* node = m_Components[BaseComponentType_Camera].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentCamera* pComponent = (ComponentCamera*)node;

        if( pComponent->m_BaseType == BaseComponentType_Camera && pComponent->IsEnabled() == true )
        {
            pComponent->OnDrawFrame();
        }
    }
}

void ComponentSystemManager::OnDrawFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    // draw all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_Draw.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_Draw* pCallbackStruct = (ComponentCallbackStruct_Draw*)pNode;
        ComponentBase* pComponent = (ComponentBase*)pCallbackStruct->pObj;

        if( pComponent->ExistsOnLayer( pCamera->m_LayersToRender ) )
        {
            if( pComponent->IsVisible() )
            {
                (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( pCamera, pMatViewProj, pShaderOverride );
            }
        }
    }
}

void ComponentSystemManager::OnFileRenamed(const char* fullpathbefore, const char* fullpathafter)
{
    // call all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnFileRenamed.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnFileRenamed* pCallbackStruct = (ComponentCallbackStruct_OnFileRenamed*)pNode;

        (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( fullpathbefore, fullpathafter );
    }
}

void ComponentSystemManager::MoveInputHandlersToFront(CPPListNode* pOnTouch, CPPListNode* pOnButtons, CPPListNode* pOnKeys)
{
    if( pOnTouch )
        m_pComponentCallbackList_OnTouch.MoveHead( pOnTouch );
    if( pOnButtons )
        m_pComponentCallbackList_OnButtons.MoveHead( pOnButtons );
    if( pOnKeys )
        m_pComponentCallbackList_OnKeys.MoveHead( pOnKeys );
}

void ComponentSystemManager::DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    // always use 4 bone version.
    // TODO: this might fail with 1-3 bones, but works with 0 since bone attribs and uniforms should default to 0.
    Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    if( pShader->ActivateAndProgramShader() )
    {
        // draw all components that registered a callback.
        for( CPPListNode* pNode = m_pComponentCallbackList_Draw.HeadNode.Next; pNode->Next; pNode = pNode->Next )
        {
            ComponentCallbackStruct_Draw* pCallbackStruct = (ComponentCallbackStruct_Draw*)pNode;

            ComponentBase* pComponent = (ComponentBase*)pCallbackStruct->pObj;

            {
                if( pComponent->IsVisible() &&
                    pComponent->ExistsOnLayer( pCamera->m_LayersToRender ) != 0 &&
                    pComponent->ExistsOnLayer( Layer_EditorUnselectable ) == 0 )
                {
                    ColorByte tint( 0, 0, 0, 0 );

                    unsigned int sceneid = pComponent->m_pGameObject->GetSceneID();
                    unsigned int id = pComponent->m_pGameObject->GetID();
                    
                    id = (sceneid * 100000 + id) * 641; // 1, 641, 6700417, 4294967297, 

                    if( 1 )                 tint.r = id%256;
                    if( id > 256 )          tint.g = (id>>8)%256;
                    if( id > 256*256 )      tint.b = (id>>16)%256;
                    if( id > 256*256*256 )  tint.a = (id>>24)%256;

                    pShader->ProgramTint( tint );

                    (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( pCamera, pMatViewProj, pShaderOverride );
                }
            }
        }

        pShader->DeactivateShader();
    }
}

#if !MYFW_USING_WX
SceneInfo* ComponentSystemManager::GetSceneInfo(int sceneid)
{
    MyAssert( sceneid >= 0 && sceneid < 10 );
    return &m_pSceneInfoMap[sceneid];
}

unsigned int ComponentSystemManager::GetSceneIDFromFullpath(const char* fullpath)
{
    for( int i=0; i<10; i++ )
    {
        unsigned int sceneid = i;
        SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

        if( strcmp( pSceneInfo->fullpath, fullpath ) == 0 )
            return sceneid;
    }

    MyAssert( false ); // fullpath not found, that's fine when used from gameobject loading.
    return -1;
}
#else
void ComponentSystemManager::CreateNewScene(const char* scenename, unsigned int sceneid)
{
    MyAssert( m_pSceneInfoMap[sceneid].treeid.IsOk() == false );

    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    wxTreeItemId treeid = g_pPanelObjectList->AddObject( m_pSceneHandler, SceneHandler::StaticOnLeftClick, SceneHandler::StaticOnRightClick, rootid, scenename );
    g_pPanelObjectList->SetDragAndDropFunctions( treeid, SceneHandler::StaticOnDrag, SceneHandler::StaticOnDrop );
    m_pSceneInfoMap[sceneid].treeid = treeid;
}

wxTreeItemId ComponentSystemManager::GetTreeIDForScene(int sceneid)
{
    return m_pSceneInfoMap[sceneid].treeid;
}

unsigned int ComponentSystemManager::GetSceneIDFromFullpath(const char* fullpath)
{
    typedef std::map<int, SceneInfo>::iterator it_type;
    for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); iterator++ )
    {
        unsigned int sceneid = iterator->first;
        SceneInfo* pSceneInfo = &iterator->second;

        if( strcmp( pSceneInfo->fullpath, fullpath ) == 0 )
            return sceneid;
    }

    MyAssert( false ); // fullpath not found, that's fine when used from gameobject loading.
    return -1;
}

unsigned int ComponentSystemManager::GetSceneIDFromSceneTreeID(wxTreeItemId treeid)
{
    typedef std::map<int, SceneInfo>::iterator it_type;
    for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); iterator++ )
    {
        unsigned int sceneid = iterator->first;
        SceneInfo* pSceneInfo = &iterator->second;

        if( pSceneInfo->treeid == treeid )
            return sceneid;
    }

    MyAssert( false ); // treeid not found, it should be.
    return -1;
}

SceneInfo* ComponentSystemManager::GetSceneInfo(int sceneid)
{
    return &m_pSceneInfoMap[sceneid];
}

void ComponentSystemManager::DrawSingleObject(MyMatrix* pMatViewProj, GameObject* pObject)
{
    for( unsigned int i=0; i<pObject->m_Components.Count(); i++ )
    {
        ComponentRenderable* pComponent = dynamic_cast<ComponentRenderable*>( pObject->m_Components[i] );

        if( pComponent )
        {
            pComponent->Draw( pMatViewProj );
        }
    }
}
#endif //MYFW_USING_WX

void ComponentSystemManager::OnLoad()
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)node;
            pComponent->OnLoad();
        }
    }
}

void ComponentSystemManager::OnPlay()
{
    for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentBase* pComponent = (ComponentBase*)node;

        MyAssert( pComponent->IsEnabled() == true );
        
        if( pComponent->m_pGameObject->IsEnabled() == true )
            pComponent->OnPlay();
    }
}

void ComponentSystemManager::OnStop()
{
    for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentBase* pComponent = (ComponentBase*)node;
        pComponent->OnStop();
    }
}

bool ComponentSystemManager::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    // send touch events to all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnTouch.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnTouch* pCallbackStruct = (ComponentCallbackStruct_OnTouch*)pNode;

        if( (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( action, id, x, y, pressure, size ) )
            return true;
    }

    // then regular scene input handlers.
    for( CPPListNode* node = m_Components[BaseComponentType_InputHandler].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)node;

        if( pComponent->m_BaseType == BaseComponentType_InputHandler )
        {
            if( pComponent->OnTouch( action, id, x, y, pressure, size ) == true )
                return true;
        }
    }

    //// then send input to all the scripts.
    //for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    //{
    //    ComponentLuaScript* pComponent = ((ComponentBase*)node)->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)node : 0;

    //    if( pComponent )
    //    {
    //        if( pComponent->OnTouch( action, id, x, y, pressure, size ) == true )
    //            return true;
    //    }
    //}

    return false;
}

bool ComponentSystemManager::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    // send button events to all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnButtons.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnButtons* pCallbackStruct = (ComponentCallbackStruct_OnButtons*)pNode;

        if( (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( action, id ) )
            return true;
    }

    // then regular scene input handlers.
    for( CPPListNode* node = m_Components[BaseComponentType_InputHandler].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)node;

        if( pComponent->m_BaseType == BaseComponentType_InputHandler )
        {
            if( pComponent->OnButtons( action, id ) == true )
                return true;
        }
    }

    //// then send input to all the scripts.
    //for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    //{
    //    ComponentLuaScript* pComponent = ((ComponentBase*)node)->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)node : 0;

    //    if( pComponent )
    //    {
    //        if( pComponent->OnButtons( action, id ) == true )
    //            return true;
    //    }
    //}

    return false;
}

bool ComponentSystemManager::OnKeys(GameCoreButtonActions action, int keycode, int unicodechar)
{
    // send keyboard events to all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnKeys.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnKeys* pCallbackStruct = (ComponentCallbackStruct_OnKeys*)pNode;

        if( (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( action, keycode, unicodechar ) )
            return true;
    }

    // then regular scene input handlers.
    for( CPPListNode* node = m_Components[BaseComponentType_InputHandler].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)node;

        if( pComponent->m_BaseType == BaseComponentType_InputHandler )
        {
            if( pComponent->OnKeys( action, keycode, unicodechar ) == true )
                return true;
        }
    }

    //// then send input to all the scripts.
    //for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    //{
    //    ComponentLuaScript* pComponent = ((ComponentBase*)node)->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)node : 0;

    //    if( pComponent )
    //    {
    //        if( pComponent->OnKeys( action, keycode, unicodechar ) == true )
    //            return true;
    //    }
    //}

    return false;
}
