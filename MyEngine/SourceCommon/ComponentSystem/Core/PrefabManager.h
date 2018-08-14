//
// Copyright (c) 2017-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __PrefabManager_H__
#define __PrefabManager_H__

class PrefabObject;
class PrefabFile;
class PrefabManager;

#if MYFW_USING_WX
class PrefabObjectWxEventHandler : public wxEvtHandler
{
public:
    enum RightClickOptions
    {
        RightClick_DeletePrefab = 1000,
    };

public:
    PrefabObject* m_pPrefabObject;

public:
    PrefabObjectWxEventHandler()
    {
        m_pPrefabObject = 0;
    };
    void OnPopupClick(wxEvent &evt);
};
#endif

class PrefabReference
{
private:
    uint32 m_PrefabID; // Needed for loading, but should otherwise match up with m_pPrefab->GetID()

protected:
    // Always points to the root prefab object.
    PrefabObject* m_pPrefab;
    
    // These variables could point to the root prefab object or any of it's children.
    uint32 m_ChildID; // 0 if pointing to root of prefab.
    GameObject* m_pGameObject;

    // Experimenting with partial prefabs existing in the scene.
    // If a child is separated from it's original parent, then we mark it as "deleted" in that GameObject.
    GameObject* m_pOriginalPrefabParent;

    // Indicates whether or not m_pGameObject is part of the editor instance of the prefab.
    bool m_IsMasterPrefabGameObject;

public:
    PrefabReference();
    PrefabReference(PrefabObject* pPrefab, uint32 childid, bool setgameobject);
    
    // Getters
    PrefabObject* GetPrefab() { return m_pPrefab; }
    GameObject* GetGameObject() { return m_pGameObject; }
    GameObject* GetOriginalParent() { return m_pOriginalPrefabParent; }
    uint32 GetChildID() { return m_ChildID; }

    // Methods used by GameObject during load in cases where scene was loaded before Prefab file.
    void StoreIDsWhileLoading(uint32 prefabid, uint32 childid);
    void FinishLoadingPrefab(PrefabFile* pPrefabFile);

    void SetAsMasterPrefabGameObject() { m_IsMasterPrefabGameObject = true; }
    bool IsMasterPrefabGameObject() { return m_IsMasterPrefabGameObject; }
    void SetOriginalParent(GameObject* pGameObject);
};

class PrefabObject : public CPPListNode
{
    friend class PrefabFile;
    friend class PrefabManager;

public:
    static const int MAX_PREFAB_NAME_LENGTH = 30;

protected:
    char m_Name[MAX_PREFAB_NAME_LENGTH];
    cJSON* m_jPrefab;
    PrefabFile* m_pPrefabFile;

    uint32 m_PrefabID;
    uint32 m_NextChildPrefabID;

public:
    PrefabObject();
    ~PrefabObject();
    void Init(PrefabFile* pFile, const char* name, uint32 prefabid);
    void SetName(const char* name);
    void SetPrefabJSONObject(cJSON* jPrefab, bool createmastergameobjects);
    void SetPrefabID(uint32 prefabid);
    
    const char* GetName();
    uint32 GetID() { return m_PrefabID; }
    cJSON* GetJSONObject();

    PrefabFile* GetPrefabFile() { return m_pPrefabFile; }

    uint32 GetNextChildPrefabIDAndIncrement();

#if MYFW_EDITOR
protected:
    GameObject* m_pGameObject; // Each prefab is instantiated in editor so inheritance code can compare values.

#if MYFW_USING_WX
    // Event handler for right click menu.
    PrefabObjectWxEventHandler m_WxEventHandler;
    wxTreeItemId m_TreeID;
#endif //MYFW_USING_WX

public:
    GameObject* GetGameObject(uint32 childid = 0);
    GameObject* FindChildGameObject(uint32 childid, GameObject* pRootObject = 0);

#if MYFW_USING_WX
    void AddToObjectList(wxTreeItemId parent, cJSON* jPrefab, GameObject* pGameObject);
#endif //MYFW_USING_WX

