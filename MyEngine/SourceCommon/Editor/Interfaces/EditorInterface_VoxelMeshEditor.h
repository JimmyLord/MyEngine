//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorInterface_VoxelMeshEditor_H__
#define __EditorInterface_VoxelMeshEditor_H__

class EditorInterface_VoxelMeshEditor : public EditorInterface
{
public:

protected:
    ComponentVoxelWorld* m_pVoxelWorld;
    ComponentVoxelMesh* m_pVoxelMesh;

    bool m_CapturedRightMouse;

    unsigned int m_CurrentBlockType;

public:
    EditorInterface_VoxelMeshEditor();
    virtual ~EditorInterface_VoxelMeshEditor();

    virtual void Initialize();

    virtual void OnActivated();
    virtual void OnDeactivated();
    virtual void OnDrawFrame(unsigned int canvasid);

    virtual bool HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);

    virtual void RenderObjectIDsToFBO();

    void CancelCurrentOperation();

    void SaveVoxelMesh();

    ComponentVoxelMesh* GetMeshBeingEdited() { return m_pVoxelMesh; }
    void SetWorldToEdit(ComponentVoxelWorld* pVoxelWorld);
    void SetMeshToEdit(ComponentVoxelMesh* pVoxelMesh);

    bool RayCast(Vector2 mousepos, VoxelRayCastResult* pResult);
};

#endif //__EditorInterface_VoxelMeshEditor_H__
