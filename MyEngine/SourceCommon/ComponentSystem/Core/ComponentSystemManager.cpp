//
// Copyright (c) 2014-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentSystemManager.h"
#include "PrefabManager.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentInputHandler.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/ComponentTypeManager.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/EngineComponents/ComponentLuaScript.h"
#include "ComponentSystem/FrameworkComponents/ComponentCameraShadow.h"
#include "ComponentSystem/FrameworkComponents/ComponentSprite.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DCollisionObject.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "../../../Framework/MyFramework/SourceCommon/RenderGraphs/RenderGraph_Base.h"
#include "../../../Framework/MyFramework/SourceCommon/RenderGraphs/RenderGraph_Flat.h"
#include "../../../Framework/MyFramework/SourceCommon/RenderGraphs/RenderGraph_Octree.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

#if MYFW_USING_BOX2D
#include "Physics/EngineBox2DContactListener.h"
#endif

#if MYFW_EDITOR
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/GameObjectTemplateManager.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/Exporters/ExportBox2DScene.h"
#endif //MYFW_EDITOR

ComponentSystemManager* g_pComponentSystemManager = nullptr;

ComponentSystemManager::ComponentSystemManager(ComponentTypeManager* pTypeManager, EngineCore* pEngineCore)
{
    g_pComponentSystemManager = this;

    m_pEngineCore = pEngineCore;
    m_pComponentTypeManager = pTypeManager;
    if( m_pComponentTypeManager )
    {
        m_pComponentTypeManager->SetComponentSystemManager( this );
    }
#if MYFW_EDITOR
    m_pSceneHandler = MyNew SceneHandler();
    m_pGameObjectTemplateManager = MyNew GameObjectTemplateManager();
#endif

    // Create a pool of transform changed callback objects.
    if( m_pComponentTransform_TransformChangedCallbackPool.IsInitialized() == false )
        m_pComponentTransform_TransformChangedCallbackPool.AllocateObjects( CALLBACK_POOL_SIZE );

    // Initialize managers.
    m_pPrefabManager = MyNew PrefabManager( m_pEngineCore );

    EventManager* pEventManager = m_pEngineCore->GetManagers()->GetEventManager();
    pEventManager->RegisterForEvents( "GameObjectEnable", this, &ComponentSystemManager::StaticOnEvent );

#if MYFW_EDITOR
    m_pEngineCore->GetManagers()->GetMaterialManager()->RegisterMaterialCreatedCallback( this, StaticOnMaterialCreated );
    m_pEngineCore->GetManagers()->GetFileManager()->RegisterFileUnloadedCallback( this, StaticOnFileUnloaded );
    m_pEngineCore->GetManagers()->GetFileManager()->RegisterFindAllReferencesCallback( this, StaticOnFindAllReferences );
    m_pEngineCore->GetSoundManager()->RegisterSoundCueCreatedCallback( this, StaticOnSoundCueCreated );
    m_pEngineCore->GetSoundManager()->RegisterSoundCueUnloadedCallback( this, StaticOnSoundCueUnloaded );

    // This class adds to SoundCue's refcount when storing cue in m_Files,
    //    so increment this number to prevent editor from allowing it to be unloaded if ref'ed by game code.
    m_pEngineCore->GetSoundManager()->Editor_AddToNumRefsPlacedOnSoundCueBySystem();
#endif

    m_NextSceneID = SCENEID_MainScene;

    m_WaitingForFilesToFinishLoading = false;
    m_StartGamePlayWhenDoneLoading = false;

    m_TimeScale = 1;

    //m_pRenderGraph = MyNew RenderGraph_Flat();
    int depth = 3;
    m_pRenderGraph = MyNew RenderGraph_Octree( m_pEngineCore, depth, -32, -32, -32, 32, 32, 32 );

    for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        m_pSceneInfoMap[i].Reset();
    }

#if MYFW_EDITOR
    //// Mark the scene for engine object as being used, so it will be unloaded.
    //m_pSceneInfoMap[SCENEID_EngineObjects].m_InUse = true;
#endif //MYFW_EDITOR

    // Create a scene for "Unmanaged/Runtime" objects.
    m_pSceneInfoMap[SCENEID_Unmanaged].m_InUse = true;
}

ComponentSystemManager::~ComponentSystemManager()
{
    // Prefab objects are in scene 0, so let the PrefabManager delete them before the sceneinfo does below.
    SAFE_DELETE( m_pPrefabManager );

    // Unload all runtime created objects.
    UnloadScene( SCENEID_Unmanaged, false );

    // Reset all scenes, i.e. Delete all GameObjects from each scene.
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
    
    SAFE_DELETE( m_pRenderGraph );

    // If a component didn't unregister its callbacks, assert.
    MyAssert( m_pComponentCallbackList_Tick.GetHead() == nullptr );
    MyAssert( m_pComponentCallbackList_OnSurfaceChanged.GetHead() == nullptr );
    MyAssert( m_pComponentCallbackList_Draw.GetHead() == nullptr );
    MyAssert( m_pComponentCallbackList_OnTouch.GetHead() == nullptr );
    MyAssert( m_pComponentCallbackList_OnButtons.GetHead() == nullptr );
    MyAssert( m_pComponentCallbackList_OnKeys.GetHead() == nullptr );
    MyAssert( m_pComponentCallbackList_OnFileRenamed.GetHead() == nullptr );
    
    m_pComponentTransform_TransformChangedCallbackPool.FreeObjects();

    g_pComponentSystemManager = nullptr;
}

