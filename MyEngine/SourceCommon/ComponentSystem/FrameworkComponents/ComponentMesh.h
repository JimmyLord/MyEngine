//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMesh_H__
#define __ComponentMesh_H__

class ComponentTransform;
class SceneGraphObject;

extern const char* OpenGLPrimitiveTypeStrings[7];

class ComponentMesh : public ComponentRenderable
{
protected:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentMesh ); //_VARIABLE_LIST

public:
    static const int MAX_SUBMESHES = 4;

public:
    MyMesh* m_pMesh;

    bool m_WaitingToAddToSceneGraph;
    SceneGraphObject* m_pSceneGraphObjects[MAX_SUBMESHES];
    MaterialDefinition* m_pMaterials[MAX_SUBMESHES];
    int m_GLPrimitiveType;
    int m_PointSize;

protected:
    // Vars to allow script object callbacks
    ComponentLuaScript* m_pComponentLuaScript;

public:
    ComponentMesh();
    virtual ~ComponentMesh();
    SetClassnameWithParent( "MeshComponent", ComponentRenderable ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    // ComponentBase overrides
    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jComponentMesh, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMesh&)*pObject; }
    ComponentMesh& operator=(const ComponentMesh& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void OnPlay();

    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((ComponentMesh*)pObjectPtr)->OnEvent( pEvent ); }
    virtual bool OnEvent(MyEvent* pEvent);

    //virtual void OnGameObjectEnabled();
    //virtual void OnGameObjectDisabled();

    // ComponentRenderable overrides
    virtual MaterialDefinition* GetMaterial(int submeshindex) { return m_pMaterials[submeshindex]; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);

    virtual void SetVisible(bool visible);
    //virtual bool IsVisible();

    virtual void SetMesh(MyMesh* pMesh);

    virtual bool IsMeshReady();
    virtual void MeshFinishedLoading();

    virtual void AddToSceneGraph();
    virtual void RemoveFromSceneGraph();
    virtual void PushChangesToSceneGraphObjects();

    virtual MyAABounds* GetBounds();

    // Mesh draw callback
    static void StaticSetupCustomUniformsCallback(void* pObjectPtr, Shader_Base* pShader) { ((ComponentMesh*)pObjectPtr)->SetupCustomUniformsCallback( pShader ); }
    void SetupCustomUniformsCallback(Shader_Base* pShader);

    // Lua script deleted callbacks.
    static void StaticOnLuaScriptDeleted(void* pObjectPtr, ComponentBase* pComponent) { ((ComponentMesh*)pObjectPtr)->OnLuaScriptDeleted( pComponent ); }
    void OnLuaScriptDeleted(ComponentBase* pComponent);

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_EDITOR
    virtual ComponentVariable* GetComponentVariableForMaterial(int submeshindex);

#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    int m_MaterialExpandButtonControlIDs[MAX_SUBMESHES];
    bool m_MaterialExpanded[MAX_SUBMESHES];

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMesh*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX
    virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
    virtual void VariableAddedToWatchPanel(ComponentVariable* pVar);

#if MYFW_USING_WX
    // Watch panel callbacks.
    static void StaticOnExpandMaterialClicked(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging) { ((ComponentMesh*)pObjectPtr)->OnExpandMaterialClicked( controlid ); }
    void OnExpandMaterialClicked(int controlid);

#endif //MYFW_USING_WX

    // Component variable callbacks. //_VARIABLE_LIST
    void* OnDropMaterial(ComponentVariable* pVar, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentMesh_H__