    void RebuildPrefabJSONObjectFromMasterGameObject();

#if MYFW_USING_WX
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId treeid, unsigned int count) { ((PrefabObject*)pObjectPtr)->OnLeftClick( treeid, count, true ); }
    void OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId treeid) { ((PrefabObject*)pObjectPtr)->OnRightClick( treeid ); }
    void OnRightClick(wxTreeItemId treeid);

    static void StaticOnDrag(void* pObjectPtr) { ((PrefabObject*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, int x, int y) { ((PrefabObject*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, int x, int y);
#endif //MYFW_USING_WX
#endif //MYFW_EDITOR
};

class PrefabFile
{
    friend class PrefabManager;

protected:
    uint32 m_NextPrefabID;
    uint32 m_NextPrefabComponentID;
    MyFileObject* m_pFile;

    bool m_HasAnythingChanged;

    CPPListHead m_Prefabs;

public:
    PrefabFile(MyFileObject* pFile);
    ~PrefabFile();

    uint32 GetNextPrefabIDAndIncrement();
    uint32 GetNextPrefabComponentIDAndIncrement();
    MyFileObject* GetFile() { return m_pFile; }
    PrefabObject* GetFirstPrefabByName(const char* name);
    PrefabObject* GetPrefabByID(uint32 prefabid);

    void SetHasAnythingChanged() { m_HasAnythingChanged = true; }
    bool HasAnythingChanged() { return m_HasAnythingChanged; }

    static void StaticOnFileFinishedLoading(void* pObjectPtr, MyFileObject* pFile) { ((PrefabFile*)pObjectPtr)->OnFileFinishedLoading( pFile ); }
    void OnFileFinishedLoading(MyFileObject* pFile);

#if MYFW_EDITOR
    void Save();
    void RemovePrefab(PrefabObject* pPrefab);
    PrefabObject* GetFirstPrefab() { return (PrefabObject*)m_Prefabs.GetHead(); }
    void AddExistingPrefab(PrefabObject* pPrefab, PrefabObject* pPreviousPrefab); // used to undo delete in editor
#endif

#if MYFW_USING_WX
    wxTreeItemId m_TreeID;

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId treeid, unsigned int count) { ((PrefabFile*)pObjectPtr)->OnLeftClick( treeid, count, true ); }
    void OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId treeid) { ((PrefabFile*)pObjectPtr)->OnRightClick( treeid ); }
    void OnRightClick(wxTreeItemId treeid);
#endif
};

class PrefabManager
{
protected:
#if MYFW_EDITOR
    std::vector<PrefabFile*> m_pPrefabFiles;
#else
    MyList<PrefabFile*> m_pPrefabFiles;
#endif

protected:
#if MYFW_EDITOR
    void LoadFileNow(const char* prefabfilename);
#endif

public:
    PrefabManager();
    virtual ~PrefabManager();

    unsigned int GetNumberOfFiles();
    void SetNumberOfFiles(unsigned int numfiles); // for non-editor build, on scene load, allocate enough entries for # of files.

    PrefabFile* GetLoadedPrefabFileByIndex(unsigned int fileindex);
    PrefabFile* GetLoadedPrefabFileByFullPath(const char* fullpath);
    PrefabFile* RequestFile(const char* prefabfilename);

    void UnloadAllPrefabFiles();

#if MYFW_EDITOR
    PrefabObject* CreatePrefabInFile(unsigned int fileindex, const char* prefabname, GameObject* pGameObject);

    void CreateFile(const char* relativepath);
    bool CreateOrLoadFile();

    void SaveAllPrefabs(bool saveunchanged = false);

    PrefabObject* FindPrefabContainingGameObject(GameObject* pGameObject);

    void DeleteSelectedPrefabs();
#endif
};

#endif //__PrefabManager_H__