// TODO: Put in some sort of priority/sorting system for callbacks.
#define MYFW_COMPONENTSYSTEMMANAGER_IMPLEMENT_CALLBACK_REGISTER_FUNCTIONS(CallbackType) \
    void ComponentSystemManager::RegisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct) \
    { \
        MyAssert( pCallbackStruct->pFunc != nullptr && pCallbackStruct->pObj != nullptr && pCallbackStruct->Prev == nullptr && pCallbackStruct->Next == nullptr ); \
        m_pComponentCallbackList_##CallbackType.AddTail( pCallbackStruct ); \
    } \
    void ComponentSystemManager::UnregisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct) \
    { \
        if( pCallbackStruct->Prev ) \
        { \
            pCallbackStruct->Remove(); \
            pCallbackStruct->Next = nullptr; \
            pCallbackStruct->Prev = nullptr; \
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
    MyAssert( pCallbackStruct->pFunc != nullptr && pCallbackStruct->pObj != nullptr );
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
        .beginClass<MyFileInfo>( "MyFileInfo" )
            .addFunction( "GetFile", &MyFileInfo::GetFile ) // MyFileObject* GetFile()
            .addFunction( "GetShaderGroup", &MyFileInfo::GetShaderGroup ) // ShaderGroup* GetShaderGroup()
        .endClass();

    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSystemManager>( "ComponentSystemManager" )
            .addFunction( "SetTimeScale", &ComponentSystemManager::SetTimeScale ) // void ComponentSystemManager::SetTimeScale(float scale)
            .addFunction( "Editor_CreateGameObject", &ComponentSystemManager::EditorLua_CreateGameObject ) // GameObject* ComponentSystemManager::EditorLua_CreateGameObject(const char* name, uint32 sceneID, bool isfolder, bool hastransform)
            .addFunction( "DeleteGameObject", &ComponentSystemManager::DeleteGameObject ) // void ComponentSystemManager::DeleteGameObject(GameObject* pObject, bool deleteComponents)
            .addFunction( "CopyGameObject", &ComponentSystemManager::CopyGameObject ) // GameObject* ComponentSystemManager::CopyGameObject(GameObject* pObject, const char* newName)
            .addFunction( "FindGameObjectByName", &ComponentSystemManager::FindGameObjectByName ) // GameObject* ComponentSystemManager::FindGameObjectByName(const char* name)
            .addFunction( "GetGameObjectsInRange", &ComponentSystemManager::Lua_GetGameObjectsInRange ) // luabridge::LuaRef ComponentSystemManager::Lua_GetGameObjectsInRange(Vector3 pos, float range, unsigned int flags)
            .addFunction( "Editor_LoadDataFile", &ComponentSystemManager::EditorLua_LoadDataFile ) // MyFileInfo* ComponentSystemManager::EditorLua_LoadDataFile(const char* relativePath, uint32 sceneID, const char* fullSourceFilePath, bool convertifrequired)
            .addFunction( "Editor_GetFirstGameObjectFromScene", &ComponentSystemManager::EditorLua_GetFirstGameObjectFromScene ) // GameObject* ComponentSystemManager::EditorLua_GetFirstGameObjectFromScene(uint32 sceneID)
        .endClass();
}
#endif //MYFW_USING_LUA

void ComponentSystemManager::MoveAllFilesNeededForLoadingScreenToStartOfFileList(GameObject* first)
{
    MyAssert( first != nullptr );
    if( first == nullptr )
        return;

    for( GameObject* pGameObject = first; pGameObject; pGameObject = pGameObject->GetNext() )
    {
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
                        FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
                        pFileManager->MoveFileToFrontOfFileLoadedList( pScriptFile );
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

void ComponentSystemManager::AddListOfFilesUsedToJSONObject(SceneID sceneID, cJSON* jFileArray)
{
    // TODO: There are currently many ways a file can be loaded into a secondary scene without being in this list.
    //       Need to adjust code in various components to account for this.

    // loop through both lists of files
    for( int fileList=0; fileList<2; fileList++ )
    {
        CPPListNode* pFirstNode = nullptr;

        if( fileList == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNode->GetNext() )
        {
            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;
        
            MyAssert( pFileInfo );
            if( pFileInfo == nullptr )
                continue;

            if( pFileInfo->GetSceneID() != sceneID )
                continue;
        
            MyFileObject* pFile = pFileInfo->GetFile();
            if( pFile != nullptr )
            {
                // Skip over shader include files.
                if( pFile->IsA( "MyFileShader" ) )
                {
                    MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;
                    if( pShaderFile && pShaderFile->m_IsAnIncludeFile )
                    {
                        MyAssert( false ); // Shader include files shouldn't be in the file list.
                        continue;
                    }
                }

                cJSON* jFile = cJSON_CreateObject();
                cJSON_AddItemToObject( jFile, "Path", cJSON_CreateString( pFile->GetFullPath() ) );
                cJSON_AddItemToArray( jFileArray, jFile );

                // Save the source path if there is one.
                if( pFileInfo->GetSourceFileFullPath()[0] != '\0' )
                {
                    cJSON_AddItemToObject( jFile, "SourcePath", cJSON_CreateString( pFileInfo->GetSourceFileFullPath() ) );
                }
            }
            else
            {
                cJSON* jFile = cJSON_CreateObject();
                cJSON_AddItemToObject( jFile, "Path", cJSON_CreateString( pFileInfo->GetSourceFileFullPath() ) );
                cJSON_AddItemToArray( jFileArray, jFile );
            }
        }
    }
}

char* ComponentSystemManager::SaveSceneToJSON(SceneID sceneID)
{
    cJSON* jRoot = cJSON_CreateObject();
    cJSON* jFileArray = cJSON_CreateArray();
    cJSON* jGameObjectArray = cJSON_CreateArray();
    cJSON* jTransformArray = cJSON_CreateArray();
    cJSON* jComponentArray = cJSON_CreateArray();

    cJSON_AddItemToObject( jRoot, "Files", jFileArray );
    cJSON_AddItemToObject( jRoot, "GameObjects", jGameObjectArray );
    cJSON_AddItemToObject( jRoot, "Transforms", jTransformArray );
    cJSON_AddItemToObject( jRoot, "Components", jComponentArray );

    bool savingallscenes = (sceneID == SCENEID_TempPlayStop);

    // Move files used by gameobjects that start with "Load" to front of file list.
    {
        for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( m_pSceneInfoMap[i].m_InUse == false )
                continue;

            SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

            if( pSceneInfo->m_GameObjects.GetHead() )
            {
                MoveAllFilesNeededForLoadingScreenToStartOfFileList( pSceneInfo->m_GameObjects.GetHead() );
            }
        }
    }

    // Add the files used.
    AddListOfFilesUsedToJSONObject( sceneID, jFileArray );

    // Add the game objects and their transform components.
    {
        for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
            if( m_pSceneInfoMap[i].m_InUse == false )
                continue;

            SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

            GameObject* first = pSceneInfo->m_GameObjects.GetHead();
            if( first && ( first->GetSceneID() == sceneID || savingallscenes ) )
            {
                SaveGameObjectListToJSONArray( jGameObjectArray, jTransformArray, first, savingallscenes );
            }
        }
    }

    // Add each of the component types, don't save components of unmanaged objects.
    {
        for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
        {
            for( CPPListNode* pNode = m_Components[i].GetHead(); pNode; pNode = pNode->GetNext() )
            {
                ComponentBase* pComponent = (ComponentBase*)pNode;
                if( pComponent->GetGameObject()->IsManaged() &&
                    ( pComponent->GetGameObject()->GetSceneID() == sceneID || savingallscenes )
                  )
                {
                    cJSON_AddItemToArray( jComponentArray, pComponent->ExportAsJSONObject( savingallscenes, true ) );
                }
            }
        }
    }

    char* saveString = cJSON_Print( jRoot );
    cJSON_Delete( jRoot );

    return saveString;
}

#if MYFW_USING_BOX2D
char* ComponentSystemManager::ExportBox2DSceneToJSON(SceneID sceneID)
{
#if MYFW_EDITOR
    return ::ExportBox2DSceneToJSON( this, sceneID );
#else
    return nullptr;
#endif //MYFW_EDITOR
}
#endif //MYFW_USING_BOX2D

void ComponentSystemManager::SaveGameObjectListToJSONArray(cJSON* jGameObjectArray, cJSON* jTransformArray, GameObject* first, bool savesceneid)
{
    for( GameObject* pGameObject = first; pGameObject; pGameObject = pGameObject->GetNext() )
    {
        // Only save managed GameObjects, they will be marked as unmanaged if deleted and still in undo list.
        if( pGameObject->IsManaged() )
        {
            cJSON_AddItemToArray( jGameObjectArray, pGameObject->ExportAsJSONObject( savesceneid ) );

            ComponentBase* pComponent = pGameObject->GetTransform();
            if( pComponent )
                cJSON_AddItemToArray( jTransformArray, pComponent->ExportAsJSONObject( savesceneid, true ) );

            GameObject* pFirstChild = pGameObject->GetFirstChild();
            if( pFirstChild )
            {
                SaveGameObjectListToJSONArray( jGameObjectArray, jTransformArray, pFirstChild, savesceneid );
            }
        }
    }
}

MyFileInfo* ComponentSystemManager::GetFileInfoIfUsedByScene(MyFileObject* pFile, SceneID sceneID)
{
    // Loop through both lists of files.
    for( int fileList=0; fileList<2; fileList++ )
    {
        CPPListNode* pFirstNode = nullptr;

        if( fileList == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNode->GetNext() )
        {
            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            if( sceneID == SCENEID_AllScenes || sceneID == SCENEID_Any || pFileInfo->GetSceneID() == sceneID )
            {
                if( pFileInfo->GetFile() == pFile )
                    return pFileInfo;
            }
        }
    }

    return nullptr;
}

MyFileInfo* ComponentSystemManager::GetFileInfoIfUsedByScene(const char* fullPath, SceneID sceneID)
{
    // Loop through both lists of files.
    for( int fileList=0; fileList<2; fileList++ )
    {
        CPPListNode* pFirstNode = nullptr;

        if( fileList == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNode->GetNext() )
        {
            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            if( sceneID == SCENEID_AllScenes || sceneID == SCENEID_Any || pFileInfo->GetSceneID() == sceneID )
            {
                if( pFileInfo->GetFile() == nullptr )
                {
                    if( strcmp( pFileInfo->GetSourceFileFullPath(), fullPath ) == 0 )
                        return pFileInfo;
                }
                else
                {
                    if( strcmp( pFileInfo->GetFile()->GetFullPath(), fullPath ) == 0 )
                        return pFileInfo;
                }
            }
        }
    }

    return nullptr;
}

MyFileObject* ComponentSystemManager::GetFileObjectIfUsedByScene(const char* fullPath, SceneID sceneID)
{
    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( fullPath, sceneID );

    if( pFileInfo )
        return pFileInfo->GetFile();

    return nullptr;
}

MyFileInfo* ComponentSystemManager::AddToFileList(MyFileObject* pFile, SceneID sceneID)
{
    return AddToFileList( pFile, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sceneID);
}
MyFileInfo* ComponentSystemManager::AddToFileList(MyMesh* pMesh, SceneID sceneID)
{
    return AddToFileList( nullptr, pMesh, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sceneID);
}
MyFileInfo* ComponentSystemManager::AddToFileList(ShaderGroup* pShaderGroup, SceneID sceneID)
{
    return AddToFileList( nullptr, nullptr, pShaderGroup, nullptr, nullptr, nullptr, nullptr, nullptr, sceneID);
}
MyFileInfo* ComponentSystemManager::AddToFileList(TextureDefinition* pTexture, SceneID sceneID)
{
    return AddToFileList( nullptr, nullptr, nullptr, pTexture, nullptr, nullptr, nullptr, nullptr, sceneID);
}
MyFileInfo* ComponentSystemManager::AddToFileList(MaterialDefinition* pMaterial, SceneID sceneID)
{
    return AddToFileList( nullptr, nullptr, nullptr, nullptr, pMaterial, nullptr, nullptr, nullptr, sceneID);
}
MyFileInfo* ComponentSystemManager::AddToFileList(SoundCue* pSoundCue, SceneID sceneID)
{
    return AddToFileList( nullptr, nullptr, nullptr, nullptr, nullptr, pSoundCue, nullptr, nullptr, sceneID);
}
MyFileInfo* ComponentSystemManager::AddToFileList(SpriteSheet* pSpriteSheet, SceneID sceneID)
{
    return AddToFileList( nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, pSpriteSheet, nullptr, sceneID);
}
MyFileInfo* ComponentSystemManager::AddToFileList(My2DAnimInfo* p2DAnimInfo, SceneID sceneID)
{
    return AddToFileList( nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, p2DAnimInfo, sceneID);
}

MyFileInfo* ComponentSystemManager::AddToFileList(MyFileObject* pFile, MyMesh* pMesh, ShaderGroup* pShaderGroup, TextureDefinition* pTexture, MaterialDefinition* pMaterial, SoundCue* pSoundCue, SpriteSheet* pSpriteSheet, My2DAnimInfo* p2DAnimInfo, SceneID sceneID)
{
    MyFileInfo* pFileInfo = MyNew MyFileInfo();

    // Store pFile so we can free it when scene is unloaded or file is unloaded in editor.
    pFileInfo->SetFile( pFile );

    pFileInfo->SetMesh( pMesh );
    pFileInfo->SetShaderGroup( pShaderGroup );
    pFileInfo->SetTexture( pTexture );
    pFileInfo->SetMaterial( pMaterial );
    pFileInfo->SetSoundCue( pSoundCue );
    pFileInfo->SetSpriteSheet( pSpriteSheet );
    pFileInfo->SetPrefabFile( nullptr );
    pFileInfo->Set2DAnimInfo( p2DAnimInfo );

    pFileInfo->SetSceneID( sceneID );

    m_Files.AddTail( pFileInfo );

    return pFileInfo;
}

MyFileInfo* ComponentSystemManager::EditorLua_LoadDataFile(const char* relativePath, uint32 sceneID, const char* fullSourceFilePath, bool convertifrequired)
{
    return LoadDataFile( relativePath, (SceneID)sceneID, fullSourceFilePath, convertifrequired );
}

MyFileInfo* ComponentSystemManager::LoadDataFile(const char* relativePath, SceneID sceneID, const char* fullSourceFilePath, bool convertIfRequired)
{
    MyAssert( relativePath );

    // Each scene loaded will add ref's to the file.
    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( relativePath, sceneID );

    // If the file is already tagged as being used by this scene, don't request/addref it.
    if( pFileInfo != nullptr )
    {
        return pFileInfo;
        //LOGInfo( LOGTag, "%s already in scene, reloading\n", relativePath );
        //FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
        //pFileManager->ReloadFile( pFile );
        //OnFileUpdated_CallbackFunction( pFile );
    }
    else
    {
        MyFileObject* pFile = nullptr;

        size_t fullLen = 0;
        if( fullSourceFilePath )
            fullLen = strlen( fullSourceFilePath );
        size_t relLen = strlen( relativePath );

#if MYFW_EDITOR
#if MYFW_WINDOWS
        if( convertIfRequired && fullSourceFilePath )
        {
            WIN32_FIND_DATAA datafiledata;
            memset( &datafiledata, 0, sizeof( datafiledata ) );
            HANDLE datafilehandle = FindFirstFileA( relativePath, &datafiledata );
            if( datafilehandle != INVALID_HANDLE_VALUE )
                FindClose( datafilehandle );

            WIN32_FIND_DATAA sourcefiledata;
            memset( &sourcefiledata, 0, sizeof( sourcefiledata ) );
            HANDLE sourcefilehandle = FindFirstFileA( fullSourceFilePath, &sourcefiledata );
            if( sourcefilehandle != INVALID_HANDLE_VALUE )
                FindClose( sourcefilehandle );

            // If the source file is newer than the data file (or data file doesn't exist), reimport it.
            if( sourcefiledata.ftLastWriteTime.dwHighDateTime >= datafiledata.ftLastWriteTime.dwHighDateTime ||
                ( sourcefiledata.ftLastWriteTime.dwHighDateTime == datafiledata.ftLastWriteTime.dwHighDateTime &&
                  sourcefiledata.ftLastWriteTime.dwLowDateTime >= datafiledata.ftLastWriteTime.dwLowDateTime ) )
            {
                MyFileObject* pFile = ImportDataFile( sceneID, fullSourceFilePath );

                if( pFile )
                {
                    pFileInfo = AddToFileList( pFile, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sceneID );
                    return pFileInfo;
                }
            }
        }
#else
        if( convertIfRequired && fullSourceFilePath )
        {
            struct stat datafiledata;
            memset( &datafiledata, 0, sizeof( datafiledata ) );
            stat( relativePath, &datafiledata );

            struct stat sourcefiledata;
            memset( &sourcefiledata, 0, sizeof( sourcefiledata ) );
            stat( fullSourceFilePath, &sourcefiledata );

            // If the source file is newer than the data file (or data file doesn't exist), reimport it.
            if( sourcefiledata.st_mtime >= datafiledata.st_mtime )
            {
                MyFileObject* pFile = ImportDataFile( sceneID, fullSourceFilePath );

                if( pFile )
                {
                    pFileInfo = AddToFileList( pFile, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sceneID );
                    return pFileInfo;
                }
            }
        }
#endif // Windows vs OSX/Linux
#endif //MYFW_EDITOR
        
        // store pFile so we can free it afterwards.
        pFileInfo = AddToFileList( pFile, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sceneID );

        TextureDefinition* pTexture = nullptr;

        // Load textures differently than other files.
        if( relLen > 4 && strcmp( &relativePath[relLen-4], ".png" ) == 0 )
        {
            TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();

            // Check if the texture is already loaded and create it if not.
            pTexture = pTextureManager->FindTexture( relativePath );

            if( pTexture == nullptr )
            {
                // Find the file and add it to the FileInfo object.
                EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( m_pEngineCore->GetManagers()->GetFileManager() );
                pFile = pEngineFileManager->RequestFile_UntrackedByScene( relativePath );
                pFileInfo->SetFile( pFile );
                pFile->Release(); // Release ref added by RequestFile.

                pTexture = pTextureManager->CreateTexture( pFile );
                MyAssert( pFile == pTexture->GetFile() );
                pFile = pTexture->GetFile();

                // Update the fileinfo block with the texture.
                pFileInfo->SetTexture( pTexture );
                pTexture->Release(); // Release ref added by CreateTexture.
            }
            else
            {
                MyAssert( false ); // The texture shouldn't be loaded and not in the list of files used.
            }
        }
        else if( relLen > 4 && strcmp( &relativePath[relLen-4], ".wav" ) == 0 )
        {
#if !MYFW_EDITOR
            // Raw wav's shouldn't be loadable in standalone builds, scenes should only reference sound cues.
            MyAssert( false );
#else
            // Let SoundPlayer (SDL on windows) load the wav files
            SoundCue* pCue = m_pEngineCore->GetSoundManager()->CreateCue( "new cue" );
            m_pEngineCore->GetSoundManager()->AddSoundToCue( pCue, relativePath );
            pCue->SaveSoundCue( nullptr );

            pFileInfo->SetSoundCue( pCue );
            pFileInfo->SetFile( pCue->GetFile() );
            //strcpy_s( pFileInfo->GetSourceFileFullPath(), MAX_PATH, relativePath );
#endif //!MYFW_EDITOR
            return nullptr;
        }
#if MYFW_EDITOR
        else if( relLen > 14 && strcmp( &relativePath[relLen-14], ".myspritesheet" ) == 0 )
        {
            // In editor builds, fully load spritesheets immediately.
            EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( m_pEngineCore->GetManagers()->GetFileManager() );
            pFile = pEngineFileManager->LoadFileNow( relativePath );
            pFileInfo->SetFile( pFile );
            if( pFile )
            {
                pFile->Release(); // Release ref added by LoadFileNow if file was found.
            }
        }
#endif
        else
        {
            // Call untracked request since we're in the tracking code, just to avoid unnecessary repeat of LoadDataFile() call.
            EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( m_pEngineCore->GetManagers()->GetFileManager() );
            pFile = pEngineFileManager->RequestFile_UntrackedByScene( relativePath );
            pFileInfo->SetFile( pFile );
            pFile->Release(); // Release ref added by RequestFile.
        }

        // If the extension of the source file is different than that of the file we're loading,
        //   then store the source file path in the fileobject, so we can detect/reconvert if that file changes.
        if( relLen > 4 && fullLen > 4 &&
            ( strcmp( &relativePath[relLen-4], &fullSourceFilePath[fullLen-4] ) != 0 ) )
        {
            char path[MAX_PATH];
            strcpy_s( path, MAX_PATH, fullSourceFilePath );
            const char* relativePath = GetRelativePath( path );

            char finalpath[MAX_PATH];

            // Store the relative path if the file is relative... otherwise store the full path.
            if( relativePath == nullptr )
                strcpy_s( finalpath, MAX_PATH, fullSourceFilePath );
            else
                strcpy_s( finalpath, MAX_PATH, relativePath );

            FixSlashesInPath( finalpath );

            pFileInfo->SetSourceFileFullPath( finalpath );
        }

        // If not file was loaded (file not found), return now.
        if( pFile == nullptr )
        {
            return pFileInfo;
        }

        // If we're loading a mesh file type, create a mesh.
        {
            if( strcmp( pFile->GetExtensionWithDot(), ".obj" ) == 0 ||
                strcmp( pFile->GetExtensionWithDot(), ".mymesh" ) == 0 )
            {
                MyMesh* pMesh = MyNew MyMesh( m_pEngineCore );
                pMesh->SetSourceFile( pFile );

                pFileInfo->SetMesh( pMesh );

                pMesh->Release();
            }
        }

        // If we're loading an .glsl file, create a ShaderGroup.
        if( strcmp( pFile->GetExtensionWithDot(), ".glsl" ) == 0 )
        {
            ShaderGroup* pShaderGroup = m_pEngineCore->GetManagers()->GetShaderGroupManager()->FindShaderGroupByFile( pFile );

            if( pShaderGroup == nullptr )
            {
                TextureDefinition* pErrorTexture = m_pEngineCore->GetManagers()->GetTextureManager()->GetErrorTexture();
                ShaderGroupManager* pShaderGroupManager = m_pEngineCore->GetManagers()->GetShaderGroupManager();

                pShaderGroup = MyNew ShaderGroup( m_pEngineCore, pFile, pErrorTexture );
                pFileInfo->SetShaderGroup( pShaderGroup );
                pShaderGroup->Release();
            }
            else
            {
                pFileInfo->SetShaderGroup( pShaderGroup );
            }
        }

        // If we're loading a .mymaterial file, create a Material.
        if( strcmp( pFile->GetExtensionWithDot(), ".mymaterial" ) == 0 )
        {
            MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
            MaterialDefinition* pMat = pMaterialManager->LoadMaterial( pFile->GetFullPath() );
            pFileInfo->SetMaterial( pMat );
            pMat->Release();
        }

        // If we're loading a .myprefabs file, add it to the prefab manager.
        if( strcmp( pFile->GetExtensionWithDot(), ".myprefabs" ) == 0 )
        {
            PrefabFile* pPrefabFile = m_pPrefabManager->RequestFile( pFile->GetFullPath() );
            pFileInfo->SetPrefabFile( pPrefabFile );
        }

        // If we're loading a .my2daniminfo file, do nothing for now.
        if( strcmp( pFile->GetExtensionWithDot(), ".my2daniminfo" ) == 0 )
        {
            // Create the anim info block and load the file.
            My2DAnimInfo* pAnimInfo = MyNew My2DAnimInfo();
            pAnimInfo->SetSourceFile( pFile );
            pFileInfo->Set2DAnimInfo( pAnimInfo );
            pAnimInfo->Release();
        }

        // If we're loading a .myspritesheet, we create a material for each texture in the sheet.
        if( strcmp( pFile->GetExtensionWithDot(), ".myspritesheet" ) == 0 )
        {
            //ShaderGroup* pShaderGroup = m_pEngineCore->GetManagers()->GetShaderGroupManager->FindShaderGroupByFilename( "Data/DataEngine/Shaders/Shader_TextureTint.glsl" );
            // TODO: Allow the user to choose a shader.
            MyFileInfo* pFileInfo = LoadDataFile( "Data/Shaders/Shader_Texture.glsl", SCENEID_MainScene, nullptr, false );
            ShaderGroup* pShaderGroup = pFileInfo->GetShaderGroup();

            SpriteSheet* pSpriteSheet = MyNew SpriteSheet( m_pEngineCore );
            pSpriteSheet->Create( pFile->GetFullPath(), pShaderGroup, MyRE::MinFilter_Linear, MyRE::MagFilter_Linear, false, true );

            pFileInfo->SetSpriteSheet( pSpriteSheet );

            m_FilesStillLoading.MoveHead( pFileInfo );

#if MYFW_EDITOR
            // Create a default my2daniminfo file for this spritesheet.
            {
                char newFilename[MAX_PATH];
                sprintf_s( newFilename, "%s.my2daniminfo", pFile->GetFilenameWithoutExtension() );
                char animFullPath[MAX_PATH];
                pFile->GenerateNewFullPathFilenameInSameFolder( newFilename, animFullPath, MAX_PATH );
                if( FileManager::DoesFileExist( animFullPath ) )
                {
                    // Load the existing animation file.
                    LoadDataFile( animFullPath, sceneID, nullptr, false );
                }
                else
                {
                    // Create a new animation file.
                    FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
                    MyFileObject* pFile = pFileManager->CreateFileObject( animFullPath );
                    My2DAnimInfo* pAnimInfo = MyNew My2DAnimInfo();
                    pAnimInfo->SetSourceFile( pFile );
                    AddToFileList( pFile, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, pAnimInfo, sceneID );
                    pAnimInfo->Release();

                    pAnimInfo->LoadFromSpriteSheet( pSpriteSheet, 0.2f );
                    pAnimInfo->SaveAnimationControlFile();
                }
            }
#endif
        }

        // If we're loading a .mycue file, create a Sound Cue.
        if( strcmp( pFile->GetExtensionWithDot(), ".mycue" ) == 0 )
        {
            SoundCue* pSoundCue = m_pEngineCore->GetSoundManager()->LoadCue( pFile->GetFullPath() );
            pFileInfo->SetSoundCue( pSoundCue );
            pSoundCue->Release();
        }
    }

    return pFileInfo;
}

#if MYFW_EDITOR
MyFileObject* ComponentSystemManager::ImportDataFile(SceneID sceneID, const char* fullSourceFilePath)
{
    MyAssert( fullSourceFilePath );
    if( fullSourceFilePath == nullptr )
        return nullptr;

    //const char* relativePath = GetRelativePath( fullSourceFilePath );
    //size_t relLen = strlen( relativePath );

    size_t fullLen = 0;
    fullLen = strlen( fullSourceFilePath );

    // Convert any .fbx files into a mymesh and load that.
    if( ( fullLen > 4 && strcmp( &fullSourceFilePath[fullLen-4], ".fbx" ) == 0 ) ||
        ( fullLen > 4 && strcmp( &fullSourceFilePath[fullLen-4], ".obj" ) == 0 ) ||
        ( fullLen > 6 && strcmp( &fullSourceFilePath[fullLen-6], ".blend" ) == 0 )
      )
    {
        // Run MeshTool to convert the mesh and put the result in "Data/Meshes".
        const int paramsBufferSize = MAX_PATH * 2 + 50;
        char params[paramsBufferSize]; // 2 full paths plus a few extra chars

        char workingdir[MAX_PATH];
#if MYFW_WINDOWS
        _getcwd( workingdir, MAX_PATH * sizeof(char) );
#else
        getcwd( workingdir, MAX_PATH * sizeof(char) );
#endif

        char filename[MAX_PATH];
        for( int i=(int)strlen(fullSourceFilePath)-1; i>=0; i-- )
        {
            if( fullSourceFilePath[i] == '\\' || fullSourceFilePath[i] == '/' || i == 0 )
            {
                strcpy_s( filename, MAX_PATH, &fullSourceFilePath[i+1] );
                break;
            }
        }

        sprintf_s( params, paramsBufferSize, "-s %s -o Data/Meshes/%s -m Data/Materials", fullSourceFilePath, filename );

        LOGInfo( LOGTag, "Converting %s to mymesh %s\n", fullSourceFilePath, params );

#if MYFW_WINDOWS
        SHELLEXECUTEINFOA info = { 0 };
        info.cbSize = sizeof( SHELLEXECUTEINFOA );
        info.fMask = SEE_MASK_NOASYNC; //SEE_MASK_NOCLOSEPROCESS;
        info.hwnd = 0;
        info.lpVerb = nullptr;
        info.lpFile = "Tools\\MeshTool.exe";
        info.lpParameters = params;
        info.lpDirectory = workingdir;
        info.nShow = SW_SHOWNOACTIVATE;
        info.hInstApp = 0;
        
        DWORD errorCode = 1;
        BOOL success = ShellExecuteExA( &info );

        // If Shell execute gives a process handle, wait for it to finish.
        if( info.hProcess )
        {
            WaitForSingleObject( info.hProcess, INFINITE );
            GetExitCodeProcess( info.hProcess, &errorCode ); // Get actual return value from MeshTool.
            //TerminateProcess( info.hProcess );
            CloseHandle( info.hProcess );
        }
        else if( success == 1 ) // If it simple returns success, we're good?
        {
            errorCode = 0;
        }

#elif MYFW_OSX
        char commandWithParams[PATH_MAX*3];
        sprintf( commandWithParams, "Tools/MeshTool %s", params );
        int errorCode = system( commandWithParams );
#else
        int errorCode = 1;
        LOGError( LOGTag, "Mesh conversion only works on Windows and OSX ATM\n" );
#endif

        if( errorCode != 0 )
        {
            LOGError( LOGTag, "Something went wrong with conversion: error(%d) %s\n", errorCode, fullSourceFilePath );
        }
        else
        {
            LOGInfo( LOGTag, "Conversion complete: %s\n", fullSourceFilePath );

#if MYFW_WINDOWS
            // The output file isn't found by loading code on Windows if this sleep isn't here.
            // TODO: Find better solution.
            Sleep( 500 ); // Might not be needed since calling "WaitForSingleObject" above.
#endif

            // Create a new relative path.
            char newrelativepath[MAX_PATH];
            sprintf_s( newrelativepath, MAX_PATH, "Data/Meshes/%s.mymesh", filename );

            MyFileInfo* pFileInfo = LoadDataFile( newrelativepath, sceneID, fullSourceFilePath, false );
            MyAssert( pFileInfo != nullptr );
            return pFileInfo->GetFile();
        }
    }

    return nullptr;
}
#endif //MYFW_EDITOR

void ComponentSystemManager::FreeDataFile(MyFileInfo* pFileInfo)
{
    delete pFileInfo;
}

void ComponentSystemManager::FreeAllDataFiles(SceneID sceneIDToClear)
{
    // Loop through both lists of files.
    for( int fileList=0; fileList<2; fileList++ )
    {
        CPPListNode* pFirstNode = nullptr;

        if( fileList == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        for( CPPListNode* pNode = pFirstNode; pNode;  )
        {
            MyFileInfo* pFile = (MyFileInfo*)pNode;
            pNode = pNode->GetNext();

            if( sceneIDToClear == SCENEID_AllScenes || pFile->GetSceneID() == sceneIDToClear )
            {
                delete pFile;
            }
        }
    }
}

void ComponentSystemManager::LoadSceneFromJSON(const char* sceneName, const char* jsonString, SceneID sceneID)
{
    cJSON* jRoot = cJSON_Parse( jsonString );

    if( jRoot == nullptr )
        return;

    cJSON* jFileArray = cJSON_GetObjectItem( jRoot, "Files" );
    cJSON* jGameobjectArray = cJSON_GetObjectItem( jRoot, "GameObjects" );
    cJSON* jTransformArray = cJSON_GetObjectItem( jRoot, "Transforms" );
    cJSON* jComponentArray = cJSON_GetObjectItem( jRoot, "Components" );

#if MYFW_EDITOR
    if( sceneID != SCENEID_TempPlayStop )
    {
        CreateNewScene( sceneName, sceneID );
    }
#else
    // Create the Box2D world.
    MyAssert( sceneID >= SCENEID_MainScene && sceneID < MAX_SCENES_LOADED_INCLUDING_UNMANAGED );
    MyAssert( m_pSceneInfoMap[sceneID].m_pBox2DWorld == nullptr );
    m_pSceneInfoMap[sceneID].m_pBox2DWorld = MyNew Box2DWorld( nullptr, nullptr, nullptr, new EngineBox2DContactListener );
#endif //MYFW_EDITOR

    // Request all files used by scene.
    if( jFileArray && sceneID != SCENEID_TempPlayStop )
    {
        for( int i=0; i<cJSON_GetArraySize( jFileArray ); i++ )
        {
            cJSON* jFile = cJSON_GetArrayItem( jFileArray, i );

            if( jFile->valuestring != nullptr )
            {
                LoadDataFile( jFile->valuestring, sceneID, nullptr, true );
            }
            else
            {
                cJSON* jPath = cJSON_GetObjectItem( jFile, "Path" );
                cJSON* jSourcePath = cJSON_GetObjectItem( jFile, "SourcePath" );
                if( jPath )
                {
                    SceneID sceneToSearch = sceneID;
                    if( sceneID == SCENEID_TempPlayStop )
                        sceneToSearch = SCENEID_AllScenes;

                    // Pass the source path in the LoadDataFile call.
                    // If the file is missing or the source file is newer, we can re-import.
                    if( jSourcePath == nullptr )
                        LoadDataFile( jPath->valuestring, sceneToSearch, nullptr, false );
                    else
                        LoadDataFile( jPath->valuestring, sceneToSearch, jSourcePath->valuestring, true );

                    // Find the file object and set it's source path.
                    MyFileInfo* pFileInfo = GetFileInfoIfUsedByScene( jPath->valuestring, sceneToSearch );
                    if( pFileInfo )
                    {
                        if( jSourcePath )
                        {
                            char path[MAX_PATH];
                            strcpy_s( path, MAX_PATH, jSourcePath->valuestring );
                            pFileInfo->SetSourceFileFullPath( path );
                        }
                    }
                }
            }
        }
    }

    bool getSceneIDFromEachObject = (sceneID == SCENEID_TempPlayStop);

    // Create/init all the game objects.
    if( jGameobjectArray )
    {
        for( int i=0; i<cJSON_GetArraySize( jGameobjectArray ); i++ )
        {
            cJSON* jGameObject = cJSON_GetArrayItem( jGameobjectArray, i );

            if( getSceneIDFromEachObject )
                cJSONExt_GetUnsignedInt( jGameObject, "SceneID", (unsigned int*)&sceneID );

            unsigned int id = -1;
            cJSONExt_GetUnsignedInt( jGameObject, "ID", &id );
            MyAssert( id != -1 );

            // LEGACY: Support for old scene files with folders in them, now stored as "SubType".
            bool isFolder = false;
            cJSONExt_GetBool( jGameObject, "IsFolder", &isFolder );

            bool hasTransform = true;
            cJSON* jSubtype = cJSON_GetObjectItem( jGameObject, "SubType" );
            if( jSubtype )
            {
                if( strcmp( jSubtype->valuestring, "Folder" ) == 0 )
                {
                    isFolder = true;
                    hasTransform = false;
                }
                else if( strcmp( jSubtype->valuestring, "Logic" ) == 0 )
                {
                    hasTransform = false;
                }
            }

            // Find an existing game object with the same id or create a new one.
            GameObject* pGameObject = FindGameObjectByID( sceneID, id );
            if( pGameObject )
            {
                MyAssert( pGameObject->GetSceneID() == sceneID );
            }

            if( pGameObject == nullptr )
            {
                pGameObject = CreateGameObject( true, sceneID, isFolder, hasTransform );
            }

            pGameObject->ImportFromJSONObject( jGameObject, sceneID );
            MyAssert( pGameObject->m_pEngineCore == GetEngineCore() );
        
            unsigned int gameObjectID = pGameObject->GetID();
            if( gameObjectID > m_pSceneInfoMap[sceneID].m_NextGameObjectID )
                m_pSceneInfoMap[sceneID].m_NextGameObjectID = gameObjectID + 1;
        }
    }

    // Setup all the game object transforms.
    if( jTransformArray )
    {
        for( int i=0; i<cJSON_GetArraySize( jTransformArray ); i++ )
        {
            cJSON* jTransform = cJSON_GetArrayItem( jTransformArray, i );
        
            if( getSceneIDFromEachObject )
            {
                cJSONExt_GetUnsignedInt( jTransform, "SceneID", (unsigned int*)&sceneID );
            }

            unsigned int gameObjectID = 0;
            cJSONExt_GetUnsignedInt( jTransform, "GOID", &gameObjectID );
            MyAssert( gameObjectID > 0 );

            GameObject* pGameObject = FindGameObjectByID( sceneID, gameObjectID );
            MyAssert( pGameObject );

            if( pGameObject )
            {
                pGameObject->SetID( gameObjectID );

                if( pGameObject->GetTransform() )
                {
                    pGameObject->GetTransform()->ImportFromJSONObject( jTransform, sceneID );
                }
                else
                {
                    unsigned int parentGOID = 0;
                    cJSONExt_GetUnsignedInt( jTransform, "ParentGOID", &parentGOID );
                
                    if( parentGOID > 0 )
                    {
                        GameObject* pParentGameObject = FindGameObjectByID( sceneID, parentGOID );
                        MyAssert( pParentGameObject );

                        pGameObject->SetParentGameObject( pParentGameObject );
                        if( pGameObject->GetTransform() )
                        {
                            pGameObject->GetTransform()->SetWorldTransformIsDirty();
                        }
                    }
                }

                if( gameObjectID >= m_pSceneInfoMap[sceneID].m_NextGameObjectID )
                {
                    m_pSceneInfoMap[sceneID].m_NextGameObjectID = gameObjectID + 1;
                }
            }
        }
    }

    // Load inheritance info (i.e. Parent GameObjects) for objects that inherit from others.
    if( jGameobjectArray )
    {
        for( int i=0; i<cJSON_GetArraySize( jGameobjectArray ); i++ )
        {
            cJSON* jGameObject = cJSON_GetArrayItem( jGameobjectArray, i );

            if( getSceneIDFromEachObject )
                cJSONExt_GetUnsignedInt( jGameObject, "SceneID", (unsigned int*)&sceneID );

            unsigned int id = -1;
            cJSONExt_GetUnsignedInt( jGameObject, "ID", &id );
            MyAssert( id != -1 );

            // Find the existing game object with the same id.
            GameObject* pGameObject = FindGameObjectByID( sceneID, id );
            MyAssert( pGameObject );

            pGameObject->ImportInheritanceInfoFromJSONObject( jGameObject );
        }
    }

    if( jComponentArray )
    {
        // Create all the components, not loading component properties.
        for( int i=0; i<cJSON_GetArraySize( jComponentArray ); i++ )
        {
            cJSON* jComponent = cJSON_GetArrayItem( jComponentArray, i );
        
            if( getSceneIDFromEachObject )
            {
                cJSONExt_GetUnsignedInt( jComponent, "SceneID", (unsigned int*)&sceneID );
            }

            unsigned int gameObjectID = 0;
            cJSONExt_GetUnsignedInt( jComponent, "GOID", &gameObjectID );
            MyAssert( gameObjectID > 0 );
            GameObject* pGameObject = FindGameObjectByID( sceneID, gameObjectID );
            MyAssert( pGameObject );

            CreateComponentFromJSONObject( pGameObject, jComponent );
        }

        // Load all the components properties after all components are created.
        for( int i=0; i<cJSON_GetArraySize( jComponentArray ); i++ )
        {
            cJSON* jComponent = cJSON_GetArrayItem( jComponentArray, i );
        
            if( getSceneIDFromEachObject )
            {
                cJSONExt_GetUnsignedInt( jComponent, "SceneID", (unsigned int*)&sceneID );
            }

            unsigned int componentID;
            cJSONExt_GetUnsignedInt( jComponent, "ID", &componentID );

            ComponentBase* pComponent = FindComponentByID( componentID, sceneID );
            MyAssert( pComponent );

            if( pComponent )
            {
                MyAssert( pComponent->GetComponentSystemManager() == this );
                if( pComponent->GetGameObject()->IsEnabled() == false )
                {
                    pComponent->OnGameObjectDisabled();
                }

                pComponent->ImportFromJSONObject( jComponent, sceneID );
            }
        }

        // Second pass on loading component properties for components that rely on other components being initialized.
        for( int i=0; i<cJSON_GetArraySize( jComponentArray ); i++ )
        {
            cJSON* jComponent = cJSON_GetArrayItem( jComponentArray, i );
        
            if( getSceneIDFromEachObject )
            {
                cJSONExt_GetUnsignedInt( jComponent, "SceneID", (unsigned int*)&sceneID );
            }

            unsigned int componentID;
            cJSONExt_GetUnsignedInt( jComponent, "ID", &componentID );

            ComponentBase* pComponent = FindComponentByID( componentID, sceneID );
            MyAssert( pComponent );

            if( pComponent )
            {
                pComponent->FinishImportingFromJSONObject( jComponent );
            }
        }
    }

    cJSON_Delete( jRoot );

    SyncAllRigidBodiesToObjectTransforms();
}

ComponentBase* ComponentSystemManager::CreateComponentFromJSONObject(GameObject* pGameObject, cJSON* jComponent)
{
    SceneID sceneID = pGameObject->GetSceneID();

    cJSON* jType = cJSON_GetObjectItem( jComponent, "Type" );
    MyAssert( jType );
    int type = -1;
    if( jType )
    {
        type = m_pComponentTypeManager->GetTypeByName( jType->valuestring );

        if( type == -1 )
        {
            LOGError( LOGTag, "Unknown component in scene file: %s\n", jType->valuestring );
            MyAssert( false );
        }
        else
        {
            ComponentBase* pComponent = nullptr;

            // If the JSON block has a component id, check if that component already exists.
            unsigned int id = 0;
            {
                cJSONExt_GetUnsignedInt( jComponent, "ID", &id );

                if( id != 0 )
                {
                    pComponent = FindComponentByID( id, sceneID );
                    if( pComponent )
                    {
                        MyAssert( pComponent->GetSceneID() == sceneID );

                        if( id >= m_pSceneInfoMap[sceneID].m_NextComponentID )
                            m_pSceneInfoMap[sceneID].m_NextComponentID = id + 1;

                        MyAssert( pGameObject->m_pEngineCore == GetEngineCore() );
                        MyAssert( pComponent->GetComponentSystemManager() == this );

                        return pComponent;
                    }
                }
            }

            // Create a new component of this type on the game object.
            MyAssert( g_pComponentSystemManager == this );
            pComponent = pGameObject->AddNewComponent( type, sceneID, g_pComponentSystemManager );

            // If this component had an id set in the scene file, then use it.
            if( id != 0 )
            {
                pComponent->SetID( id );
                if( id >= m_pSceneInfoMap[sceneID].m_NextComponentID )
                    m_pSceneInfoMap[sceneID].m_NextComponentID = id + 1;
            }

            return pComponent;
        }
    }

    return nullptr;
}

void ComponentSystemManager::FinishLoading(bool lockWhileLoading, SceneID sceneID, bool playWhenFinishedLoading)
{
    m_StartGamePlayWhenDoneLoading = playWhenFinishedLoading;
    m_WaitingForFilesToFinishLoading = lockWhileLoading;

    if( lockWhileLoading )
    {
        if( m_FilesStillLoading.GetHead() != nullptr )
            return;

        for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            MyAssert( pFileInfo && pFileInfo->GetFile() );
            if( pFileInfo->GetFile()->IsFinishedLoading() == false ) // still loading
                return;
        }

        m_WaitingForFilesToFinishLoading = false;
    }

    OnLoad( sceneID );

    if( playWhenFinishedLoading )
    {
#if MYFW_EDITOR
        g_pEngineCore->Editor_QuickSaveScene( "temp_editor_onplay.scene" );
#endif

        g_pEngineCore->RegisterGameplayButtons();
        OnPlay( sceneID );
        m_StartGamePlayWhenDoneLoading = false;
    }
}

void ComponentSystemManager::SyncAllRigidBodiesToObjectTransforms()
{
    for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode; pNode = pNode->GetNext() )
    {
#if MYFW_USING_BULLET
        if( ((ComponentBase*)pNode)->GetType() == ComponentType_3DCollisionObject )
        {
            Component3DCollisionObject* pComponent = (Component3DCollisionObject*)pNode;

            pComponent->SyncRigidBodyToTransform();
        }
#endif
    }
}

