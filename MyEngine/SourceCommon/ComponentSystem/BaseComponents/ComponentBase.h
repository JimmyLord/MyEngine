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

//#if !MYFW_USING_WX
//#define wxCoord float
//#define wxMenu void
//#endif //MYFW_USING_WX
//
//typedef void (ComponentBase::*CVarFunc)(ComponentVariable* pVar);
//typedef void (ComponentBase::*CVarFunc_Int)(ComponentVariable* pVar, int someint);
//typedef void (ComponentBase::*CVarFunc_wxMenu)(ComponentVariable* pVar, wxMenu* pMenu);
//typedef void* (ComponentBase::*CVarFunc_DropTarget)(ComponentVariable* pVar, wxCoord x, wxCoord y);
//typedef void* (ComponentBase::*CVarFunc_ValueChanged)(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue, void* newpointer);
//typedef void* (ComponentBase::*CVarFunc_Pointer)(ComponentVariable* pVar);
//
//typedef void* (ComponentBase::*CVarFunc_GetPointerValue)(ComponentVariable* pVar);
//typedef void (ComponentBase::*CVarFunc_SetPointerValue)(ComponentVariable* pVar, void* newvalue);
//typedef const char* (ComponentBase::*CVarFunc_GetPointerDesc)(ComponentVariable* pVar);
//typedef void (ComponentBase::*CVarFunc_SetPointerDesc)(ComponentVariable* pVar, const char* newdesc);
//
//typedef bool (ComponentBase::*CVarFunc_ShouldVariableBeAdded)(ComponentVariable* pVar);

