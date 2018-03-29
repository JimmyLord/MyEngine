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
#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Base.h"
#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Flat.h"
#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Octree.h"

#if MYFW_EDITOR
#include "../SourceEditor/Exporters/ExportBox2DScene.h"
#endif //MYFW_EDITOR

ComponentSystemManager* g_pComponentSystemManager = 0;

ComponentSystemManager::ComponentSystemManager(ComponentTypeManager* typemanager)
{
    g_pComponentSystemManager = this;

    m_pComponentTypeManager = typemanager;
#if MYFW_EDITOR
    m_pSceneHandler = MyNew SceneHandler();
    m_pGameObjectTemplateManager = MyNew GameObjectTemplateManager();
#endif

    m_pPrefabManager = MyNew PrefabManager();

#if MYFW_EDITOR
    g_pMaterialManager->RegisterMaterialCreatedCallback( this, StaticOnMaterialCreated );
    g_pFileManager->RegisterFileUnloadedCallback( this, StaticOnFileUnloaded );
    g_pFileManager->RegisterFindAllReferencesCallback( this, StaticOnFindAllReferences );
    g_pGameCore->GetSoundManager()->RegisterSoundCueCreatedCallback( this, StaticOnSoundCueCreated );
    g_pGameCore->GetSoundManager()->RegisterSoundCueUnloadedCallback( this, StaticOnSoundCueUnloaded );

    // This class adds to SoundCue's refcount when storing cue in m_Files
    //    so increment this number to prevent editor from allowing it to be unloaded if ref'ed by game code
    g_pGameCore->GetSoundManager()->Editor_AddToNumRefsPlacedOnSoundCueBySystem();
#endif

    m_NextSceneID = SCENEID_MainScene;

    m_WaitingForFilesToFinishLoading = false;
    m_StartGamePlayWhenDoneLoading = false;

    m_TimeScale = 1;

    //m_pSceneGraph = MyNew SceneGraph_Flat();
    int depth = 3;
    m_pSceneGraph = MyNew SceneGraph_Octree( depth, -32, -32, -32, 32, 32, 32 );

    for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        m_pSceneInfoMap[i].Reset();
    }

#if MYFW_USING_WX
    // Add icons to the object list tree
    {
        wxImageList* pImageList = new wxImageList(16,16);

        wxBitmap bitmap_scene( "Data/DataEngine/EditorIcons/IconScene.bmp", wxBITMAP_TYPE_BMP );
        wxBitmap bitmap_gameobject( "Data/DataEngine/EditorIcons/IconGameObject.bmp", wxBITMAP_TYPE_BMP );
        wxBitmap bitmap_folder( "Data/DataEngine/EditorIcons/IconFolder.bmp", wxBITMAP_TYPE_BMP );// = wxArtProvider::GetBitmap( wxART_FOLDER, wxART_OTHER, wxSize(16,16) );
        wxBitmap bitmap_logicobject( "Data/DataEngine/EditorIcons/IconLogicObject.bmp", wxBITMAP_TYPE_BMP );// = wxArtProvider::GetBitmap( wxART_FOLDER, wxART_OTHER, wxSize(16,16) );
        wxBitmap bitmap_component( "Data/DataEngine/EditorIcons/IconComponent.bmp", wxBITMAP_TYPE_BMP );
        wxBitmap bitmap_prefab( "Data/DataEngine/EditorIcons/IconPrefab.bmp", wxBITMAP_TYPE_BMP );

        // make sure bitmaps loaded
        //    will happen if DataEngine folder isn't there... run "Windows-CreateSymLinksForData.bat"
        MyAssert( bitmap_scene.IsOk() );

        if( bitmap_scene.IsOk() )
        {
            // Order added must match ObjectListIconTypes enum order
            pImageList->Add( bitmap_scene );          // ObjectListIcon_Scene,
            pImageList->Add( bitmap_gameobject );     // ObjectListIcon_GameObject,
            pImageList->Add( bitmap_folder );         // ObjectListIcon_Folder,
            pImageList->Add( bitmap_logicobject );    // ObjectListIcon_LogicObject,
            pImageList->Add( bitmap_component );      // ObjectListIcon_Component,
            pImageList->Add( bitmap_prefab );         // ObjectListIcon_Prefab,
        }

        g_pPanelObjectList->AssignImageListToObjectTree( pImageList );
    }

    // Add click callbacks to the root of the objects tree
    g_pPanelObjectList->SetTreeRootData( this, ComponentSystemManager::StaticOnLeftClick, ComponentSystemManager::StaticOnRightClick );

    // Create a scene for "Unmanaged/Runtime" objects.
    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    wxTreeItemId treeid = g_pPanelObjectList->AddObject( m_pSceneHandler, SceneHandler::StaticOnLeftClick, SceneHandler::StaticOnRightClick, rootid, "Unmanaged", ObjectListIcon_Scene );
    g_pPanelObjectList->SetDragAndDropFunctions( treeid, SceneHandler::StaticOnDrag, SceneHandler::StaticOnDrop );
    SceneInfo scene;
    m_pSceneInfoMap[SCENEID_Unmanaged].m_InUse = true;
    m_pSceneInfoMap[SCENEID_Unmanaged].m_TreeID = treeid;

    // Mark the scene for engine object as being used, so it will be unloaded.
    m_pSceneInfoMap[SCENEID_EngineObjects].m_InUse = true;
#else
    // Create a scene for "Unmanaged/Runtime" objects.
    m_pSceneInfoMap[SCENEID_Unmanaged].m_InUse = true;
#endif //MYFW_USING_WX
}

ComponentSystemManager::~ComponentSystemManager()
{
    // Prefab objects are in scene 0, so let the PrefabManager delete them before the sceneinfo does below.
    SAFE_DELETE( m_pPrefabManager );

    // Unload all runtime created objects.
    UnloadScene( SCENEID_Unmanaged, false );

    // Reset all scenes, i.e. Delete all GameObjects from each scene
    for( unsigned int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse == false )
            continue;

        SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

        pSceneInfo->Reset();
    }

    while( m_Files.GetHead() )
        delete m_Files.RemHead();

    while( m_FilesStillLoading.GetHead() )
        delete m_FilesStillLoading.RemHead();

#if MYFW_EDITOR
    SAFE_DELETE( m_pSceneHandler );
    SAFE_DELETE( m_pGameObjectTemplateManager );
#endif
    SAFE_DELETE( m_pComponentTypeManager );
    
    SAFE_DELETE( m_pSceneGraph );

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
        MyAssert( pCallbackStruct->pFunc != 0 && pCallbackStruct->pObj != 0 && pCallbackStruct->Prev == 0 && pCallbackStruct->Next == 0 ); \
        m_pComponentCallbackList_##CallbackType.AddTail( pCallbackStruct ); \
    } \
    void ComponentSystemManager::UnregisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct) \
    { \
        if( pCallbackStruct->Prev ) \
        { \
            pCallbackStruct->Remove(); \
            pCallbackStruct->Next = 0; \
            pCallbackStruct->Prev = 0; \
        } \
    }

MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( Tick );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnSurfaceChanged );
//MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( Draw ); // declared manually below
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnTouch );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnButtons );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnKeys );
MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS( OnFileRenamed );

void ComponentSystemManager::RegisterComponentCallback_Draw(ComponentCallbackStruct_Draw* pCallbackStruct)
{
    MyAssert( pCallbackStruct->pFunc != 0 && pCallbackStruct->pObj != 0 );
    m_pComponentCallbackList_Draw.AddTail( pCallbackStruct );
}

void ComponentSystemManager::UnregisterComponentCallback_Draw(ComponentCallbackStruct_Draw* pCallbackStruct)
{
    if( pCallbackStruct->Prev )
        pCallbackStruct->Remove();
}

#if MYFW_USING_LUA
void ComponentSystemManager::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSystemManager>( "ComponentSystemManager" )
            .addFunction( "SetTimeScale", &ComponentSystemManager::SetTimeScale ) // void ComponentSystemManager::SetTimeScale(float scale)
            .addFunction( "CreateGameObject", &ComponentSystemManager::CreateGameObject ) // GameObject* ComponentSystemManager::CreateGameObject(bool manageobject, SceneID sceneid, bool isfolder, bool hastransform, PrefabReference* pPrefabRef)
            .addFunction( "DeleteGameObject", &ComponentSystemManager::DeleteGameObject ) // void ComponentSystemManager::DeleteGameObject(GameObject* pObject, bool deletecomponents)
            .addFunction( "CopyGameObject", &ComponentSystemManager::CopyGameObject ) // GameObject* ComponentSystemManager::CopyGameObject(GameObject* pObject, const char* newname)
            .addFunction( "FindGameObjectByName", &ComponentSystemManager::FindGameObjectByName ) // GameObject* ComponentSystemManager::FindGameObjectByName(const char* name)
        .endClass();
}
#endif //MYFW_USING_LUA

void ComponentSystemManager::MoveAllFilesNeededForLoadingScreenToStartOfFileList(GameObject* first)
{
    MyAssert( first != 0 );
    if( first == 0 )
        return;

    for( CPPListNode* pNode = first; pNode; pNode = pNode->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)pNode;

        if( strncmp( pGameObject->GetName(), "Load", 4 ) == 0 )
        {
            for( unsigned int i=0; i<pGameObject->GetComponentCount(); i++ )
            {
                ComponentBase* pComponent = pGameObject->GetComponentByIndex( i );

                // move sprite material files to front of list.
                if( pComponent->IsA( "SpriteComponent" ) )
                {
                    MySprite* pSprite = ((ComponentSprite*)pComponent)->m_pSprite;
                    if( pSprite && pSprite->GetMaterial() )
                    {
                        pSprite->GetMaterial()->MoveAssociatedFilesToFrontOfFileList();
                    }
                }

#if MYFW_USING_LUA
                // move lua scripts to front of list.
                if( pComponent->IsA( "LuaScriptComponent" ) )
                {
                    MyFileObject* pScriptFile = ((ComponentLuaScript*)pComponent)->GetScriptFile();
                    if( pScriptFile )
                    {
                        g_pFileManager->MoveFileToFrontOfFileLoadedList( pScriptFile );
                    }
                }
#endif //MYFW_USING_LUA
            }
        }

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            MoveAllFilesNeededForLoadingScreenToStartOfFileList( pFirstChild );
        }
    }
}

void ComponentSystemManager::AddListOfFilesUsedToJSONObject(SceneID sceneid, cJSON* filearray)
{
    // TODO: there are currently many ways a file can be loaded into a secondary scene without being in this list.
    //       need to adjust code in various components to account for this.

    // loop through both lists of files
    for( int filelist=0; filelist<2; filelist++ )
    {
        CPPListNode* pFirstNode = 0;

        if( filelist == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNode->GetNext() )
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
                cJSON_AddItemToObject( jFile, "Path", cJSON_CreateString( pFile->GetFullPath() ) );
                cJSON_AddItemToArray( filearray, jFile );

                // Save the source path if there is one.
                if( pFileInfo->m_SourceFileFullPath[0] != 0 )
                {
                    cJSON_AddItemToObject( jFile, "SourcePath", cJSON_CreateString( pFileInfo->m_SourceFileFullPath ) );
                }
            }
            else
            {
                cJSON* jFile = cJSON_CreateObject();
                cJSON_AddItemToObject( jFile, "Path", cJSON_CreateString( pFileInfo->m_SourceFileFullPath ) );
                cJSON_AddItemToArray( filearray, jFile );
            }
        }
    }
}