void ComponentSystemManager::UnloadScene(SceneID sceneIDToClear, bool clearUnmanagedComponents)
{
    // Remove all components, except ones attached to unmanaged game objects (if wanted).
    {
        for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
        {
            for( CPPListNode* pNode = m_Components[i].GetHead(); pNode;  )
            {
                ComponentBase* pComponent = (ComponentBase*)pNode;
                
                pNode = pNode->GetNext();

                SceneID sceneID = pComponent->GetSceneID();

                if( (pComponent->GetGameObject()->IsManaged() || clearUnmanagedComponents) &&
                    (sceneIDToClear == SCENEID_AllScenes || sceneID == sceneIDToClear) )
                {
                    DeleteComponent( pComponent );
                }
                else if( pComponent->GetID() > m_pSceneInfoMap[sceneID].m_NextComponentID )
                {
                    // Not sure how this could happen.
                    MyAssert( false );
                    m_pSceneInfoMap[sceneID].m_NextComponentID = pComponent->GetID() + 1;
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

        if( sceneIDToClear == SCENEID_AllScenes || (SceneID)i == sceneIDToClear )
        {
            GameObject* pNextGameObject = nullptr;
            for( GameObject* pGameObject = pSceneInfo->m_GameObjects.GetHead(); pGameObject; pGameObject = pNextGameObject )
            {
                pNextGameObject = pGameObject->GetNext();

                SceneID sceneID = pGameObject->GetSceneID();
                unsigned int gameObjectID = pGameObject->GetID();

                MyAssert( (SceneID)i == sceneID );

                if( (pGameObject->IsManaged() || clearUnmanagedComponents) )
                {
                    DeleteGameObject( pGameObject, true );
                }
                else if( sceneID == sceneIDToClear && gameObjectID > m_pSceneInfoMap[sceneID].m_NextGameObjectID )
                {
                    m_pSceneInfoMap[sceneID].m_NextGameObjectID = gameObjectID + 1;
                }
            }
        }
    }

    // If unloading all scenes, unload all prefab files.
    if( sceneIDToClear == SCENEID_AllScenes )
    {
        m_pPrefabManager->UnloadAllPrefabFiles();
    }

    // Release any file ref's added by this scene.
    FreeAllDataFiles( sceneIDToClear );

    // If clearing all scenes, 
    if( sceneIDToClear == SCENEID_AllScenes )
    {
        // Reset the scene counter, so the new "first" scene loaded will be 1.
        g_pComponentSystemManager->ResetSceneIDCounter();

        // Don't clear unmanaged objects.
        for( int sceneID=0; sceneID<MAX_SCENES_LOADED; sceneID++ )
        {
            if( sceneID == SCENEID_Unmanaged || sceneID == SCENEID_EngineObjects )
                continue;

            if( m_pSceneInfoMap[sceneID].m_InUse == false )
                continue;

            m_pSceneInfoMap[sceneID].Reset();
        }
    }
    else if( sceneIDToClear != SCENEID_Unmanaged ) // If clearing any scene other than the unmanaged scene.
    {
        MyAssert( m_pSceneInfoMap[sceneIDToClear].m_InUse == true );
        m_pSceneInfoMap[sceneIDToClear].Reset();
    }
}

bool ComponentSystemManager::IsSceneLoaded(const char* fullPath)
{
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse )
        {
            if( m_pSceneInfoMap[i].m_FullPath[0] != '\0' )
            {
                if( strcmp( m_pSceneInfoMap[i].m_FullPath, fullPath ) == 0 )
                    return true;

                const char* relativePath = GetRelativePath( m_pSceneInfoMap[i].m_FullPath );
                if( relativePath != nullptr && strcmp( relativePath, fullPath ) == 0 )
                    return true;
            }
        }
    }

    return false;
}

SceneID ComponentSystemManager::FindSceneID(const char* fullPath)
{
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse )
        {
            if( strcmp( m_pSceneInfoMap[i].m_FullPath, fullPath ) == 0 )
                return (SceneID)i;
        }
    }

    return SCENEID_NotFound;
}

