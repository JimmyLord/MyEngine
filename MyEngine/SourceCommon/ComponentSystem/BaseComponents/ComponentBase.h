//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentBase_H__
#define __ComponentBase_H__

class GameObject;

enum BaseComponentTypes
{
    BaseComponentType_Data,
    BaseComponentType_Camera,
    BaseComponentType_InputHandler,
    BaseComponentType_Updateable,
    BaseComponentType_Renderable,
    BaseComponentType_MenuPage, // not crazy about approach, but will handle input/update/render.
    BaseComponentType_None,
    BaseComponentType_NumTypes = BaseComponentType_None
};

class ComponentVariable;

class ComponentBase : public CPPListNode
#if MYFW_USING_WX
, public wxEvtHandler
#endif
{
protected:
    bool m_Enabled;
    unsigned int m_SceneIDLoadedFrom; // 0 for runtime generated.
    unsigned int m_ID;

    // an unsigned int of all divorced components variables, only maintained in editor builds.
    unsigned int m_DivorcedVariables; // moved outside USING_WX block to allow load/save in game mode.

public:
    BaseComponentTypes m_BaseType;
    int m_Type;
    GameObject* m_pGameObject;

public:
    ComponentBase();
    virtual ~ComponentBase();
    SetClassnameBase( "BaseComponent" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);
    virtual cJSON* ExportReferenceAsJSONObject();

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentBase&)*pObject; }
    ComponentBase& operator=(const ComponentBase& other);

    bool m_CallbacksRegistered;
    virtual void RegisterCallbacks() {}
    virtual void UnregisterCallbacks() {}

    virtual void OnLoad();
    virtual void OnPlay() {}
    virtual void OnStop() {}

    virtual bool OnEvent(MyEvent* pEvent);

    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();
    virtual void SetEnabled(bool enabled);
    void SetSceneID(unsigned int sceneid) { m_SceneIDLoadedFrom = sceneid; }
    void SetID(unsigned int id) { m_ID = id; }

    bool IsEnabled() { return m_Enabled; }
    unsigned int GetSceneID() { return m_SceneIDLoadedFrom; }
    SceneInfo* GetSceneInfo();
    unsigned int GetID() { return m_ID; }

    // pre-DrawCallback functions
    virtual bool IsVisible() { return true; }
    virtual bool ExistsOnLayer(unsigned int layerflags) { return true; }

protected:
    static void ClearAllVariables_Base(CPPListHead* pComponentVariableList);
    static ComponentVariable* AddVariable_Base(CPPListHead* pComponentVariableList, const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);
    static ComponentVariable* AddVariablePointer_Base(CPPListHead* pComponentVariableList, const char* label, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_GetPointerValue pGetPointerValueCallBackFunc, CVarFunc_SetPointerValue pSetPointerValueCallBackFunc, CVarFunc_GetPointerDesc pGetPointerDescCallBackFunc, CVarFunc_SetPointerDesc pSetPointerDescCallBackFunc, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);
    static ComponentVariable* AddVariableEnum_Base(CPPListHead* pComponentVariableList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);
    static ComponentVariable* AddVariableFlags_Base(CPPListHead* pComponentVariableList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc);
    virtual CPPListHead* GetComponentVariableList() { /*MyAssert( false );*/ return 0; } // = 0; TODO: make this pure virual once MYFW_COMPONENT_DECLARE_VARIABLE_LIST and MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST are in each component
#if MYFW_USING_WX
    void FillPropertiesWindowWithVariables();
    bool DoAllMultiSelectedVariabledHaveTheSameValue(ComponentVariable* pVar);
    void AddVariableToPropertiesWindow(ComponentVariable* pVar);
    int FindVariablesControlIDByLabel(const char* label);
#endif
    void ExportVariablesToJSON(cJSON* jComponent);
    void ImportVariablesFromJSON(cJSON* jsonobj, const char* singlelabeltoimport = 0);

#if MYFW_USING_WX
public:
    enum RightClickOptions
    {
        RightClick_DivorceVariable = 1000,
        RightClick_MarryVariable,
    };

    //virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar) { return true; }
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // an array of all components of this type selected (when more than 1 is selected)
    std::vector<ComponentBase*> m_MultiSelectedComponents;

    // an unsigned int of all divorced components variables, only maintained in editor builds.
    //unsigned int m_DivorcedVariables; // moved outside USING_WX block to allow load/save in game mode.
    bool IsDivorced(int index);
    void SetDivorced(int index, bool divorced);
    bool DoesVariableMatchParent(ComponentVariable* pVar, int controlcomponent);
    void SyncUndivorcedVariables(ComponentBase* pSourceComponent);
    void SyncVariable(ComponentBase* pChildComponent, ComponentVariable* pVar);
    void SyncVariableInChildren(ComponentVariable* pVar);
    void SyncVariableInChildrenInGameObjectListWithNewValue(GameObject* first, ComponentVariable* pVar);
    void SyncVariableInGameObjectWithNewValue(GameObject* pGameObject, ComponentVariable* pVar);

    ComponentBase* FindMatchingComponentInParent();

    // Watch panel callbacks for component variables.
    // if any variables value changed, then react.
    static void StaticOnValueChangedVariable(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue) { ((ComponentBase*)pObjectPtr)->OnValueChangedVariable( controlid, directlychanged, finishedchanging, oldvalue ); }
    void OnValueChangedVariable(int controlid, bool directlychanged, bool finishedchanging, double oldvalue);

    static void StaticOnDropVariable(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentBase*)pObjectPtr)->OnDropVariable(controlid, x, y); }
    void OnDropVariable(int controlid, wxCoord x, wxCoord y);
    void OnDropVariable(ComponentVariable* pVar, int controlcomponent, wxCoord x, wxCoord y);

    ComponentBaseEventHandlerForComponentVariables m_ComponentBaseEventHandlerForComponentVariables;
    static void StaticOnRightClickVariable(void* pObjectPtr, int controlid) { ((ComponentBase*)pObjectPtr)->OnRightClickVariable(controlid); }
    void OnRightClickVariable(int controlid);

    ComponentVariable* FindComponentVariableForControl(int controlid);
    static ComponentVariable* FindComponentVariableByLabel(CPPListHead* list, const char* label);
    void UpdateChildrenWithNewValue(bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void UpdateChildrenInGameObjectListWithNewValue(GameObject* first, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void UpdateGameObjectWithNewValue(GameObject* pGameObject, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void UpdateOtherComponentWithNewValue(ComponentBase* pComponent, bool directlychanged, bool ignoreDivorceStatus, bool fromdraganddrop, ComponentVariable* pVar, int controlcomponent, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void CopyValueFromParent(ComponentVariable* pVar);

    // to show/hide the components controls in watch panel
    //static bool m_PanelWatchBlockVisible; // each class needs it's own static bool, so if one component of this type is off, they all are.
    bool* m_pPanelWatchBlockVisible; // pointer to the bool above, must be set by each component.
    int m_ControlID_ComponentTitleLabel;
    static void StaticOnComponentTitleLabelClicked(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue) { ((ComponentBase*)pObjectPtr)->OnComponentTitleLabelClicked( controlid, finishedchanging ); }
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

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, wxCoord x, wxCoord y) { ((ComponentBase*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);

    // Scene right-click options
    virtual void AddRightClickOptionsToMenu(wxMenu* pMenu, int baseid) {}
    virtual void OnRightClickOptionClicked(wxEvent &evt, int baseid) {}
#endif //MYFW_USING_WX
};

#endif //__ComponentBase_H__
