//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
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
    friend class EditorCommand_RestorePrefabComponent; // for m_DeletedPrefabComponentIDs
    friend class EditorMainFrame_ImGui; // for m_DeletedPrefabChildIDs and m_DeletedPrefabComponentIDs
    friend class ComponentSystemManager; // for ComponentSystemManager::DeleteGameObject

    static const int MAX_COMPONENTS = 8; // TODO: fix this hardcodedness

protected:
    PrefabReference m_PrefabRef;
    GameObject* m_pGameObjectThisInheritsFrom; // for variables, if set, any changes to the parent will be reflected in this object.

    GameObject* m_pParentGameObject;
    CPPListHead m_ChildList; // Child game objects, contains the only copy of pointers to each child.

    ComponentGameObjectProperties m_Properties;

    bool m_Enabled;
    bool m_IsFolder;
    SceneID m_SceneID;
    unsigned int m_ID;
    SceneID m_PhysicsSceneID; // can't be SCENEID_Unmanaged (runtime scene) if a collision component exists
    char* m_Name; // this a copy of the string passed in.
    ComponentObjectPool* m_pOriginatingPool; // If this isn't 0, this object came from a pool and should be returned to it.
    bool m_Managed;

    CPPListHead m_pOnDeleteCallbacks;

    ComponentTransform* m_pComponentTransform;
    MyList<ComponentBase*> m_Components; // component system manager is responsible for deleting these components.

protected:
    void NotifyOthersThisWasDeleted();

public:
    GameObject(bool managed, SceneID sceneid, bool isfolder, bool hastransform, PrefabReference* pPrefabRef);
    virtual ~GameObject();
    SetClassnameBase( "GameObject" ); // only first 8 character count.

    PrefabReference* GetPrefabRef() { return &m_PrefabRef; }
    bool IsPrefabInstance() { return m_PrefabRef.GetPrefab() != 0; }

    bool IsFolder() { return m_IsFolder; }
    GameObject* GetGameObjectThisInheritsFrom() { return m_pGameObjectThisInheritsFrom; }
    CPPListHead* GetChildList() { return &m_ChildList; }
    GameObject* GetFirstChild() { return (GameObject*)m_ChildList.GetHead(); }
    void SetGameObjectThisInheritsFrom(GameObject* pObj);

    // Parent gameobject, is terms of transform (should also work with folders/gameobject without transforms)
    GameObject* GetParentGameObject() { return m_pParentGameObject; }

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    cJSON* ExportAsJSONObject(bool savesceneid);
    void ImportFromJSONObject(cJSON* jGameObject, SceneID sceneid);
    void ImportInheritanceInfoFromJSONObject(cJSON* jGameObject);
    cJSON* ExportReferenceAsJSONObject(SceneID refsceneid);
    cJSON* ExportAsJSONPrefab(PrefabObject* pPrefab, bool assignNewChildIDs, bool assignNewComponentIDs);

    void SetFlags(unsigned int flags) { return m_Properties.SetFlags( flags ); }
    unsigned int GetFlags() { return m_Properties.GetFlags(); }
    ComponentGameObjectProperties* GetPropertiesComponent() { return &m_Properties; }

    void SetEnabled(bool enabled, bool affectchildren);
    void RegisterAllComponentCallbacks(bool ignoreenabledflag);
    void UnregisterAllComponentCallbacks(bool ignoreenabledflag);
    void SetSceneID(SceneID sceneid, bool assignnewgoid = true);
    void SetID(unsigned int id);
    void SetName(const char* name);
    void SetOriginatingPool(ComponentObjectPool* pPool);

    void SetParentGameObject(GameObject* pNewParentGameObject);
    bool IsParentedTo(GameObject* pPotentialParent, bool onlycheckdirectparent);

    void SetManaged(bool managed);
    bool IsManaged() { return m_Managed; }

    bool IsEnabled() { return m_Enabled; }
    SceneID GetSceneID() { return m_SceneID; }
    SceneInfo* GetSceneInfo() { return g_pComponentSystemManager->GetSceneInfo( m_SceneID ); }
    unsigned int GetID() { return m_ID; }
    const char* GetName() { return m_Name; }

    void SetPhysicsSceneID(SceneID id) { m_PhysicsSceneID = id; }
    SceneID GetPhysicsSceneID() { return m_PhysicsSceneID; }

    unsigned int GetComponentCountIncludingCore();
    ComponentBase* GetComponentByIndexIncludingCore(unsigned int index);

    unsigned int GetComponentCount();
    ComponentBase* GetComponentByIndex(unsigned int index);

    ComponentBase* AddNewComponent(int componenttype, SceneID sceneid, ComponentSystemManager* pComponentSystemManager = g_pComponentSystemManager);
    ComponentBase* AddExistingComponent(ComponentBase* pComponent, bool resetcomponent);
    ComponentBase* RemoveComponent(ComponentBase* pComponent);

    ComponentBase* FindComponentByPrefabComponentID(unsigned int prefabComponentID);
    ComponentBase* FindComponentByID(unsigned int componentID);

    ComponentBase* GetFirstComponentOfBaseType(BaseComponentTypes basetype);
    ComponentBase* GetNextComponentOfBaseType(ComponentBase* pLastComponent);

    ComponentBase* GetFirstComponentOfType(const char* type);
    ComponentBase* GetNextComponentOfType(ComponentBase* pLastComponent);

    // Exposed to Lua, change elsewhere if function signature changes.
    ComponentTransform* GetTransform() { return m_pComponentTransform; }

    // TODO: find a way to find an arbitrary component type that would be accessible from lua script.
    // Exposed to Lua, change elsewhere if function signature changes.
    ComponentAnimationPlayer* GetAnimationPlayer()      { return (ComponentAnimationPlayer*)GetFirstComponentOfType( "AnimPlayerComponent" ); }
    Component3DCollisionObject* Get3DCollisionObject()  { return (Component3DCollisionObject*)GetFirstComponentOfType( "3DCollisionObjectComponent" ); }
    Component2DCollisionObject* Get2DCollisionObject()  { return (Component2DCollisionObject*)GetFirstComponentOfType( "2DCollisionObjectComponent" ); }
    ComponentParticleEmitter* GetParticleEmitter()      { return (ComponentParticleEmitter*)GetFirstComponentOfType( "ParticleEmitterComponent" ); }
    ComponentVoxelWorld* GetVoxelWorld()                { return (ComponentVoxelWorld*)GetFirstComponentOfType( "VoxelWorldComponent" ); }
    ComponentAudioPlayer* GetAudioPlayer()              { return (ComponentAudioPlayer*)GetFirstComponentOfType( "AudioPlayerComponent" ); }
    ComponentObjectPool* GetObjectPool()                { return (ComponentObjectPool*)GetFirstComponentOfType( "ObjectPoolComponent" ); }

    MaterialDefinition* GetMaterial();
    void SetMaterial(MaterialDefinition* pMaterial);

    void SetScriptFile(MyFileObject* pFile);

    void ReturnToPool();

    // Callbacks
    void RegisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback);
    void UnregisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback);

    static void StaticOnGameObjectDeleted(void* pObjectPtr, GameObject* pGameObject) { ((GameObject*)pObjectPtr)->OnGameObjectDeleted( pGameObject ); }
    void OnGameObjectDeleted(GameObject* pGameObject);

    static void StaticOnTransformChanged(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor) { ((GameObject*)pObjectPtr)->OnTransformChanged( newpos, newrot, newscale, changedbyuserineditor ); }
    void OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor);

