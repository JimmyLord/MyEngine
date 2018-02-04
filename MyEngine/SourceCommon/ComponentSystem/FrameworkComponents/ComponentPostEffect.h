//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentPostEffect_H__
#define __ComponentPostEffect_H__

#include "../../Camera/Camera2D.h"
#include "../../Camera/Camera3D.h"

class ComponentTransform;

class ComponentPostEffect : public ComponentData
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentPostEffect );

public:
    MySprite* m_pFullScreenQuad;

    MaterialDefinition* m_pMaterial;

public:
    ComponentPostEffect();
    virtual ~ComponentPostEffect();
    SetClassnameBase( "PostEffectComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentPostEffect&)*pObject; }
    ComponentPostEffect& operator=(const ComponentPostEffect& other);

    virtual void RegisterCallbacks() {} // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: change this component to use callbacks.

    virtual MaterialDefinition* GetMaterial() { return m_pMaterial; }
    virtual void SetMaterial(MaterialDefinition* pMaterial);
    
    void Render(FBODefinition* pFBO);

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual ComponentVariable* GetComponentVariableForMaterial(int submeshindex);

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentPostEffect*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
    
    // Watch panel callbacks.
    static void StaticOnDropMaterial(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentPostEffect*)pObjectPtr)->OnDropMaterial(controlid, x, y); }
    void OnDropMaterial(int controlid, wxCoord x, wxCoord y);
    
    //void* OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y);    
#endif //MYFW_USING_WX
    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentPostEffect_H__
