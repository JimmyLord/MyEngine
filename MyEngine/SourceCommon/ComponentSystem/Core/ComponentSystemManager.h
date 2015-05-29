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

typedef void (*ComponentTickCallbackFunction)(void* obj, double TimePassed);
struct ComponentTickCallbackStruct
{
    void* pObj;
    ComponentTickCallbackFunction pFunc;

    // == op needed for unregister of callback.
    inline bool operator ==(const ComponentTickCallbackStruct& o)
    {
        return this->pObj == o.pObj && this->pFunc == o.pFunc;
    }
};

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
    CPPListHead m_ComponentsCamera;
    CPPListHead m_ComponentsInputHandler;
    CPPListHead m_ComponentsUpdateable;
    CPPListHead m_ComponentsRenderable;
    CPPListHead m_ComponentsData;

    // a list of components that want an update call without being in the list above.
    static const int MAX_COMPONENT_TICK_CALLBACKS = 100; // TODO: fix this hardcodedness
    MyList<ComponentTickCallbackStruct> m_pComponentTickCallbackList;

    unsigned int m_NextGameObjectID;
    unsigned int m_NextComponentID;

protected:
#if MYFW_USING_WX
    std::vector<FileUpdatedCallbackStruct> m_pFileUpdatedCallbackList;
#endif //MYFW_USING_WX

    bool IsFileUsedByScene(const char* fullpath, unsigned int sceneid);

public:
    ComponentSystemManager(ComponentTypeManager* typemanager);
    virtual ~ComponentSystemManager();

    static void LuaRegister(lua_State* luastate);

    char* SaveSceneToJSON();
    void LoadDatafile(const char* relativepath, unsigned int sceneid);
    void LoadSceneFromJSON(const char* jsonstr, unsigned int sceneid);

    void SyncAllRigidBodiesToObjectTransforms();

    // can clear everything except editor objects/components
    // unmanaged components are mainly editor objects and deleted objects in undo stack of editor... might want to rethink that.
    void UnloadScene(unsigned int sceneidtoclear = UINT_MAX, bool clearunmanagedcomponents = true);

    GameObject* CreateGameObject(bool manageobject = true);
    void UnmanageGameObject(GameObject* pObject);
    void ManageGameObject(GameObject* pObject);
    void DeleteGameObject(GameObject* pObject, bool deletecomponents);
#if MYFW_USING_WX
    GameObject* EditorCopyGameObject(GameObject* pObject);
#endif
    GameObject* CopyGameObject(GameObject* pObject, const char* newname);

    GameObject* FindGameObjectByID(unsigned int id);
    GameObject* FindGameObjectByName(const char* name);
    ComponentCamera* GetFirstCamera();

    ComponentBase* AddComponent(ComponentBase* pComponent);
    void DeleteComponent(ComponentBase* pComponent);

    ComponentBase* FindComponentByID(unsigned int id, unsigned int sceneid = UINT_MAX);

    void Tick(double TimePassed);
    void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    void OnDrawFrame();
    void OnDrawFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);
    void DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);

    void OnLoad();
    void OnPlay();
    void OnStop();

    bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);

    void RegisterComponentTickCallback(ComponentTickCallbackFunction pFunc, void* pObj);
    void UnregisterComponentTickCallback(ComponentTickCallbackFunction pFunc, void* pObj);

public:
#if MYFW_USING_WX
    void DrawSingleObject(MyMatrix* pMatViewProj, GameObject* pObject); // used to draw an animated mesh into the debug FBO

    void OnFileUpdated(MyFileObject* pFile);
    void Editor_RegisterFileUpdatedCallback(FileUpdatedCallbackFunction pFunc, void* pObj);
    void AddAllMaterialsToFilesList();

    static void StaticOnLeftClick(void* pObjectPtr, unsigned int count) { ((ComponentSystemManager*)pObjectPtr)->OnLeftClick( count, true ); }
    static void StaticOnRightClick(void* pObjectPtr) { ((ComponentSystemManager*)pObjectPtr)->OnRightClick(); }
    void OnLeftClick(unsigned int count, bool clear);
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnMemoryPanelFileSelectedLeftClick(void* pObjectPtr) { ((ComponentSystemManager*)pObjectPtr)->OnMemoryPanelFileSelectedLeftClick(); }
    void OnMemoryPanelFileSelectedLeftClick();
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
    }

    virtual ~MyFileInfo()
    {
        this->Remove();

        SAFE_RELEASE( m_pFile );
        SAFE_RELEASE( m_pMesh );
        SAFE_RELEASE( m_pShaderGroup );
    }

    MyFileObject* m_pFile;
    unsigned int m_SceneID;

    MyMesh* m_pMesh; // a mesh may have been created alongside the file.
    ShaderGroup* m_pShaderGroup; // a shadergroup may have been created alongside the file.
};

#endif //__ComponentSystemManager_H__