char* ComponentSystemManager::SaveSceneToJSON(SceneID sceneid)
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

    bool savingallscenes = (sceneid == SCENEID_TempPlayStop);

    // move files used by gameobjects that start with "Load" to front of file list.
    {
#if 0 //MYFW_USING_WX
        typedef std::map<int, SceneInfo>::iterator it_type;
        for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); )
        {
            SceneID sceneid = iterator->first;
            SceneInfo* pSceneInfo = &iterator->second;
#else
        for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( m_pSceneInfoMap[i].m_InUse == false )
                continue;

            SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];
#endif //MYFW_USING_WX

            if( (GameObject*)pSceneInfo->m_GameObjects.GetHead() )
            {
                MoveAllFilesNeededForLoadingScreenToStartOfFileList( (GameObject*)pSceneInfo->m_GameObjects.GetHead() );
            }
        }
    }

    // add the files used.
    AddListOfFilesUsedToJSONObject( sceneid, filearray );

    // add the game objects and their transform components.
    {
#if 0 //MYFW_USING_WX
        typedef std::map<int, SceneInfo>::iterator it_type;
        for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); )
        {
            SceneID sceneid = iterator->first;
            SceneInfo* pSceneInfo = &iterator->second;
#else
        for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( m_pSceneInfoMap[i].m_InUse == false )
                continue;

            SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];
#endif //MYFW_USING_WX

            GameObject* first = (GameObject*)pSceneInfo->m_GameObjects.GetHead();
            if( first && ( first->GetSceneID() == sceneid || savingallscenes ) )
            {
                SaveGameObjectListToJSONArray( gameobjectarray, transformarray, first, savingallscenes );
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
                    cJSON_AddItemToArray( componentarray, pComponent->ExportAsJSONObject( savingallscenes, true ) );
                }
            }
        }
    }

    char* savestring = cJSON_Print( root );
    cJSON_Delete(root);

    return savestring;
}

char* ComponentSystemManager::ExportBox2DSceneToJSON(SceneID sceneid)
{
#if MYFW_EDITOR
    return ::ExportBox2DSceneToJSON( this, sceneid );
#else
    return 0;
#endif //MYFW_EDITOR
}

void ComponentSystemManager::SaveGameObjectListToJSONArray(cJSON* gameobjectarray, cJSON* transformarray, GameObject* first, bool savesceneid)
{
    for( CPPListNode* pNode = first; pNode; pNode = pNode->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)pNode;

        // Only save managed GameObjects, they will be marked as unmanaged if deleted and still in undo list.
        if( pGameObject->IsManaged() )
        {
            cJSON_AddItemToArray( gameobjectarray, pGameObject->ExportAsJSONObject( savesceneid ) );

            ComponentBase* pComponent = pGameObject->GetTransform();
            if( pComponent )
                cJSON_AddItemToArray( transformarray, pComponent->ExportAsJSONObject( savesceneid, true ) );

            GameObject* pFirstChild = pGameObject->GetFirstChild();
            if( pFirstChild )
            {
                SaveGameObjectListToJSONArray( gameobjectarray, transformarray, pFirstChild, savesceneid );
            }
        }
    }
}

MyFileInfo* ComponentSystemManager::GetFileInfoIfUsedByScene(const char* fullpath, SceneID sceneid)
{
    // loop through both lists of files
    for( int filelist=0; filelist<2; filelist++ )
    {
        CPPListNode* pFirstNode = 0;

        if( filelist == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNode->GetNext() )
        {
            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            if( sceneid == SCENEID_AllScenes || pFileInfo->m_SceneID == sceneid )
            {
                if( pFileInfo->m_pFile == 0 )
                {
                    if( strcmp( pFileInfo->m_SourceFileFullPath, fullpath ) == 0 )
                        return pFileInfo;
                }
                else
                {
                    if( strcmp( pFileInfo->m_pFile->GetFullPath(), fullpath ) == 0 )
                        return pFileInfo;
                }
            }
        }
    }

    return 0;
}

MyFileObject* ComponentSystemManager::GetFileObjectIfUsedByScene(const char* fullpath, SceneID sceneid)
{
    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( fullpath, sceneid );

    if( pFileInfo )
        return pFileInfo->m_pFile;

    return 0;
}

MyFileInfo* ComponentSystemManager::AddToFileList(MyFileObject* pFile, MyMesh* pMesh, ShaderGroup* pShaderGroup, TextureDefinition* pTexture, MaterialDefinition* pMaterial, SoundCue* pSoundCue, SpriteSheet* pSpriteSheet, SceneID sceneid)
{
    // store pFile so we can free it afterwards.
    MyFileInfo* pFileInfo = MyNew MyFileInfo();
    pFileInfo->m_pFile = pFile;

    pFileInfo->m_pMesh = pMesh;
    pFileInfo->m_pShaderGroup = pShaderGroup;
    pFileInfo->m_pTexture = pTexture;
    pFileInfo->m_pMaterial = pMaterial;
    pFileInfo->m_pSoundCue = pSoundCue;
    pFileInfo->m_pSpriteSheet = pSpriteSheet;
    pFileInfo->m_pPrefabFile = 0;

    pFileInfo->m_SceneID = sceneid;

    m_Files.AddTail( pFileInfo );

    return pFileInfo;
}

MyFileObject* ComponentSystemManager::LoadDataFile(const char* relativepath, SceneID sceneid, const char* fullsourcefilepath, bool convertifrequired)
{
    MyAssert( relativepath );

    // each scene loaded will add ref's to the file.
    MyFileObject* pFile = GetFileObjectIfUsedByScene( relativepath, sceneid );

    // if the file is already tagged as being used by this scene, don't request/addref it.
    if( pFile != 0 )
    {
        return pFile;
        //LOGInfo( LOGTag, "%s already in scene, reloading\n", relativepath );
        //g_pFileManager->ReloadFile( pFile );
        //OnFileUpdated_CallbackFunction( pFile );
    }
    else
    {
        size_t fulllen = 0;
        if( fullsourcefilepath )
            fulllen = strlen( fullsourcefilepath );
        size_t rellen = strlen( relativepath );

#if MYFW_USING_WX
#if MYFW_WINDOWS
        if( convertifrequired && fullsourcefilepath )
        {
            WIN32_FIND_DATAA datafiledata;
            memset( &datafiledata, 0, sizeof( datafiledata ) );
            HANDLE datafilehandle = FindFirstFileA( relativepath, &datafiledata );
            if( datafilehandle != INVALID_HANDLE_VALUE )
                FindClose( datafilehandle );

            WIN32_FIND_DATAA sourcefiledata;
            memset( &sourcefiledata, 0, sizeof( sourcefiledata ) );
            HANDLE sourcefilehandle = FindFirstFileA( fullsourcefilepath, &sourcefiledata );
            if( sourcefilehandle != INVALID_HANDLE_VALUE )
                FindClose( sourcefilehandle );

            // If the source file is newer than the data file (or data file doesn't exist), reimport it.
            if( sourcefiledata.ftLastWriteTime.dwHighDateTime >= datafiledata.ftLastWriteTime.dwHighDateTime ||
                ( sourcefiledata.ftLastWriteTime.dwHighDateTime == datafiledata.ftLastWriteTime.dwHighDateTime &&
                  sourcefiledata.ftLastWriteTime.dwLowDateTime >= datafiledata.ftLastWriteTime.dwLowDateTime ) )
            {
                MyFileObject* pFile = ImportDataFile( sceneid, fullsourcefilepath );

                if( pFile )
                    return pFile;
            }
        }
#else
        if( convertifrequired && fullsourcefilepath )
        {
            struct stat datafiledata;
            memset( &datafiledata, 0, sizeof( datafiledata ) );
            stat( relativepath, &datafiledata );

            struct stat sourcefiledata;
            memset( &sourcefiledata, 0, sizeof( sourcefiledata ) );
            stat( fullsourcefilepath, &sourcefiledata );

            // If the source file is newer than the data file (or data file doesn't exist), reimport it.
            if( sourcefiledata.st_mtime >= datafiledata.st_mtime )
            {
                MyFileObject* pFile = ImportDataFile( sceneid, fullsourcefilepath );

                if( pFile )
                    return pFile;
            }
        }
#endif // Windows vs OSX/Linux
#endif //MYFW_USING_WX
        
        // store pFile so we can free it afterwards.
        MyFileInfo* pFileInfo = AddToFileList( pFile, 0, 0, 0, 0, 0, 0, sceneid );

        TextureDefinition* pTexture = 0;

        // load textures differently than other files.
        if( rellen > 4 && strcmp( &relativepath[rellen-4], ".png" ) == 0 )
        {
            // check if the texture is already loaded and create it if not.
            pTexture = g_pTextureManager->FindTexture( relativepath );
            if( pTexture == 0 )
            {
                // Find the file and add it to the object, 
                pFile = g_pEngineFileManager->RequestFile_UntrackedByScene( relativepath );
                pFileInfo->m_pFile = pFile;

                pTexture = g_pTextureManager->CreateTexture( pFile );
                MyAssert( pFile == pTexture->GetFile() );
                pFile = pTexture->GetFile();

                // Update the fileinfo block with the texture.
                pFileInfo->m_pTexture = pTexture;
            }
            else
            {
                MyAssert( false ); // the texture shouldn't be loaded and not in the list of files used.

                // Update the fileinfo block with the texture.
                pFileInfo->m_pTexture = pTexture;

                // Add a ref to the file for our scene file list.
                pFile = pTexture->GetFile();
                pFile->AddRef();
            }
        }
        else if( rellen > 4 && strcmp( &relativepath[rellen-4], ".wav" ) == 0 )
        {
#if !MYFW_USING_WX
            // raw wav's shouldn't be loadable in standalone builds, scenes should only reference sound cues
            MyAssert( false );
#else
            // Let SoundPlayer (SDL on windows) load the wav files
            SoundCue* pCue = g_pGameCore->GetSoundManager()->CreateCue( "new cue" );
            g_pGameCore->GetSoundManager()->AddSoundToCue( pCue, relativepath );
            pCue->SaveSoundCue( 0 );

            pFileInfo->m_pSoundCue = pCue;
            pFileInfo->m_pFile = pCue->GetFile();
            pFileInfo->m_pFile->AddRef();
            //strcpy_s( pFileInfo->m_SourceFileFullPath, MAX_PATH, relativepath );
#endif //!MYFW_USING_WX
            return 0;
        }
        else
        {
            // Call untracked request since we're in the tracking code, just to avoid unnecessary repeat of LoadDataFile() call.
            pFile = g_pEngineFileManager->RequestFile_UntrackedByScene( relativepath );
            pFileInfo->m_pFile = pFile;
#if MYFW_USING_WX
            // TODO: wasn't used and causes a crash in release mode... look into it.
            //pFile->SetCustomLeftClickCallback( StaticOnMemoryPanelFileSelectedLeftClick, this );
#endif
        }

        // if the extension of the source file is different than that of the file we're loading,
        //  then store the source file path in the fileobject, so we can detect/reconvert if that file changes.
        if( rellen > 4 && fulllen > 4 &&
            ( strcmp( &relativepath[rellen-4], &fullsourcefilepath[fulllen-4] ) != 0 ) )
        {
            char path[MAX_PATH];
            strcpy_s( path, MAX_PATH, fullsourcefilepath );
            const char* relativepath = GetRelativePath( path );

            // store the relative path if the file is relative... otherwise store the full path.
            if( relativepath == 0 )
                strcpy_s( pFileInfo->m_SourceFileFullPath, MAX_PATH, fullsourcefilepath );
            else
                strcpy_s( pFileInfo->m_SourceFileFullPath, MAX_PATH, relativepath );

            FixSlashesInPath( pFileInfo->m_SourceFileFullPath );
        }

        // if we're loading a mesh file type, create a mesh.
        {
            if( strcmp( pFile->GetExtensionWithDot(), ".obj" ) == 0 ||
                strcmp( pFile->GetExtensionWithDot(), ".mymesh" ) == 0 )
            {
                pFileInfo->m_pMesh = MyNew MyMesh();
                pFileInfo->m_pMesh->SetSourceFile( pFile );
            }
        }

        // if we're loading an .glsl file, create a ShaderGroup.
        if( strcmp( pFile->GetExtensionWithDot(), ".glsl" ) == 0 )
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

        // if we're loading a .mymaterial file, create a Material.
        if( strcmp( pFile->GetExtensionWithDot(), ".mymaterial" ) == 0 )
        {
            pFileInfo->m_pMaterial = g_pMaterialManager->LoadMaterial( pFile->GetFullPath() );
        }

        // if we're loading a .myprefabs file, add it to the prefab manager.
        if( strcmp( pFile->GetExtensionWithDot(), ".myprefabs" ) == 0 )
        {
            pFileInfo->m_pPrefabFile = m_pPrefabManager->RequestFile( pFile->GetFullPath() );
        }

        // if we're loading a .myspritesheet, we create a material for each texture in the sheet
        if( strcmp( pFile->GetExtensionWithDot(), ".myspritesheet" ) == 0 )
        {
            ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFilename( "Data/DataEngine/Shaders/Shader_TextureTint.glsl" );

            pFileInfo->m_pSpriteSheet = MyNew SpriteSheet();
            pFileInfo->m_pSpriteSheet->Create( pFile->GetFullPath(), pShaderGroup, GL_LINEAR, GL_LINEAR, false, true );

            m_FilesStillLoading.MoveHead( pFileInfo );
        }

        // if we're loading a .mycue file, create a Sound Cue.
        if( strcmp( pFile->GetExtensionWithDot(), ".mycue" ) == 0 )
        {
            pFileInfo->m_pSoundCue = g_pGameCore->GetSoundManager()->LoadCue( pFile->GetFullPath() );
        }
    }

    return pFile;
}

