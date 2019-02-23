//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentBase_H__
#define __ComponentBase_H__

#include "ComponentVariable.h"
#include "ComponentVariableValue.h"

class ComponentBase;
class GameObject;
class SceneInfo;

enum BaseComponentTypes
{
    BaseComponentType_Data,
    BaseComponentType_Camera,
    BaseComponentType_InputHandler,
    BaseComponentType_Updateable,
    BaseComponentType_Renderable,
    BaseComponentType_MenuPage, // Not crazy about approach, but will handle input/update/render.
    BaseComponentType_None,
    BaseComponentType_NumTypes = BaseComponentType_None
};

class ComponentVariable;

typedef void ComponentDeletedCallbackFunc(void* pObjectPtr, ComponentBase* pComponent);
struct ComponentDeletedCallbackStruct : CPPListNode
{
    void* pObj;
    ComponentDeletedCallbackFunc* pFunc;
};

class ComponentBase : public CPPListNode
#if MYFW_USING_WX
, public wxEvtHandler
#endif
{
public:
    enum EnabledState
    {
        EnabledState_Disabled_ManuallyDisabled,
        EnabledState_Disabled_EnableWithGameObject,
        EnabledState_Enabled,
    };

protected:
    EnabledState m_EnabledState;
    SceneID m_SceneIDLoadedFrom;
    unsigned int m_ID; // Unique ID within a scene, used when quick-loading scene to find matching component.

    // An unsigned int of all divorced components variables, only maintained in editor builds.
    unsigned int m_DivorcedVariables; // Moved outside USING_EDITOR block to allow load/save in game mode.

    BaseComponentTypes m_BaseType;
    int m_Type;
    GameObject* m_pGameObject;

    CPPListHead m_pOnDeleteCallbacks;

    bool m_CallbacksRegistered;

protected:
    void NotifyOthersThisWasDeleted();

    virtual void RegisterCallbacks() {}
    virtual void UnregisterCallbacks() {}

public:
    ComponentBase();
    virtual ~ComponentBase();
    SetClassnameBase( "BaseComponent" ); // Only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool saveSceneID, bool saveID);
    virtual void ImportFromJSONObject(cJSON* jComponent, SceneID sceneID);
    virtual void FinishImportingFromJSONObject(cJSON* jComponent);
    virtual cJSON* ExportReferenceAsJSONObject() const;

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentBase&)*pObject; }
    ComponentBase& operator=(const ComponentBase& other);

public:
    virtual void OnLoad();
    virtual void OnPlay() {}
    virtual void OnStop() {}

    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();

    // Getters.
    BaseComponentTypes GetBaseType() { return m_BaseType; }
    int GetType() { return m_Type; }
    GameObject* GetGameObject() { return m_pGameObject; }
    bool IsEnabled() { return m_EnabledState == EnabledState_Enabled; }
    EnabledState GetEnabledState() { return m_EnabledState; }
    SceneID GetSceneID() const { return m_SceneIDLoadedFrom; }
    SceneInfo* GetSceneInfo();
    unsigned int GetID() { return m_ID; }

    // Setters.
    void SetType(int type) { m_Type = type; }
    void SetGameObject(GameObject* object) { m_pGameObject = object; }
    virtual bool SetEnabled(bool enableComponent); // Returns if state changed.
    void SetSceneID(SceneID sceneid) { m_SceneIDLoadedFrom = sceneid; }
    void SetID(unsigned int id) { m_ID = id; }

    // pre-DrawCallback functions.
    virtual bool IsVisible() { return true; }
    virtual bool ExistsOnLayer(unsigned int layerflags) { return true; }

    // Callbacks.
    void RegisterOnDeleteCallback(void* pObj, ComponentDeletedCallbackFunc* pCallback);
    void UnregisterOnDeleteCallback(void* pObj, ComponentDeletedCallbackFunc* pCallback);

public:
    static void ClearAllVariables_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList);
    static ComponentVariable* AddVariable_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);
    static ComponentVariable* AddVariablePointer_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_GetPointerValue pGetPointerValueCallBackFunc, CVarFunc_SetPointerValue pSetPointerValueCallBackFunc, CVarFunc_GetPointerDesc pGetPointerDescCallBackFunc, CVarFunc_SetPointerDesc pSetPointerDescCallBackFunc, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);
    static ComponentVariable* AddVariableEnum_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);
    static ComponentVariable* AddVariableFlags_Base(TCPPListHead<ComponentVariable*>* pComponentVariableList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);

    static void AddVariableToWatchPanel(void* pObject, ComponentVariable* pVar, ComponentBase* pComponent);
protected:
    virtual TCPPListHead<ComponentVariable*>* GetComponentVariableList() { /*MyAssert( false );*/ return 0; } // = 0; TODO: Make this pure virtual once MYFW_COMPONENT_DECLARE_VARIABLE_LIST and MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST are in each component.
#if MYFW_USING_WX
    void FillPropertiesWindowWithVariables();
#endif
    bool DoAllMultiSelectedVariabledHaveTheSameValue(ComponentVariable* pVar);
#if MYFW_USING_WX
    void AddVariableToPropertiesWindow(ComponentVariable* pVar);
    int FindVariablesControlIDByLabel(const char* label);
#endif
#if MYFW_USING_IMGUI
public:
    //virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false) {};
    virtual void AddAllVariablesToWatchPanel();
protected:
    ComponentVariableValue m_ComponentVariableValueWhenControlSelected;
    ImGuiID m_ImGuiControlIDForCurrentlySelectedVariable;
    bool m_LinkNextUndoCommandToPrevious;

    void TestForVariableModificationAndCreateUndoCommand(ImGuiID id, bool modified, ComponentVariable* pVar);
