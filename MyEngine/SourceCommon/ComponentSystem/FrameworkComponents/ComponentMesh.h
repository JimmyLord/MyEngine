//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMesh_H__
#define __ComponentMesh_H__

class ComponentTransform;

extern const char* OpenGLPrimitiveTypeStrings[7];

class ComponentMesh : public ComponentRenderable
{
    static const int MAX_SUBMESHES = 4;

public:
    MyMesh* m_pMesh;

    MyList<MaterialDefinition*> m_MaterialList;
    int m_GLPrimitiveType;
    int m_PointSize;

public:
    ComponentMesh();
    virtual ~ComponentMesh();
    SetClassnameBase( "MeshComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jComponentMesh, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMesh&)*pObject; }
    ComponentMesh& operator=(const ComponentMesh& other);

    virtual void RegisterCallbacks() {} // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: change this component to use callbacks.

    virtual MaterialDefinition* GetMaterial(int submeshindex) { return m_MaterialList[submeshindex]; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);
    virtual void Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    int m_ControlID_Material[MAX_SUBMESHES];

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMesh*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);
    
    // Watch panel callbacks.
    static void StaticOnDropMaterial(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentMesh*)pObjectPtr)->OnDropMaterial(controlid, x, y); }
    void OnDropMaterial(int controlid, wxCoord x, wxCoord y);
#endif //MYFW_USING_WX
};

#endif //__ComponentMesh_H__