// Exposed to Lua, change elsewhere if function signature changes.
GameObject* ComponentSystemManager::EditorLua_CreateGameObject(const char* name, uint32 sceneID, bool isFolder, bool hasTransform)
{
    GameObject* pGameObject = CreateGameObject( true, (SceneID)sceneID, isFolder, hasTransform, nullptr );

    pGameObject->SetName( name );

    return pGameObject;
}

GameObject* ComponentSystemManager::CreateGameObject(bool manageObject, SceneID sceneID, bool isFolder, bool hasTransform, PrefabReference* pPrefabRef)
{
    GameObject* pGameObject = MyNew GameObject( m_pEngineCore, manageObject, sceneID, isFolder, hasTransform, pPrefabRef );
    
    {
        unsigned int id = GetNextGameObjectIDAndIncrement( sceneID );
        pGameObject->SetID( id );

        //if( manageObject )
        {
            GetSceneInfo( sceneID )->m_GameObjects.AddTail( pGameObject );
        }

        // If we're not in editor mode, place this gameobject in unmanaged scene so it will be destroyed when gameplay is stopped.
        if( g_pEngineCore->IsInEditorMode() == false )
        {
            pGameObject->SetSceneID( SCENEID_Unmanaged );
        }
    }

    return pGameObject;
}

GameObject* ComponentSystemManager::CreateGameObjectFromPrefab(PrefabObject* pPrefab, bool manageObject, SceneID sceneID)
{
    return CreateGameObjectFromPrefab( pPrefab, pPrefab->GetJSONObject(), 0, manageObject, sceneID );
}

