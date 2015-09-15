//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
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

enum ComponentVariableTypes
{
    ComponentVariableType_Int,
    ComponentVariableType_UnsignedInt,
    ComponentVariableType_Char,
    ComponentVariableType_UnsignedChar,
    ComponentVariableType_Bool,
    ComponentVariableType_Float,
    ComponentVariableType_Double,
    ComponentVariableType_ColorFloat,
    ComponentVariableType_ColorByte,

    ComponentVariableType_Vector3,

    ComponentVariableType_GameObjectPtr,
    ComponentVariableType_ComponentPtr,

    ComponentVariableType_NumTypes,
};

class ComponentVariable : public CPPListNode
{
public:
    const char* m_Label;
    ComponentVariableTypes m_Type;
    size_t m_Offset; // offset into class of the variable.
    bool m_SaveLoad;
    bool m_DisplayInWatch;
    const char* m_WatchLabel; // if 0 will use m_Label if needed.
    PanelWatchCallbackValueChanged m_pOnValueChangedCallbackFunc;
    int m_ControlID;

public:
    ComponentVariable(const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc)
    {
        m_Label = label;
        m_Type = type;
        m_Offset = offset;
        m_SaveLoad = saveload;
        m_DisplayInWatch = displayinwatch;
        m_WatchLabel = watchlabel;
        if( m_WatchLabel == 0 )
            m_WatchLabel = label;
        m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;
        m_ControlID = -1;
    }
};

class ComponentBase : public CPPListNode
#if MYFW_USING_WX
, public wxEvtHandler
#endif
{
protected:
    bool m_Enabled;
    unsigned int m_SceneIDLoadedFrom; // 0 for runtime generated.
    unsigned int m_ID;

public:
    BaseComponentTypes m_BaseType;
    int m_Type;
    GameObject* m_pGameObject;

public:
    ComponentBase();
    virtual ~ComponentBase();
    SetClassnameBase( "BaseComponent" ); // only first 8 character count.

    static void LuaRegister(lua_State* luastate);

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentBase&)*pObject; }
    ComponentBase& operator=(const ComponentBase& other);

    bool m_CallbacksRegistered;
    virtual void RegisterCallbacks() {}
    virtual void UnregisterCallbacks() {}

    virtual void OnLoad();
    virtual void OnPlay() {}
    virtual void OnStop() {}
    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();

    virtual void SetEnabled(bool enabled);
    void SetSceneID(unsigned int sceneid) { m_SceneIDLoadedFrom = sceneid; }
    void SetID(unsigned int id) { m_ID = id; }

    bool IsEnabled() { return m_Enabled; }
    unsigned int GetSceneID() { return m_SceneIDLoadedFrom; }
    unsigned int GetID() { return m_ID; }

protected:
    static CPPListHead m_ComponentVariableList; // ComponentVariable type
    static void ClearAllVariables();
    static void AddVariable(const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel, PanelWatchCallbackValueChanged pOnValueChangedCallBackFunc);
    void ImportVariablesFromJSON(cJSON* jsonobj, const char* singlelabeltoimport = 0);

#if MYFW_USING_WX
public:
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // if any variables value changed, then react.
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentBase*)pObjectPtr)->OnValueChanged( controlid, finishedchanging, oldvalue ); }
    void OnValueChanged(int controlid, bool finishedchanging, double oldvalue);

    // to show/hide the components controls in watch panel
    //static bool m_PanelWatchBlockVisible; // each class needs it's own static bool, so if one component of this type is off, they all are.
    bool* m_pPanelWatchBlockVisible; // pointer to the bool above, must be set by each component.
    int m_ControlID_ComponentTitleLabel;
    static void StaticOnComponentTitleLabelClicked(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentBase*)pObjectPtr)->OnComponentTitleLabelClicked( controlid, finishedchanging ); }
    void OnComponentTitleLabelClicked(int controlid, bool finishedchanging);

    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentBase*)pObjectPtr)->OnLeftClick( count, true ); }
    virtual void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear) {};

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((ComponentBase*)pObjectPtr)->OnRightClick(); }
    virtual void OnRightClick();
    virtual void AppendItemsToRightClickMenu(wxMenu* pMenu);
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((ComponentBase*)pObjectPtr)->OnDrag(); }
    virtual void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, wxCoord x, wxCoord y) { ((ComponentBase*)pObjectPtr)->OnDrop(controlid, x, y); }
    virtual void OnDrop(int controlid, wxCoord x, wxCoord y);
#endif //MYFW_USING_WX
};

#endif //__ComponentBase_H__
