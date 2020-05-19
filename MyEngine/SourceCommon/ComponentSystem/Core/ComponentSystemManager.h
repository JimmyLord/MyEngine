//
// Copyright (c) 2014-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentSystemManager_H__
#define __ComponentSystemManager_H__

#include "SceneHandler.h"
#include "ComponentSystem/BaseComponents/ComponentBase.h"

class ComponentRenderable;
class ComponentSystemManager;
class ComponentTypeManager;
class PrefabManager;
class PrefabFile;
class PrefabObject;
class PrefabReference;
class GameObjectTemplateManager;
class SceneHandler;
class GameObject;
class ComponentBase;
class ComponentCamera;
class ComponentLight;
class RenderGraph_Base;
class RenderGraphObject;
class MyFileInfo; // At bottom of this file.

enum ObjectListIconTypes
{
    ObjectListIcon_Scene,
    ObjectListIcon_GameObject,
    ObjectListIcon_Folder,
    ObjectListIcon_LogicObject,
    ObjectListIcon_Component,
    ObjectListIcon_Prefab,
};

extern ComponentSystemManager* g_pComponentSystemManager;

#if MYFW_EDITOR
typedef void FileUpdatedCallbackFunction(void* obj, MyFileObject* pFile);
struct FileUpdatedCallbackStruct
{
    void* pObj;
    FileUpdatedCallbackFunction* pFunc;
};
#endif //MYFW_EDITOR

// Define structures to hold callback funcs and objects.
#define MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT(CallbackType) \
    struct ComponentCallbackStruct_##CallbackType : public CPPListNode \
    { \
        ComponentBase* pObj; \
        ComponentCallbackFunc_##CallbackType pFunc; \
        ComponentCallbackStruct_##CallbackType() \
        { \
            pObj = nullptr; \
            pFunc = nullptr; \
        } \
    };

// Callback registration function macros.
#define MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS(CallbackType) \
    CPPListHead m_pComponentCallbackList_##CallbackType; \
    void RegisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct); \
    void UnregisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct);

// Register/Unregister component callback macros - used by components.
#define MYFW_FILL_COMPONENT_CALLBACK_STRUCT(ComponentClass, CallbackType) \
    m_CallbackStruct_##CallbackType.pObj = this; \
    m_CallbackStruct_##CallbackType.pFunc = (ComponentCallbackFunc_##CallbackType)&ComponentClass::CallbackType##Callback;

#define MYFW_REGISTER_COMPONENT_CALLBACK(ComponentClass, CallbackType) \
    m_CallbackStruct_##CallbackType.pObj = this; \
    m_CallbackStruct_##CallbackType.pFunc = (ComponentCallbackFunc_##CallbackType)&ComponentClass::CallbackType##Callback; \
    g_pComponentSystemManager->RegisterComponentCallback_##CallbackType( &m_CallbackStruct_##CallbackType );

#define MYFW_UNREGISTER_COMPONENT_CALLBACK(CallbackType) \
    g_pComponentSystemManager->UnregisterComponentCallback_##CallbackType( &m_CallbackStruct_##CallbackType );

#define MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED(CallbackType) \
    MyAssert( m_CallbackStruct_##CallbackType.Prev == nullptr );

// Declare callback objects/functions - used by components.
#define MYFW_DECLARE_COMPONENT_CALLBACK_TICK() \
    ComponentCallbackStruct_Tick m_CallbackStruct_Tick; \
    void TickCallback(float deltaTime);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED() \
    ComponentCallbackStruct_OnSurfaceChanged m_CallbackStruct_OnSurfaceChanged; \
    void OnSurfaceChangedCallback(uint32 x, uint32 y, uint32 width, uint32 height, unsigned int desiredAspectWidth, unsigned int desiredAspectHeight);

#define MYFW_DECLARE_COMPONENT_CALLBACK_DRAW() \
    ComponentCallbackStruct_Draw m_CallbackStruct_Draw; \
    void DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride); \
    virtual ComponentCallbackStruct_Draw* GetDrawCallback() { return &m_CallbackStruct_Draw; }

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH() \
    ComponentCallbackStruct_OnTouch m_CallbackStruct_OnTouch; \
    bool OnTouchCallback(int action, int id, float x, float y, float pressure, float size);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS() \
    ComponentCallbackStruct_OnButtons m_CallbackStruct_OnButtons; \
    bool OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS() \
    ComponentCallbackStruct_OnKeys m_CallbackStruct_OnKeys; \
    bool OnKeysCallback(GameCoreButtonActions action, int keyCode, int unicodeChar);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED() \
    ComponentCallbackStruct_OnFileRenamed m_CallbackStruct_OnFileRenamed; \
    void OnFileRenamedCallback(const char* fullPathBefore, const char* fullPathAfter);

