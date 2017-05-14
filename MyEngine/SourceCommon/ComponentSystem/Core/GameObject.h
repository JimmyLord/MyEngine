//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameObject_H__
#define __GameObject_H__

typedef void (*GameObjectDeletedCallbackFunc)(void* pObjectPtr, GameObject* pGameObject);
struct GameObjectDeletedCallbackStruct : CPPListNode
{
    void* pObj;
    GameObjectDeletedCallbackFunc pFunc;
};

class GameObject : public CPPListNode
#if MYFW_USING_WX
, public wxEvtHandler
#endif
{
    friend class EditorCommand_DeleteObjects; // for NotifyOthersThisWasDeleted()

    static const int MAX_COMPONENTS = 8; // TODO: fix this hardcodedness

protected:
    PrefabObject* m_pPrefab;
    GameObject* m_pGameObjectThisInheritsFrom; // for variables, if set, any changes to the parent will be reflected in this object.

    GameObject* m_pParentGameObject;
    CPPListHead m_ChildList; // child game objects

    ComponentGameObjectProperties m_Properties;

    bool m_Enabled;
    bool m_IsFolder;
    unsigned int m_SceneID; // 0 for runtime generated.
    unsigned int m_ID;
    unsigned int m_PhysicsSceneID; // can't be 0 (runtime scene) if a collision component exists
    char* m_Name; // this a copy of the string passed in.
    bool m_Managed;

    CPPListHead m_pOnDeleteCallbacks;

protected:
    void NotifyOthersThisWasDeleted();

public:
    ComponentTransform* m_pComponentTransform;
    MyList<ComponentBase*> m_Components; // component system manager is responsible for deleting these components.

public:
    GameObject(bool managed, int sceneid, bool isfolder, bool hastransform, PrefabObject* pPrefab);
    virtual ~GameObject();
    SetClassnameBase( "GameObject" ); // only first 8 character count.

    PrefabObject* GetPrefab() { return m_pPrefab; }
    bool IsPrefab() { return m_pPrefab != 0; }

    bool IsFolder() { return m_IsFolder; }
    GameObject* GetGameObjectThisInheritsFrom() { return m_pGameObjectThisInheritsFrom; }
    CPPListHead* GetChildList() { return &m_ChildList; }
    GameObject* GetFirstChild() { return (GameObject*)m_ChildList.GetHead(); }
    void SetGameObjectThisInheritsFrom(GameObject* pObj) { m_pGameObjectThisInheritsFrom = pObj; }

    // Parent gameobject, is terms of transform (should also work with folders/gameobject without transforms)
    GameObject* GetParentGameObject() { return m_pParentGameObject; }

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    cJSON* ExportAsJSONObject(bool savesceneid);
    void ImportFromJSONObject(cJSON* jGameObject, unsigned int sceneid);
    cJSON* ExportReferenceAsJSONObject(unsigned int refsceneid);
    cJSON* ExportAsJSONPrefab();

    void SetFlags(unsigned int flags) { return m_Properties.SetFlags( flags ); }
    unsigned int GetFlags() { return m_Properties.GetFlags(); }

    void SetEnabled(bool enabled);
    void RegisterAllComponentCallbacks(bool ignoreenabledflag);
    void UnregisterAllComponentCallbacks(bool ignoreenabledflag);
    void SetSceneID(unsigned int sceneid, bool assignnewgoid = true);
    void SetID(unsigned int id);
    void SetName(const char* name);
    void SetParentGameObject(GameObject* pParentGameObject);
    void SetManaged(bool managed);
    bool IsManaged() { return m_Managed; }

    bool IsEnabled() { return m_Enabled; }
    unsigned int GetSceneID() { return m_SceneID; }
    SceneInfo* GetSceneInfo() { return g_pComponentSystemManager->GetSceneInfo( m_SceneID ); }
    unsigned int GetID() { return m_ID; }
    const char* GetName() { return m_Name; }

    void SetPhysicsSceneID(unsigned int id) { m_PhysicsSceneID = id; }
    unsigned int GetPhysicsSceneID() { return m_PhysicsSceneID; }

    ComponentBase* AddNewComponent(int componenttype, unsigned int sceneid, ComponentSystemManager* pComponentSystemManager = g_pComponentSystemManager);
    ComponentBase* AddExistingComponent(ComponentBase* pComponent, bool resetcomponent);
    ComponentBase* RemoveComponent(ComponentBase* pComponent);

    ComponentBase* FindComponentByID(unsigned int componentid);

    ComponentBase* GetFirstComponentOfBaseType(BaseComponentTypes basetype);
    ComponentBase* GetNextComponentOfBaseType(ComponentBase* pLastComponent);

    ComponentBase* GetFirstComponentOfType(const char* type);
    ComponentBase* GetNextComponentOfType(ComponentBase* pLastComponent);

    ComponentTransform* GetTransform() { return m_pComponentTransform; }

    // TODO: find a way to find an arbitrary component type that would be accessible from lua script.
    ComponentAnimationPlayer* GetAnimationPlayer()      { return (ComponentAnimationPlayer*)GetFirstComponentOfType( "AnimPlayerComponent" ); }
    ComponentCollisionObject* GetCollisionObject()      { return (ComponentCollisionObject*)GetFirstComponentOfType( "CollisionObjectComponent" ); }
    Component2DCollisionObject* Get2DCollisionObject()  { return (Component2DCollisionObject*)GetFirstComponentOfType( "2DCollisionObjectComponent" ); }
    ComponentParticleEmitter* GetParticleEmitter()      { return (ComponentParticleEmitter*)GetFirstComponentOfType( "ParticleEmitterComponent" ); }
    ComponentVoxelWorld* GetVoxelWorld()                { return (ComponentVoxelWorld*)GetFirstComponentOfType( "VoxelWorldComponent" ); }
    ComponentAudioPlayer* GetAudioPlayer()              { return (ComponentAudioPlayer*)GetFirstComponentOfType( "AudioPlayer" ); }

    MaterialDefinition* GetMaterial();
    void SetMaterial(MaterialDefinition* pMaterial);

    void SetScriptFile(MyFileObject* pFile);

    // Callbacks
    void RegisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback);
    void UnregisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback);

    static void StaticOnGameObjectDeleted(void* pObjectPtr, GameObject* pGameObject) { ((GameObject*)pObjectPtr)->OnGameObjectDeleted( pGameObject ); }
    void OnGameObjectDeleted(GameObject* pGameObject);

    static void StaticOnTransformChanged(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor) { ((GameObject*)pObjectPtr)->OnTransformChanged( newpos, newrot, newscale, changedbyuserineditor ); }
    void OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor);

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_DuplicateGameObject = 1000,
        RightClick_CreateChild,
        RightClick_ClearParent,
        RightClick_CreatePrefab,
            // next 10000 values reserved for prefab scenes that are open. 10000 also hardcoded in cpp file
        RightClick_DeleteGameObject = RightClick_CreatePrefab + 10000,
        RightClick_DeleteFolder,
        RightClick_DuplicateFolder,
    };

    static void StaticOnTitleLabelClicked(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue) { ((GameObject*)pObjectPtr)->OnTitleLabelClicked( controlid, finishedchanging ); }
    void OnTitleLabelClicked(int controlid, bool finishedchanging);

    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((GameObject*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((GameObject*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((GameObject*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, wxCoord x, wxCoord y) { ((GameObject*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);

    static void StaticOnLabelEdit(void* pObjectPtr, wxTreeItemId id, wxString newlabel) { ((GameObject*)pObjectPtr)->OnLabelEdit( newlabel ); }
    void OnLabelEdit(wxString newlabel);

    void UpdateObjectListIcon();

    // Prefab Loading Vars
    uint32 m_PrefabID;
    void FinishLoadingPrefab(PrefabFile* pPrefabFile, uint32 prefabid);
    static void StaticOnPrefabFileFinishedLoading(void* pObjectPtr, MyFileObject* pFile) { ((GameObject*)pObjectPtr)->OnPrefabFileFinishedLoading( pFile ); }
    void OnPrefabFileFinishedLoading(MyFileObject* pFile);

    void Editor_SetPrefab(PrefabObject* pPrefab) { m_pPrefab = pPrefab; UpdateObjectListIcon(); } // used when deleting prefabs
#endif //MYFW_USING_WX
};

#endif //__GameObject_H__
