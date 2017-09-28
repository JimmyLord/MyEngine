//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentSystemManager_H__
#define __ComponentSystemManager_H__

class ComponentSystemManager;
class PrefabManager;
class PrefabFile;
class PrefabObject;
class GameObjectTemplateManager;
class SceneHandler;
class GameObject;
class ComponentBase;
class ComponentCamera;
class ComponentLight;
class SceneGraph_Base;
class SceneGraphObject;
class MyFileInfo; // at bottom of this file.

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

#if MYFW_USING_WX
typedef void (*FileUpdatedCallbackFunction)(void* obj, MyFileObject* pFile);
struct FileUpdatedCallbackStruct
{
    void* pObj;
    FileUpdatedCallbackFunction pFunc;
};
#endif //MYFW_USING_WX

// Define structures to hold callback funcs and objects.
#define MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT(CallbackType) \
    struct ComponentCallbackStruct_##CallbackType : public CPPListNode \
    { \
        ComponentBase* pObj; \
        ComponentCallbackFunc_##CallbackType pFunc; \
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

// Declare callback objects/functions - used by components.
#define MYFW_DECLARE_COMPONENT_CALLBACK_TICK() \
    ComponentCallbackStruct_Tick m_CallbackStruct_Tick; \
    void TickCallback(double TimePassed);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED() \
    ComponentCallbackStruct_OnSurfaceChanged m_CallbackStruct_OnSurfaceChanged; \
    void OnSurfaceChangedCallback(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);

#define MYFW_DECLARE_COMPONENT_CALLBACK_DRAW() \
    ComponentCallbackStruct_Draw m_CallbackStruct_Draw; \
    void DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride); \
    virtual ComponentCallbackStruct_Draw* GetDrawCallback() { return &m_CallbackStruct_Draw; }

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH() \
    ComponentCallbackStruct_OnTouch m_CallbackStruct_OnTouch; \
    bool OnTouchCallback(int action, int id, float x, float y, float pressure, float size);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS() \
    ComponentCallbackStruct_OnButtons m_CallbackStruct_OnButtons; \
    bool OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS() \
    ComponentCallbackStruct_OnKeys m_CallbackStruct_OnKeys; \
    bool OnKeysCallback(GameCoreButtonActions action, int keycode, int unicodechar);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED() \
    ComponentCallbackStruct_OnFileRenamed m_CallbackStruct_OnFileRenamed; \
    void OnFileRenamedCallback(const char* fullpathbefore, const char* fullpathafter);

// Callback function prototypes and structs.
typedef void (ComponentBase::*ComponentCallbackFunc_Tick)(double TimePassed);
typedef void (ComponentBase::*ComponentCallbackFunc_OnSurfaceChanged)(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
typedef void (ComponentBase::*ComponentCallbackFunc_Draw)(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);
typedef bool (ComponentBase::*ComponentCallbackFunc_OnTouch)(int action, int id, float x, float y, float pressure, float size);
typedef bool (ComponentBase::*ComponentCallbackFunc_OnButtons)(GameCoreButtonActions action, GameCoreButtonIDs id);
typedef bool (ComponentBase::*ComponentCallbackFunc_OnKeys)(GameCoreButtonActions action, int keycode, int unicodechar);
typedef void (ComponentBase::*ComponentCallbackFunc_OnFileRenamed)(const char* fullpathbefore, const char* fullpathafter);

MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( Tick );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnSurfaceChanged );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( Draw );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnTouch );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnButtons );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnKeys );
MYFW_COMPONENTSYSTEMMANAGER_DEFINE_CALLBACK_STRUCT( OnFileRenamed );