#endif
    void ExportVariablesToJSON(cJSON* jComponent);
    void ImportVariablesFromJSON(cJSON* jsonobj, const char* singlelabeltoimport = 0);

#if MYFW_EDITOR
protected:
    // Unique to a single gameobject on a single prefab, used to match components between gameobjects and their prefabs.
    // Set to 0 if GameObject has no parent/prefab or if this component doesn't exist on the parent/prefab.
    unsigned int m_PrefabComponentID;

public:
    // An array of all components of this type selected (when more than 1 is selected).
    std::vector<ComponentBase*> m_MultiSelectedComponents;

public:
    enum RightClickOptions
    {
        RightClick_DeleteComponent = 1000,
    };

    enum WatchPanelRightClickOptions
    {
        RightClick_DivorceVariable = 1000,
        RightClick_MarryVariable,
    };

    //virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar) { return true; }
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
#endif

    bool IsDivorced(int index);
    void SetDivorced(int index, bool divorced);
    bool DoesVariableMatchParent(ComponentVariable* pVar, int controlcomponent);
    void SyncUndivorcedVariables(ComponentBase* pSourceComponent);
    void SyncVariable(ComponentBase* pChildComponent, ComponentVariable* pVar);
    void SyncVariableInChildren(ComponentVariable* pVar);
    void SyncVariableInChildrenInGameObjectListWithNewValue(GameObject* pFirstGameObject, ComponentVariable* pVar);
    void SyncVariableInGameObjectWithNewValue(GameObject* pGameObject, ComponentVariable* pVar);

    void SetPrefabComponentID(unsigned int id) { m_PrefabComponentID = id; }
    unsigned int GetPrefabComponentID() { return m_PrefabComponentID; }
    ComponentBase* FindMatchingComponentInParent();

    // Watch panel callbacks for component variables.
    // If any variables value changed, then react.
    static void StaticOnValueChangedVariable(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging) { ((ComponentBase*)pObjectPtr)->OnValueChangedVariable( controlid, directlychanged, finishedchanging, oldvalue, valuewaschangedbydragging, 0 ); }
    void OnValueChangedVariable(int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging, ComponentVariableValue* pNewValue);
    void OnValueChangedVariable(ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging, ComponentVariableValue* pNewValue);

    static ComponentVariable* FindComponentVariableByLabel(TCPPListHead<ComponentVariable*>* list, const char* label);

#if MYFW_USING_WX
    static void StaticOnDropVariable(void* pObjectPtr, int controlid, int x, int y) { ((ComponentBase*)pObjectPtr)->OnDropVariable(controlid, x, y); }
    void OnDropVariable(int controlid, int x, int y);
#endif //MYFW_USING_WX
    void* OnDropVariable(ComponentVariable* pVar, int controlcomponent, int x, int y, bool addundocommand = false);

#if MYFW_USING_WX
    ComponentBaseEventHandlerForComponentVariables m_ComponentBaseEventHandlerForComponentVariables;
    static void StaticOnRightClickVariable(void* pObjectPtr, int controlid) { ((ComponentBase*)pObjectPtr)->OnRightClickVariable(controlid); }
    void OnRightClickVariable(int controlid);

    ComponentVariable* FindComponentVariableForControl(int controlid);
#endif //MYFW_USING_WX

    double GetCurrentValueFromVariable(ComponentVariable* pVar, int controlcomponent);
    void ChangeValueInNonPointerVariable(ComponentVariable* pVar, int controlcomponent, bool addundocommand, double changetoapply, double changeforundo);

    // There are currently 2 versions of this, one for wx and one for imgui based editor.
    void CopyValueFromOtherComponent(ComponentVariable* pVar, int controlcomponent, ComponentBase* pOtherComponent, bool addundocommand);

    void UpdateChildrenWithNewValue(bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer);
    void UpdateChildrenInGameObjectListWithNewValue(GameObject* pFirstGameObject, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer);
    void UpdateGameObjectWithNewValue(GameObject* pGameObject, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool directlychanged, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer);
    void UpdateOtherComponentWithNewValue(ComponentBase* pComponent, bool directlychanged, bool ignoreDivorceStatus, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool finishedchanging, double oldvalue, void* oldpointer, int x, int y, void* newpointer);

#if MYFW_USING_WX
    // to show/hide the components controls in watch panel
    //static bool m_PanelWatchBlockVisible; // each class needs it's own static bool, so if one component of this type is off, they all are.
    bool* m_pPanelWatchBlockVisible; // pointer to the bool above, must be set by each component.
    int m_ControlID_ComponentTitleLabel;
    static void StaticOnComponentTitleLabelClicked(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging) { ((ComponentBase*)pObjectPtr)->OnComponentTitleLabelClicked( controlid, finishedchanging ); }
    void OnComponentTitleLabelClicked(int controlid, bool finishedchanging);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentBase*)pObjectPtr)->OnLeftClick( count, true ); }
    virtual void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false) {};

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((ComponentBase*)pObjectPtr)->OnRightClick(); }
    virtual void OnRightClick();
    virtual void AppendItemsToRightClickMenu(wxMenu* pMenu);
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((ComponentBase*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, int x, int y) { ((ComponentBase*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, int x, int y);

    // Scene right-click options
    virtual void AddRightClickOptionsToMenu(wxMenu* pMenu, int baseid) {}
    virtual void OnRightClickOptionClicked(wxEvent &evt, int baseid) {}
#endif //MYFW_USING_WX

    virtual bool IsReferencingFile(MyFileObject* pFile);
    void OnRightClickAction(int action);
#endif //MYFW_EDITOR
};

#endif //__ComponentBase_H__