//#if MYFW_USING_WX
//#define AddVar(pList,label,type,offset,saveload,displayinwatch,watchlabel, onvaluechanged,ondrop,onbuttonpressed) AddVariable(pList,label,type,offset,saveload,displayinwatch,watchlabel, onvaluechanged,ondrop,onbuttonpressed);
//#define AddVarPointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, onvaluechanged,ondrop,onbuttonpressed) AddVariablePointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, onvaluechanged,ondrop,onbuttonpressed);
//#define AddVarEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed) AddVariableEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed);
//#define AddVarFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed) AddVariableFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed);
//#else
//#define AddVar(pList,label,type,offset,saveload,displayinwatch,watchlabel, ...) AddVariable(pList,label,type,offset,saveload,displayinwatch,watchlabel, 0,0,0);
//#define AddVarPointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, ...) AddVariablePointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, 0,0,0);
//#define AddVarEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, ...) AddVariableEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, 0,0,0);
//#define AddVarFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, ...) AddVariableFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, 0,0,0);
//#endif
//
//#define MYFW_COMPONENT_DECLARE_VARIABLE_LIST(ComponentName) \
//    static CPPListHead m_ComponentVariableList_##ComponentName; /* ComponentVariable type */ \
//    static int m_ComponentVariableListRefCount_##ComponentName; \
//    static void RegisterVariables(CPPListHead* pList, ComponentName* pThis); \
//    static void ClearAllVariables() { m_ComponentVariableListRefCount_##ComponentName--; if( m_ComponentVariableListRefCount_##ComponentName == 0 ) ClearAllVariables_Base( &m_ComponentVariableList_##ComponentName ); } \
//    static ComponentVariable* AddVariable(CPPListHead* pList, const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
//    { return AddVariable_Base( pList, label, type, offset, saveload, displayinwatch, watchlabel, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
//    static ComponentVariable* AddVariablePointer(CPPListHead* pList, const char* label, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_GetPointerValue pGetPointerValueCallBackFunc, CVarFunc_SetPointerValue pSetPointerValueCallBackFunc, CVarFunc_GetPointerDesc pGetPointerDescCallBackFunc, CVarFunc_SetPointerDesc pSetPointerDescCallBackFunc, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
//    { return AddVariablePointer_Base( pList, label, saveload, displayinwatch, watchlabel, pGetPointerValueCallBackFunc, pSetPointerValueCallBackFunc, pGetPointerDescCallBackFunc, pSetPointerDescCallBackFunc, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
//    static ComponentVariable* AddVariableEnum(CPPListHead* pList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
//    { return AddVariableEnum_Base( pList, label, offset, saveload, displayinwatch, watchlabel, numenums, ppStrings, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
//    static ComponentVariable* AddVariableFlags(CPPListHead* pList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
//    { return AddVariableFlags_Base( pList, label, offset, saveload, displayinwatch, watchlabel, numenums, ppStrings, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
//    static bool ComponentVariablesHaveBeenRegistered() \
//    { \
//        if( m_ComponentVariableList_##ComponentName.GetHead() == 0 ) m_ComponentVariableListRefCount_##ComponentName = 0; \
//        m_ComponentVariableListRefCount_##ComponentName++; \
//        return (m_ComponentVariableListRefCount_##ComponentName != 1); \
//    } \
//    virtual CPPListHead* GetComponentVariableList() { return &m_ComponentVariableList_##ComponentName; }
//
//#define MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST(ComponentName) \
//    CPPListHead ComponentName::m_ComponentVariableList_##ComponentName; \
//    int ComponentName::m_ComponentVariableListRefCount_##ComponentName;
//
//#define MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR() \
//    if( ComponentVariablesHaveBeenRegistered() == false ) RegisterVariables( GetComponentVariableList(), this );
//
//#define MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR() \
//    ClearAllVariables();
//
//#if MYFW_USING_WX
//class ComponentBaseEventHandlerForComponentVariables : public wxEvtHandler
//{
//public:
//    ComponentBase* pComponent;
//    ComponentVariable* pVar;
//
//public:
//    ComponentBaseEventHandlerForComponentVariables()
//    {
//        pComponent = 0;
//        pVar = 0;
//    };
//    void OnPopupClick(wxEvent &evt);
//};
//#endif

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
    bool DoesVariableMatchParent(int controlid, ComponentVariable* pVar);
    void SyncUndivorcedVariables(ComponentBase* pSourceComponent);
    void SyncVariable(ComponentBase* pChildComponent, ComponentVariable* pVar);
    void SyncVariableInChildren(ComponentVariable* pVar);
    void SyncVariableInChildrenInGameObjectListWithNewValue(GameObject* first, ComponentVariable* pVar);
    void SyncVariableInGameObjectWithNewValue(GameObject* pGameObject, ComponentVariable* pVar);

    // Watch panel callbacks for component variables.
    // if any variables value changed, then react.
    static void StaticOnValueChangedVariable(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentBase*)pObjectPtr)->OnValueChangedVariable( controlid, finishedchanging, oldvalue ); }
    void OnValueChangedVariable(int controlid, bool finishedchanging, double oldvalue);

    static void StaticOnDropVariable(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentBase*)pObjectPtr)->OnDropVariable(controlid, x, y); }
    void OnDropVariable(int controlid, wxCoord x, wxCoord y);

    ComponentBaseEventHandlerForComponentVariables m_ComponentBaseEventHandlerForComponentVariables;
    static void StaticOnRightClickVariable(void* pObjectPtr, int controlid) { ((ComponentBase*)pObjectPtr)->OnRightClickVariable(controlid); }
    void OnRightClickVariable(int controlid);

    ComponentVariable* FindComponentVariableForControl(int controlid);
    static ComponentVariable* FindComponentVariableByLabel(CPPListHead* list, const char* label);
    void UpdateChildrenWithNewValue(bool fromdraganddrop, ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void UpdateChildrenInGameObjectListWithNewValue(GameObject* first, bool fromdraganddrop, ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void UpdateGameObjectWithNewValue(GameObject* pGameObject, bool fromdraganddrop, ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void UpdateOtherComponentWithNewValue(ComponentBase* pComponent, bool ignoreDivorceStatus, bool fromdraganddrop, ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue, void* oldpointer, wxCoord x, wxCoord y, void* newpointer);
    void CopyValueFromParent(ComponentVariable* pVar);

    // to show/hide the components controls in watch panel
    //static bool m_PanelWatchBlockVisible; // each class needs it's own static bool, so if one component of this type is off, they all are.
    bool* m_pPanelWatchBlockVisible; // pointer to the bool above, must be set by each component.
    int m_ControlID_ComponentTitleLabel;
    static void StaticOnComponentTitleLabelClicked(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentBase*)pObjectPtr)->OnComponentTitleLabelClicked( controlid, finishedchanging ); }
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
