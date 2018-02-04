//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentVariable_H__
#define __ComponentVariable_H__

class ComponentBase;
class ComponentVariable;
class ComponentVariableValue;

enum ComponentVariableTypes
{
    ComponentVariableType_Int,
    ComponentVariableType_Enum,
    ComponentVariableType_Flags,
    ComponentVariableType_UnsignedInt,
    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,
    ComponentVariableType_Bool,
    ComponentVariableType_Float,
    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,
    ComponentVariableType_ColorByte,

    ComponentVariableType_Vector2,
    ComponentVariableType_Vector3,
    
    ComponentVariableType_Vector2Int,
    ComponentVariableType_Vector3Int,

    // NOTE: Pointer types must come after value types for ComponentVariableValue::CopyNonPointerValueIntoVariable() 
    ComponentVariableType_FirstPointerType,

    ComponentVariableType_GameObjectPtr = ComponentVariableType_FirstPointerType,
    ComponentVariableType_ComponentPtr,
    ComponentVariableType_FilePtr,
    ComponentVariableType_MaterialPtr,
    ComponentVariableType_SoundCuePtr,

    ComponentVariableType_PointerIndirect,

    ComponentVariableType_NumTypes,
};

#if !MYFW_USING_WX
#define wxCoord float
#define wxMenu void
#endif //MYFW_USING_WX

typedef void (ComponentBase::*CVarFunc)(ComponentVariable* pVar);
typedef void (ComponentBase::*CVarFunc_Int)(ComponentVariable* pVar, int someint);
typedef void (ComponentBase::*CVarFunc_wxMenu)(ComponentVariable* pVar, wxMenu* pMenu);
typedef void* (ComponentBase::*CVarFunc_DropTarget)(ComponentVariable* pVar, wxCoord x, wxCoord y);
typedef void* (ComponentBase::*CVarFunc_ValueChanged)(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue);
typedef void* (ComponentBase::*CVarFunc_Pointer)(ComponentVariable* pVar);

typedef void* (ComponentBase::*CVarFunc_GetPointerValue)(ComponentVariable* pVar);
typedef void (ComponentBase::*CVarFunc_SetPointerValue)(ComponentVariable* pVar, void* newvalue);
typedef const char* (ComponentBase::*CVarFunc_GetPointerDesc)(ComponentVariable* pVar);
typedef void (ComponentBase::*CVarFunc_SetPointerDesc)(ComponentVariable* pVar, const char* newdesc);

typedef bool (ComponentBase::*CVarFunc_ShouldVariableBeAdded)(ComponentVariable* pVar);
typedef void (ComponentBase::*CVarFunc_VariableAddedToInterface)(ComponentVariable* pVar);

class ComponentVariable : public CPPListNode
{
public:
    const char* m_Label;
    ComponentVariableTypes m_Type;
    size_t m_Offset; // offset into class of the variable.
    bool m_SaveLoad;
    bool m_DisplayInWatch;
    const char* m_WatchLabel; // if 0 will use m_Label if needed.
    int m_NumEnumStrings;
    const char** m_ppEnumStrings;

#if MYFW_EDITOR
    float m_FloatLowerLimit;
    float m_FloatUpperLimit;

    CVarFunc_DropTarget m_pOnDropCallbackFunc;
    CVarFunc m_pOnButtonPressedCallbackFunc;
    CVarFunc_ValueChanged m_pOnValueChangedCallbackFunc;

    CVarFunc_ShouldVariableBeAdded m_pShouldVariableBeAddedCallbackFunc;
    CVarFunc_VariableAddedToInterface m_pVariableAddedToInterfaceCallbackFunc;
    CVarFunc_wxMenu m_pOnRightClickCallbackFunc;
    CVarFunc_Int m_pOnPopupClickCallbackFunc;
#endif //MYFW_EDITOR

    CVarFunc_GetPointerValue m_pGetPointerValueCallBackFunc;
    CVarFunc_SetPointerValue m_pSetPointerValueCallBackFunc;
    CVarFunc_GetPointerDesc m_pGetPointerDescCallBackFunc;
    CVarFunc_SetPointerDesc m_pSetPointerDescCallBackFunc;

    int m_ControlID;
    int m_Index; // convenience, used when setting divorces status.

public:
    ComponentVariable(const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel,
        CVarFunc_GetPointerValue pGetPointerValueCallBackFunc, CVarFunc_SetPointerValue pSetPointerValueCallBackFunc,
        CVarFunc_GetPointerDesc pGetPointerDescCallBackFunc, CVarFunc_SetPointerDesc pSetPointerDescCallBackFunc,
        CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc );

#if MYFW_EDITOR
    void AddCallback_ShouldVariableBeAdded(CVarFunc_ShouldVariableBeAdded pFunc);
    void AddCallback_VariableAddedToInterface(CVarFunc_VariableAddedToInterface pFunc);
    void AddCallback_OnRightClick(CVarFunc_wxMenu pRightClickFunc, CVarFunc_Int pPopupClickFunc);