GameObject* ComponentSystemManager::CreateGameObjectFromPrefab(PrefabObject* pPrefab, cJSON* jPrefab, uint32 prefabChildID, bool manageObject, SceneID sceneID)
{
    MyAssert( pPrefab != nullptr );
    MyAssert( jPrefab != nullptr );
    GameObject* pGameObject = nullptr;

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

    // SceneID should only be SCENEID_Unmanaged if this is the master prefab gameobject created for the editor.
    bool creatingMasterGameObjectForPrefab = false;
    if( sceneID == SCENEID_Unmanaged )
        creatingMasterGameObjectForPrefab = true;

    if( creatingMasterGameObjectForPrefab )
    {
        MyAssert( manageObject == false );

        PrefabReference prefabRef( pPrefab, prefabChildID, false );
        prefabRef.SetAsMasterPrefabGameObject();
        pGameObject = CreateGameObject( manageObject, sceneID, isfolder, hastransform, &prefabRef );
    }
    else
    {
        PrefabReference prefabRef( pPrefab, prefabChildID, true );
        pGameObject = CreateGameObject( manageObject, sceneID, isfolder, hastransform, &prefabRef );
    }
    
    cJSON* jName = cJSON_GetObjectItem( jPrefab, "Name" );
    MyAssert( jName ); // If this trips, prefab file is likely old, every object should now have a name field.
    pGameObject->SetName( jName->valuestring );

    // If this subobject of the prefab contains a "PrefabID", then it's a nested prefab and needs to "inherit" from the other prefab.
#if MYFW_EDITOR
    uint32 prefabID = 0;
    cJSONExt_GetUnsignedInt( jPrefab, "PrefabID", &prefabID );
    if( prefabID != 0 )
    {
        GameObject* pOtherPrefabGameObject = nullptr;
        if( creatingMasterGameObjectForPrefab )
        {
            PrefabObject* pPrefabWeInheritFrom = pPrefab->GetPrefabFile()->GetPrefabByID( prefabID );
            uint32 otherPrefabChildID = 0;
            cJSONExt_GetUnsignedInt( jPrefab, "PrefabChildID", &otherPrefabChildID );
            pOtherPrefabGameObject = pPrefabWeInheritFrom->FindChildGameObject( otherPrefabChildID );
        }
        else
        {
            pOtherPrefabGameObject = pPrefab->FindChildGameObject( prefabChildID );
        }

        pGameObject->Editor_SetGameObjectThisInheritsFromIgnoringPrefabRef( pOtherPrefabGameObject );
    }
#endif //MYFW_EDITOR

    // Create matching components in new GameObject.
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
                    pComponent->ImportFromJSONObject( jComponent, sceneID );
                    pComponent->FinishImportingFromJSONObject( jComponent );
                    pComponent->OnLoad();
                }
            }
        }

        // Create children.
        cJSON* jChildrenArray = cJSON_GetObjectItem( jPrefab, "Children" );
        if( jChildrenArray )
        {
            int childarraysize = cJSON_GetArraySize( jChildrenArray );

            for( int i=0; i<childarraysize; i++ )
            {
                cJSON* jChildGameObject = cJSON_GetArrayItem( jChildrenArray, i );
                GameObject* pChildGameObject = nullptr;
            
                uint32 prefabchildid = 0;
                cJSONExt_GetUnsignedInt( jChildGameObject, "ChildID", &prefabchildid );

                // Create the child game object.
                if( sceneID == SCENEID_Unmanaged )
                {
                    // SceneID should only be SCENEID_Unmanaged if this is the temporary prefab gameobject created for the editor.
                    MyAssert( manageObject == false );

                    pChildGameObject = CreateGameObjectFromPrefab( pPrefab, jChildGameObject, prefabchildid, false, SCENEID_Unmanaged );
                    pChildGameObject->SetEnabled( false, false );
                }
                else
                {
                    pChildGameObject = CreateGameObjectFromPrefab( pPrefab, jChildGameObject, prefabchildid, true, sceneID );
                }
                MyAssert( pChildGameObject != nullptr );

                pChildGameObject->SetParentGameObject( pGameObject );
                cJSON* jTransform = cJSON_GetObjectItem( jChildGameObject, "LocalTransform" );
                pChildGameObject->m_pComponentTransform->ImportLocalTransformFromJSONObject( jTransform );
            }
        }
    }

    return pGameObject;
}

#if MYFW_EDITOR
GameObject* ComponentSystemManager::CreateGameObjectFromTemplate(unsigned int templateID, SceneID sceneID)
{
    MyAssert( templateID < m_pGameObjectTemplateManager->GetNumberOfTemplates() );

    GameObject* pGameObject = CreateGameObject( true, sceneID, false, true );
    
    const char* templatename = m_pGameObjectTemplateManager->GetTemplateName( templateID );
    pGameObject->SetName( templatename );

    cJSON* jTemplate = m_pGameObjectTemplateManager->GetTemplateJSONObject( templateID );

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
                pComponent->ImportFromJSONObject( jComponent, sceneID );
                pComponent->FinishImportingFromJSONObject( jComponent );
                pComponent->OnLoad();
            }
        }
    }

    // Don't delete jTemplate.

    return pGameObject;
}
#endif //MYFW_EDITOR

void ComponentSystemManager::UnmanageGameObject(GameObject* pObject, bool unmanageChildren)
{
    MyAssert( pObject && pObject->IsManaged() == true );

    // Remove all components from their respective component lists.
    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        ComponentBase* pComponent = pObject->GetComponentByIndex( i );

        // Remove from list and clear CPPListNode prev/next.
        pComponent->Remove();
        pComponent->Prev = nullptr;
        pComponent->Next = nullptr;
    }

    pObject->SetManaged( false );

    // Recurse through children.
    if( unmanageChildren )
    {
        GameObject* pChild = pObject->GetFirstChild();

        while( pChild )
        {
            UnmanageGameObject( pChild, true );
            pChild = pChild->GetNext();
        }
    }
}

void ComponentSystemManager::ManageGameObject(GameObject* pObject, bool manageChildren)
{
    MyAssert( pObject && pObject->IsManaged() == false );

    // Add all the gameobject's components back into the component lists.
    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        ComponentBase* pComponent = pObject->GetComponentByIndex( i );

        // GameObject::AddExistingComponent adds components to system manager list even if unmanaged.  So, double check it's not in list.
        if( pComponent->Prev == nullptr )
        {
            AddComponent( pComponent );
        }
    }

    pObject->SetManaged( true );

    // Recurse through children.
    if( manageChildren )
    {
        GameObject* pChild = pObject->GetFirstChild();

        while( pChild )
        {
            ManageGameObject( pChild, true );
            pChild = pChild->GetNext();
        }
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentSystemManager::DeleteGameObject(GameObject* pObject, bool deleteComponents)
{
    if( deleteComponents )
    {
        while( pObject->m_Components.Count() )
        {
            ComponentBase* pComponent = pObject->m_Components.RemoveIndex( 0 );

            pComponent->SetEnabled( false );
            delete pComponent;
        }
    }

    SAFE_DELETE( pObject );
}

#if MYFW_EDITOR
GameObject* ComponentSystemManager::EditorCopyGameObject(GameObject* pObject, bool newObjectInheritsFromOld)
{
    GameObject* newGameObject = nullptr;

    EditorCommand_CopyGameObject* pCommand = MyNew EditorCommand_CopyGameObject( pObject, newObjectInheritsFromOld );
    if( g_pEngineCore->IsInEditorMode() )
    {
        g_pEngineCore->GetCommandStack()->Do( pCommand );
        newGameObject = pCommand->GetCreatedObject();
    }
    else
    {
        // If we're not in editor mode, execute the command and delete it.
        pCommand->Do();
        newGameObject = pCommand->GetCreatedObject();
        delete pCommand;
    }

    return newGameObject;
}
#endif

// Exposed to Lua, change elsewhere if function signature changes.
GameObject* ComponentSystemManager::CopyGameObject(GameObject* pObject, const char* newName, bool disableNewObject)
{
    if( pObject == nullptr )
        return nullptr;

    // Place the new object in the unmanaged scene, unless we're in the editor.
    SceneID sceneID = SCENEID_Unmanaged;
    if( g_pEngineCore->IsInEditorMode() )
        sceneID = pObject->GetSceneID();

    GameObject* pNewObject = CreateGameObject( true, sceneID, pObject->IsFolder(),
                                               pObject->GetTransform() ? true : false, pObject->GetPrefabRef() );

    if( newName )
        pNewObject->SetName( newName );

    pNewObject->SetEnabled( disableNewObject ? false : pObject->IsEnabled(), false );
    pNewObject->SetPhysicsSceneID( pObject->GetPhysicsSceneID() );
    pNewObject->SetFlags( pObject->GetFlags() );

    // Copy the object inherited from if in editor mode.
    if( g_pEngineCore->IsInEditorMode() )
    {
        // If inheriting from a prefab, this was handled by the CreateGameObject call above.
        if( pObject->GetPrefabRef() == nullptr )
        {
            pNewObject->SetGameObjectThisInheritsFrom( pObject->GetGameObjectThisInheritsFrom() );
        }
    }

    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        ComponentBase* pComponent = nullptr;

        if( g_pEngineCore->IsInEditorMode() )
            pComponent = pNewObject->AddNewComponent( pObject->GetComponentByIndex( i )->GetType(), pNewObject->GetSceneID(), g_pComponentSystemManager );
        else
            pComponent = pNewObject->AddNewComponent( pObject->GetComponentByIndex( i )->GetType(), SCENEID_Unmanaged, g_pComponentSystemManager );

        if( disableNewObject )
        {
            pComponent->OnGameObjectDisabled();
        }

        pComponent->CopyFromSameType_Dangerous( pObject->GetComponentByIndex( i ) );

        pComponent->OnLoad();
    }

    // Call OnPlay for all components whether they are enabled or disabled.
    if( g_pEngineCore->IsInEditorMode() == false )
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

    // If the object we're copying was parented, set the parent.
    if( pObject->GetParentGameObject() != nullptr )
    {
        pNewObject->SetParentGameObject( pObject->GetParentGameObject() );
    }

    if( pObject->IsFolder() == false )
    {
        if( pObject->GetTransform() != nullptr )
        {
            *pNewObject->GetTransform() = *pObject->GetTransform();
        }
    }

    // Recursively copy children.
    GameObject* pChild = pObject->GetFirstChild();
    while( pChild )
    {
        GameObject* pNewChild = CopyGameObject( pChild, pChild->GetName(), disableNewObject );
        pNewChild->SetParentGameObject( pNewObject );

        pChild = pChild->GetNext();
    }

    return pNewObject;
}

unsigned int ComponentSystemManager::GetNextGameObjectIDAndIncrement(SceneID sceneID)
{
    SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( sceneID );
    
    MyAssert( pSceneInfo );
    if( pSceneInfo == nullptr )
        return 0; // We have problems if this happens.

    pSceneInfo->m_NextGameObjectID++;
    return pSceneInfo->m_NextGameObjectID-1;
}

unsigned int ComponentSystemManager::GetNextComponentIDAndIncrement(SceneID sceneID)
{
    SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( sceneID );
    
    MyAssert( pSceneInfo );
    if( pSceneInfo == nullptr )
        return 0; // We have problems if this happens.

    pSceneInfo->m_NextComponentID++;
    return pSceneInfo->m_NextComponentID-1;
}

GameObject* ComponentSystemManager::EditorLua_GetFirstGameObjectFromScene(uint32 sceneID)
{
    return GetFirstGameObjectFromScene( (SceneID)sceneID );
}

GameObject* ComponentSystemManager::GetFirstGameObjectFromScene(SceneID sceneID)
{
    SceneInfo* pSceneInfo = GetSceneInfo( sceneID );
    if( pSceneInfo == nullptr )
        return nullptr;

    GameObject* pGameObject = pSceneInfo->m_GameObjects.GetHead();
    if( pGameObject )
    {
        MyAssert( pGameObject->GetSceneID() == sceneID );
        return pGameObject;
    }

    return nullptr;
}

GameObject* ComponentSystemManager::FindGameObjectByID(SceneID sceneID, unsigned int goid)
{
    SceneInfo* pSceneInfo = GetSceneInfo( sceneID );
    if( pSceneInfo == nullptr )
        return nullptr;

    if( pSceneInfo->m_GameObjects.GetHead() )
        return FindGameObjectByIDFromList( pSceneInfo->m_GameObjects.GetHead(), goid );

    return nullptr;
}

GameObject* ComponentSystemManager::FindGameObjectByIDFromList(GameObject* list, unsigned int goid)
{
    MyAssert( list != nullptr );
    if( list == nullptr )
        return nullptr;

    for( GameObject* pGameObject = list; pGameObject != nullptr; pGameObject = pGameObject->GetNext() )
    {
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

    return nullptr;
}

// Exposed to Lua, change elsewhere if function signature changes.
GameObject* ComponentSystemManager::FindGameObjectByName(const char* name)
{
    for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse == false )
            continue;

        SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

        if( pSceneInfo->m_GameObjects.GetHead() )
        {
            GameObject* pGameObjectFound = FindGameObjectByNameFromList( pSceneInfo->m_GameObjects.GetHead(), name );
            if( pGameObjectFound )
                return pGameObjectFound;
        }
    }

    return nullptr;
}

GameObject* ComponentSystemManager::FindGameObjectByNameInScene(SceneID sceneID, const char* name)
{
    MyAssert( sceneID < MAX_SCENES_LOADED_INCLUDING_UNMANAGED );

    MyAssert( m_pSceneInfoMap[sceneID].m_InUse == true );

    SceneInfo* pSceneInfo = &m_pSceneInfoMap[sceneID];

    if( pSceneInfo->m_GameObjects.GetHead() )
    {
        GameObject* pGameObjectFound = FindGameObjectByNameFromList( pSceneInfo->m_GameObjects.GetHead(), name );
        if( pGameObjectFound )
            return pGameObjectFound;
    }

    return nullptr;
}