class ComponentSystemManager
#if MYFW_USING_WX
: public wxEvtHandler
#endif //MYFW_USING_WX
{
public:
    static const int MAX_SCENES_LOADED = 10;

public:
    ComponentTypeManager* m_pComponentTypeManager; // memory managed, delete this.

    // List of files used including a scene id and the source file (if applicable)
    CPPListHead m_Files;
    CPPListHead m_FilesStillLoading;

    // a component can only exist in one of these lists ATM
    CPPListHead m_Components[BaseComponentType_NumTypes];

    bool m_WaitingForFilesToFinishLoading;
    bool m_StartGamePlayWhenDoneLoading;

    float m_TimeScale;

    SceneGraph_Base* m_pSceneGraph;

protected:
#if MYFW_USING_WX
    std::vector<FileUpdatedCallbackStruct> m_pFileUpdatedCallbackList;
#endif //MYFW_USING_WX

    MyFileInfo* GetFileInfoIfUsedByScene(const char* fullpath, unsigned int sceneid);
    MyFileObject* GetFileObjectIfUsedByScene(const char* fullpath, unsigned int sceneid);

public:
    ComponentSystemManager(ComponentTypeManager* typemanager);
    virtual ~ComponentSystemManager();

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    void SetTimeScale(float scale) { m_TimeScale = scale; }

    void MoveAllFilesNeededForLoadingScreenToStartOfFileList(GameObject* first);
    void AddListOfFilesUsedToJSONObject(unsigned int sceneid, cJSON* filearray);
    char* SaveSceneToJSON(unsigned int sceneid);
    char* ExportBox2DSceneToJSON(unsigned int sceneid);
    void SaveGameObjectListToJSONArray(cJSON* gameobjectarray, cJSON* transformarray, GameObject* first, bool savesceneid);
    
    MyFileInfo* AddToFileList(MyFileObject* pFile, MyMesh* pMesh, ShaderGroup* pShaderGroup, TextureDefinition* pTexture, MaterialDefinition* pMaterial, SoundCue* pSoundCue, SpriteSheet* pSpriteSheet, unsigned int sceneid);
    MyFileObject* LoadDataFile(const char* relativepath, unsigned int sceneid, const char* fullsourcefilepath, bool convertifrequired);
    MyFileObject* ImportDataFile(unsigned int sceneid, const char* fullsourcefilepath);
    void FreeDataFile(MyFileInfo* pFileInfo);
    void FreeAllDataFiles(unsigned int sceneidtoclear);

    void LoadSceneFromJSON(const char* scenename, const char* jsonstr, unsigned int sceneid);
    ComponentBase* CreateComponentFromJSONObject(GameObject* pGameObject, cJSON* jComponent);
    void FinishLoading(bool lockwhileloading, unsigned int sceneid, bool playwhenfinishedloading);

    void SyncAllRigidBodiesToObjectTransforms();

    // can clear everything except editor objects/components
    // unmanaged components are mainly editor objects and deleted objects in undo stack of editor... might want to rethink that.
    void UnloadScene(unsigned int sceneidtoclear = UINT_MAX, bool clearunmanagedcomponents = true);
    bool IsSceneLoaded(const char* fullpath);
    unsigned int FindSceneID(const char* fullpath);

    GameObject* CreateGameObject(bool manageobject = true, int sceneid = 0, bool isfolder = false, bool hastransform = true, PrefabObject* pPrefab = 0);
    GameObject* CreateGameObjectFromPrefab(PrefabObject* pPrefab, bool manageobject, int sceneid);
#if MYFW_USING_WX
    GameObject* CreateGameObjectFromTemplate(unsigned int templateid, int sceneid);
#endif
    void UnmanageGameObject(GameObject* pObject);
    void ManageGameObject(GameObject* pObject);
    void DeleteGameObject(GameObject* pObject, bool deletecomponents);
#if MYFW_USING_WX
    GameObject* EditorCopyGameObject(GameObject* pObject, bool NewObjectInheritsFromOld);
#endif
    GameObject* CopyGameObject(GameObject* pObject, const char* newname);

    unsigned int GetNextGameObjectIDAndIncrement(unsigned int sceneid);
    unsigned int GetNextComponentIDAndIncrement(unsigned int sceneid);

    GameObject* GetFirstGameObjectFromScene(unsigned int sceneid);
    GameObject* FindGameObjectByID(unsigned int sceneid, unsigned int goid);
    GameObject* FindGameObjectByIDFromList(GameObject* list, unsigned int goid);
    GameObject* FindGameObjectByName(const char* name);
    GameObject* FindGameObjectByNameInScene(unsigned int sceneid, const char* name);
    GameObject* FindGameObjectByNameFromList(GameObject* list, const char* name);
    GameObject* FindGameObjectByJSONRef(cJSON* pJSONGameObjectRef, unsigned int defaultsceneid);
    ComponentBase* FindComponentByJSONRef(cJSON* pJSONComponentRef, unsigned int defaultsceneid);
    ComponentCamera* GetFirstCamera(bool prefereditorcam = false);
    ComponentBase* GetFirstComponentOfType(const char* type);
    ComponentBase* GetNextComponentOfType(ComponentBase* pLastComponent);

    ComponentBase* AddComponent(ComponentBase* pComponent);
    void DeleteComponent(ComponentBase* pComponent);

    ComponentBase* FindComponentByID(unsigned int id, unsigned int sceneid = UINT_MAX);

    // Main events, most should call component callbacks.
    void Tick(double TimePassed);
    void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    void OnDrawFrame();
    void OnDrawFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);
    void OnFileRenamed(const char* fullpathbefore, const char* fullpathafter);

    void OnLoad(unsigned int sceneid);
    void OnPlay(unsigned int sceneid);
    void OnStop(unsigned int sceneid);

    bool OnEvent(MyEvent* pEvent);

    bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
    bool OnKeys(GameCoreButtonActions action, int keycode, int unicodechar);

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
    void DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);

    // Scene management.
    unsigned int m_NextSceneID;
    unsigned int GetNextSceneID() { return m_NextSceneID++; }
    void ResetSceneIDCounter() { m_NextSceneID = 1; }
    SceneInfo* GetSceneInfo(int sceneid);
    unsigned int GetSceneIDFromFullpath(const char* fullpath);
