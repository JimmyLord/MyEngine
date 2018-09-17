//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMeshPrimitive_H__
#define __ComponentMeshPrimitive_H__

class ComponentTransform;

enum ComponentMeshPrimitives // Saved as string, order can change.
{
    ComponentMeshPrimitive_Plane,
    ComponentMeshPrimitive_Icosphere,
    ComponentMeshPrimitive_2DCircle,
    ComponentMeshPrimitive_Grass,
    ComponentMeshPrimitive_NumTypes,
};

extern const char* ComponentMeshPrimitiveTypeStrings[ComponentMeshPrimitive_NumTypes];

class ComponentMeshPrimitive : public ComponentMesh
{
protected:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentMeshPrimitive ); //_VARIABLE_LIST

public:
    ComponentMeshPrimitives m_MeshPrimitiveType;

    // Plane
    Vector2 m_Plane_Size;
    Vector2Int m_Plane_VertCount;
    bool m_Plane_UVsPerQuad;
    Vector2 m_Plane_UVStart;
    Vector2 m_Plane_UVRange;

    // Sphere
    float m_Sphere_Radius;

public:
    ComponentMeshPrimitive();
    virtual ~ComponentMeshPrimitive();
    SetClassnameWith2Parents( "MeshPrimitiveComponent", ComponentMesh, ComponentRenderable ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMeshPrimitive&)*pObject; }
    virtual ComponentMeshPrimitive& operator=(const ComponentMeshPrimitive& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    void CreatePrimitive();

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_EDITOR
    bool m_PrimitiveSettingsChangedAtRuntime;
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    int m_ControlID_MeshPrimitiveType;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMeshPrimitive*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    // Component variable callbacks. //_VARIABLE_LIST
    virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
#endif //MYFW_USING_WX
    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentMeshPrimitive_H__
