//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentLight_H__
#define __ComponentLight_H__

#include "ComponentSystem/BaseComponents/ComponentData.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"

class ComponentLight : public ComponentData
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentLight );

protected:
    LightTypes m_LightType;

    MyLight* m_pLight;

public:
    ComponentLight(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentLight();
    SetClassnameBase( "LightComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool saveSceneID, bool saveID) override;
    virtual void ImportFromJSONObject(cJSON* jComponent, SceneID sceneID) override;

    virtual void Reset() override;
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) override { *this = (ComponentLight&)*pObject; }
    ComponentLight& operator=(const ComponentLight& other);

    virtual void RegisterCallbacks() override;
    virtual void UnregisterCallbacks() override;

    virtual void OnGameObjectEnabled() override;
    virtual void OnGameObjectDisabled() override;

    static void StaticOnTransformChanged(void* pObjectPtr, const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor) { ((ComponentLight*)pObjectPtr)->OnTransformChanged( newPos, newRot, newScale, changedByUserInEditor ); }
    void OnTransformChanged(const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor);

    // Setters.
    virtual bool SetEnabled(bool enableComponent) override;

    // pre-DrawCallback functions
    virtual bool IsVisible() override;
    virtual bool ExistsOnLayer(unsigned int layerFlags) override;

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
#if MYFW_EDITOR
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
#endif //MYFW_EDITOR
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_EDITOR
    float m_LightSphereRenderTimeRemaining;

#if MYFW_USING_IMGUI
    virtual void AddAllVariablesToWatchPanel(CommandStack* pCommandStack) override;
#endif

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentLight_H__