// Callback function prototypes and structs.
typedef void (ComponentBase::*ComponentCallbackFunc_Tick)(float deltaTime);
typedef void (ComponentBase::*ComponentCallbackFunc_OnSurfaceChanged)(uint32 x, uint32 y, uint32 width, uint32 height, unsigned int desiredAspectWidth, unsigned int desiredaspectHeight);
typedef void (ComponentBase::*ComponentCallbackFunc_Draw)(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride);
typedef bool (ComponentBase::*ComponentCallbackFunc_OnTouch)(int action, int id, float x, float y, float pressure, float size);
typedef bool (ComponentBase::*ComponentCallbackFunc_OnButtons)(GameCoreButtonActions action, GameCoreButtonIDs id);
typedef bool (ComponentBase::*ComponentCallbackFunc_OnKeys)(GameCoreButtonActions action, int keyCode, int unicodeChar);
typedef void (ComponentBase::*ComponentCallbackFunc_OnFileRenamed)(const char* fullPathBefore, const char* fullPathAfter);

MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( Tick );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnSurfaceChanged );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( Draw );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnTouch );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnButtons );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnKeys );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnFileRenamed );

typedef void TransformChangedCallbackFunc(void* pObjectPtr, const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor);
struct TransformChangedCallbackStruct : public CPPListNode
{
    void* pObj;
    TransformChangedCallbackFunc* pFunc;
};

class ComponentSystemManager
{
protected:
    EngineCore* m_pEngineCore;

    ComponentTypeManager* m_pComponentTypeManager; // Memory managed, delete this.

    // List of files used including a scene id and the source file (if applicable).
    CPPListHead m_Files;
    CPPListHead m_FilesStillLoading;

    // A component can only exist in one of these lists ATM.
    CPPListHead m_Components[BaseComponentType_NumTypes];

    bool m_WaitingForFilesToFinishLoading;
    bool m_StartGamePlayWhenDoneLoading;

    float m_TimeScale;

    RenderGraph_Base* m_pRenderGraph;

    // Pool for transform changed callbacks.
    static const int CALLBACK_POOL_SIZE = 1000;
    MySimplePool<TransformChangedCallbackStruct> m_pComponentTransform_TransformChangedCallbackPool;

#if MYFW_EDITOR
    std::vector<FileUpdatedCallbackStruct> m_pFileUpdatedCallbackList;
#endif //MYFW_EDITOR

public:
    ComponentSystemManager(ComponentTypeManager* pTypeManager, EngineCore* pEngineCore);
    virtual ~ComponentSystemManager();

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    // Getters.
    EngineCore* GetEngineCore() { return m_pEngineCore; }
    float GetTimeScale() { return m_TimeScale; }
    RenderGraph_Base* GetRenderGraph() { return m_pRenderGraph; }
    ComponentTypeManager* GetComponentTypeManager() { return m_pComponentTypeManager; }
    MySimplePool<TransformChangedCallbackStruct>* GetTransformChangedCallbackPool() { return &m_pComponentTransform_TransformChangedCallbackPool; }

    // Setters.
    void SetTimeScale(float scale) { m_TimeScale = scale; } // Exposed to Lua, change elsewhere if function signature changes.

    void MoveAllFilesNeededForLoadingScreenToStartOfFileList(GameObject* first);
    void AddListOfFilesUsedToJSONObject(SceneID sceneID, cJSON* jFileArray);
    char* SaveSceneToJSON(SceneID sceneID);
#if MYFW_USING_BOX2D
    char* ExportBox2DSceneToJSON(SceneID sceneID);
#endif //MYFW_USING_BOX2D
    void SaveGameObjectListToJSONArray(cJSON* jGameObjectArray, cJSON* jTransformArray, GameObject* first, bool saveSceneID);
    
