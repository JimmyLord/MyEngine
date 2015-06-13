//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMeshPrimitive_H__
#define __ComponentMeshPrimitive_H__

class ComponentTransform;

enum ComponentMeshPrimitives // Saved as int, must stay in order.
{
    ComponentMeshPrimitive_Plane,
    ComponentMeshPrimitive_Icosphere,
    ComponentMeshPrimitive_NumTypes,
};

extern const char* ComponentMeshPrimitiveTypeStrings[ComponentMeshPrimitive_NumTypes];

class ComponentMeshPrimitive : public ComponentMesh
{
public:
    ComponentMeshPrimitives m_MeshPrimitiveType;

    // Plane
    Vector2 m_Plane_Size;
    Vector2Int m_Plane_VertCount;
    Vector2 m_Plane_UVStart;
    Vector2 m_Plane_UVRange;

    // Sphere
    float m_Sphere_Radius;

public:
    ComponentMeshPrimitive();
    virtual ~ComponentMeshPrimitive();
    SetClassnameBase( "MeshPrimitiveComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMeshPrimitive&)*pObject; }
    virtual ComponentMeshPrimitive& operator=(const ComponentMeshPrimitive& other);

    void CreatePrimitive();

    virtual void Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    int m_ControlID_MeshPrimitiveType;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMeshPrimitive*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging) { ((ComponentMeshPrimitive*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentMeshPrimitive_H__
