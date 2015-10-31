//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentRenderable_H__
#define __ComponentRenderable_H__

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

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentRenderable&)*pObject; }
    ComponentRenderable& operator=(const ComponentRenderable& other);

    virtual void RegisterCallbacks() {} // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: change this component to use callbacks.

    virtual MaterialDefinition* GetMaterial(int submeshindex) { return 0; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);
    void Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

    virtual void SetVisible(bool visible) { m_Visible = visible; }
    virtual bool IsVisible();

    void SetLayersThisExistsOn(unsigned int layers) { m_LayersThisExistsOn = layers; }
    unsigned int GetLayersThisExistsOn() { return m_LayersThisExistsOn; }
    bool ExistsOnLayer(unsigned int layer) { return (m_LayersThisExistsOn & layer) ? true : false; }

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentRenderable*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    // Component variable callbacks. //_VARIABLE_LIST
    //void* OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y);
    //void* OnValueChanged(ComponentVariable* pVar, bool finishedchanging, double oldvalue);
#endif //MYFW_USING_WX
};

#endif //__ComponentRenderable_H__
