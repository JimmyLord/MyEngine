//
// Copyright (c) 2014-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMesh_H__
#define __ComponentMesh_H__

#include "ComponentSystem/BaseComponents/ComponentRenderable.h"

class ComponentLuaScript;
class ComponentTransform;
class RenderGraphObject;

extern const char* OpenGLPrimitiveTypeStrings[7];

class ComponentMesh : public ComponentRenderable
{
protected:
    // Component Variable List.
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentMesh ); //_VARIABLE_LIST

public:
    static const int MAX_SUBMESHES = 4;

public:
    MyMesh* m_pMesh;

    bool m_WaitingToAddToRenderGraph;
    RenderGraphObject* m_pRenderGraphObjects[MAX_SUBMESHES];
    MaterialDefinition* m_pMaterials[MAX_SUBMESHES];
    MyRE::PrimitiveTypes m_GLPrimitiveType;
    int m_PointSize;

protected:
    // Vars to allow script object callbacks.
    ComponentLuaScript* m_pComponentLuaScript;

public:
    ComponentMesh(ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentMesh();
    SetClassnameWithParent( "MeshComponent", ComponentRenderable ); // Only first 8 characters count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luaState);
#endif //MYFW_USING_LUA

    // ComponentBase overrides.
    virtual cJSON* ExportAsJSONObject(bool saveSceneID, bool saveID);
    virtual void ImportFromJSONObject(cJSON* jComponent, SceneID sceneID);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMesh&)*pObject; }
    ComponentMesh& operator=(const ComponentMesh& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void OnPlay();

    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((ComponentMesh*)pObjectPtr)->OnEvent( pEvent ); }
    bool OnEvent(MyEvent* pEvent);

    //virtual void OnGameObjectEnabled();
    //virtual void OnGameObjectDisabled();

    static void StaticOnTransformChanged(void* pObjectPtr, Vector3& newPos, Vector3& newRot, Vector3& newScale, bool changedByUserInEditor) { ((ComponentMesh*)pObjectPtr)->OnTransformChanged( newPos, newRot, newScale, changedByUserInEditor ); }
    void OnTransformChanged(Vector3& newPos, Vector3& newRot, Vector3& newScale, bool changedByUserInEditor);

    // ComponentRenderable overrides.
    virtual MaterialDefinition* GetMaterial(int submeshIndex) { return m_pMaterials[submeshIndex]; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshIndex);

    virtual void SetVisible(bool visible);
    //virtual bool IsVisible();

    virtual void SetMesh(MyMesh* pMesh);

    virtual bool IsMeshReady();
    virtual void MeshFinishedLoading();

    virtual void AddToRenderGraph();
    virtual void RemoveFromRenderGraph();
    virtual void PushChangesToRenderGraphObjects();

    virtual MyAABounds* GetBounds();

    // Mesh draw callback.
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
    virtual ComponentVariable* GetComponentVariableForMaterial(int submeshIndex);

    virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
    virtual void VariableAddedToWatchPanel(ComponentVariable* pVar);

    // Component variable callbacks. //_VARIABLE_LIST
    void* OnDropMaterial(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);

#if _DEBUG && MYFW_WINDOWS
    void TriggerBreakpointOnNextDraw(int submeshIndex);
#endif //_DEBUG && MYFW_WINDOWS

#endif //MYFW_EDITOR
};

#endif //__ComponentMesh_H__