#if MYFW_USING_WX
MyFileObject* ComponentSystemManager::ImportDataFile(SceneID sceneid, const char* fullsourcefilepath)
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
        SHELLEXECUTEINFOA info = { 0 };
        info.cbSize = sizeof( SHELLEXECUTEINFOA );
        info.fMask = SEE_MASK_NOASYNC; //SEE_MASK_NOCLOSEPROCESS;
        info.hwnd = 0;
        info.lpVerb = 0;
        info.lpFile = "Tools\\MeshTool.exe";
        info.lpParameters = params;
        info.lpDirectory = workingdir;
        info.nShow = SW_SHOWNOACTIVATE;
        info.hInstApp = 0;

        DWORD errorcode = 1;
        BOOL success = ShellExecuteExA( &info );
        if( info.hProcess )
        {
            WaitForSingleObject( info.hProcess, INFINITE );
            GetExitCodeProcess( info.hProcess, &errorcode ); // Get actual return value from MeshTool.
            //TerminateProcess( info.hProcess );
            CloseHandle( info.hProcess );
        }
#elif MYFW_OSX
        char commandwithparams[PATH_MAX*3];
        sprintf( commandwithparams, "Tools/MeshTool %s", params );
        int errorcode = system( commandwithparams );
#else
        int errorcode = 1;
        LOGError( LOGTag, "Mesh conversion only works on Windows and OSX ATM\n" );
#endif

        if( errorcode != 0 )
        {
            LOGError( LOGTag, "Something went wrong with conversion: error(%d) %s\n", errorcode, fullsourcefilepath );
        }
        else
        {
            LOGInfo( LOGTag, "Conversion complete: %s\n", fullsourcefilepath );

#if MYFW_WINDOWS
            // The output file isn't found by loading code on Windows if this sleep isn't here.
            // TODO: find better solution.
            Sleep( 500 ); // Might not be needed since calling "WaitForSingleObject" above.
#endif

            // Create a new relative path.
            char newrelativepath[MAX_PATH];
            sprintf_s( newrelativepath, MAX_PATH, "Data/Meshes/%s.mymesh", filename );

            return LoadDataFile( newrelativepath, sceneid, fullsourcefilepath, false );
        }
    }

    return 0;
}
#endif //MYFW_USING_WX

void ComponentSystemManager::FreeDataFile(MyFileInfo* pFileInfo)
{
    delete pFileInfo;
}

void ComponentSystemManager::FreeAllDataFiles(SceneID sceneidtoclear)
{
    // loop through both lists of files
    for( int filelist=0; filelist<2; filelist++ )
    {
        CPPListNode* pFirstNode = 0;

        if( filelist == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        for( CPPListNode* pNode = pFirstNode; pNode;  )
        {
            MyFileInfo* pFile = (MyFileInfo*)pNode;
            pNode = pNode->GetNext();

            if( sceneidtoclear == SCENEID_AllScenes || pFile->m_SceneID == sceneidtoclear )
            {
                delete pFile;
            }
        }
    }
}

void ComponentSystemManager::LoadSceneFromJSON(const char* scenename, const char* jsonstr, SceneID sceneid)
{
    checkGlError( "ComponentSystemManager::LoadSceneFromJSON" );

    cJSON* root = cJSON_Parse( jsonstr );

    if( root == 0 )
        return;

    cJSON* filearray = cJSON_GetObjectItem( root, "Files" );
    cJSON* gameobjectarray = cJSON_GetObjectItem( root, "GameObjects" );
    cJSON* transformarray = cJSON_GetObjectItem( root, "Transforms" );
    cJSON* componentarray = cJSON_GetObjectItem( root, "Components" );

#if MYFW_EDITOR
    if( sceneid != SCENEID_TempPlayStop )
    {
        CreateNewScene( scenename, sceneid );
    }
#else
    // create the box2d world.
    MyAssert( sceneid >= SCENEID_MainScene && sceneid < MAX_SCENES_LOADED_INCLUDING_UNMANAGED );
    MyAssert( m_pSceneInfoMap[sceneid].m_pBox2DWorld == 0 );
    m_pSceneInfoMap[sceneid].m_pBox2DWorld = MyNew Box2DWorld( 0, 0, new EngineBox2DContactListener );
#endif //MYFW_EDITOR

    // request all files used by scene.
    if( filearray && sceneid != SCENEID_TempPlayStop )
    {
        for( int i=0; i<cJSON_GetArraySize( filearray ); i++ )
        {
            cJSON* jFile = cJSON_GetArrayItem( filearray, i );

            if( jFile->valuestring != 0 )
            {
                LoadDataFile( jFile->valuestring, sceneid, 0, true );
            }
            else
            {
                cJSON* jPath = cJSON_GetObjectItem( jFile, "Path" );
                cJSON* jSourcePath = cJSON_GetObjectItem( jFile, "SourcePath" );
                if( jPath )
                {
                    SceneID scenetosearch = sceneid;
                    if( sceneid == SCENEID_TempPlayStop )
                        scenetosearch = SCENEID_AllScenes;

                    // Pass the source path in the LoadDataFile call.
                    // If the file is missing or the source file is newer, we can re-import.
                    if( jSourcePath == 0 )
                        LoadDataFile( jPath->valuestring, scenetosearch, 0, false );
                    else
                        LoadDataFile( jPath->valuestring, scenetosearch, jSourcePath->valuestring, true );

                    // Find the file object and set it's source path.
                    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( jPath->valuestring, scenetosearch );
                    MyAssert( pFileInfo );
                    if( pFileInfo )
                    {
                        if( jSourcePath )
                        {
                            strcpy_s( pFileInfo->m_SourceFileFullPath, MAX_PATH, jSourcePath->valuestring );
                        }
                    }
                }
            }
        }
    }

    bool getsceneidfromeachobject = (sceneid == SCENEID_TempPlayStop);

    // Create/init all the game objects
    if( gameobjectarray )
    {
        for( int i=0; i<cJSON_GetArraySize( gameobjectarray ); i++ )
        {
            cJSON* jGameObject = cJSON_GetArrayItem( gameobjectarray, i );

            if( getsceneidfromeachobject )
                cJSONExt_GetUnsignedInt( jGameObject, "SceneID", (unsigned int*)&sceneid );

            unsigned int id = -1;
            cJSONExt_GetUnsignedInt( jGameObject, "ID", &id );
            MyAssert( id != -1 );

            // LEGACY: support for old scene files with folders in them, now stored as "SubType"
            bool isfolder = false;
            cJSONExt_GetBool( jGameObject, "IsFolder", &isfolder );

            bool hastransform = true;
            cJSON* jSubtype = cJSON_GetObjectItem( jGameObject, "SubType" );
            if( jSubtype )
            {
                if( strcmp( jSubtype->valuestring, "Folder" ) == 0 )
                {
                    isfolder = true;
                    hastransform = false;
                }
                else if( strcmp( jSubtype->valuestring, "Logic" ) == 0 )
                {
                    hastransform = false;
                }
            }

            // Find an existing game object with the same id or create a new one.
            GameObject* pGameObject = FindGameObjectByID( sceneid, id );
            if( pGameObject )
            {
                MyAssert( pGameObject->GetSceneID() == sceneid );
            }

            if( pGameObject == 0 )
            {
                pGameObject = CreateGameObject( true, sceneid, isfolder, hastransform );
            }

            pGameObject->ImportFromJSONObject( jGameObject, sceneid );
        
            unsigned int gameobjectid = pGameObject->GetID();
            if( gameobjectid > m_pSceneInfoMap[sceneid].m_NextGameObjectID )
                m_pSceneInfoMap[sceneid].m_NextGameObjectID = gameobjectid + 1;
        }
    }

    // setup all the game object transforms
    if( transformarray )
    {
        for( int i=0; i<cJSON_GetArraySize( transformarray ); i++ )
        {
            cJSON* transformobj = cJSON_GetArrayItem( transformarray, i );
        
            if( getsceneidfromeachobject )
                cJSONExt_GetUnsignedInt( transformobj, "SceneID", (unsigned int*)&sceneid );

            unsigned int goid = 0;
            cJSONExt_GetUnsignedInt( transformobj, "GOID", &goid );
            MyAssert( goid > 0 );

            GameObject* pGameObject = FindGameObjectByID( sceneid, goid );
            MyAssert( pGameObject );

            if( pGameObject )
            {
                pGameObject->SetID( goid );

                if( pGameObject->GetTransform() )
                {
                    pGameObject->GetTransform()->ImportFromJSONObject( transformobj, sceneid );
                }
                else
                {
                    unsigned int parentgoid = 0;
                    cJSONExt_GetUnsignedInt( transformobj, "ParentGOID", &parentgoid );
                
                    if( parentgoid > 0 )
                    {
                        GameObject* pParentGameObject = FindGameObjectByID( sceneid, parentgoid );
                        MyAssert( pParentGameObject );

                        pGameObject->SetParentGameObject( pParentGameObject );
                        if( pGameObject->GetTransform() )
                        {
                            pGameObject->GetTransform()->SetWorldTransformIsDirty();
                        }
                    }
                }

                if( goid >= m_pSceneInfoMap[sceneid].m_NextGameObjectID )
                    m_pSceneInfoMap[sceneid].m_NextGameObjectID = goid + 1;
            }
        }
    }

    // Load inheritance info (i.e. Parent GameObjects) for objects that inherit from others.
    if( gameobjectarray )
    {
        for( int i=0; i<cJSON_GetArraySize( gameobjectarray ); i++ )
        {
            cJSON* jGameObject = cJSON_GetArrayItem( gameobjectarray, i );

            if( getsceneidfromeachobject )
                cJSONExt_GetUnsignedInt( jGameObject, "SceneID", (unsigned int*)&sceneid );

            unsigned int id = -1;
            cJSONExt_GetUnsignedInt( jGameObject, "ID", &id );
            MyAssert( id != -1 );

            // Find the existing game object with the same id.
            GameObject* pGameObject = FindGameObjectByID( sceneid, id );
            MyAssert( pGameObject );

            pGameObject->ImportInheritanceInfoFromJSONObject( jGameObject );
        }
    }

    // create all the other components, not loading component properties
    if( componentarray )
    {
        for( int i=0; i<cJSON_GetArraySize( componentarray ); i++ )
        {
            cJSON* componentobj = cJSON_GetArrayItem( componentarray, i );
        
            if( getsceneidfromeachobject )
                cJSONExt_GetUnsignedInt( componentobj, "SceneID", (unsigned int*)&sceneid );

            unsigned int id = 0;
            cJSONExt_GetUnsignedInt( componentobj, "GOID", &id );
            MyAssert( id > 0 );
            GameObject* pGameObject = FindGameObjectByID( sceneid, id );
            MyAssert( pGameObject );

            CreateComponentFromJSONObject( pGameObject, componentobj );
        }

        // load all the components properties, to ensure components are already loaded
        for( int i=0; i<cJSON_GetArraySize( componentarray ); i++ )
        {
            cJSON* componentobj = cJSON_GetArrayItem( componentarray, i );
        
            if( getsceneidfromeachobject )
                cJSONExt_GetUnsignedInt( componentobj, "SceneID", (unsigned int*)&sceneid );

            unsigned int id;
            cJSONExt_GetUnsignedInt( componentobj, "ID", &id );

            ComponentBase* pComponent = FindComponentByID( id, sceneid );
            MyAssert( pComponent );

            if( pComponent )
            {
                pComponent->ImportFromJSONObject( componentobj, sceneid );
            }
        }
    }

    cJSON_Delete( root );

    SyncAllRigidBodiesToObjectTransforms();
}

ComponentBase* ComponentSystemManager::CreateComponentFromJSONObject(GameObject* pGameObject, cJSON* jComponent)
{
    SceneID sceneid = pGameObject->GetSceneID();

    cJSON* typeobj = cJSON_GetObjectItem( jComponent, "Type" );
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
        ComponentBase* pComponent = 0;

        // if the JSON block has a component id, check if that component already exists
        unsigned int id = 0;
        {
            cJSONExt_GetUnsignedInt( jComponent, "ID", &id );

            if( id != 0 )
            {
                pComponent = FindComponentByID( id, sceneid );
                if( pComponent )
                {
                    MyAssert( pComponent->GetSceneID() == sceneid );

                    if( id >= m_pSceneInfoMap[sceneid].m_NextComponentID )
                        m_pSceneInfoMap[sceneid].m_NextComponentID = id + 1;

                    return pComponent;
                }
            }
        }

        // Create a new component of this type on the game object.
        pComponent = pGameObject->AddNewComponent( type, sceneid );

        // If this component had an id set in the scene file, then use it.
        if( id != 0 )
        {
            pComponent->SetID( id );
            if( id >= m_pSceneInfoMap[sceneid].m_NextComponentID )
                m_pSceneInfoMap[sceneid].m_NextComponentID = id + 1;
        }

        return pComponent;
    }

    return 0;
}