    void SetEditorLimits(float lowerlimit, float upperlimit);
#endif //MYFW_EDITOR
};

#if MYFW_EDITOR
#define AddVar(pList,label,type,offset,saveload,displayinwatch,watchlabel, onvaluechanged,ondrop,onbuttonpressed) AddVariable(pList,label,type,offset,saveload,displayinwatch,watchlabel, onvaluechanged,ondrop,onbuttonpressed);
#define AddVarPointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, onvaluechanged,ondrop,onbuttonpressed) AddVariablePointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, onvaluechanged,ondrop,onbuttonpressed);
#define AddVarEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed) AddVariableEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed);
#define AddVarFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed) AddVariableFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, onvaluechanged,ondrop,onbuttonpressed);
#else
#define AddVar(pList,label,type,offset,saveload,displayinwatch,watchlabel, ...) AddVariable(pList,label,type,offset,saveload,displayinwatch,watchlabel, 0,0,0);
#define AddVarPointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, ...) AddVariablePointer(pList,label,saveload,displayinwatch,watchlabel,getptrvalue,setptrvalue,getptrdesc,setptrdesc, 0,0,0);
#define AddVarEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, ...) AddVariableEnum(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, 0,0,0);
#define AddVarFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, ...) AddVariableFlags(pList,label,offset,saveload,displayinwatch,watchlabel,numenums,ppStrings, 0,0,0);
#endif

#define MYFW_COMPONENT_DECLARE_VARIABLE_LIST(ComponentName) \
    static CPPListHead m_ComponentVariableList_##ComponentName; /* ComponentVariable type */ \
    static int m_ComponentVariableListRefCount_##ComponentName; \
    static void RegisterVariables(CPPListHead* pList, ComponentName* pThis); \
    static void ClearAllVariables() { m_ComponentVariableListRefCount_##ComponentName--; if( m_ComponentVariableListRefCount_##ComponentName == 0 ) ClearAllVariables_Base( &m_ComponentVariableList_##ComponentName ); } \
    static ComponentVariable* AddVariable(CPPListHead* pList, const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
    { return AddVariable_Base( pList, label, type, offset, saveload, displayinwatch, watchlabel, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
    static ComponentVariable* AddVariablePointer(CPPListHead* pList, const char* label, bool saveload, bool displayinwatch, const char* watchlabel, CVarFunc_GetPointerValue pGetPointerValueCallBackFunc, CVarFunc_SetPointerValue pSetPointerValueCallBackFunc, CVarFunc_GetPointerDesc pGetPointerDescCallBackFunc, CVarFunc_SetPointerDesc pSetPointerDescCallBackFunc, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
    { return AddVariablePointer_Base( pList, label, saveload, displayinwatch, watchlabel, pGetPointerValueCallBackFunc, pSetPointerValueCallBackFunc, pGetPointerDescCallBackFunc, pSetPointerDescCallBackFunc, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
    static ComponentVariable* AddVariableEnum(CPPListHead* pList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
    { return AddVariableEnum_Base( pList, label, offset, saveload, displayinwatch, watchlabel, numenums, ppStrings, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
    static ComponentVariable* AddVariableFlags(CPPListHead* pList, const char* label, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, int numenums, const char** ppStrings, CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc) \
    { return AddVariableFlags_Base( pList, label, offset, saveload, displayinwatch, watchlabel, numenums, ppStrings, pOnValueChangedCallBackFunc, pOnDropCallBackFunc, pOnButtonPressedCallBackFunc ); } \
    static bool ComponentVariablesHaveBeenRegistered() \
    { \
        if( m_ComponentVariableList_##ComponentName.GetHead() == 0 ) m_ComponentVariableListRefCount_##ComponentName = 0; \
        m_ComponentVariableListRefCount_##ComponentName++; \
        return (m_ComponentVariableListRefCount_##ComponentName != 1); \
    } \
    virtual CPPListHead* GetComponentVariableList() { return &m_ComponentVariableList_##ComponentName; }

#define MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST(ComponentName) \
    CPPListHead ComponentName::m_ComponentVariableList_##ComponentName; \
    int ComponentName::m_ComponentVariableListRefCount_##ComponentName;

#define MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR() \
    if( ComponentVariablesHaveBeenRegistered() == false ) RegisterVariables( GetComponentVariableList(), this );

#define MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR() \
    ClearAllVariables();

#if MYFW_USING_WX
class ComponentBaseEventHandlerForComponentVariables : public wxEvtHandler
{
public:
    ComponentBase* pComponent;
    ComponentVariable* pVar;

public:
    ComponentBaseEventHandlerForComponentVariables()
    {
        pComponent = 0;
        pVar = 0;
    };
    void OnPopupClick(wxEvent &evt);
};
#endif

#endif //__ComponentBase_H__