    MyFileInfo* GetFileInfoIfUsedByScene(MyFileObject* pFile, SceneID sceneID);
    MyFileInfo* GetFileInfoIfUsedByScene(const char* fullPath, SceneID sceneID);
    MyFileObject* GetFileObjectIfUsedByScene(const char* fullPath, SceneID sceneID);

    MyFileInfo* AddToFileList(MyFileObject* pFile, SceneID sceneID);
    MyFileInfo* AddToFileList(MyMesh* pMesh, SceneID sceneID);
    MyFileInfo* AddToFileList(ShaderGroup* pShaderGroup, SceneID sceneID);
    MyFileInfo* AddToFileList(TextureDefinition* pTexture, SceneID sceneID);
    MyFileInfo* AddToFileList(MaterialDefinition* pMaterial, SceneID sceneID);
    MyFileInfo* AddToFileList(SoundCue* pSoundCue, SceneID sceneID);
    MyFileInfo* AddToFileList(SpriteSheet* pSpriteSheet, SceneID sceneID);
    MyFileInfo* AddToFileList(My2DAnimInfo* p2DAnimInfo, SceneID sceneID);
    MyFileInfo* AddToFileList(MyFileObject* pFile, MyMesh* pMesh, ShaderGroup* pShaderGroup, TextureDefinition* pTexture, MaterialDefinition* pMaterial, SoundCue* pSoundCue, SpriteSheet* pSpriteSheet, My2DAnimInfo* p2DAnimInfo, SceneID sceneID);

    MyFileInfo* EditorLua_LoadDataFile(const char* relativePath, uint32 sceneID, const char* fullSourceFilePath, bool convertifrequired);
    MyFileInfo* LoadDataFile(const char* relativePath, SceneID sceneID, const char* fullSourceFilePath, bool convertIfRequired);
#if MYFW_EDITOR
    MyFileObject* ImportDataFile(SceneID sceneID, const char* fullSourceFilePath);
#endif
    void FreeDataFile(MyFileInfo* pFileInfo);
    void FreeAllDataFiles(SceneID sceneIDToClear);

    void LoadSceneFromJSON(const char* sceneName, const char* jsonString, SceneID sceneID);
    ComponentBase* CreateComponentFromJSONObject(GameObject* pGameObject, cJSON* jComponent);
    void FinishLoading(bool lockWhileLoading, SceneID sceneID, bool playWhenFinishedLoading);

    void SyncAllRigidBodiesToObjectTransforms();

    // Can clear everything except editor objects/components.
    // Unmanaged components are mainly editor objects and deleted objects in undo stack of editor... might want to rethink that.
    void UnloadScene(SceneID sceneIDToClear = SCENEID_AllScenes, bool clearUnmanagedComponents = true);
    bool IsSceneLoaded(const char* fullPath);
    SceneID FindSceneID(const char* fullPath);

    GameObject* EditorLua_CreateGameObject(const char* name, uint32 sceneID, bool isFolder, bool hasTransform);
    GameObject* CreateGameObject(bool manageObject = true, SceneID sceneID = SCENEID_Unmanaged, bool isFolder = false, bool hasTransform = true, PrefabReference* pPrefabRef = nullptr);
    GameObject* CreateGameObjectFromPrefab(PrefabObject* pPrefab, bool manageObject, SceneID sceneID);
    GameObject* CreateGameObjectFromPrefab(PrefabObject* pPrefab, cJSON* jPrefab, uint32 prefabChildID, bool manageObject, SceneID sceneID);
#if MYFW_EDITOR
    GameObject* CreateGameObjectFromTemplate(unsigned int templateID, SceneID sceneID);
#endif
    void UnmanageGameObject(GameObject* pObject, bool unmanageChildren);
    void ManageGameObject(GameObject* pObject, bool manageChildren);
    void DeleteGameObject(GameObject* pObject, bool deleteComponents);
#if MYFW_EDITOR
    GameObject* EditorCopyGameObject(GameObject* pObject, bool newObjectInheritsFromOld);
#endif
    GameObject* CopyGameObject(GameObject* pObject, const char* newName, bool disableNewObject);