void ComponentSystemManager::FinishLoading(bool lockwhileloading, SceneID sceneid, bool playwhenfinishedloading)
{
    m_StartGamePlayWhenDoneLoading = playwhenfinishedloading;
    m_WaitingForFilesToFinishLoading = lockwhileloading;

    if( lockwhileloading )
    {
        if( m_FilesStillLoading.GetHead() != 0 )
            return;

        for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            MyAssert( pFileInfo && pFileInfo->m_pFile );
            if( pFileInfo->m_pFile->IsFinishedLoading() == false ) // still loading
                return;
        }

        m_WaitingForFilesToFinishLoading = false;
    }

    OnLoad( sceneid );

    if( playwhenfinishedloading )
    {
#if MYFW_EDITOR
        g_pEngineCore->Editor_QuickSaveScene( "temp_editor_onplay.scene" );
#endif

        g_pEngineCore->RegisterGameplayButtons();
        OnPlay( sceneid );
        m_StartGamePlayWhenDoneLoading = false;
    }
}

void ComponentSystemManager::SyncAllRigidBodiesToObjectTransforms()
{
    for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode; pNode = pNode->GetNext() )
    {
        if( ((ComponentBase*)pNode)->m_Type == ComponentType_3DCollisionObject )
        {
            Component3DCollisionObject* pComponent = (Component3DCollisionObject*)pNode;

            pComponent->SyncRigidBodyToTransform();
        }
    }
}

void ComponentSystemManager::UnloadScene(SceneID sceneidtoclear, bool clearunmanagedcomponents)
{
    checkGlError( "start of ComponentSystemManager::UnloadScene" );

    // Remove all components, except ones attached to unmanaged game objects (if wanted).
    {
        for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
        {
            for( CPPListNode* pNode = m_Components[i].GetHead(); pNode;  )
            {
                ComponentBase* pComponent = (ComponentBase*)pNode;
                
                pNode = pNode->GetNext();

                SceneID sceneid = pComponent->GetSceneID();

                if( (pComponent->m_pGameObject->IsManaged() || clearunmanagedcomponents) &&
                    (sceneidtoclear == SCENEID_AllScenes || sceneid == sceneidtoclear) )
                {
                    DeleteComponent( pComponent );
                }
                else if( pComponent->GetID() > m_pSceneInfoMap[sceneid].m_NextComponentID )
                {
                    // Not sure how this could happen.
                    MyAssert( false );
                    m_pSceneInfoMap[sceneid].m_NextComponentID = pComponent->GetID() + 1;
                }
            }
        }
    }

    // Delete all game objects from the scene (or all scenes).
    for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse == false )
            continue;

        SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

        if( sceneidtoclear == SCENEID_AllScenes || (SceneID)i == sceneidtoclear )
        {
            for( CPPListNode* pNode = pSceneInfo->m_GameObjects.GetHead(); pNode;  )
            {
                GameObject* pGameObject = (GameObject*)pNode;

                pNode = pNode->GetNext();

                SceneID sceneid = pGameObject->GetSceneID();
                unsigned int gameobjectid = pGameObject->GetID();

                MyAssert( (SceneID)i == sceneid );

                if( (pGameObject->IsManaged() || clearunmanagedcomponents) )
                {
                    DeleteGameObject( pGameObject, true );
                }
                else if( sceneid == sceneidtoclear && gameobjectid > m_pSceneInfoMap[sceneid].m_NextGameObjectID )
                {
                    m_pSceneInfoMap[sceneid].m_NextGameObjectID = gameobjectid + 1;
                }
            }
        }
    }

    // If unloading all scenes, unload all prefab files.
    if( sceneidtoclear == SCENEID_AllScenes )
    {
        m_pPrefabManager->UnloadAllPrefabFiles();
    }

    // Release any file ref's added by this scene.
    FreeAllDataFiles( sceneidtoclear );

    // If clearing all scenes, 
    if( sceneidtoclear == SCENEID_AllScenes )
    {
        // Reset the scene counter, so the new "first" scene loaded will be 1.
        g_pComponentSystemManager->ResetSceneIDCounter();

        // Don't clear unmanaged objects.
        for( int sceneid=0; sceneid<MAX_SCENES_LOADED; sceneid++ )
        {
            if( sceneid == SCENEID_Unmanaged || sceneid == SCENEID_EngineObjects )
                continue;

            if( m_pSceneInfoMap[sceneid].m_InUse == false )
                continue;

#if MYFW_USING_WX
            MyAssert( m_pSceneInfoMap[sceneid].m_TreeID.IsOk() );
            if( m_pSceneInfoMap[sceneid].m_TreeID.IsOk() )
            {
                g_pPanelObjectList->m_pTree_Objects->Delete( m_pSceneInfoMap[sceneid].m_TreeID );
            }
#endif

            m_pSceneInfoMap[sceneid].Reset();
        }
    }
    else if( sceneidtoclear != SCENEID_Unmanaged ) // If clearing any scene other than the unmanaged scene.
    {
#if MYFW_USING_WX
        MyAssert( m_pSceneInfoMap[sceneidtoclear].m_TreeID.IsOk() );
        if( m_pSceneInfoMap[sceneidtoclear].m_TreeID.IsOk() )
        {
            g_pPanelObjectList->m_pTree_Objects->Delete( m_pSceneInfoMap[sceneidtoclear].m_TreeID );
        }
#endif

        MyAssert( m_pSceneInfoMap[sceneidtoclear].m_InUse == true );
        m_pSceneInfoMap[sceneidtoclear].Reset();
    }

    checkGlError( "end of ComponentSystemManager::UnloadScene" );
}

bool ComponentSystemManager::IsSceneLoaded(const char* fullpath)
{
#if 0 //MYFW_USING_WX
    typedef std::map<int, SceneInfo>::iterator it_type;
    for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); )
    {
        SceneInfo* pSceneInfo = &iterator->second;

        if( pSceneInfo->m_InUse )
        {
            if( strcmp( pSceneInfo->m_FullPath, fullpath ) == 0 )
                return true;

            const char* relativepath = GetRelativePath( pSceneInfo->m_FullPath );
            if( relativepath != 0 && strcmp( relativepath, fullpath ) == 0 )
                return true;

            iterator++;
        }
    }
#else
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse )
        {
            if( m_pSceneInfoMap[i].m_FullPath[0] != 0 )
            {
                if( strcmp( m_pSceneInfoMap[i].m_FullPath, fullpath ) == 0 )
                    return true;

                const char* relativepath = GetRelativePath( m_pSceneInfoMap[i].m_FullPath );
                if( relativepath != 0 && strcmp( relativepath, fullpath ) == 0 )
                    return true;
            }
        }
    }
#endif

    return false;
}

SceneID ComponentSystemManager::FindSceneID(const char* fullpath)
{
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse )
        {
            if( strcmp( m_pSceneInfoMap[i].m_FullPath, fullpath ) == 0 )
                return (SceneID)i;
        }
    }

    return SCENEID_NotFound;
}

// Exposed to Lua, change elsewhere if function signature changes.
GameObject* ComponentSystemManager::CreateGameObject(bool manageobject, SceneID sceneid, bool isfolder, bool hastransform, PrefabReference* pPrefabRef)
{
    GameObject* pGameObject = MyNew GameObject( manageobject, sceneid, isfolder, hastransform, pPrefabRef );
    
    {
        unsigned int id = GetNextGameObjectIDAndIncrement( sceneid );
        pGameObject->SetID( id );

        //if( manageobject )
        {
            GetSceneInfo( sceneid )->m_GameObjects.AddTail( pGameObject );
        }

        // if we're not in editor mode, place this gameobject in unmanaged scene so it will be destroyed when gameplay is stopped.
        if( g_pEngineCore->IsInEditorMode() == false )
        {
            pGameObject->SetSceneID( SCENEID_Unmanaged );
        }
    }

    return pGameObject;
}

GameObject* ComponentSystemManager::CreateGameObjectFromPrefab(PrefabObject* pPrefab, bool manageobject, SceneID sceneid)
{
    return CreateGameObjectFromPrefab( pPrefab, pPrefab->GetJSONObject(), 0, manageobject, sceneid );
}

