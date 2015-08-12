//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentSystemManager_H__
#define __ComponentSystemManager_H__

class ComponentSystemManager;
class SceneHandler;
class GameObject;
class ComponentBase;
class ComponentCamera;
class ComponentLight;
class MyFileInfo; // at bottom of this file.

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
        void* pObj; \
        ComponentCallbackFunction_##CallbackType pFunc; \
    };

// Callback registration function macros.
#define MYFW_COMPONENTSYSTEMMANAGER_DECLARE_CALLBACK_REGISTER_FUNCTIONS(CallbackType) \
    CPPListHead m_pComponentCallbackList_##CallbackType; \
    void RegisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct); \
    void UnregisterComponentCallback_##CallbackType(ComponentCallbackStruct_##CallbackType* pCallbackStruct);

// Register/Unregister component callback macros - used by components.
#define MYFW_REGISTER_COMPONENT_CALLBACK(CallbackType) \
    m_CallbackStruct_##CallbackType.pObj = this; \
    m_CallbackStruct_##CallbackType.pFunc = &StaticCallback_##CallbackType; \
    g_pComponentSystemManager->RegisterComponentCallback_##CallbackType( &m_CallbackStruct_##CallbackType );

#define MYFW_UNREGISTER_COMPONENT_CALLBACK(CallbackType) \
    g_pComponentSystemManager->UnregisterComponentCallback_##CallbackType( &m_CallbackStruct_##CallbackType );

// Declare callback objects/functions - used by components.
#define MYFW_DECLARE_COMPONENT_CALLBACK_TICK(ComponentClass) \
    ComponentCallbackStruct_Tick m_CallbackStruct_Tick; \
    static void StaticCallback_Tick(void* pObjectPtr, double TimePassed) { ((ComponentClass*)pObjectPtr)->TickCallback( TimePassed ); } \
    void TickCallback(double TimePassed);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(ComponentClass) \
    ComponentCallbackStruct_OnSurfaceChanged m_CallbackStruct_OnSurfaceChanged; \
    static void StaticCallback_OnSurfaceChanged(void* pObjectPtr, unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight) { ((ComponentClass*)pObjectPtr)->OnSurfaceChangedCallback( startx, starty, width, height, desiredaspectwidth, desiredaspectheight ); } \
    void OnSurfaceChangedCallback(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);

#define MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(ComponentClass) \
    ComponentCallbackStruct_Draw m_CallbackStruct_Draw; \
    static void StaticCallback_Draw(void* pObjectPtr, ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride) { ((ComponentClass*)pObjectPtr)->DrawCallback( pCamera, pMatViewProj, pShaderOverride ); } \
    void DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(ComponentClass) \
    ComponentCallbackStruct_OnTouch m_CallbackStruct_OnTouch; \
    static bool StaticCallback_OnTouch(void* pObjectPtr, int action, int id, float x, float y, float pressure, float size) { return ((ComponentClass*)pObjectPtr)->OnTouchCallback( action, id, x, y, pressure, size ); } \
    bool OnTouchCallback(int action, int id, float x, float y, float pressure, float size);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(ComponentClass) \
    ComponentCallbackStruct_OnButtons m_CallbackStruct_OnButtons; \
    static bool StaticCallback_OnButtons(void* pObjectPtr, GameCoreButtonActions action, GameCoreButtonIDs id) { return ((ComponentClass*)pObjectPtr)->OnButtonsCallback( action, id ); } \
    bool OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(ComponentClass) \
    ComponentCallbackStruct_OnKeys m_CallbackStruct_OnKeys; \
    static bool StaticCallback_OnKeys(void* pObjectPtr, GameCoreButtonActions action, int keycode, int unicodechar) { return ((ComponentClass*)pObjectPtr)->OnKeysCallback( action, keycode, unicodechar ); } \
    bool OnKeysCallback(GameCoreButtonActions action, int keycode, int unicodechar);

#define MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(ComponentClass) \
    ComponentCallbackStruct_OnFileRenamed m_CallbackStruct_OnFileRenamed; \
    static void StaticCallback_OnFileRenamed(void* pObjectPtr, const char* fullpathbefore, const char* fullpathafter) { ((ComponentClass*)pObjectPtr)->OnFileRenamedCallback( fullpathbefore, fullpathafter ); } \
    void OnFileRenamedCallback(const char* fullpathbefore, const char* fullpathafter);