    unsigned int GetNextGameObjectIDAndIncrement(SceneID sceneID);
    unsigned int GetNextComponentIDAndIncrement(SceneID sceneID);

    GameObject* EditorLua_GetFirstGameObjectFromScene(uint32 sceneID);
    GameObject* GetFirstGameObjectFromScene(SceneID sceneID);
    GameObject* FindGameObjectByID(SceneID sceneID, unsigned int goid);
    GameObject* FindGameObjectByIDFromList(GameObject* list, unsigned int goid);
    GameObject* FindGameObjectByName(const char* name);
    GameObject* FindGameObjectByNameInScene(SceneID sceneID, const char* name);
    GameObject* FindGameObjectByNameFromList(GameObject* list, const char* name);
    GameObject* FindGameObjectByJSONRef(cJSON* jGameObjectRef, SceneID defaultSceneID, bool requireSceneBeLoaded);
    GameObject* GetGameObjectsInRange(Vector3 pos, float range, unsigned int flags);
#if MYFW_USING_LUA
    luabridge::LuaRef Lua_GetGameObjectsInRange(Vector3 pos, float range, unsigned int flags);
#endif
    ComponentBase* FindComponentByJSONRef(cJSON* jComponentRef, SceneID defaultSceneID);
    ComponentCamera* GetFirstCamera(bool preferEditorCam = false);
    ComponentBase* GetFirstComponentOfType(const char* type);
    ComponentBase* GetNextComponentOfType(ComponentBase* pLastComponent);

    ComponentBase* AddComponent(ComponentBase* pComponent);
    void DeleteComponent(ComponentBase* pComponent);

    ComponentBase* FindComponentByID(unsigned int id, SceneID sceneID = SCENEID_AllScenes);

    // Main events, most should call component callbacks.
    void Tick(float deltaTime);
    void OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height, unsigned int desiredAspectWidth, unsigned int desiredaspectHeight);
    void OnDrawFrame();
    void DrawFrame(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride, bool drawOpaques, bool drawTransparents, EmissiveDrawOptions emissiveDrawOption, bool drawOverlays);
    void DrawOverlays(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride);
    void OnFileRenamed(const char* fullPathBefore, const char* fullPathAfter);

    void OnLoad(SceneID sceneID);
    void OnPlay(SceneID sceneID);
    void OnStop(SceneID sceneID);

    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((ComponentSystemManager*)pObjectPtr)->OnEvent( pEvent ); }
    bool OnEvent(MyEvent* pEvent);

    bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
    bool OnKeys(GameCoreButtonActions action, int keyCode, int unicodeChar);

    // Callback helpers.
    MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS( Tick );
    MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS( OnSurfaceChanged );
    MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS( Draw );
    MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS( OnTouch );
    MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS( OnButtons );
    MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS( OnKeys );
    MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS( OnFileRenamed );

    void MoveInputHandlersToFront(CPPListNode* pOnTouch, CPPListNode* pOnButtons, CPPListNode* pOnKeys);

    // Other utility functions.
    void DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride);

    // Scene management.
    SceneID m_NextSceneID;
    SceneID GetNextSceneID();
    void ResetSceneIDCounter();
    SceneInfo* GetSceneInfo(SceneID sceneID);
    SceneID GetSceneIDFromFullpath(const char* fullPath, bool requireSceneBeLoaded); // Returns SCENEID_NotFound if scene isn't found.
#if MYFW_EDITOR
    SceneHandler* m_pSceneHandler;
    void CreateNewScene(const char* sceneName, SceneID sceneID);
    unsigned int GetNumberOfScenesLoaded();
    //std::map<int, SceneInfo> m_pSceneInfoMap;

    GameObjectTemplateManager* m_pGameObjectTemplateManager;
    void Editor_GetListOfGameObjectsThatUsePrefab(std::vector<GameObject*>* pGameObjectList, PrefabObject* pPrefabToFind);
    int LogAllReferencesForFile(MyFileObject* pFile);
    int LogAllReferencesForFileInGameObject(MyFileObject* pFile, GameObject* pGameObject);
    GameObject* ParseLog_GameObject(const char* line);
    MaterialDefinition* ParseLog_Material(const char* line);