#if MYFW_USING_WX
    SceneHandler* m_pSceneHandler;
    void CreateNewScene(const char* scenename, unsigned int sceneid);
    wxTreeItemId GetTreeIDForScene(int sceneid);
    unsigned int GetSceneIDFromSceneTreeID(wxTreeItemId treeid);
    unsigned int GetNumberOfScenesLoaded();
    //std::map<int, SceneInfo> m_pSceneInfoMap;

    GameObjectTemplateManager* m_pGameObjectTemplateManager;
    void Editor_GetListOfGameObjectsThatUsePrefab(std::vector<GameObject*>* pGameObjectList, PrefabObject* pPrefabToFind);
    void LogAllReferencesForFile(MyFileObject* pFile);
    GameObject* ParseLog_GameObject(const char* line);
    MaterialDefinition* ParseLog_Material(const char* line);
#endif
    PrefabManager* m_pPrefabManager;
    SceneInfo m_pSceneInfoMap[MAX_SCENES_LOADED];

    // SceneGraph Functions
    SceneGraph_Base* GetSceneGraph() { return m_pSceneGraph; }
    void AddMeshToSceneGraph(ComponentBase* pComponent, MyMesh* pMesh, MaterialDefinition** pMaterialList, int primitive, int pointsize, SceneGraphFlags flags, unsigned int layers, SceneGraphObject** pOutputList);
    SceneGraphObject* AddSubmeshToSceneGraph(ComponentBase* pComponent, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitive, int pointsize, SceneGraphFlags flags, unsigned int layers);
    void RemoveObjectFromSceneGraph(SceneGraphObject* pSceneGraphObject);

public:
#if MYFW_USING_WX
    void DrawSingleObject(MyMatrix* pMatViewProj, GameObject* pObject, ShaderGroup* pShaderOverride); // used to draw an animated mesh into the debug FBO

    void CheckForUpdatedDataSourceFiles(bool initialcheck);
    void OnFileUpdated(MyFileObject* pFile);
    void Editor_RegisterFileUpdatedCallback(FileUpdatedCallbackFunction pFunc, void* pObj);

    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentSystemManager*)pObjectPtr)->OnLeftClick( count, true ); }
    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((ComponentSystemManager*)pObjectPtr)->OnRightClick(); }
    void OnLeftClick(unsigned int count, bool clear);
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnMemoryPanelFileSelectedLeftClick(void* pObjectPtr) { ((ComponentSystemManager*)pObjectPtr)->OnMemoryPanelFileSelectedLeftClick(); }
    void OnMemoryPanelFileSelectedLeftClick();

    static void StaticOnMaterialCreated(void* pObjectPtr, MaterialDefinition* pMaterial) { ((ComponentSystemManager*)pObjectPtr)->OnMaterialCreated( pMaterial ); }
    void OnMaterialCreated(MaterialDefinition* pMaterial);

    static void StaticOnSoundCueCreated(void* pObjectPtr, SoundCue* pSoundCue) { ((ComponentSystemManager*)pObjectPtr)->OnSoundCueCreated( pSoundCue ); }
    void OnSoundCueCreated(SoundCue* pSoundCue);

    static void StaticOnSoundCueUnloaded(void* pObjectPtr, SoundCue* pSoundCue) { ((ComponentSystemManager*)pObjectPtr)->OnSoundCueUnloaded( pSoundCue ); }
    void OnSoundCueUnloaded(SoundCue* pSoundCue);

    static void StaticOnFileUnloaded(void* pObjectPtr, MyFileObject* pFile) { ((ComponentSystemManager*)pObjectPtr)->OnFileUnloaded( pFile ); }
    void OnFileUnloaded(MyFileObject* pFile);

    static void StaticOnFindAllReferences(void* pObjectPtr, MyFileObject* pFile) { ((ComponentSystemManager*)pObjectPtr)->OnFindAllReferences( pFile ); }
    void OnFindAllReferences(MyFileObject* pFile);    
#endif //MYFW_USING_WX
};

class MyFileInfo : public CPPListNode
{
public:
    // m_pFile might be 0 (for wav's for example, but m_SourceFileFullPath should be set in those cases)
    MyFileObject* m_pFile;
    char m_SourceFileFullPath[MAX_PATH];
    unsigned int m_SceneID;

    MyMesh* m_pMesh; // a mesh may have been created alongside the file.
    ShaderGroup* m_pShaderGroup; // a shadergroup may have been created alongside the file.
    TextureDefinition* m_pTexture; //a texture may have been created alongside the file.
    MaterialDefinition* m_pMaterial; //a material may have been created alongside the file.
    SoundCue* m_pSoundCue; //a sound cue may have been created alongside the file.
    SpriteSheet* m_pSpriteSheet; //a sprite sheet may have been created alongside the file.
    PrefabFile* m_pPrefabFile; // a prefab file may have been created alongside the file

    bool m_DidInitialCheckIfSourceFileWasUpdated;

public:
    MyFileInfo()
    {
        m_pFile = 0;
        m_SourceFileFullPath[0] = 0; // store the source file (fbx, obj, etc) that the data file was converted from.
        m_SceneID = 0;

        m_pMesh = 0;
        m_pShaderGroup = 0;
        m_pTexture = 0;
        m_pMaterial = 0;
        m_pSoundCue = 0;
        m_pSpriteSheet = 0;

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
    }
};

#endif //__ComponentSystemManager_H__