GameObject* ComponentSystemManager::FindGameObjectByNameFromList(GameObject* list, const char* name)
{
    MyAssert( list != nullptr );
    if( list == nullptr )
        return nullptr;

    for( GameObject* pGameObject = list; pGameObject != nullptr; pGameObject = pGameObject->GetNext() )
    {
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

    return nullptr;
}

GameObject* ComponentSystemManager::FindGameObjectByJSONRef(cJSON* jGameObjectRef, SceneID defaultSceneID, bool requireSceneBeLoaded)
{
    // see GameObject::ExportReferenceAsJSONObject

    cJSON* jScenePath = cJSON_GetObjectItem( jGameObjectRef, "Scene" );
    SceneID sceneID = defaultSceneID;
    if( jScenePath )
    {
        MyAssert( jScenePath->valuestring != nullptr && jScenePath->valuestring[0] != '\0' );

        sceneID = GetSceneIDFromFullpath( jScenePath->valuestring, requireSceneBeLoaded );
        if( sceneID == SCENEID_NotFound )
        {
            LOGError( LOGTag, "FindGameObjectByJSONRef: Scene not loaded: '%s'.\n", jScenePath->valuestring );
            return nullptr; // Scene isn't loaded, so object can't be found.
        }
        
        // TODO: Saving will throw all reference info away and piss people off :)
    }

    unsigned int goid = -1;
    cJSONExt_GetUnsignedInt( jGameObjectRef, "GOID", &goid );
    MyAssert( goid != -1 );

    return FindGameObjectByID( sceneID, goid );
}

GameObject* ComponentSystemManager::GetGameObjectsInRange(Vector3 pos, float range, unsigned int flags)
{
    // TODO: Return more than 1 object.
    //       Also, create a SceneGraph...

    SceneInfo* pScene = GetSceneInfo( SCENEID_MainScene );
    for( GameObject* pGameObject = pScene->m_GameObjects.GetHead(); pGameObject != nullptr; pGameObject = pGameObject->GetNext() )
    {
        if( pGameObject->GetPropertiesComponent()->GetFlags() & flags )
        {
            Vector3 worldPos = pGameObject->GetTransform()->GetWorldPosition();
            if( (worldPos - pos).LengthSquared() < range*range )
            {
                return pGameObject;
            }
        }
    }

    return nullptr;
}

#if MYFW_USING_LUA
luabridge::LuaRef ComponentSystemManager::Lua_GetGameObjectsInRange(Vector3 pos, float range, unsigned int flags)
{
    // Build a Lua table storing all GameObjects in range.
    luabridge::LuaRef gameObjectTable = luabridge::newTable( m_pEngineCore->GetLuaGameState()->m_pLuaState );

    SceneInfo* pScene = GetSceneInfo( SCENEID_MainScene );
    for( GameObject* pGameObject = pScene->m_GameObjects.GetHead(); pGameObject != nullptr; pGameObject = pGameObject->GetNext() )
    {
        if( pGameObject->GetPropertiesComponent()->GetFlags() & flags )
        {
            Vector3 worldPos = pGameObject->GetTransform()->GetWorldPosition();
            if( (worldPos - pos).LengthSquared() < range*range )
            {
                gameObjectTable.append( pGameObject );
            }
        }
    }

    return gameObjectTable;
}
#endif //MYFW_USING_LUA

ComponentBase* ComponentSystemManager::FindComponentByJSONRef(cJSON* jComponentRef, SceneID defaultSceneID)
{
    GameObject* pGameObject = FindGameObjectByJSONRef( jComponentRef, defaultSceneID, true );
    if( pGameObject == nullptr )
    {
        LOGError( LOGTag, "A referenced component wasn't found.\n" );
    }
    else //if( pGameObject )
    {
        unsigned int componentid = -1;
        cJSONExt_GetUnsignedInt( jComponentRef, "ComponentID", &componentid );
        MyAssert( componentid != -1 );

        return pGameObject->FindComponentByID( componentid );
    }

    return nullptr;
}

ComponentCamera* ComponentSystemManager::GetFirstCamera(bool preferEditorCam)
{
#if MYFW_EDITOR
    if( preferEditorCam && g_pEngineCore->IsInEditorMode() )
    {
        return g_pEngineCore->GetEditorState()->GetEditorCamera();
    }
    else
#endif
    {
        for( CPPListNode* pNode = m_Components[BaseComponentType_Camera].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
        {
            ComponentCamera* pCamera = (ComponentCamera*)pNode;

            // Skip unmanaged cameras (editor cam).
            if( pCamera->GetGameObject()->IsManaged() == true )
            {
                MyAssert( pCamera->GetType() == ComponentType_Camera );

                return pCamera;
            }
        }
    }

    return nullptr;
}

ComponentBase* ComponentSystemManager::GetFirstComponentOfType(const char* type)
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
        {
            if( ((ComponentBase*)pNode)->IsA( type ) )
                return (ComponentBase*)pNode;
        }
    }

    return nullptr; // Component not found.
}

ComponentBase* ComponentSystemManager::GetNextComponentOfType(ComponentBase* pLastComponent)
{
    MyAssert( pLastComponent != nullptr );

    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        bool foundlast = false;
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
        {
            if( pLastComponent == pNode )
                foundlast = true;
            else if( foundlast && ((ComponentBase*)pNode)->IsA( pLastComponent->GetClassname() ) )
                return (ComponentBase*)pNode;
        }
    }

    return nullptr; // Component not found.
}

ComponentBase* ComponentSystemManager::AddComponent(ComponentBase* pComponent)
{
    MyAssert( pComponent->GetBaseType() >= 0 && pComponent->GetBaseType() < BaseComponentType_NumTypes ); // Shouldn't happen.
    MyAssert( pComponent->Prev == nullptr && pComponent->Next == nullptr );
    MyAssert( pComponent->GetComponentSystemManager() == this );

    m_Components[pComponent->GetBaseType()].AddTail( pComponent );

    return pComponent;
}

void ComponentSystemManager::DeleteComponent(ComponentBase* pComponent)
{
    if( pComponent->GetGameObject() )
    {
        pComponent->GetGameObject()->RemoveComponent( pComponent );
    }

    pComponent->SetEnabled( false );
    SAFE_DELETE( pComponent );
}

ComponentBase* ComponentSystemManager::FindComponentByID(unsigned int id, SceneID sceneID)
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode;  )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            pNode = pNode->GetNext();
            if( pComponent->GetID() == id && pComponent->GetSceneID() == sceneID )
                return pComponent;
        }
    }

    return nullptr;
}

void ComponentSystemManager::Tick(float deltaTime)
{
    if( m_WaitingForFilesToFinishLoading )
    {
        // TODO: This is hardcoded to the first scene.
        FinishLoading( true, SCENEID_MainScene, m_StartGamePlayWhenDoneLoading );
    }

    // Tick the files that are still loading, if handled by us (spritesheets only ATM).
    CPPListNode* pNextNode;
    for( CPPListNode* pNode = m_FilesStillLoading.GetHead(); pNode; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        MyFileInfo* pFileInfo = (MyFileInfo*)pNode;
        
        MyAssert( pFileInfo );
        MyAssert( pFileInfo->GetSpriteSheet() );

        pFileInfo->GetSpriteSheet()->Tick( deltaTime );

        if( pFileInfo->GetSpriteSheet()->IsFullyLoaded() )
            m_Files.MoveTail( pNode );
    }

#if MYFW_EDITOR
    CheckForUpdatedDataSourceFiles( true );
#endif

    //deltaTime *= m_TimeScale;

    // Update all Components:

    // All scripts.
    //for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    //{
    //    ComponentUpdateable* pComponent = (ComponentUpdateable*)pNode;

    //    if( pComponent->m_BaseType == BaseComponentType_Updateable && pComponent->m_Type == ComponentType_LuaScript )
    //    {
    //        pComponent->Tick( deltaTime );
    //    }
    //}

    //// Don't tick objects if time is 0, useful for debugging, shouldn't be done otherwise
    //if( deltaTime == 0 )
    //    return;

    // Then all other "Updateables".
    for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        ComponentUpdateable* pComponent = (ComponentUpdateable*)pNode;

        if( pComponent->GetBaseType() == BaseComponentType_Updateable ) //&& pComponent->m_Type != ComponentType_LuaScript )
        {
            pComponent->Tick( deltaTime );
        }
    }

    // Update all components that registered a tick callback... might unregister themselves while in their callback
    for( CPPListNode* pNode = m_pComponentCallbackList_Tick.GetHead(); pNode != nullptr; pNode = pNextNode )
    {
        pNextNode = pNode->GetNext();

        ComponentCallbackStruct_Tick* pCallbackStruct = (ComponentCallbackStruct_Tick*)pNode;
        MyAssert( pCallbackStruct->pFunc != nullptr );

        (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( deltaTime );
    }

    // Update all cameras after game objects are updated.
    for( CPPListNode* pNode = m_Components[BaseComponentType_Camera].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        ComponentCamera* pComponent = (ComponentCamera*)pNode;

        if( pComponent->GetBaseType() == BaseComponentType_Camera )
        {
            pComponent->Tick( deltaTime );
        }
    }
}

void ComponentSystemManager::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height, unsigned int desiredAspectWidth, unsigned int desiredaspectHeight)
{
    for( CPPListNode* pNode = m_Components[BaseComponentType_Camera].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        ComponentCamera* pCamera = (ComponentCamera*)pNode;

        if( pCamera->GetBaseType() == BaseComponentType_Camera )
        {
            // TODO: Fix this hack, don't resize unmanaged cams (a.k.a. editor camera).
            if( pCamera->GetGameObject()->IsManaged() == true )
            {
                pCamera->OnSurfaceChanged( x, y, width, height, desiredAspectWidth, desiredaspectHeight );
            }
        }
    }

    //// For now, tell menu pages that the aspect ratio changed.
    //for( CPPListNode* pNode = m_Components[BaseComponentType_MenuPage].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    //{
    //    ComponentMenuPage* pComponent = (ComponentMenuPage*)pNode;

    //    pComponent->OnSurfaceChanged( x, y, width, height, desiredAspectWidth, desiredaspectHeight );
    //}

    // Notify all components that registered a callback of a change to the surfaces/aspect ratio.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnSurfaceChanged.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnSurfaceChanged* pCallbackStruct = (ComponentCallbackStruct_OnSurfaceChanged*)pNode;

        (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( x, y, width, height, desiredAspectWidth, desiredaspectHeight );
    }
}

void ComponentSystemManager::OnDrawFrame()
{
    for( CPPListNode* pNode = m_Components[BaseComponentType_Camera].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        ComponentCamera* pCamera = (ComponentCamera*)pNode;

        if( pCamera->GetBaseType() == BaseComponentType_Camera && pCamera->IsEnabled() == true )
        {
            pCamera->OnDrawFrame();
        }
    }
}

void ComponentSystemManager::DrawFrame(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride, bool drawOpaques, bool drawTransparents, EmissiveDrawOptions emissiveDrawOption, bool drawOverlays)
{
    // Draw all objects in the scene graph.
    {
        Vector3 campos = pCamera->m_pComponentTransform->GetLocalPosition();
        Vector3 camrot = pCamera->m_pComponentTransform->GetLocalRotation();

        // Find nearest shadow casting light. TODO: handle this better.
        MyMatrix* pShadowVP = nullptr;
        TextureDefinition* pShadowTex = nullptr;
        if( g_ActiveShaderPass == ShaderPass_Main )
        {
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
            if( pObject )
            {
                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
                if( pComponent )
                {
                    ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : nullptr;
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

        //RenderGraphFlags baseFlags = (RenderGraphFlags)0;
        //if( drawEmissives )
        //    baseFlags = (RenderGraphFlags)(baseFlags | RenderGraphFlag_Emissive);

        if( drawOpaques )
        {
            //RenderGraphFlags flags = (RenderGraphFlags)(baseFlags | RenderGraphFlag_Opaque);
            //baseFlags = (RenderGraphFlags)(baseFlags | ~RenderGraphFlag_Emissive);
            m_pRenderGraph->Draw( true, emissiveDrawOption, pCamera->m_LayersToRender, &campos, &camrot, pMatProj, pMatView, pShadowVP, pShadowTex, pShaderOverride, nullptr );
        }

        if( drawTransparents )
        {
            //RenderGraphFlags flags = (RenderGraphFlags)(baseFlags | RenderGraphFlag_Transparent);
            m_pRenderGraph->Draw( false, emissiveDrawOption, pCamera->m_LayersToRender, &campos, &camrot, pMatProj, pMatView, pShadowVP, pShadowTex, pShaderOverride, nullptr );
        }
    }
    
    if( drawOverlays )
    {
        DrawOverlays( pCamera, pMatProj, pMatView, pShaderOverride );
    }
}

void ComponentSystemManager::DrawOverlays(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    // Draw all components that registered a callback, used mostly for debug info (camera/light icons, collision info).
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
                    (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( pCamera, pMatProj, pMatView, pShaderOverride );
                }
            }
        }
    }
}

void ComponentSystemManager::OnFileRenamed(const char* fullPathBefore, const char* fullPathAfter)
{
    // Call all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnFileRenamed.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnFileRenamed* pCallbackStruct = (ComponentCallbackStruct_OnFileRenamed*)pNode;

        (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( fullPathBefore, fullPathAfter );
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
    if( pShaderOverride == nullptr )
        return;

    if( pComponent == nullptr )
        return;

    ColorByte tint( 0, 0, 0, 0 );

    SceneID sceneID = pComponent->GetGameObject()->GetSceneID();
    unsigned int id = pComponent->GetGameObject()->GetID();
                    
    id = UINT_MAX - (sceneID * 100000 + id) * 641; // 1, 641, 6700417, 4294967297, 

    if( 1 )                 tint.r = id%256;
    if( id > 256 )          tint.g = (id>>8)%256;
    if( id > 256*256 )      tint.b = (id>>16)%256;
    if( id > 256*256*256 )  tint.a = (id>>24)%256;

    Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    MyAssert( MyGLDebug_IsProgramActive( pShader->m_ProgramHandle ) == true );
    pShader->ProgramTint( tint );
}

void ProgramSceneIDs(RenderGraphObject* pObject, ShaderGroup* pShaderOverride)
{
    ComponentBase* pComponent = (ComponentBase*)(pObject->m_pUserData);

    ProgramSceneIDs( pComponent, pShaderOverride );
}

void ComponentSystemManager::DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    // Always use 4 bone version.
    // TODO: This might fail with 1-3 bones,
    //       but should work with 0 bones since bone attribs are set to 100% weight on bone 0
    //       and bone 0 transform uniform is set to identity.
    Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
    if( pShader->Activate() )
    {
        // Draw all objects in the scene graph.
        {
            Vector3 campos = pCamera->m_pComponentTransform->GetLocalPosition();
            Vector3 camrot = pCamera->m_pComponentTransform->GetLocalRotation();

            m_pRenderGraph->Draw( true, EmissiveDrawOption_EitherEmissiveOrNot, pCamera->m_LayersToRender, &campos, &camrot, pMatProj, pMatView, nullptr, nullptr, pShaderOverride, ProgramSceneIDs );
            m_pRenderGraph->Draw( false, EmissiveDrawOption_EitherEmissiveOrNot, pCamera->m_LayersToRender, &campos, &camrot, pMatProj, pMatView, nullptr, nullptr, pShaderOverride, ProgramSceneIDs );
        }

        // Draw all components that registered a callback.
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

                    SceneID sceneID = pComponent->GetGameObject()->GetSceneID();
                    unsigned int id = pComponent->GetGameObject()->GetID();
                    
                    id = UINT_MAX - (sceneID * 100000 + id) * 641; // 1, 641, 6700417, 4294967297, 

                    if( 1 )                 tint.r = id%256;
                    if( id > 256 )          tint.g = (id>>8)%256;
                    if( id > 256*256 )      tint.b = (id>>16)%256;
                    if( id > 256*256*256 )  tint.a = (id>>24)%256;

                    pShader->ProgramTint( tint );

                    (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( pCamera, pMatProj, pMatView, pShaderOverride );
                }
            }
        }

        pShader->DeactivateShader( nullptr, false );
    }
}