GameObject* ComponentSystemManager::CreateGameObjectFromPrefab(PrefabObject* pPrefab, cJSON* jPrefab, uint32 prefabchildid, bool manageobject, SceneID sceneid)
{
    MyAssert( pPrefab != 0 );
    GameObject* pGameObject = 0;

    // Set values based on SubType.
    bool isfolder = false;
    bool hastransform = true;
    cJSON* jSubtype = cJSON_GetObjectItem( jPrefab, "SubType" );
    if( jSubtype )
    {
        if( strcmp( jSubtype->valuestring, "Folder" ) == 0 )
        {
            isfolder = true;
            hastransform = false;
        }
        else if( strcmp( jSubtype->valuestring, "Logic" ) == 0 )
        {
            hastransform = false;
        }
    }

    if( sceneid == SCENEID_Unmanaged )
    {
        // Sceneid should only be SCENEID_Unmanaged if this is the master prefab gameobject created for the editor.
        MyAssert( manageobject == false );

        PrefabReference prefabRef( pPrefab, prefabchildid, false );
        prefabRef.SetAsMasterPrefabGameObject();
        pGameObject = CreateGameObject( manageobject, sceneid, isfolder, hastransform, &prefabRef );
    }
    else
    {
        PrefabReference prefabRef( pPrefab, prefabchildid, true );
        pGameObject = CreateGameObject( manageobject, sceneid, isfolder, hastransform, &prefabRef );
    }
    
    cJSON* jName = cJSON_GetObjectItem( jPrefab, "Name" );
    MyAssert( jName ); // If this trips, prefab file is likely old, every object should now have a name field.
    pGameObject->SetName( jName->valuestring );

    if( jPrefab )
    {
        cJSON* jComponentArray = cJSON_GetObjectItem( jPrefab, "Components" );
        if( jComponentArray )
        {
            int componentarraysize = cJSON_GetArraySize( jComponentArray );

            for( int i=0; i<componentarraysize; i++ )
            {
                cJSON* jComponent = cJSON_GetArrayItem( jComponentArray, i );

                ComponentBase* pComponent = CreateComponentFromJSONObject( pGameObject, jComponent );
                MyAssert( pComponent );
                if( pComponent )
                {
                    pComponent->ImportFromJSONObject( jComponent, sceneid );
                    pComponent->OnLoad();
                }
            }
        }

        // Create children
        cJSON* jChildrenArray = cJSON_GetObjectItem( jPrefab, "Children" );
        if( jChildrenArray )
        {
            int childarraysize = cJSON_GetArraySize( jChildrenArray );

            for( int i=0; i<childarraysize; i++ )
            {
                cJSON* jChildGameObject = cJSON_GetArrayItem( jChildrenArray, i );
                GameObject* pChildGameObject = 0;
            
                uint32 prefabchildid = 0;
                cJSONExt_GetUnsignedInt( jChildGameObject, "ChildID", &prefabchildid );

                // Create the child game object.
                if( sceneid == SCENEID_Unmanaged )
                {
                    // Sceneid should only be SCENEID_Unmanaged if this is the temporary prefab gameobject created for the editor.
                    MyAssert( manageobject == false );

                    pChildGameObject = CreateGameObjectFromPrefab( pPrefab, jChildGameObject, prefabchildid, false, SCENEID_Unmanaged );
                    pChildGameObject->SetEnabled( false, false );
                }
                else
                {
                    pChildGameObject = CreateGameObjectFromPrefab( pPrefab, jChildGameObject, prefabchildid, true, sceneid );

#if MYFW_USING_WX
                    // Move as last item in parent.
                    GameObject* pLastChild = (GameObject*)pGameObject->GetChildList()->GetTail();
                    if( pLastChild != 0 )
                        g_pPanelObjectList->Tree_MoveObject( pChildGameObject, pLastChild, false );
                    else
                        g_pPanelObjectList->Tree_MoveObject( pChildGameObject, pGameObject, true );
#endif
                }
                MyAssert( pChildGameObject != 0 );

                pChildGameObject->SetParentGameObject( pGameObject );
                cJSON* jTransform = cJSON_GetObjectItem( jChildGameObject, "LocalTransform" );
                pChildGameObject->m_pComponentTransform->ImportLocalTransformFromJSONObject( jTransform );
            }
        }
    }

    // Don't delete jPrefab

    return pGameObject;
}

#if MYFW_EDITOR
GameObject* ComponentSystemManager::CreateGameObjectFromTemplate(unsigned int templateid, SceneID sceneid)
{
    MyAssert( templateid < m_pGameObjectTemplateManager->GetNumberOfTemplates() );

    GameObject* pGameObject = CreateGameObject( true, sceneid, false, true );
    
    const char* templatename = m_pGameObjectTemplateManager->GetTemplateName( templateid );
    pGameObject->SetName( templatename );

    cJSON* jTemplate = m_pGameObjectTemplateManager->GetTemplateJSONObject( templateid );

    if( jTemplate )
    {
        Vector3 scale(1);
        cJSONExt_GetFloatArray( jTemplate, "Scale", &scale.x, 3 );
        pGameObject->GetTransform()->SetLocalScale( scale );

        cJSON* jComponentArray = cJSON_GetObjectItem( jTemplate, "Components" );
        int arraysize = cJSON_GetArraySize( jComponentArray );

        for( int i=0; i<arraysize; i++ )
        {
            cJSON* jComponent = cJSON_GetArrayItem( jComponentArray, i );

            ComponentBase* pComponent = CreateComponentFromJSONObject( pGameObject, jComponent );
            MyAssert( pComponent );
            if( pComponent )
            {
                pComponent->ImportFromJSONObject( jComponent, sceneid );
                pComponent->OnLoad();
            }
        }
    }

    // Don't delete jTemplate

    return pGameObject;
}
#endif //MYFW_USING_WX

void ComponentSystemManager::UnmanageGameObject(GameObject* pObject, bool unmanagechildren)
{
    MyAssert( pObject && pObject->IsManaged() == true );

    // Remove all components from their respective component lists.
    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        ComponentBase* pComponent = pObject->GetComponentByIndex( i );

        // Remove from list and clear CPPListNode prev/next.
        pComponent->Remove();
        pComponent->Prev = 0;
        pComponent->Next = 0;
    }

    pObject->SetManaged( false );

    // Recurse through children.
    if( unmanagechildren )
    {
        GameObject* pChild = pObject->GetFirstChild();

        while( pChild )
        {
            UnmanageGameObject( pChild, true );
            pChild = (GameObject*)pChild->GetNext();
        }
    }
}

void ComponentSystemManager::ManageGameObject(GameObject* pObject, bool managechildren)
{
    MyAssert( pObject && pObject->IsManaged() == false );

    // Add all the gameobject's components back into the component lists.
    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        AddComponent( pObject->GetComponentByIndex( i ) );
    }

    pObject->SetManaged( true );

    // Recurse through children.
    if( managechildren )
    {
        GameObject* pChild = pObject->GetFirstChild();

        while( pChild )
        {
            ManageGameObject( pChild, true );
            pChild = (GameObject*)pChild->GetNext();
        }
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentSystemManager::DeleteGameObject(GameObject* pObject, bool deletecomponents)
{
#if MYFW_USING_WX
    if( g_pEngineCore->GetEditorState()->IsGameObjectSelected( pObject ) )
    {
        g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
    }
#endif

    if( deletecomponents )
    {
        while( pObject->m_Components.Count() )
        {
            ComponentBase* pComponent = pObject->m_Components.RemoveIndex( 0 );

#if MYFW_USING_WX
            if( g_pEngineCore->GetEditorState()->IsComponentSelected( pComponent ) )
            {
                g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
            }
#endif

            pComponent->SetEnabled( false );
            delete pComponent;
        }
    }

    SAFE_DELETE( pObject );
}

#if MYFW_EDITOR
GameObject* ComponentSystemManager::EditorCopyGameObject(GameObject* pObject, bool NewObjectInheritsFromOld)
{
    GameObject* newgameobject = 0;

    EditorCommand_CopyGameObject* pCommand = MyNew EditorCommand_CopyGameObject( pObject, NewObjectInheritsFromOld );
    if( g_pEngineCore->IsInEditorMode() )
    {
        g_pEngineCore->GetCommandStack()->Do( pCommand );
        newgameobject = pCommand->GetCreatedObject();
    }
    else
    {
        // if we're not in editor mode, execute the command and delete it.
        pCommand->Do();
        newgameobject = pCommand->GetCreatedObject();
        delete pCommand;
    }

    return newgameobject;
}
#endif

// Exposed to Lua, change elsewhere if function signature changes.
GameObject* ComponentSystemManager::CopyGameObject(GameObject* pObject, const char* newname)
{
    if( pObject == 0 )
        return 0;

    // place the new object in the unmanaged scene, unless we're in the editor.
    SceneID sceneid = SCENEID_Unmanaged;
    if( g_pEngineCore->IsInEditorMode() )
        sceneid = pObject->GetSceneID();

    GameObject* pNewObject = CreateGameObject( true, sceneid, pObject->IsFolder(),
                                               pObject->GetTransform() ? true : false, pObject->GetPrefabRef() );

    if( newname )
        pNewObject->SetName( newname );

    pNewObject->SetEnabled( pObject->IsEnabled(), false );
    pNewObject->SetPhysicsSceneID( pObject->GetPhysicsSceneID() );
    pNewObject->SetFlags( pObject->GetFlags() );

    if( g_pEngineCore->IsInEditorMode() )
    {
        pNewObject->SetGameObjectThisInheritsFrom( pObject->GetGameObjectThisInheritsFrom() );
    }

    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        ComponentBase* pComponent = 0;

        if( g_pEngineCore->IsInEditorMode() )
            pComponent = pNewObject->AddNewComponent( pObject->GetComponentByIndex( i )->m_Type, pNewObject->GetSceneID() );
        else
            pComponent = pNewObject->AddNewComponent( pObject->GetComponentByIndex( i )->m_Type, SCENEID_Unmanaged );

        pComponent->CopyFromSameType_Dangerous( pObject->GetComponentByIndex( i ) );

        pComponent->OnLoad();
    }

    // Call OnPlay for all components.
    if( g_pEngineCore->IsInEditorMode() == false )
    {
        if( pNewObject->IsEnabled() == true )
        {
            for( unsigned int i=0; i<pNewObject->GetComponentCount(); i++ )
            {
                if( pNewObject->GetComponentByIndex( i )->IsA( "2DJoint-" ) == false )
                    pNewObject->GetComponentByIndex( i )->OnPlay();
            }
            // Call OnPlay for joints after everything else, will allow physics bodies to be created first.
            for( unsigned int i=0; i<pNewObject->GetComponentCount(); i++ )
            {
                if( pNewObject->GetComponentByIndex( i )->IsA( "2DJoint-" ) == true )
                    pNewObject->GetComponentByIndex( i )->OnPlay();
            }
        }
    }

    // if the object we're copying was parented, set the parent.
    if( pObject->GetParentGameObject() != 0 )
    {
        pNewObject->SetParentGameObject( pObject->GetParentGameObject() );

#if MYFW_USING_WX
        // Place the child under the parent in the object list
        g_pPanelObjectList->Tree_MoveObject( pNewObject, pObject->GetParentGameObject(), true );
#endif
    }

    if( pObject->IsFolder() == false )
    {
        MyAssert( pObject->GetTransform() != 0 );
        *pNewObject->GetTransform() = *pObject->GetTransform();
    }

    // Recursively copy children.
    GameObject* pChild = pObject->GetFirstChild();
    while( pChild )
    {
        GameObject* pNewChild = CopyGameObject( pChild, pChild->GetName() );
        pNewChild->SetParentGameObject( pNewObject );

#if MYFW_USING_WX
        // Place the child under the parent in the object list
        g_pPanelObjectList->Tree_MoveObject( pNewChild, pNewObject, true );
#endif

        pChild = (GameObject*)pChild->GetNext();
    }

    return pNewObject;
}

unsigned int ComponentSystemManager::GetNextGameObjectIDAndIncrement(SceneID sceneid)
{
    SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( sceneid );
    
    MyAssert( pSceneInfo );
    if( pSceneInfo == 0 )
        return 0; // we have problems if this happens.

    pSceneInfo->m_NextGameObjectID++;
    return pSceneInfo->m_NextGameObjectID-1;
}