// Callback function prototypes and structs.
typedef void (*ComponentCallbackFunction_Tick)(void* obj, double TimePassed);
typedef void (*ComponentCallbackFunction_OnSurfaceChanged)(void* obj, unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
typedef void (*ComponentCallbackFunction_Draw)(void* obj, ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);
typedef bool (*ComponentCallbackFunction_OnTouch)(void* obj, int action, int id, float x, float y, float pressure, float size);
typedef bool (*ComponentCallbackFunction_OnButtons)(void* obj, GameCoreButtonActions action, GameCoreButtonIDs id);
typedef bool (*ComponentCallbackFunction_OnKeys)(void* obj, GameCoreButtonActions action, int keycode, int unicodechar);
typedef void (*ComponentCallbackFunction_OnFileRenamed)(void* obj, const char* fullpathbefore, const char* fullpathafter);

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
    ComponentTypeManager* m_pComponentTypeManager; // memory managed, delete this.
    CPPListHead m_GameObjects;

    // List of files used including a scene id
    CPPListHead m_Files;

    // a component can only exist in one of these lists ATM
    CPPListHead m_Components[BaseComponentType_NumTypes];

    bool m_WaitingForFilesToFinishLoading;
    bool m_StartGamePlayWhenDoneLoading;

protected:
#if MYFW_USING_WX
    std::vector<FileUpdatedCallbackStruct> m_pFileUpdatedCallbackList;
#endif //MYFW_USING_WX

    MyFileObject* GetFileObjectIfUsedByScene(const char* fullpath, unsigned int sceneid);

public:
    ComponentSystemManager(ComponentTypeManager* typemanager);
    virtual ~ComponentSystemManager();

    static void LuaRegister(lua_State* luastate);

    void MoveAllFilesNeededForLoadingScreenToStartOfFileList();
    char* SaveSceneToJSON(unsigned int sceneid);
    MyFileObject* LoadDatafile(const char* relativepath, unsigned int sceneid);
    void LoadSceneFromJSON(const char* scenename, const char* jsonstr, unsigned int sceneid);
    void FinishLoading(bool lockwhileloading, bool playwhenfinishedloading);

    void SyncAllRigidBodiesToObjectTransforms();

    // can clear everything except editor objects/components
    // unmanaged components are mainly editor objects and deleted objects in undo stack of editor... might want to rethink that.
    void UnloadScene(unsigned int sceneidtoclear = UINT_MAX, bool clearunmanagedcomponents = true);

    GameObject* CreateGameObject(bool manageobject = true, int sceneid = 0);
    void UnmanageGameObject(GameObject* pObject);
    void ManageGameObject(GameObject* pObject);
    void DeleteGameObject(GameObject* pObject, bool deletecomponents);
#if MYFW_USING_WX
    GameObject* EditorCopyGameObject(GameObject* pObject);
#endif
    GameObject* CopyGameObject(GameObject* pObject, const char* newname);

    unsigned int GetNextGameObjectIDAndIncrement(unsigned int sceneid);

    GameObject* FindGameObjectByID(unsigned int sceneid, unsigned int goid);
    GameObject* FindGameObjectByName(const char* name);
    GameObject* FindGameObjectByJSONRef(cJSON* pJSONGameObjectRef);
    ComponentCamera* GetFirstCamera();
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

    void OnLoad();
    void OnPlay();
    void OnStop();

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
    SceneInfo* GetSceneInfo(int sceneid);
    unsigned int GetSceneIDFromFullpath(const char* fullpath);
#if MYFW_USING_WX
    SceneHandler* m_pSceneHandler;
    void CreateNewScene(const char* scenename, unsigned int sceneid);
    wxTreeItemId GetTreeIDForScene(int sceneid);
    unsigned int GetSceneIDFromSceneTreeID(wxTreeItemId treeid);
    std::map<int, SceneInfo> m_pSceneInfoMap;
#else
    SceneInfo m_pSceneInfoMap[10];
#endif

public:
#if MYFW_USING_WX
    void DrawSingleObject(MyMatrix* pMatViewProj, GameObject* pObject); // used to draw an animated mesh into the debug FBO

    void OnFileUpdated(MyFileObject* pFile);
    void Editor_RegisterFileUpdatedCallback(FileUpdatedCallbackFunction pFunc, void* pObj);
    void AddAllMaterialsToFilesList();

    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentSystemManager*)pObjectPtr)->OnLeftClick( count, true ); }
    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((ComponentSystemManager*)pObjectPtr)->OnRightClick(); }
    void OnLeftClick(unsigned int count, bool clear);
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnMemoryPanelFileSelectedLeftClick(void* pObjectPtr) { ((ComponentSystemManager*)pObjectPtr)->OnMemoryPanelFileSelectedLeftClick(); }
    void OnMemoryPanelFileSelectedLeftClick();

    static void StaticOnMaterialCreated(void* pObjectPtr, MaterialDefinition* pMaterial) { ((ComponentSystemManager*)pObjectPtr)->OnMaterialCreated( pMaterial ); }
    void OnMaterialCreated(MaterialDefinition* pMaterial);
#endif //MYFW_USING_WX
};

class MyFileInfo : public CPPListNode
{
public:
    MyFileInfo()
    {
        m_pFile = 0;
        m_SceneID = 0;

        m_pMesh = 0;
        m_pShaderGroup = 0;
        m_pTexture = 0;
        m_pMaterial = 0;
    }

    virtual ~MyFileInfo()
    {
        this->Remove();

        SAFE_RELEASE( m_pFile );
        SAFE_RELEASE( m_pMesh );
        SAFE_RELEASE( m_pShaderGroup );
        SAFE_RELEASE( m_pTexture );
        SAFE_RELEASE( m_pMaterial );
    }

    MyFileObject* m_pFile;
    unsigned int m_SceneID;

    MyMesh* m_pMesh; // a mesh may have been created alongside the file.
    ShaderGroup* m_pShaderGroup; // a shadergroup may have been created alongside the file.
    TextureDefinition* m_pTexture; //a texture may have been created alongside the file.
    MaterialDefinition* m_pMaterial; //a material may have been created alongside the file.
};

#endif //__ComponentSystemManager_H__