SceneID ComponentSystemManager::GetNextSceneID()
{
    // TODO: Search through list for an unused scene and return that ID.
    SceneID nextid = m_NextSceneID;

    m_NextSceneID = (SceneID)(m_NextSceneID+1);

    MyAssert( nextid >= SCENEID_MainScene && nextid < MAX_SCENES_LOADED );
    
    return nextid;
}

void ComponentSystemManager::ResetSceneIDCounter()
{
    m_NextSceneID = SCENEID_MainScene;
}

SceneInfo* ComponentSystemManager::GetSceneInfo(SceneID sceneID)
{
    MyAssert( sceneID >= 0 && sceneID < MAX_SCENES_CREATED );
    return &m_pSceneInfoMap[sceneID];
}

// Returns SCENEID_NotFound if scene isn't found.
SceneID ComponentSystemManager::GetSceneIDFromFullpath(const char* fullPath, bool requireSceneBeLoaded)
{
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        SceneID sceneID = (SceneID)i;
        SceneInfo* pSceneInfo = &m_pSceneInfoMap[i];

        if( strcmp( pSceneInfo->m_FullPath, fullPath ) == 0 )
            return sceneID;

        const char* relativePath = GetRelativePath( pSceneInfo->m_FullPath );
        if( relativePath != nullptr && strcmp( relativePath, fullPath ) == 0 )
            return sceneID;
    }

    // Assert if 'fullPath' isn't found and we required it.
    // Some GameObjects and Prefabs may contain references to objects in other files which may not be loaded.
    // That code will explicitly pass in false for 'requireSceneBeLoaded'.
    if( requireSceneBeLoaded )
    {
        MyAssert( false );
    }

    return SCENEID_NotFound;
}

#if MYFW_EDITOR
void ComponentSystemManager::CreateNewScene(const char* sceneName, SceneID sceneID)
{
    MyAssert( sceneID >= SCENEID_MainScene && sceneID < MAX_SCENES_LOADED );
    m_pSceneInfoMap[sceneID].m_InUse = true;

    // Create the box2d world, pass in a material for the debug renderer.
    ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
#if MYFW_USING_BOX2D
    m_pSceneInfoMap[sceneID].m_pBox2DWorld = MyNew Box2DWorld( g_pEngineCore->GetMaterial_Box2DDebugDraw(), &pCamera->m_Camera3D.m_matProj, &pCamera->m_Camera3D.m_matView, new EngineBox2DContactListener );
#else
    m_pSceneInfoMap[sceneID].m_pBox2DWorld = nullptr;
#endif
}

unsigned int ComponentSystemManager::GetNumberOfScenesLoaded()
{
    unsigned int numLoaded = 0;
    for( int i=0; i<MAX_SCENES_LOADED; i++ )
    {
        if( m_pSceneInfoMap[i].m_InUse )
            numLoaded++;
    }

    return numLoaded;
}

void EditorInternal_GetListOfGameObjectsThatUsePrefab(std::vector<GameObject*>* pGameObjectList, CPPListNode* pListToSearch, PrefabObject* pPrefabToFind)
{
    for( GameObject* pGameObject = (GameObject*)pListToSearch; pGameObject; pGameObject = pGameObject->GetNext() )
    {
        if( pGameObject->GetPrefabRef()->GetPrefab() == pPrefabToFind )
        {
            pGameObjectList->push_back( pGameObject );
        }

        // Search children of GameObject.
        TCPPListHead<GameObject*>* pChildList = pGameObject->GetChildList();
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

// Returns number of references.
int ComponentSystemManager::LogAllReferencesForFile(MyFileObject* pFile)
{
    LOGInfo( LOGTag, "Finding references to %s in all scenes:\n", pFile->GetFullPath() );

    int numRefs = 0;

    // Check all gameobjects/components in all scenes for a reference to the file.
    //   ATM: This only checks variables registered as "component vars".
    for( unsigned int sceneIndex=0; sceneIndex<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; sceneIndex++ )
    {
        if( m_pSceneInfoMap[sceneIndex].m_InUse == false )
            continue;

        for( GameObject* pGameObject = m_pSceneInfoMap[sceneIndex].m_GameObjects.GetHead(); pGameObject; pGameObject = pGameObject->GetNext() )
        {
            numRefs += LogAllReferencesForFileInGameObject( pFile, pGameObject );
        }
    }

    // Check materials for references to files.
    MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
    MaterialDefinition* pMaterial = pMaterialManager->GetFirstMaterial();
    while( pMaterial != nullptr )
    {
        if( pMaterial->IsReferencingFile( pFile ) )
        {
            if( pMaterial->GetFile() )
                LOGInfo( LOGTag, "    (Material) %s (0x%x)\n", pMaterial->GetFile()->GetFullPath(), pMaterial );
            else
                LOGInfo( LOGTag, "    (Unsaved Material) %s (0x%x)\n", pMaterial->GetName(), pMaterial );

            numRefs++;
        }

        pMaterial = (MaterialDefinition*)pMaterial->GetNext();
    }

    LOGInfo( LOGTag, "Done: %d references found\n", numRefs );

    return numRefs;
}

// Returns number of references.
int ComponentSystemManager::LogAllReferencesForFileInGameObject(MyFileObject* pFile, GameObject* pGameObject)
{
    int numRefs = 0;

    for( unsigned int componentindex=0; componentindex<pGameObject->GetComponentCountIncludingCore(); componentindex++ )
    {
        ComponentBase* pComponent = pGameObject->GetComponentByIndexIncludingCore( componentindex );

        if( pComponent )
        {
            if( pComponent->IsReferencingFile( pFile ) )
            {
                SceneID sceneIndex = pComponent->GetSceneID();

                if( sceneIndex == SCENEID_Unmanaged )
                {
                    if( pGameObject->IsPrefabInstance() )
                    {
                        LOGInfo( LOGTag, "    (Prefab) %s :: %s :: %s (0x%x)\n", pGameObject->GetPrefabRef()->GetPrefab()->GetPrefabFile()->GetFile()->GetFullPath(), pGameObject->GetName(), pComponent->GetClassname(), pComponent );
                    }
                }
                else
                {
                    LOGInfo( LOGTag, "    (GameObject) %s :: %s :: %s (0x%x)\n", m_pSceneInfoMap[sceneIndex].m_FullPath, pGameObject->GetName(), pComponent->GetClassname(), pComponent );
                }
                numRefs++;
            }
        }
    }

    // Recurse through children.
    GameObject* pChild = pGameObject->GetFirstChild();
    while( pChild )
    {
        numRefs += LogAllReferencesForFileInGameObject( pFile, pChild );

        pChild = pChild->GetNext();
    }

    return numRefs;
}

GameObject* ComponentSystemManager::ParseLog_GameObject(const char* line)
{
    // Example strings to parse.
    // "    (GameObject) SceneFileName :: GameObjectName :: ComponentName"
    // "    (Prefab) PrefabFileName :: GameObjectName :: ComponentName"

    int pos = 0;
    char sceneName[100] = "";
    char gameobjectName[100] = "";
    char componentName[100] = "";

    bool isGameObject = false;
    bool isPrefab = false;

    // First, make sure this is a GameObject or Prefab.
    {
        const char* pos2 = strstr( line, "(GameObject) " );
        const char* pos3 = strstr( line, "(Prefab) " );

        if( pos2 )
        {
            isGameObject = true;
            pos = (int)(pos2 - line) + (int)strlen("(GameObject) ");
        }
        else if( pos3 )
        {
            isPrefab = true;
            pos = (int)(pos3 - line) + (int)strlen("(Prefab) ");
        }
        else
        {
            return nullptr;
        }
    }

    // Grab the Scene/Prefab filename (can contain spaces).
    {
        const char* pos2 = strstr( &line[pos], " :: " );
        if( pos2 == 0 ) return nullptr;
        strncpy_s( sceneName, 100, &line[pos], pos2 - &line[pos] );
        pos += (int)(pos2 - &line[pos]) + (int)strlen(" :: ");
    }

    // Grab the GameObject name (can contain spaces).
    {
        const char* pos2 = strstr( &line[pos], " :: " );
        if( pos2 == 0 ) return nullptr;
        strncpy_s( gameobjectName, 100, &line[pos], pos2 - &line[pos] );
        pos += (int)(pos2 - &line[pos]) + (int)strlen(" :: ");
    }

    // Grab the Component name (won't contain spaces).
    {
        const char* pos2 = strstr( &line[pos], " " );
        if( pos2 == 0 ) return nullptr;
        strncpy_s( componentName, 100, &line[pos], pos2 - &line[pos] );
    }

    // Find the GameObject.
    if( isGameObject )
    {
        SceneID sceneID = g_pComponentSystemManager->FindSceneID( sceneName );

        if( sceneID != SCENEID_NotFound )
        {
            MyAssert( sceneID >= 0 && sceneID < MAX_SCENES_LOADED_INCLUDING_UNMANAGED );

            GameObject* pGameObject = g_pComponentSystemManager->FindGameObjectByNameInScene( sceneID, gameobjectName );
            return pGameObject;
        }
    }
    else if( isPrefab )
    {
        PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByFullPath( sceneName );

        if( pPrefabFile )
        {
            PrefabObject* pPrefab = pPrefabFile->GetFirstPrefabByName( gameobjectName );

            if( pPrefab )
            {
                return pPrefab->GetGameObject();
            }
        }
    }

    return nullptr;
}

MaterialDefinition* ComponentSystemManager::ParseLog_Material(const char* line)
{
    // Example strings to parse.
    // "    (GameObject) SceneFileName :: GameObjectName :: ComponentName"
    // "    (Prefab) PrefabFileName :: GameObjectName :: ComponentName"

    int pos = 0;
    char materialName[100] = "";

    // First, make sure this is a Material.
    {
        const char* pos2 = strstr( line, "(Material) " );
        if( pos2 == 0 ) return nullptr;
        pos = (int)(pos2 - line) + (int)strlen("(Material) ");
    }

    // Grab the Material filename (can contain spaces).
    {
        const char* pos2 = strstr( &line[pos], " (" );
        if( pos2 == 0 ) return nullptr;
        strncpy_s( materialName, 100, &line[pos], pos2 - &line[pos] );
    }

    // Find the GameObject.
    {
        MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
        MaterialDefinition* pMaterial = pMaterialManager->FindMaterialByFilename( materialName );

        return pMaterial;
    }

    return nullptr;
}
#endif //MYFW_EDITOR

//SceneInfo* ComponentSystemManager::GetSceneInfo(SceneID sceneID)
//{
//    MyAssert( m_pSceneInfoMap[sceneID].m_InUse == true );
//    return &m_pSceneInfoMap[sceneID];
//}

//void ComponentSystemManager::m_pGameObjectTemplateManager

void ComponentSystemManager::AddMeshToRenderGraph(ComponentBase* pComponent, MyMesh* pMesh, MaterialDefinition** pMaterialList, MyRE::PrimitiveTypes primitiveType, int pointSize, unsigned int layers, RenderGraphObject** pOutputList)
{
    MyAssert( pComponent != nullptr );
    MyAssert( pComponent->GetGameObject() != nullptr );
    MyAssert( pMesh != nullptr );
    MyAssert( pMaterialList != nullptr );
    MyAssert( pOutputList != nullptr );
    MyAssert( pMesh->GetSubmeshListCount() > 0 );

    MyMatrix* pMatWorld = pComponent->GetGameObject()->GetTransform()->GetWorldTransform();

    for( unsigned int i=0; i<pMesh->GetSubmeshListCount(); i++ )
    {
        MyAssert( pOutputList[i] == nullptr );
        
        pOutputList[i] = m_pRenderGraph->AddObject( pMatWorld, pMesh, pMesh->GetSubmesh( i ), pMaterialList[i], primitiveType, pointSize, layers, pComponent );
    }
}

RenderGraphObject* ComponentSystemManager::AddSubmeshToRenderGraph(ComponentBase* pComponent, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, unsigned int layers)
{
    MyAssert( pComponent != nullptr );
    MyAssert( pComponent->GetGameObject() != nullptr );
    MyAssert( pSubmesh != nullptr );

    MyMatrix* pMatWorld = pComponent->GetGameObject()->GetTransform()->GetWorldTransform();

    return m_pRenderGraph->AddObject( pMatWorld, nullptr, pSubmesh, pMaterial, primitiveType, pointSize, layers, pComponent );
}

void ComponentSystemManager::RemoveObjectFromRenderGraph(RenderGraphObject* pRenderGraphObject)
{
    m_pRenderGraph->RemoveObject( pRenderGraphObject );
}

void ComponentSystemManager::OnLoad(SceneID sceneID)
{
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;
            
            if( sceneID != SCENEID_AllScenes && pComponent->GetSceneID() != sceneID )
                continue;

            pComponent->OnLoad();
        }
    }
}

void ComponentSystemManager::OnPlay(SceneID sceneID)
{
    // TODO: Find a better solution than 2 passes, sort the OnPlay callback lists(once OnPlay is a callback of course...).

    // First pass, everything but 2d physics joints.
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;

            //MyAssert( pComponent->IsEnabled() == true );

            if( sceneID != SCENEID_AllScenes && pComponent->GetSceneID() != sceneID )
                continue;

            if( pComponent->IsEnabled() == true )
            {
                if( pComponent->IsA( "2DJoint-" ) == false )
                    pComponent->OnPlay();
            }
        }
    }

    // Second pass, only 2d physics joints.
    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;

            //MyAssert( pComponent->IsEnabled() == true );

            if( sceneID != SCENEID_AllScenes && pComponent->GetSceneID() != sceneID )
                continue;

            if( pComponent->IsEnabled() == true )
            {
                if( pComponent->IsA( "2DJoint-" ) == true )
                    pComponent->OnPlay();
            }
        }
    }
}