unsigned int ComponentSystemManager::GetNextComponentIDAndIncrement(SceneID sceneid)
{
    SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( sceneid );
    
    MyAssert( pSceneInfo );
    if( pSceneInfo == 0 )
        return 0; // we have problems if this happens.

    pSceneInfo->m_NextComponentID++;
    return pSceneInfo->m_NextComponentID-1;
}

GameObject* ComponentSystemManager::GetFirstGameObjectFromScene(SceneID sceneid)
{
    SceneInfo* pSceneInfo = GetSceneInfo( sceneid );
    if( pSceneInfo == 0 )
        return 0;

    GameObject* pGameObject = (GameObject*)pSceneInfo->m_GameObjects.GetHead();
    if( pGameObject )
    {
        MyAssert( pGameObject->GetSceneID() == sceneid );
        return pGameObject;
    }

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByID(SceneID sceneid, unsigned int goid)
{
    SceneInfo* pSceneInfo = GetSceneInfo( sceneid );
    if( pSceneInfo == 0 )
        return 0;

    if( pSceneInfo->m_GameObjects.GetHead() )
        return FindGameObjectByIDFromList( (GameObject*)pSceneInfo->m_GameObjects.GetHead(), goid );

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByIDFromList(GameObject* list, unsigned int goid)
{
    MyAssert( list != 0 );
    if( list == 0 )
        return 0;

    for( CPPListNode* node = list; node != 0; node = node->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)node;

        if( pGameObject->IsManaged() && pGameObject->GetID() == goid )
            return pGameObject;

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            GameObject* pGameObjectFound = FindGameObjectByIDFromList( pFirstChild, goid );
            if( pGameObjectFound )
                return pGameObjectFound;
        }
    }

    return 0;
}

// Exposed to Lua, change elsewhere if function signature changes.
GameObject* ComponentSystemManager::FindGameObjectByName(const char* name)
{
#if 0 //MYFW_USING_WX
    typedef std::map<int, SceneInfo>::iterator it_type;
    for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); )
    {
        SceneID sceneid = iterator->first;
        SceneInfo* pSceneInfo = &iterator->second;
#else
    for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse == false )
            continue;

        SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];
#endif //MYFW_USING_WX

        if( pSceneInfo->m_GameObjects.GetHead() )
        {
            GameObject* pGameObjectFound = FindGameObjectByNameFromList( (GameObject*)pSceneInfo->m_GameObjects.GetHead(), name );
            if( pGameObjectFound )
                return pGameObjectFound;
        }
    }

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByNameInScene(SceneID sceneid, const char* name)
{
    MyAssert( sceneid < MAX_SCENES_LOADED_INCLUDING_UNMANAGED );

    MyAssert( m_pSceneInfoMap[sceneid].m_InUse == true );

    SceneInfo* pSceneInfo = &m_pSceneInfoMap[sceneid];

    if( pSceneInfo->m_GameObjects.GetHead() )
    {
        GameObject* pGameObjectFound = FindGameObjectByNameFromList( (GameObject*)pSceneInfo->m_GameObjects.GetHead(), name );
        if( pGameObjectFound )
            return pGameObjectFound;
    }

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByNameFromList(GameObject* list, const char* name)
{
    MyAssert( list != 0 );
    if( list == 0 )
        return 0;

    for( CPPListNode* node = list; node != 0; node = node->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)node;

        if( strcmp( pGameObject->GetName(), name ) == 0 )
            return pGameObject;

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            GameObject* pGameObjectFound = FindGameObjectByNameFromList( pFirstChild, name );
            if( pGameObjectFound )
                return pGameObjectFound;
        }
    }

    return 0;
}

GameObject* ComponentSystemManager::FindGameObjectByJSONRef(cJSON* pJSONGameObjectRef, SceneID defaultsceneid)
{
    // see GameObject::ExportReferenceAsJSONObject

    cJSON* jScenePath = cJSON_GetObjectItem( pJSONGameObjectRef, "Scene" );
    SceneID sceneid = defaultsceneid;
    if( jScenePath )
    {
        sceneid = GetSceneIDFromFullpath( jScenePath->valuestring );
        if( sceneid == SCENEID_NotFound )
            return 0; // scene isn't loaded, so object can't be found.
        //TODO: saving will throw all reference info away and piss people off :)
    }

    unsigned int goid = -1;
    cJSONExt_GetUnsignedInt( pJSONGameObjectRef, "GOID", &goid );
    MyAssert( goid != -1 );

    return FindGameObjectByID( sceneid, goid );
}

ComponentBase* ComponentSystemManager::FindComponentByJSONRef(cJSON* pJSONComponentRef, SceneID defaultsceneid)
{
    GameObject* pGameObject = FindGameObjectByJSONRef( pJSONComponentRef, defaultsceneid );
    MyAssert( pGameObject );
    if( pGameObject )
    {
        unsigned int componentid = -1;
        cJSONExt_GetUnsignedInt( pJSONComponentRef, "ComponentID", &componentid );
        MyAssert( componentid != -1 );

        return pGameObject->FindComponentByID( componentid );
    }

    return 0;
}

ComponentCamera* ComponentSystemManager::GetFirstCamera(bool prefereditorcam)
{
#if MYFW_USING_WX
    if( prefereditorcam && g_pEngineCore->IsInEditorMode() )
    {
        return g_pEngineCore->GetEditorState()->GetEditorCamera();
    }
    else
#endif
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

ComponentBase* ComponentSystemManager::FindComponentByID(unsigned int id, SceneID sceneid)
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
        // TODO: this is hardcoded to the first scene.
        FinishLoading( true, SCENEID_MainScene, m_StartGamePlayWhenDoneLoading );
    }

    // tick the files that are still loading, if handled by us (spritesheets only ATM)
    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileInfo* pFileInfo = (MyFileInfo*)pNode;
        
        MyAssert( pFileInfo );
        MyAssert( pFileInfo->m_pSpriteSheet );

        pFileInfo->m_pSpriteSheet->Tick( TimePassed );

        if( pFileInfo->m_pSpriteSheet->IsFullyLoaded() )
            m_Files.MoveTail( pNode );
    }

#if MYFW_USING_WX
    CheckForUpdatedDataSourceFiles( true );
#endif

    //TimePassed *= m_TimeScale;

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

    //// don't tick objects if time is 0, useful for debugging, shouldn't be done otherwise
    //if( TimePassed == 0 )
    //    return;

    // then all other "Updateables".
    for( CPPListNode* node = m_Components[BaseComponentType_Updateable].GetHead(); node != 0; node = node->GetNext() )
    {
        ComponentUpdateable* pComponent = (ComponentUpdateable*)node;

        if( pComponent->m_BaseType == BaseComponentType_Updateable ) //&& pComponent->m_Type != ComponentType_LuaScript )
        {
            pComponent->Tick( TimePassed );
        }
    }

    // update all components that registered a tick callback... might unregister themselves while in their callback
    for( CPPListNode* pNode = m_pComponentCallbackList_Tick.GetHead(); pNode != 0; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

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
    checkGlError( "start of ComponentSystemManager::OnDrawFrame()" );

    // Draw all objects in the scene graph
    {
        Vector3 campos = pCamera->m_pComponentTransform->GetLocalPosition();
        Vector3 camrot = pCamera->m_pComponentTransform->GetLocalRotation();

        // Find nearest shadow casting light. TODO: handle this better.
        MyMatrix* pShadowVP = 0;
        TextureDefinition* pShadowTex = 0;
        if( g_ActiveShaderPass == ShaderPass_Main )
        {
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
            if( pObject )
            {
                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
                if( pComponent )
                {
                    ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : 0;
                    if( pShadowCam )
                    {
                        pShadowVP = pShadowCam->GetViewProjMatrix();
#if 1
                        pShadowTex = pShadowCam->GetFBO()->GetDepthTexture();
#else
                        pShadowTex = pShadowCam->GetFBO()->GetColorTexture( 0 );
#endif
                    }
                }
            }
        }

        m_pSceneGraph->Draw( SceneGraphFlag_Opaque, pCamera->m_LayersToRender, &campos, &camrot, pMatViewProj, pShadowVP, pShadowTex, pShaderOverride, 0 );
        m_pSceneGraph->Draw( SceneGraphFlag_Transparent, pCamera->m_LayersToRender, &campos, &camrot, pMatViewProj, pShadowVP, pShadowTex, pShaderOverride, 0 );
    }
    
    // Draw all components that registered a callback, used mostly for debug info (camera/light icons, collision info)
    // Also used for menu pages.
    {
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

    checkGlError( "end of ComponentSystemManager::OnDrawFrame()" );
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

void ProgramSceneIDs(ComponentBase* pComponent, ShaderGroup* pShaderOverride)
{
    if( pShaderOverride == 0 )
        return;

    if( pComponent == 0 )
        return;

    ColorByte tint( 0, 0, 0, 0 );

    SceneID sceneid = pComponent->m_pGameObject->GetSceneID();
    unsigned int id = pComponent->m_pGameObject->GetID();
                    
    id = (sceneid * 100000 + id) * 641; // 1, 641, 6700417, 4294967297, 

    if( 1 )                 tint.r = id%256;
    if( id > 256 )          tint.g = (id>>8)%256;
    if( id > 256*256 )      tint.b = (id>>16)%256;
    if( id > 256*256*256 )  tint.a = (id>>24)%256;

    checkGlError( "ComponentSystemManager::DrawMousePickerFrame - ProgramSceneIDs before setting tint" );

    Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    MyAssert( MyGLDebug_IsProgramActive( pShader->m_ProgramHandle ) == true );
    pShader->ProgramTint( tint );

    checkGlError( "ComponentSystemManager::DrawMousePickerFrame - ProgramSceneIDs after setting tint" );
}

void ProgramSceneIDs(SceneGraphObject* pObject, ShaderGroup* pShaderOverride)
{
    ComponentBase* pComponent = (ComponentBase*)(pObject->m_pUserData);

    ProgramSceneIDs( pComponent, pShaderOverride );
}

void ComponentSystemManager::DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    // always use 4 bone version.
    // TODO: this might fail with 1-3 bones,
    //       but should work with 0 bones since bone attribs are set to 100% weight on bone 0
    //       and bone 0 transform uniform is set to identity.
    Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    if( pShader->ActivateAndProgramShader() )
    {
        // Draw all objects in the scene graph
        {
            Vector3 campos = pCamera->m_pComponentTransform->GetLocalPosition();
            Vector3 camrot = pCamera->m_pComponentTransform->GetLocalRotation();

            m_pSceneGraph->Draw( SceneGraphFlag_Opaque, pCamera->m_LayersToRender, &campos, &camrot, pMatViewProj, 0, 0, pShaderOverride, ProgramSceneIDs );
            m_pSceneGraph->Draw( SceneGraphFlag_Transparent, pCamera->m_LayersToRender, &campos, &camrot, pMatViewProj, 0, 0, pShaderOverride, ProgramSceneIDs );
        }

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

                    SceneID sceneid = pComponent->m_pGameObject->GetSceneID();
                    unsigned int id = pComponent->m_pGameObject->GetID();
                    
                    id = (sceneid * 100000 + id) * 641; // 1, 641, 6700417, 4294967297, 

                    if( 1 )                 tint.r = id%256;
                    if( id > 256 )          tint.g = (id>>8)%256;
                    if( id > 256*256 )      tint.b = (id>>16)%256;
                    if( id > 256*256*256 )  tint.a = (id>>24)%256;

                    checkGlError( "ComponentSystemManager::DrawMousePickerFrame" );

                    pShader->ProgramTint( tint );

                    checkGlError( "ComponentSystemManager::DrawMousePickerFrame" );

                    (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( pCamera, pMatViewProj, pShaderOverride );

                    checkGlError( "ComponentSystemManager::DrawMousePickerFrame" );
                }
            }
        }

        pShader->DeactivateShader( 0, false );
    }
}

