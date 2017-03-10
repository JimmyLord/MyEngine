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

class PrefabObject
{
    friend class PrefabFile;
    friend class PrefabManager;

    static const int MAX_PREFAB_NAME_LENGTH = 30;

protected:
    char m_Name[MAX_PREFAB_NAME_LENGTH];
    cJSON* m_jPrefab;
#if MYFW_USING_WX
    GameObject* m_pGameObject; // each prefab is instantiated in editor so inheritance code can compare values.
#endif

public:
    PrefabObject();
    void Init(PrefabFile* pFile, const char* name);
    void SetName(const char* name);
    
    const char* GetName();
    cJSON* GetJSONObject();

#if MYFW_USING_WX
    void Save();

    wxTreeItemId m_TreeID;

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
    MyFileObject* m_pFile;

#if MYFW_USING_WX
    std::vector<PrefabObject*> m_Prefabs;
#else
    MyList<PrefabObject*> m_Prefabs;
#endif

public:
    PrefabFile(MyFileObject* pFile);
    ~PrefabFile();

    MyFileObject* GetFile() { return m_pFile; }

    static void StaticOnFileFinishedLoading(void* pObjectPtr, MyFileObject* pFile) { ((PrefabFile*)pObjectPtr)->OnFileFinishedLoading( pFile ); }
    void OnFileFinishedLoading(MyFileObject* pFile);

#if MYFW_USING_WX
    void Save();

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
    PrefabFile* RequestFile(const char* prefabfilename);

#if MYFW_USING_WX
    void CreatePrefabInFile(unsigned int fileindex, const char* prefabname, GameObject* pGameObject);

    void CreateFile(const char* relativepath);
    bool CreateOrLoadFile();
#endif
};

#endif //__PrefabManager_H__