void ComponentSystemManager::OnStop(SceneID sceneID)
{
    SetTimeScale( 1 );

    for( unsigned int i=0; i<BaseComponentType_NumTypes; i++ )
    {
        for( CPPListNode* pNode = m_Components[i].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
        {
            ComponentBase* pComponent = (ComponentBase*)pNode;

            if( sceneID != SCENEID_AllScenes && pComponent->GetSceneID() != sceneID )
                continue;

            pComponent->OnStop();
        }
    }
}

bool ComponentSystemManager::OnEvent(MyEvent* pEvent)
{
    if( pEvent->IsType( "GameObjectEnable" ) )
    {
        GameObject* pGameObject = (GameObject*)pEvent->GetPointer( "GameObject" );
        bool enable = pEvent->GetBool( "Enable" );
        bool affectChildren = pEvent->GetBool( "AffectChildren" );

        pGameObject->SetEnabled( enable, affectChildren );
    }

    return false;
}

bool ComponentSystemManager::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    // Send touch events to all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnTouch.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnTouch* pCallbackStruct = (ComponentCallbackStruct_OnTouch*)pNode;

        if( (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( action, id, x, y, pressure, size ) )
            return true;
    }

    // Then regular scene input handlers.
    for( CPPListNode* pNode = m_Components[BaseComponentType_InputHandler].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)pNode;

        if( pComponent->GetBaseType() == BaseComponentType_InputHandler )
        {
            if( pComponent->OnTouch( action, id, x, y, pressure, size ) == true )
                return true;
        }
    }

    //// Then send input to all the scripts.
    //for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    //{
    //    ComponentLuaScript* pComponent = ((ComponentBase*)pNode)->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)pNode : nullptr;

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
    // Send button events to all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnButtons.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnButtons* pCallbackStruct = (ComponentCallbackStruct_OnButtons*)pNode;

        if( (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( action, id ) )
            return true;
    }

    // Then regular scene input handlers.
    for( CPPListNode* pNode = m_Components[BaseComponentType_InputHandler].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)pNode;

        if( pComponent->GetBaseType() == BaseComponentType_InputHandler )
        {
            if( pComponent->OnButtons( action, id ) == true )
                return true;
        }
    }

    //// Then send input to all the scripts.
    //for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    //{
    //    ComponentLuaScript* pComponent = ((ComponentBase*)pNode)->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)pNode : nullptr;

    //    if( pComponent )
    //    {
    //        if( pComponent->OnButtons( action, id ) == true )
    //            return true;
    //    }
    //}

    return false;
}

bool ComponentSystemManager::OnKeys(GameCoreButtonActions action, int keyCode, int unicodeChar)
{
    // Create an event and send it to all event handlers.
    EventManager* pEventManager = m_pEngineCore->GetManagers()->GetEventManager();
    MyEvent* pEvent = pEventManager->CreateNewEvent( "Keyboard" );
    pEvent->AttachInt( pEventManager, "Action", action );
    pEvent->AttachInt( pEventManager, "KeyCode", keyCode );
    pEventManager->SendEventNow( pEvent );

    // Send keyboard events to all components that registered a callback.
    for( CPPListNode* pNode = m_pComponentCallbackList_OnKeys.HeadNode.Next; pNode->Next; pNode = pNode->Next )
    {
        ComponentCallbackStruct_OnKeys* pCallbackStruct = (ComponentCallbackStruct_OnKeys*)pNode;

        if( (pCallbackStruct->pObj->*pCallbackStruct->pFunc)( action, keyCode, unicodeChar ) )
            return true;
    }

    // Then regular scene input handlers.
    for( CPPListNode* pNode = m_Components[BaseComponentType_InputHandler].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        ComponentInputHandler* pComponent = (ComponentInputHandler*)pNode;

        if( pComponent->GetBaseType() == BaseComponentType_InputHandler )
        {
            if( pComponent->OnKeys( action, keyCode, unicodeChar ) == true )
                return true;
        }
    }

    //// Then send input to all the scripts.
    //for( CPPListNode* pNode = m_Components[BaseComponentType_Updateable].GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    //{
    //    ComponentLuaScript* pComponent = ((ComponentBase*)pNode)->IsA( "LuaScriptComponent" ) ? (ComponentLuaScript*)pNode : nullptr;

    //    if( pComponent )
    //    {
    //        if( pComponent->OnKeys( action, keycode, unicodechar ) == true )
    //            return true;
    //    }
    //}

    return false;
}

#if MYFW_EDITOR
void ComponentSystemManager::DrawSingleObject(MyMatrix* pMatProj, MyMatrix* pMatView, GameObject* pObject, ShaderGroup* pShaderOverride)
{
    MyAssert( pObject != nullptr );

    for( unsigned int i=0; i<pObject->GetComponentCount(); i++ )
    {
        ComponentRenderable* pComponent = dynamic_cast<ComponentRenderable*>( pObject->GetComponentByIndex( i ) );

        if( pComponent )
        {
            pComponent->Draw( pMatProj, pMatView );

            ComponentCallbackStruct_Draw* pCallbackStruct = pComponent->GetDrawCallback();

            ComponentBase* pCallbackComponent = (ComponentBase*)pCallbackStruct->pObj;
            if( pCallbackComponent != nullptr && pCallbackStruct->pFunc != nullptr )
            {
                (pCallbackComponent->*pCallbackStruct->pFunc)( nullptr, pMatProj, pMatView, pShaderOverride );
            }
        }
    }
}

void ComponentSystemManager::DrawSingleComponent(MyMatrix* pMatProj, MyMatrix* pMatView, ComponentRenderable* pComponent, MaterialDefinition** ppMaterialOverrides, uint32 numMaterialOverrides)
{
    MyAssert( pComponent != nullptr );
    MyAssert( numMaterialOverrides <= 4 );

    if( pComponent )
    {
        pComponent->Draw( pMatProj, pMatView );

        ComponentCallbackStruct_Draw* pCallbackStruct = pComponent->GetDrawCallback();

        ComponentBase* pCallbackComponent = (ComponentBase*)pCallbackStruct->pObj;
        if( pCallbackStruct->pFunc != nullptr )
        {
            MyAssert( pCallbackComponent == pComponent );

            // Backup materials and set new ones.
            MaterialDefinition* pOldMaterials[4];
            if( numMaterialOverrides > 0 )
            {
                for( uint32 i=0; i<4; i++ )
                {
                    pOldMaterials[i] = pComponent->GetMaterial( i );
                    if( i >= numMaterialOverrides )
                    {
                        pComponent->SetMaterial( ppMaterialOverrides[0], i );
                    }
                    else
                    {
                        pComponent->SetMaterial( ppMaterialOverrides[i], i );
                    }
                }
            }

            (pComponent->*pCallbackStruct->pFunc)( nullptr, pMatProj, pMatView, nullptr );

            // Restore Materials.
            if( numMaterialOverrides > 0 )
            {
                for( uint32 i=0; i<4; i++ )
                {
                    pComponent->SetMaterial( pOldMaterials[i], i );
                }
            }
        }
    }
}

#if MYFW_EDITOR
void ComponentSystemManager::CheckForUpdatedDataSourceFiles(bool initialCheck)
{
    for( CPPListNode* pNode = m_Files.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

        if( pFileInfo->GetFile() == nullptr )
            continue;

#if MYFW_WINDOWS
        if( (initialCheck == false || pFileInfo->GetDidInitialCheckIfSourceFileWasUpdated() == false) && // Haven't done initial check.
            pFileInfo->GetFile()->GetFileLastWriteTime().dwHighDateTime != 0 && // Converted file has been loaded.
            pFileInfo->GetSourceFileFullPath()[0] != '\0' ) // We have a source file.
        {
            WIN32_FIND_DATAA data;
            memset( &data, 0, sizeof( data ) );

            HANDLE handle = FindFirstFileA( pFileInfo->GetSourceFileFullPath(), &data );
            if( handle != INVALID_HANDLE_VALUE )
                FindClose( handle );

            // If the source file is newer than the data file, reimport it.
            if( data.ftLastWriteTime.dwHighDateTime > pFileInfo->GetFile()->GetFileLastWriteTime().dwHighDateTime ||
                ( data.ftLastWriteTime.dwHighDateTime == pFileInfo->GetFile()->GetFileLastWriteTime().dwHighDateTime &&
                  data.ftLastWriteTime.dwLowDateTime > pFileInfo->GetFile()->GetFileLastWriteTime().dwLowDateTime ) )
            {
                ImportDataFile( pFileInfo->GetSceneID(), pFileInfo->GetSourceFileFullPath() );
            }

            pFileInfo->SetDidInitialCheckIfSourceFileWasUpdated();
        }
#else
        if( (initialCheck == false || pFileInfo->GetDidInitialCheckIfSourceFileWasUpdated() == false) && // Haven't done initial check.
            pFileInfo->m_pFile->GetFileLastWriteTime() != 0 && // Converted file has been loaded.
            pFileInfo->GetSourceFileFullPath()[0] != '\0' ) // We have a source file.
        {
            struct stat data;
            stat( pFileInfo->GetSourceFileFullPath(), &data );
            if( data.st_mtime == pFileInfo->m_pFile->GetFileLastWriteTime() )
            {
                ImportDataFile( pFileInfo->m_SceneID, pFileInfo->GetSourceFileFullPath() );
            }

            pFileInfo->SetDidInitialCheckIfSourceFileWasUpdated();
        }
#endif
    }
    
    // TODO: Check for updates to files that are still loading?
    //m_FilesStillLoading
}

void ComponentSystemManager::OnFileUpdated(MyFileObject* pFile)
{
    for( unsigned int i=0; i<m_pFileUpdatedCallbackList.size(); i++ )
    {
        m_pFileUpdatedCallbackList[i].pFunc( m_pFileUpdatedCallbackList[i].pObj, pFile );
    }
}

void ComponentSystemManager::Editor_RegisterFileUpdatedCallback(FileUpdatedCallbackFunction* pFunc, void* pObj)
{
    MyAssert( pFunc != nullptr );
    MyAssert( pObj != nullptr );

    FileUpdatedCallbackStruct callbackstruct;
    callbackstruct.pFunc = pFunc;
    callbackstruct.pObj = pObj;

    m_pFileUpdatedCallbackList.push_back( callbackstruct );
}
#endif //MYFW_EDITOR

void ComponentSystemManager::OnMaterialCreated(MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial );

    if( pMaterial )
    {
        // Add the material to the file list, so it can be freed on shutdown.
        AddToFileList( pMaterial->GetFile(), nullptr, nullptr, nullptr, pMaterial, nullptr, nullptr, nullptr, SCENEID_MainScene );
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
        AddToFileList( pSoundCue->GetFile(), nullptr, nullptr, nullptr, nullptr, pSoundCue, nullptr, nullptr, SCENEID_MainScene );
        pSoundCue->GetFile()->AddRef();
    }
}

void ComponentSystemManager::OnSoundCueUnloaded(SoundCue* pSoundCue) // StaticOnSoundCueUnloaded
{
    MyAssert( pSoundCue );

    // Loop through both lists of files and unload all files referencing this sound cue.
    for( int fileList=0; fileList<2; fileList++ )
    {
        CPPListNode* pFirstNode = 0;

        if( fileList == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        CPPListNode* pNextNode;
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNextNode )
        {
            pNextNode = pNode->GetNext();

            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            if( pFileInfo->GetSoundCue() == pSoundCue )
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
    for( int fileList=0; fileList<2; fileList++ )
    {
        CPPListNode* pFirstNode = nullptr;

        if( fileList == 0 )
            pFirstNode = m_Files.GetHead();
        else
            pFirstNode = m_FilesStillLoading.GetHead();
        
        CPPListNode* pNextNode;
        for( CPPListNode* pNode = pFirstNode; pNode; pNode = pNextNode )
        {
            pNextNode = pNode->GetNext();

            MyFileInfo* pFileInfo = (MyFileInfo*)pNode;

            // Unload the file.
            if( pFileInfo->GetFile() == pFile
                || (pFileInfo->GetMesh()          && pFileInfo->GetMesh()->GetFile()            == pFile)
                || (pFileInfo->GetShaderGroup()   && pFileInfo->GetShaderGroup()->GetFile()     == pFile)
                || (pFileInfo->GetTexture()       && pFileInfo->GetTexture()->GetFile()         == pFile)
                || (pFileInfo->GetMaterial()      && pFileInfo->GetMaterial()->GetFile()        == pFile)
                || (pFileInfo->GetSoundCue()      && pFileInfo->GetSoundCue()->GetFile()        == pFile)
                || (pFileInfo->GetSpriteSheet()   && pFileInfo->GetSpriteSheet()->GetJSONFile() == pFile)
                || (pFileInfo->GetPrefabFile()    && pFileInfo->GetPrefabFile()->GetFile()      == pFile)
              )
            {
                int objectRefCount = 0;
                if( pFileInfo->GetMaterial() ) objectRefCount = pFileInfo->GetMaterial()->GetRefCount();

                bool unloadFile = true;

                if( objectRefCount > 1 )
                {
                    LOGInfo( LOGTag, "File can't be unloaded while referenced: %s\n", pFile->GetFullPath() );

                    FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
                    int numRefs = pFileManager->Editor_FindAllReferences( pFile );

                    if( numRefs == 0 )
                    {
                        LOGInfo( LOGTag, "File is still referenced by an undo command. Can't unload." );
                    }
                    
                    //if( numRefs > 0 )
                    {
                        unloadFile = false;
                    }
                }
                
                if( unloadFile )
                {
                    LOGInfo( LOGTag, "File removed from scene file list: %s\n", pFile->GetFullPath() );

                    FreeDataFile( pFileInfo );
                }
            }
        }
    }
}

int ComponentSystemManager::OnFindAllReferences(MyFileObject* pFile) // StaticOnFindAllReferences
{
    return LogAllReferencesForFile( pFile );
}
#endif //MYFW_EDITOR