#endif //MYFW_EDITOR
    PrefabManager* m_pPrefabManager;
    SceneInfo m_pSceneInfoMap[MAX_SCENES_CREATED];

    // RenderGraph Functions.
    void AddMeshToRenderGraph(ComponentBase* pComponent, MyMesh* pMesh, MaterialDefinition** pMaterialList, MyRE::PrimitiveTypes primitiveType, int pointSize, unsigned int layers, RenderGraphObject** pOutputList);
    RenderGraphObject* AddSubmeshToRenderGraph(ComponentBase* pComponent, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, unsigned int layers);
    void RemoveObjectFromRenderGraph(RenderGraphObject* pRenderGraphObject);

public:
#if MYFW_EDITOR
    void DrawSingleObject(MyMatrix* pMatProj, MyMatrix* pMatView, GameObject* pObject, ShaderGroup* pShaderOverride); // used to draw an animated mesh into the debug FBO
    void DrawSingleComponent(MyMatrix* pMatProj, MyMatrix* pMatView, ComponentRenderable* pComponent, MaterialDefinition** ppMaterialOverrides, uint32 numMaterialOverrides);

#if MYFW_EDITOR
    void CheckForUpdatedDataSourceFiles(bool initialCheck);
    void OnFileUpdated(MyFileObject* pFile);
    void Editor_RegisterFileUpdatedCallback(FileUpdatedCallbackFunction* pFunc, void* pObj);
#endif //MYFW_EDITOR

    static void StaticOnMaterialCreated(void* pObjectPtr, MaterialDefinition* pMaterial) { ((ComponentSystemManager*)pObjectPtr)->OnMaterialCreated( pMaterial ); }
    void OnMaterialCreated(MaterialDefinition* pMaterial);

    static void StaticOnSoundCueCreated(void* pObjectPtr, SoundCue* pSoundCue) { ((ComponentSystemManager*)pObjectPtr)->OnSoundCueCreated( pSoundCue ); }
    void OnSoundCueCreated(SoundCue* pSoundCue);

    static void StaticOnSoundCueUnloaded(void* pObjectPtr, SoundCue* pSoundCue) { ((ComponentSystemManager*)pObjectPtr)->OnSoundCueUnloaded( pSoundCue ); }
    void OnSoundCueUnloaded(SoundCue* pSoundCue);

    static void StaticOnFileUnloaded(void* pObjectPtr, MyFileObject* pFile) { ((ComponentSystemManager*)pObjectPtr)->OnFileUnloaded( pFile ); }
    void OnFileUnloaded(MyFileObject* pFile);

    static int StaticOnFindAllReferences(void* pObjectPtr, MyFileObject* pFile) { return ((ComponentSystemManager*)pObjectPtr)->OnFindAllReferences( pFile ); }
    int OnFindAllReferences(MyFileObject* pFile);
#endif //MYFW_EDITOR
};

class MyFileInfo : public CPPListNode
{
protected:
    // m_pFile might be nullptr (for wav's for example, but m_SourceFileFullPath should be set in those cases).
    MyFileObject* m_pFile;
    char m_SourceFileFullPath[MAX_PATH];
    SceneID m_SceneID;

    MyMesh* m_pMesh; // A mesh may have been created alongside the file.
    ShaderGroup* m_pShaderGroup; // A shadergroup may have been created alongside the file.
    TextureDefinition* m_pTexture; // A texture may have been created alongside the file.
    MaterialDefinition* m_pMaterial; // A material may have been created alongside the file.
    SoundCue* m_pSoundCue; // A sound cue may have been created alongside the file.
    SpriteSheet* m_pSpriteSheet; // A sprite sheet may have been created alongside the file.
    PrefabFile* m_pPrefabFile; // A prefab file may have been created alongside the file.
    My2DAnimInfo* m_p2DAnimInfo; // A prefab file may have been created alongside the file.