SceneID ComponentSystemManager::GetNextSceneID()
{
    // TODO: search through list for an unused scene and return that ID.
    SceneID nextid = m_NextSceneID;

    m_NextSceneID = (SceneID)(m_NextSceneID+1);

    MyAssert( nextid >= SCENEID_MainScene && nextid < MAX_SCENES_LOADED );
    
    return nextid;
}

void ComponentSystemManager::ResetSceneIDCounter()
{
    m_NextSceneID = SCENEID_MainScene;
}

SceneInfo* ComponentSystemManager::GetSceneInfo(SceneID sceneid)
{
    MyAssert( sceneid >= 0 && sceneid < MAX_SCENES_CREATED );
    return &m_pSceneInfoMap[sceneid];
}

SceneID ComponentSystemManager::GetSceneIDFromFullpath(const char* fullpath)
{
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        SceneID sceneid = (SceneID)i;
        SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

        if( strcmp( pSceneInfo->m_FullPath, fullpath ) == 0 )
            return sceneid;

        const char* relativepath = GetRelativePath( pSceneInfo->m_FullPath );
        if( relativepath != 0 && strcmp( relativepath, fullpath ) == 0 )
            return sceneid;
    }

    MyAssert( false ); // fullpath not found, that's fine when used from gameobject loading.
    return SCENEID_NotFound;
}

#if MYFW_EDITOR
void ComponentSystemManager::CreateNewScene(const char* scenename, SceneID sceneid)
{
    MyAssert( sceneid >= SCENEID_MainScene && sceneid < MAX_SCENES_LOADED );
#if MYFW_USING_WX
    MyAssert( m_pSceneInfoMap[sceneid].m_TreeID.IsOk() == false );

    wxTreeItemId rootid = g_pPanelObjectList->GetTreeRoot();
    wxTreeItemId treeid = g_pPanelObjectList->AddObject( m_pSceneHandler, SceneHandler::StaticOnLeftClick, SceneHandler::StaticOnRightClick, rootid, scenename, ObjectListIcon_Scene );
    g_pPanelObjectList->SetDragAndDropFunctions( treeid, SceneHandler::StaticOnDrag, SceneHandler::StaticOnDrop );
    m_pSceneInfoMap[sceneid].m_TreeID = treeid;
#endif // MYFW_USING_WX
    m_pSceneInfoMap[sceneid].m_InUse = true;

    // create the box2d world, pass in a material for the debug renderer.
    ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
    m_pSceneInfoMap[sceneid].m_pBox2DWorld = MyNew Box2DWorld( g_pEngineCore->GetMaterial_Box2DDebugDraw(), &pCamera->m_Camera3D.m_matViewProj, new EngineBox2DContactListener );
}

#if MYFW_USING_WX
wxTreeItemId ComponentSystemManager::GetTreeIDForScene(SceneID sceneid)
{
    return m_pSceneInfoMap[sceneid].m_TreeID;
}

//SceneID ComponentSystemManager::GetSceneIDFromFullpath(const char* fullpath)
//{
//    typedef std::map<int, SceneInfo>::iterator it_type;
//    for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); iterator++ )
//    {
//        SceneID sceneid = iterator->first;
//        SceneInfo* pSceneInfo = &iterator->second;
//
//        if( strcmp( pSceneInfo->m_FullPath, fullpath ) == 0 )
//            return sceneid;
//
//        const char* relativepath = GetRelativePath( pSceneInfo->m_FullPath );
//        if( relativepath != 0 && strcmp( relativepath, fullpath ) == 0 )
//            return sceneid;
//    }
//
//    MyAssert( false ); // fullpath not found, that's fine when used from gameobject loading.
//    return SCENEID_NotFound;
//}

SceneID ComponentSystemManager::GetSceneIDFromSceneTreeID(wxTreeItemId treeid)
{
    //typedef std::map<int, SceneInfo>::iterator it_type;
    //for( it_type iterator = m_pSceneInfoMap.begin(); iterator != m_pSceneInfoMap.end(); iterator++ )
    //{
    //    SceneID sceneid = iterator->first;
    //    SceneInfo* pSceneInfo = &iterator->second;

    //    if( pSceneInfo->m_TreeID == treeid )
    //        return sceneid;
    //}

    //MyAssert( false ); // treeid not found, it should be.
    //return -1;
    for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse && m_pSceneInfoMap[i].m_TreeID == treeid )
            return (SceneID)i;
    }

    MyAssert( false ); // fullpath not found, that's fine when used from gameobject loading.
    return SCENEID_NotFound;
}
#endif //MYFW_USING_WX

unsigned int ComponentSystemManager::GetNumberOfScenesLoaded()
{
    unsigned int numloaded = 0;
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse )
            numloaded++;
    }

    return numloaded;
}

void EditorInternal_GetListOfGameObjectsThatUsePrefab(std::vector<GameObject*>* pGameObjectList, CPPListNode* pListToSearch, PrefabObject* pPrefabToFind)
{
    for( CPPListNode* pNode = pListToSearch; pNode; pNode = pNode->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)pNode;

        if( pGameObject->GetPrefabRef()->GetPrefab() == pPrefabToFind )
        {
            pGameObjectList->push_back( pGameObject );
        }

        // Search children of GameObject.
        CPPListHead* pChildList = pGameObject->GetChildList();
        if( pChildList->GetHead() )
        {
            EditorInternal_GetListOfGameObjectsThatUsePrefab( pGameObjectList, pChildList->GetHead(), pPrefabToFind );
        }
    }
}

void ComponentSystemManager::Editor_GetListOfGameObjectsThatUsePrefab(std::vector<GameObject*>* pGameObjectList, PrefabObject* pPrefabToFind)
{
    for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse == false )
            continue;

        EditorInternal_GetListOfGameObjectsThatUsePrefab( pGameObjectList, m_pSceneInfoMap[i].m_GameObjects.GetHead(), pPrefabToFind );
    }
}

void ComponentSystemManager::LogAllReferencesForFile(MyFileObject* pFile)
{
    LOGInfo( LOGTag, "Finding references to %s in all scenes:\n", pFile->GetFullPath() );

    int numrefs = 0;

    // Check all gameobjects/components in all scenes for a reference to the file.
    //   ATM: This only checks variables registered as "component vars".
    for( unsigned int sceneindex=0; sceneindex<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; sceneindex++ )
    {
        if( m_pSceneInfoMap[sceneindex].m_InUse == false )
            continue;

        for( CPPListNode* pNode = m_pSceneInfoMap[sceneindex].m_GameObjects.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            GameObject* pGameObject = (GameObject*)pNode;

            numrefs += LogAllReferencesForFileInGameObject( pFile, pGameObject );
        }
    }

    // Check materials for references to files.
    MaterialDefinition* pMaterial = g_pMaterialManager->GetFirstMaterial();
    while( pMaterial != 0 )
    {
        if( pMaterial->IsReferencingFile( pFile ) )
        {
            if( pMaterial->GetFile() )
                LOGInfo( LOGTag, "    (Material) %s (0x%x)\n", pMaterial->GetFile()->GetFullPath(), pMaterial );
            else
                LOGInfo( LOGTag, "    (Unsaved Material) %s (0x%x)\n", pMaterial->GetName(), pMaterial );

            numrefs++;
        }

        pMaterial = (MaterialDefinition*)pMaterial->GetNext();
    }

    LOGInfo( LOGTag, "Done: %d references found\n", numrefs );
}

// Returns number of references
int ComponentSystemManager::LogAllReferencesForFileInGameObject(MyFileObject* pFile, GameObject* pGameObject)
{
    int numrefs = 0;

    for( unsigned int componentindex=0; componentindex<pGameObject->GetComponentCountIncludingCore(); componentindex++ )
    {
        ComponentBase* pComponent = pGameObject->GetComponentByIndexIncludingCore( componentindex );

        if( pComponent )
        {
            if( pComponent->IsReferencingFile( pFile ) )
            {
                SceneID sceneindex = pComponent->GetSceneID();

                if( sceneindex == SCENEID_Unmanaged )
                {
                    if( pGameObject->IsPrefabInstance() )
                    {
                        LOGInfo( LOGTag, "    (Prefab) %s :: %s :: %s (0x%x)\n", pGameObject->GetPrefabRef()->GetPrefab()->GetPrefabFile()->GetFile()->GetFullPath(), pGameObject->GetName(), pComponent->GetClassname(), pComponent );
                    }
                }
                else
                {
                    LOGInfo( LOGTag, "    (GameObject) %s :: %s :: %s (0x%x)\n", m_pSceneInfoMap[sceneindex].m_FullPath, pGameObject->GetName(), pComponent->GetClassname(), pComponent );
                }
                numrefs++;
            }
        }
    }

    // Recurse through children.
    GameObject* pChild = pGameObject->GetFirstChild();
    while( pChild )
    {
        numrefs += LogAllReferencesForFileInGameObject( pFile, pChild );

        pChild = (GameObject*)pChild->GetNext();
    }

    return numrefs;
}

GameObject* ComponentSystemManager::ParseLog_GameObject(const char* line)
{
    // Example strings to parse.
    // "    (GameObject) SceneFileName :: GameObjectName :: ComponentName"
    // "    (Prefab) PrefabFileName :: GameObjectName :: ComponentName"

    int pos = 0;
    char scenename[100] = "";
    char gameobjectname[100] = "";
    char componentname[100] = "";

    bool isgameobject = false;
    bool isprefab = false;

    // First, make sure this is a GameObject or Prefab.
    {
        const char* pos2 = strstr( line, "(GameObject) " );
        const char* pos3 = strstr( line, "(Prefab) " );

        if( pos2 )
        {
            isgameobject = true;
            pos = (int)(pos2 - line) + (int)strlen("(GameObject) ");
        }
        else if( pos3 )
        {
            isprefab = true;
            pos = (int)(pos3 - line) + (int)strlen("(Prefab) ");
        }
        else
        {
            return 0;
        }
    }

    // Grab the Scene/Prefab filename (can contain spaces)
    {
        const char* pos2 = strstr( &line[pos], " :: " );
        if( pos2 == 0 ) return 0;
        strncpy_s( scenename, 100, &line[pos], pos2 - &line[pos] );
        pos += (int)(pos2 - &line[pos]) + (int)strlen(" :: ");
    }

    // Grab the GameObject name (can contain spaces)
    {
        const char* pos2 = strstr( &line[pos], " :: " );
        if( pos2 == 0 ) return 0;
        strncpy_s( gameobjectname, 100, &line[pos], pos2 - &line[pos] );
        pos += (int)(pos2 - &line[pos]) + (int)strlen(" :: ");
    }

    // Grab the Component name (won't contain spaces)
    {
        const char* pos2 = strstr( &line[pos], " " );
        if( pos2 == 0 ) return 0;
        strncpy_s( componentname, 100, &line[pos], pos2 - &line[pos] );
    }

    // Find the GameObject.
    if( isgameobject )
    {
        SceneID sceneid = g_pComponentSystemManager->FindSceneID( scenename );

        if( sceneid != SCENEID_NotFound )
        {
            MyAssert( sceneid >= 0 && sceneid < MAX_SCENES_LOADED_INCLUDING_UNMANAGED );

            GameObject* pGameObject = g_pComponentSystemManager->FindGameObjectByNameInScene( sceneid, gameobjectname );
            return pGameObject;
        }
    }
    else if( isprefab )
    {
        PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByFullPath( scenename );

        if( pPrefabFile )
        {
            PrefabObject* pPrefab = pPrefabFile->GetFirstPrefabByName( gameobjectname );

            if( pPrefab )
            {
                return pPrefab->GetGameObject();
            }
        }
    }

    return 0;
}

