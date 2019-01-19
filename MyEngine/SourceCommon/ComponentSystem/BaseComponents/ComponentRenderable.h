//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentRenderable_H__
#define __ComponentRenderable_H__

#include "ComponentSystem/Core/ComponentSystemManager.h"

class ComponentTransform;

class ComponentRenderable : public ComponentBase
{
protected:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentRenderable );

protected:
    bool m_Visible;
    unsigned int m_LayersThisExistsOn;

public:
    ComponentTransform* m_pComponentTransform;

public:
    ComponentRenderable();
    virtual ~ComponentRenderable();
    SetClassnameBase( "RenderableComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentRenderable&)*pObject; }
    ComponentRenderable& operator=(const ComponentRenderable& other);

    virtual void RegisterCallbacks() {} // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: change this component to use callbacks.

    //virtual void OnGameObjectEnabled();
    //virtual void OnGameObjectDisabled();
    virtual void SetEnabled(bool enabled);

    virtual MaterialDefinition* GetMaterial(int submeshindex) { return 0; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex) {}
    void Draw(MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

    void SetLayersThisExistsOn(unsigned int layers) { m_LayersThisExistsOn = layers; }
    unsigned int GetLayersThisExistsOn() { return m_LayersThisExistsOn; }
    virtual bool ExistsOnLayer(unsigned int layerflags) { return (m_LayersThisExistsOn & layerflags) ? true : false; }

    virtual void SetVisible(bool visible) { m_Visible = visible; }
    virtual bool IsVisible();

    virtual void AddToSceneGraph() = 0;
    virtual void RemoveFromSceneGraph() = 0;
    virtual void PushChangesToSceneGraphObjects() = 0;

    virtual MyAABounds* GetBounds() { return 0; }

public:
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
    virtual ComponentVariable* GetComponentVariableForMaterial(int submeshindex) { return 0; }
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentRenderable*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX

    // Component variable callbacks. //_VARIABLE_LIST
    //void* OnDrop(ComponentVariable* pVar, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentRenderable_H__
