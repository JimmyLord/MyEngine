//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentPostEffect_H__
#define __ComponentPostEffect_H__


#include "Camera/Camera2D.h"
#include "Camera/Camera3D.h"
#include "ComponentSystem/BaseComponents/ComponentData.h"

class ComponentTransform;

class ComponentPostEffect : public ComponentData
{
private:
    // Component Variable List.
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentPostEffect );

public:
    MySprite* m_pFullScreenQuad;

    MaterialDefinition* m_pMaterial;

public:
    ComponentPostEffect(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentPostEffect();
    SetClassnameBase( "PostEffectComponent" ); // Only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool saveSceneID, bool saveID);
    virtual void ImportFromJSONObject(cJSON* jObject, SceneID sceneID);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentPostEffect&)*pObject; }
    ComponentPostEffect& operator=(const ComponentPostEffect& other);

    virtual void RegisterCallbacks() {} // TODO: Change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: Change this component to use callbacks.

    virtual MaterialDefinition* GetMaterial() { return m_pMaterial; }
    virtual void SetMaterial(MaterialDefinition* pMaterial);
    
    void Render(FBODefinition* pFBO);

public:
#if MYFW_EDITOR
    virtual ComponentVariable* GetComponentVariableForMaterial(int submeshIndex);

    // Component variable callbacks.
    void* OnDropMaterial(ComponentVariable* pVar, bool changedByInterface, int x, int y);    
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentPostEffect_H__
