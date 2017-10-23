//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
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
public:
    // Always points to the root prefab object.
    PrefabObject* m_pPrefab;
    
//    PrefabFile* m_pRootPrefabFile;
//    PrefabObject* m_pRootPrefab;
//    
//    // These variables could point to the root prefab object or any of it's children.
//    uint32 m_PrefabID;
//    cJSON* m_jPrefab;
    GameObject* m_pGameObject;
//
//public:
//    PrefabObject* GetRootPrefab() { return m_pRootPrefab; }
//
//    cJSON* GetJSONObject() { return m_jPrefab; }
//    GameObject* GetGameObject() { return m_pGameObject; }

public:
    PrefabReference()
    {
        m_pPrefab = 0;
        m_pGameObject = 0;
    }
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
#if MYFW_USING_WX
    GameObject* m_pGameObject; // Each prefab is instantiated in editor so inheritance code can compare values.
#endif

public:
    PrefabObject();
    ~PrefabObject();
    void Init(PrefabFile* pFile, const char* name, uint32 prefabid);
    void SetName(const char* name);
    void SetPrefabJSONObject(cJSON* jPrefab);
    void SetPrefabID(uint32 prefabid) { m_PrefabID = prefabid; }
    
    const char* GetName();
    uint32 GetID() { return m_PrefabID; }
    cJSON* GetJSONObject();

    PrefabFile* GetPrefabFile() { return m_pPrefabFile; }

#if MYFW_USING_WX
    // Event handler for right click menu.
    friend class SoundManagerWxEventHandler;
    PrefabObjectWxEventHandler m_WxEventHandler;

    GameObject* GetGameObject() { return m_pGameObject; }

    void Save();

    wxTreeItemId m_TreeID;

    void AddToObjectList(wxTreeItemId parent, cJSON* jPrefab, GameObject* pGameObject);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId treeid, unsigned int count) { ((PrefabObject*)pObjectPtr)->OnLeftClick( treeid, count, true ); }
    void OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId treeid) { ((PrefabObject*)pObjectPtr)->OnRightClick( treeid ); }
    void OnRightClick(wxTreeItemId treeid);

    static void StaticOnDrag(void* pObjectPtr) { ((PrefabObject*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, wxCoord x, wxCoord y) { ((PrefabObject*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);
#endif
};

class PrefabFile
{
    friend class PrefabManager;

protected:
    uint32 m_NextPrefabID;
    MyFileObject* m_pFile;

    bool m_HasAnythingChanged;

    CPPListHead m_Prefabs;

public:
    PrefabFile(MyFileObject* pFile);
    ~PrefabFile();

    uint32 GetNextPrefabIDAndIncrement();
    MyFileObject* GetFile() { return m_pFile; }
    PrefabObject* GetFirstPrefabByName(const char* name);
    PrefabObject* GetPrefabByID(uint32 prefabid);

    void SetHasAnythingChanged() { m_HasAnythingChanged = true; }
    bool HasAnythingChanged() { return m_HasAnythingChanged; }

    static void StaticOnFileFinishedLoading(void* pObjectPtr, MyFileObject* pFile) { ((PrefabFile*)pObjectPtr)->OnFileFinishedLoading( pFile ); }
    void OnFileFinishedLoading(MyFileObject* pFile);

#if MYFW_USING_WX
    void Save();

    wxTreeItemId m_TreeID;

    void RemovePrefab(PrefabObject* pPrefab);
    void AddExistingPrefab(PrefabObject* pPrefab, PrefabObject* pPreviousPrefab); // used to undo delete in editor

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
#if MYFW_USING_WX
    std::vector<PrefabFile*> m_pPrefabFiles;
#else
    MyList<PrefabFile*> m_pPrefabFiles;
#endif

protected:
#if MYFW_USING_WX
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

#if MYFW_USING_WX
    void CreatePrefabInFile(unsigned int fileindex, const char* prefabname, GameObject* pGameObject);

    void CreateFile(const char* relativepath);
    bool CreateOrLoadFile();

    void SaveAllPrefabs(bool saveunchanged = false);

    PrefabObject* FindPrefabContainingGameObject(GameObject* pGameObject);
#endif
};

#endif //__PrefabManager_H__