public:
#if MYFW_EDITOR
    enum GameObjectOnDropActions
    {
        GameObjectOnDropAction_Default,
        GameObjectOnDropAction_Reorder,
        GameObjectOnDropAction_Reparent,
    };

    enum RightClickOptions
    {
        RightClick_DuplicateGameObject = 1000,
        RightClick_CreateChild,
        RightClick_ClearParent,
        RightClick_CreatePrefab,
            // Next 10000 values reserved for prefab scenes that are open. 10000 also hardcoded in cpp file
        RightClick_DeleteGameObject = RightClick_CreatePrefab + 10000,
        RightClick_DeleteFolder,
        RightClick_DuplicateFolder,
        RightClick_AdditionalSceneHandlerOptions,
            // Next 100000 value reserved for additional options added by SceneHandler
        RightClick_EndOfAdditionalSceneHandlerOptions = RightClick_AdditionalSceneHandlerOptions + 100000,
    };

protected:
    std::vector<uint32> m_DeletedPrefabChildIDs;
    std::vector<uint32> m_DeletedPrefabComponentIDs;

    bool IsMissingPrefabChild(uint32 childID);
    void AddPrefabChildIDToListOfDeletedPrefabChildIDs(uint32 childID);
    void RemovePrefabChildIDFromListOfDeletedPrefabChildIDs(uint32 childID);
    void AddPrefabComponentIDToListOfDeletedPrefabComponentIDs(uint32 componentID);
    void RemovePrefabComponentIDFromListOfDeletedPrefabComponentIDs(uint32 componentID);

public:
    void OnPopupClick(GameObject* pGameObject, unsigned int id);

#if MYFW_USING_WX
    static void StaticOnTitleLabelClicked(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging) { ((GameObject*)pObjectPtr)->OnTitleLabelClicked( controlid, finishedchanging ); }
    void OnTitleLabelClicked(int controlid, bool finishedchanging);

    //static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((GameObject*)pObjectPtr)->OnLeftClick( count, true ); }
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { GameObject::OnLeftClick( count, true ); }
    static void OnLeftClick(unsigned int count, bool clear);
    void ShowInWatchPanel(bool isprefab);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((GameObject*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    void AddPrefabSubmenusToMenu(wxMenu* menu, int itemidoffset);
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((GameObject*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, int x, int y) { ((GameObject*)pObjectPtr)->OnDrop(controlid, x, y, GameObjectOnDropAction_Default); }
#endif //MYFW_USING_WX
    void OnDrop(int controlid, int x, int y, GameObjectOnDropActions action);
#if MYFW_USING_WX

    static void StaticOnLabelEdit(void* pObjectPtr, wxTreeItemId id, wxString newlabel) { ((GameObject*)pObjectPtr)->OnLabelEdit( newlabel ); }
    void OnLabelEdit(wxString newlabel);

    void UpdateObjectListIcon();
#endif //MYFW_USING_WX

    // Prefab Loading Vars // Variables moved into prefabref object.
    void FinishLoadingPrefab(PrefabFile* pPrefabFile);
    static void StaticOnPrefabFileFinishedLoading(void* pObjectPtr, MyFileObject* pFile) { ((GameObject*)pObjectPtr)->OnPrefabFileFinishedLoading( pFile ); }
    void OnPrefabFileFinishedLoading(MyFileObject* pFile);

    GameObject* FindRootGameObjectOfPrefabInstance();

    void Editor_SetPrefab(PrefabReference* pPrefabRef); // Used when deleting prefabs.
    void Editor_SetGameObjectAndAllChildrenToInheritFromPrefab(PrefabObject* pPrefab, uint32 prefabChildID);
    void Editor_SetGameObjectThisInheritsFromIgnoringPrefabRef(GameObject* pObj);
    void Editor_SetMaterial(MaterialDefinition* pMaterial);

    // Editor functions
    void AddToList(std::vector<GameObject*>* pList);
#endif //MYFW_EDITOR
};

#endif //__GameObject_H__