    bool m_DidInitialCheckIfSourceFileWasUpdated;

public:
    MyFileInfo()
    {
        m_pFile = nullptr;
        m_SourceFileFullPath[0] = '\0'; // Store the source file (fbx, obj, etc) that the data file was converted from.
        m_SceneID = SCENEID_NotSet;

        m_pMesh = nullptr;
        m_pShaderGroup = nullptr;
        m_pTexture = nullptr;
        m_pMaterial = nullptr;
        m_pSoundCue = nullptr;
        m_pSpriteSheet = nullptr;

        m_DidInitialCheckIfSourceFileWasUpdated = false;
    }

    virtual ~MyFileInfo()
    {
        this->Remove();

        SAFE_RELEASE( m_pFile );
        SAFE_RELEASE( m_pMesh );
        SAFE_RELEASE( m_pShaderGroup );
        SAFE_RELEASE( m_pTexture );
        SAFE_RELEASE( m_pMaterial );
        SAFE_RELEASE( m_pSoundCue );
        SAFE_DELETE( m_pSpriteSheet );
        // SAFE_DELETE( m_pPrefabFile ); // TODO: Look into why this isn't necessary.
        SAFE_RELEASE( m_p2DAnimInfo );
    }

    // Getters.
    SceneID GetSceneID() { return m_SceneID; }
    const char* GetSourceFileFullPath() { return m_SourceFileFullPath; }
    bool GetDidInitialCheckIfSourceFileWasUpdated() { return m_DidInitialCheckIfSourceFileWasUpdated; }

    MyFileObject*       GetFile()           { return m_pFile; }
    MyMesh*             GetMesh()           { return m_pMesh; }
    ShaderGroup*        GetShaderGroup()    { return m_pShaderGroup; }
    TextureDefinition*  GetTexture()        { return m_pTexture; }
    MaterialDefinition* GetMaterial()       { return m_pMaterial; }
    SoundCue*           GetSoundCue()       { return m_pSoundCue; }
    SpriteSheet*        GetSpriteSheet()    { return m_pSpriteSheet; }
    PrefabFile*         GetPrefabFile()     { return m_pPrefabFile; }
    My2DAnimInfo*       Get2DAnimInfo()     { return m_p2DAnimInfo; }

    // Setters.
    void SetSceneID(SceneID id) { m_SceneID = id; }
    void SetSourceFileFullPath(const char* fullPath) { strcpy_s( m_SourceFileFullPath, MAX_PATH, fullPath ); }
    void SetDidInitialCheckIfSourceFileWasUpdated() { m_DidInitialCheckIfSourceFileWasUpdated = true; }

    void SetFile(MyFileObject* pFile)               { m_pFile = pFile;                  if( pFile )         pFile->AddRef(); }
    void SetMesh(MyMesh* pMesh)                     { m_pMesh = pMesh;                  if( pMesh )         pMesh->AddRef(); }
    void SetShaderGroup(ShaderGroup* pShaderGroup)  { m_pShaderGroup = pShaderGroup;    if( pShaderGroup )  pShaderGroup->AddRef(); }
    void SetTexture(TextureDefinition* pTexture)    { m_pTexture = pTexture;            if( pTexture )      pTexture->AddRef(); }
    void SetMaterial(MaterialDefinition* pMaterial) { m_pMaterial = pMaterial;          if( pMaterial )     pMaterial->AddRef(); }
    void SetSoundCue(SoundCue* pSoundCue)           { m_pSoundCue = pSoundCue;          if( pSoundCue )     pSoundCue->AddRef(); }
    void SetSpriteSheet(SpriteSheet* pSpriteSheet)  { m_pSpriteSheet = pSpriteSheet; } //if( pSpriteSheet )  pSpriteSheet->AddRef(); }
    void SetPrefabFile(PrefabFile* pPrefabFile)     { m_pPrefabFile = pPrefabFile; } //if( pPrefabFile )   pPrefabFile->AddRef(); }
    void Set2DAnimInfo(My2DAnimInfo* p2DAnimInfo)   { m_p2DAnimInfo = p2DAnimInfo;      if( p2DAnimInfo )   p2DAnimInfo->AddRef(); }
};

#endif //__ComponentSystemManager_H__
