//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentCamera_H__
#define __ComponentCamera_H__

#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Renderer_Base.h"

#include "ComponentBase.h"
#include "Camera/Camera2D.h"
#include "Camera/Camera3D.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"

class ComponentPostEffect;
class ComponentTransform;

// 32 layers strings since ComponentBase::ExportVariablesToJSON -> case ComponentVariableType::Flags is looking for 32
extern const char* g_pVisibilityLayerStrings[32]; //g_NumberOfVisibilityLayers];

class ComponentCamera : public ComponentBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentCamera );

public:
    ComponentTransform* m_pComponentTransform;

    // Needs saving.
    bool m_Orthographic;

    bool m_ClearColorBuffer;
    bool m_ClearDepthBuffer;

    // For deferred.
    bool m_Deferred;
private:
    bool m_DeferredGBufferVisible;
    FBODefinition* m_pGBuffer;

    MyFileObject* m_pDeferredShaderFile_AmbientDirectional;
    MyFileObject* m_pDeferredShaderFile_PointLight;
    ShaderGroup* m_pDeferredShader_AmbientDirectional;
    ShaderGroup* m_pDeferredShader_PointLight;
    MaterialDefinition* m_pDeferredMaterial_AmbientDirectional;
    MaterialDefinition* m_pDeferredMaterial_PointLight;

    MyMesh* m_pDeferredQuadMesh;
    MyFileObject* m_pDeferredSphereMeshFile;
    MyMesh* m_pDeferredSphereMesh;

public:
    // For ortho.
    float m_DesiredWidth;
    float m_DesiredHeight;
    float m_OrthoNearZ;
    float m_OrthoFarZ;

    // For perspective.
    float m_FieldOfView;
    float m_PerspectiveNearZ;
    float m_PerspectiveFarZ;

    unsigned int m_LayersToRender;

    // Don't need saving.
    MyViewport m_Viewport;

    Camera2D m_Camera2D;
    Camera3D m_Camera3D;

    FBODefinition* m_pPostEffectFBOs[2];

#if MYFW_EDITOR
    unsigned int m_FullClearsRequired;
#endif

protected:
    ComponentPostEffect* GetNextPostEffect(ComponentPostEffect* pLastEffect); // pass in 0 to get first effect.

public:
    ComponentCamera(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentCamera();
    SetClassnameBase( "CameraComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jComponent, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentCamera&)*pObject; }
    ComponentCamera& operator=(const ComponentCamera& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    void SetDesiredAspectRatio(float width, float height);
    void ComputeProjectionMatrices();

    virtual void Tick(float deltaTime);
    virtual void OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    virtual void OnDrawFrame();

    void SetDeferred(bool isdeferred) { m_Deferred = isdeferred; }
    void ShowDeferredGBuffer() { m_DeferredGBufferVisible = true; }

protected:
    void DrawScene();

    // Mesh draw callback
    static void StaticSetupCustomUniformsCallback(void* pObjectPtr, Shader_Base* pShader) { ((ComponentCamera*)pObjectPtr)->SetupCustomUniformsCallback( pShader ); }
    void SetupCustomUniformsCallback(Shader_Base* pShader);

public:
    // pre-DrawCallback functions
    virtual bool IsVisible();
    virtual bool ExistsOnLayer(unsigned int layerflags);

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
#if MYFW_EDITOR
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
#endif //MYFW_EDITOR
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    // Object panel callbacks.
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentCamera*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX

    // Component variable callbacks. //_VARIABLE_LIST
    virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);

    bool HandleInputForEditorCamera(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure);
    bool HandleInputFor2DTopDownEditorCamera(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure);
#endif //MYFW_EDITOR
};

#endif //__ComponentCamera_H__
