//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
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
    static const int MAX_COMPONENTS = 4; // TODO: fix this hardcodedness

protected:
    GameObject* m_pGameObjectThisInheritsFrom; // if set, any changes to the parent will be reflected in this object.
    CPPListHead m_Children; // child game object

    bool m_Enabled;
    unsigned int m_SceneID; // 0 for runtime generated.
    unsigned int m_ID;
    char* m_Name; // this a copy of the string passed in.
    bool m_Managed;

    CPPListHead m_pOnDeleteCallbacks;

protected:
    void NotifyOthersThisWasDeleted();

public:
    ComponentTransform* m_pComponentTransform;
    MyList<ComponentBase*> m_Components; // component system manager is responsible for deleting these components.

public:
    GameObject(bool managed, int sceneid);
    virtual ~GameObject();
    SetClassnameBase( "GameObject" ); // only first 8 character count.

    GameObject* GetGameObjectThisInheritsFrom() { return m_pGameObjectThisInheritsFrom; }
    CPPListHead* GetChildren() { return &m_Children; }
    void SetGameObjectThisInheritsFrom(GameObject* pObj) { m_pGameObjectThisInheritsFrom = pObj; }

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    cJSON* ExportAsJSONObject(bool savesceneid);
    void ImportFromJSONObject(cJSON* jGameObject, unsigned int sceneid);
    cJSON* ExportReferenceAsJSONObject(unsigned int refsceneid);

    void SetEnabled(bool enabled);
    void RegisterAllComponentCallbacks(bool ignoreenabledflag);
    void UnregisterAllComponentCallbacks(bool ignoreenabledflag);
    void SetSceneID(unsigned int sceneid);
    void SetID(unsigned int id);
    void SetName(const char* name);
    void SetParentGameObject(GameObject* pGameObject);
    void SetManaged(bool managed);
    bool IsManaged() { return m_Managed; }

    bool IsEnabled() { return m_Enabled; }
    unsigned int GetSceneID() { return m_SceneID; }
    SceneInfo* GetSceneInfo() { return g_pComponentSystemManager->GetSceneInfo( m_SceneID ); }
    unsigned int GetID() { return m_ID; }
    const char* GetName() { return m_Name; }

    ComponentBase* AddNewComponent(int componenttype, unsigned int sceneid, ComponentSystemManager* pComponentSystemManager = g_pComponentSystemManager);
    ComponentBase* AddExistingComponent(ComponentBase* pComponent, bool resetcomponent);
    ComponentBase* RemoveComponent(ComponentBase* pComponent);

    ComponentBase* GetFirstComponentOfBaseType(BaseComponentTypes basetype);
    ComponentBase* GetNextComponentOfBaseType(ComponentBase* pLastComponent);

    ComponentBase* GetFirstComponentOfType(const char* type);
    ComponentBase* GetNextComponentOfType(ComponentBase* pLastComponent);

    ComponentTransform* GetTransform() { return m_pComponentTransform; }
    ComponentCollisionObject* GetCollisionObject();

    // TODO: find a way to find an arbitrary component type that would be accessible from lua script.
    ComponentAnimationPlayer* GetAnimationPlayer();

    MaterialDefinition* GetMaterial();
    void SetMaterial(MaterialDefinition* pMaterial);

    void SetScriptFile(MyFileObject* pFile);

    // Callbacks
    void RegisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback);
    void UnregisterOnDeleteCallback(void* pObj, GameObjectDeletedCallbackFunc pCallback);

    static void StaticOnTransformPositionChanged(void* pObjectPtr, Vector3& newpos, bool changedbyeditor) { ((GameObject*)pObjectPtr)->OnTransformPositionChanged( newpos, changedbyeditor ); }
    void OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor);

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_DuplicateGameObject = 1000,
        RightClick_CreateChild,
        RightClick_ClearParent,
        RightClick_DeleteGameObject,
    };

    static void StaticOnTitleLabelClicked(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((GameObject*)pObjectPtr)->OnTitleLabelClicked( controlid, finishedchanging ); }
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
#endif //MYFW_USING_WX
};

#endif //__GameObject_H__
