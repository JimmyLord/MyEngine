//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMeshOBJ_H__
#define __ComponentMeshOBJ_H__

class ComponentTransform;

class ComponentMeshOBJ : public ComponentMesh
{
public:

public:
    ComponentMeshOBJ();
    virtual ~ComponentMeshOBJ();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMeshOBJ&)*pObject; }
    virtual ComponentMeshOBJ& operator=(const ComponentMeshOBJ& other);

    void SetMesh(MyMesh* pMesh);

    virtual void Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

public:
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentMeshOBJ*)pObjectPtr)->OnLeftClick( true ); }
    void OnLeftClick(bool clear);
    virtual void FillPropertiesWindow(bool clear);
    static void StaticOnDropOBJ(void* pObjectPtr) { ((ComponentMeshOBJ*)pObjectPtr)->OnDropOBJ(); }
    void OnDropOBJ();
#endif //MYFW_USING_WX
};

#endif //__ComponentMeshOBJ_H__