MaterialDefinition* ComponentSystemManager::ParseLog_Material(const char* line)
{
    // Example strings to parse.
    // "    (GameObject) SceneFileName :: GameObjectName :: ComponentName"
    // "    (Prefab) PrefabFileName :: GameObjectName :: ComponentName"

    int pos = 0;
    char materialname[100] = "";

    // First, make sure this is a Material.
    {
        const char* pos2 = strstr( line, "(Material) " );
        if( pos2 == 0 ) return 0;
        pos = (int)(pos2 - line) + (int)strlen("(Material) ");
    }

    // Grab the Material filename (can contain spaces)
    {
        const char* pos2 = strstr( &line[pos], " (" );
        if( pos2 == 0 ) return 0;
        strncpy_s( materialname, 100, &line[pos], pos2 - &line[pos] );
    }

    // Find the GameObject.
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->FindMaterialByFilename( materialname );

        return pMaterial;
    }

    return 0;
}
#endif //MYFW_EDITOR

//SceneInfo* ComponentSystemManager::GetSceneInfo(SceneID sceneid)
//{
//    MyAssert( m_pSceneInfoMap[sceneid].m_InUse == true );
//    return &m_pSceneInfoMap[sceneid];
//}

//void ComponentSystemManager::m_pGameObjectTemplateManager

void ComponentSystemManager::AddMeshToSceneGraph(ComponentBase* pComponent, MyMesh* pMesh, MaterialDefinition** pMaterialList, int primitive, int pointsize, SceneGraphFlags flags, unsigned int layers, SceneGraphObject** pOutputList)
{
    MyAssert( pComponent != 0 );
    MyAssert( pComponent->m_pGameObject != 0 );
    MyAssert( pMesh != 0 );
    MyAssert( pMaterialList != 0 );
    MyAssert( pOutputList != 0 );
    MyAssert( pMesh->GetSubmeshListCount() > 0 );

    MyMatrix* pWorldTransform = pComponent->m_pGameObject->GetTransform()->GetWorldTransform();

    for( unsigned int i=0; i<pMesh->GetSubmeshListCount(); i++ )
    {
        if( pMaterialList[i] && pMaterialList[i]->IsTransparent() )
            flags = SceneGraphFlag_Transparent;
        MyAssert( pOutputList[i] == 0 );
        pOutputList[i] = m_pSceneGraph->AddObject( pWorldTransform, pMesh, pMesh->GetSubmesh( i ), pMaterialList[i], primitive, pointsize, flags, layers, pComponent );
    }
}

SceneGraphObject* ComponentSystemManager::AddSubmeshToSceneGraph(ComponentBase* pComponent, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitive, int pointsize, SceneGraphFlags flags, unsigned int layers)
{
    MyAssert( pComponent != 0 );
    MyAssert( pComponent->m_pGameObject != 0 );
    MyAssert( pSubmesh != 0 );

    MyMatrix* pWorldTransform = pComponent->m_pGameObject->GetTransform()->GetWorldTransform();

    return m_pSceneGraph->AddObject( pWorldTransform, 0, pSubmesh, pMaterial, primitive, pointsize, flags, layers, pComponent );
}

void ComponentSystemManager::RemoveObjectFromSceneGraph(SceneGraphObject* pSceneGraphObject)
{
    m_pSceneGraph->RemoveObject( pSceneGraphObject );
}

void ComponentSystemManager::OnLoad(SceneID sceneid)
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)node;
            
            if( sceneid != SCENEID_AllScenes && pComponent->GetSceneID() != sceneid )
                continue;

            pComponent->OnLoad();
        }
    }
}

void ComponentSystemManager::OnPlay(SceneID sceneid)
{
    // TODO: find a better solution than 2 passes, sort the OnPlay callback lists(once OnPlay is a callback of course...)

    // first pass, everything but 2d physics joints.
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)node;

            //MyAssert( pComponent->IsEnabled() == true );

            if( sceneid != SCENEID_AllScenes && pComponent->GetSceneID() != sceneid )
                continue;

            if( pComponent->m_pGameObject->IsEnabled() == true )
            {
                if( pComponent->IsA( "2DJoint-" ) == false )
                    pComponent->OnPlay();
            }
        }
    }

    // second pass, only 2d physics joints.
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)node;

            //MyAssert( pComponent->IsEnabled() == true );

            if( sceneid != SCENEID_AllScenes && pComponent->GetSceneID() != sceneid )
                continue;

            if( pComponent->m_pGameObject->IsEnabled() == true )
            {
                if( pComponent->IsA( "2DJoint-" ) == true )
                    pComponent->OnPlay();
            }
        }
    }
}

void ComponentSystemManager::OnStop(SceneID sceneid)
{
    SetTimeScale( 1 );
#if MYFW_USING_WX
    g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX

    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)node;

            if( sceneid != SCENEID_AllScenes && pComponent->GetSceneID() != sceneid )
                continue;

            pComponent->OnStop();
        }
    }
}

bool ComponentSystemManager::OnEvent(MyEvent* pEvent)
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* node = m_Components[i].GetHead(); node != 0; node = node->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)node;
            if( pComponent->OnEvent( pEvent ) )
                return true;
        }
    }

    return false;
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

#if MYFW_EDITOR
void ComponentSystemManager::DrawSingleObject(MyMatrix* pMatViewProj, GameObject* pObject, ShaderGroup* pShaderOverride)
{
    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        ComponentRenderable* pComponent = dynamic_cast<ComponentRenderable*>( pObject->GetComponentByIndex( i ) );

        if( pComponent )
        {
            pComponent->Draw( pMatViewProj );

            ComponentCallbackStruct_Draw* pCallbackStruct = pComponent->GetDrawCallback();

            ComponentBase* pCallbackComponent = (ComponentBase*)pCallbackStruct->pObj;
            if( pCallbackComponent != 0 && pCallbackStruct->pFunc != 0 )
            {
                (pCallbackComponent->*pCallbackStruct->pFunc)( 0, pMatViewProj, pShaderOverride );
            }
        }
    }
}

#if MYFW_USING_WX
void ComponentSystemManager::CheckForUpdatedDataSourceFiles(bool initialcheck)
{
    for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

        if( pFileInfo->m_pFile == 0 )
            continue;

#if MYFW_WINDOWS
        if( (initialcheck == false || pFileInfo->m_DidInitialCheckIfSourceFileWasUpdated == false) && // haven't done initial check
            pFileInfo->m_pFile->GetFileLastWriteTime().dwHighDateTime != 0 && // converted file has been loaded
            pFileInfo->m_SourceFileFullPath[0] != 0 )                      // we have a source file
        {
            WIN32_FIND_DATAA data;
            memset( &data, 0, sizeof( data ) );

            HANDLE handle = FindFirstFileA( pFileInfo->m_SourceFileFullPath, &data );
            if( handle != INVALID_HANDLE_VALUE )
                FindClose( handle );

            // if the source file is newer than the data file, reimport it.
            if( data.ftLastWriteTime.dwHighDateTime > pFileInfo->m_pFile->GetFileLastWriteTime().dwHighDateTime ||
                ( data.ftLastWriteTime.dwHighDateTime == pFileInfo->m_pFile->GetFileLastWriteTime().dwHighDateTime &&
                  data.ftLastWriteTime.dwLowDateTime > pFileInfo->m_pFile->GetFileLastWriteTime().dwLowDateTime ) )
            {
                ImportDataFile( pFileInfo->m_SceneID, pFileInfo->m_SourceFileFullPath );
                bool updated = true;
            }

            pFileInfo->m_DidInitialCheckIfSourceFileWasUpdated = true;
        }
#else
        if( (initialcheck == false || pFileInfo->m_DidInitialCheckIfSourceFileWasUpdated == false) && 
            pFileInfo->m_pFile->GetFileLastWriteTime() != 0 && // converted file has been loaded
            pFileInfo->m_SourceFileFullPath[0] != 0 )                      // we have a source file
        {
            struct stat data;
            stat( pFileInfo->m_SourceFileFullPath, &data );
            if( data.st_mtime == pFileInfo->m_pFile->GetFileLastWriteTime() )
            {
                ImportDataFile( pFileInfo->m_SceneID, pFileInfo->m_SourceFileFullPath );
                bool updated = true;
            }

            pFileInfo->m_DidInitialCheckIfSourceFileWasUpdated = true;
        }
#endif
    }
    
    // TODO: check for updates to files that are still loading?
    //m_FilesStillLoading
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
#endif //MYFW_USING_WX

void ComponentSystemManager::OnMaterialCreated(MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial );

    // if this material has a file and it has a name, then save it.
    if( pMaterial && pMaterial->GetFile() )
    {
        // Add the material to the file list, so it can be freed on shutdown.
        AddToFileList( pMaterial->GetFile(), 0, 0, 0, pMaterial, 0, 0, SCENEID_MainScene );
        pMaterial->GetFile()->AddRef();
    }
}

void ComponentSystemManager::OnSoundCueCreated(SoundCue* pSoundCue)
{
    MyAssert( pSoundCue );

    if( pSoundCue )
    {
        pSoundCue->AddRef();
    }

    // If this sound cue doesn't have a file and it has a name, then save it.
    if( pSoundCue && pSoundCue->GetFile() )
    {
        // Add the sound cue to the file list, so it can be freed on shutdown.
        AddToFileList( pSoundCue->GetFile(), 0, 0, 0, 0, pSoundCue, 0, SCENEID_MainScene );
        pSoundCue->GetFile()->AddRef();
    }
}

void ComponentSystemManager::OnSoundCueUnloaded(SoundCue* pSoundCue) // StaticOnSoundCueUnloaded
{
    MyAssert( pSoundCue );

    // Loop through both lists of files and unload all files referencing this sound cue.
    for( int filelist=0; filelist<2; filelist++ )
    {
        CPPListNode* pFirstNode = 0;

        if( filelist == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        CPPListNode* pNextNode;
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNextNode )
        {
            pNextNode = pNode->GetNext();

            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            if( pFileInfo->m_pSoundCue == pSoundCue )
            {
                // Unload the file.
                FreeDataFile( pFileInfo );
            }
        }
    }
}

void ComponentSystemManager::OnFileUnloaded(MyFileObject* pFile) // StaticOnFileUnloaded
{
    MyAssert( pFile );

    // Loop through both lists of files and unload all files referencing this file.
    for( int filelist=0; filelist<2; filelist++ )
    {
        CPPListNode* pFirstNode = 0;

        if( filelist == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        CPPListNode* pNextNode;
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNextNode )
        {
            pNextNode = pNode->GetNext();

            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            // Unload the file.
            if( pFileInfo->m_pFile == pFile
                || (pFileInfo->m_pMesh          && pFileInfo->m_pMesh->GetFile()            == pFile)
                || (pFileInfo->m_pShaderGroup   && pFileInfo->m_pShaderGroup->GetFile()     == pFile)
                || (pFileInfo->m_pTexture       && pFileInfo->m_pTexture->GetFile()         == pFile)
                || (pFileInfo->m_pMaterial      && pFileInfo->m_pMaterial->GetFile()        == pFile)
                || (pFileInfo->m_pSoundCue      && pFileInfo->m_pSoundCue->GetFile()        == pFile)
                || (pFileInfo->m_pSpriteSheet   && pFileInfo->m_pSpriteSheet->GetJSONFile() == pFile)
                || (pFileInfo->m_pPrefabFile    && pFileInfo->m_pPrefabFile->GetFile()      == pFile)
              )
            {
                LOGInfo( LOGTag, "File removed from scene file list: %s\n", pFile->GetFullPath() );

                FreeDataFile( pFileInfo );
            }
        }
    }
}

void ComponentSystemManager::OnFindAllReferences(MyFileObject* pFile) // StaticOnFindAllReferences
{
    LogAllReferencesForFile( pFile );
}
#endif //MYFW_EDITOR
