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
    bool m_Visible;

public:
    ComponentTransform* m_pComponentTransform;

    //ShaderGroup* m_pShader; // managed by external forces.

    unsigned int m_LayersThisExistsOn;

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
    virtual void Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible();

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentRenderable*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);
#endif //MYFW_USING_WX
};

#endif //__ComponentRenderable_H__
