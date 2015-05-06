//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __TransformGizmo_H__
#define __TransformGizmo_H__

class EngineCore;

class TransformGizmo
{
public:
    GameObject* m_pTransformGizmos[3];

    int m_SelectedPart;

    bool m_LastIntersectResultIsValid;
    Vector3 m_LastIntersectResultUsed;

public:
    TransformGizmo();
    virtual ~TransformGizmo();

    virtual void Tick(double TimePassed, EditorState* pEditorState);

    bool HandleInput(EngineCore* pGame, int keydown, int keycode, int action, int id, float x, float y, float pressure);

    void CreateAxisObjects(float scale, MaterialDefinition* pMaterialX, MaterialDefinition* pMaterialY, MaterialDefinition* pMaterialZ, EditorState* pEditorState);
    void ScaleGizmosForMousePickRendering(bool doscale);
    
    void TranslateSelectedObjects(EngineCore* pGame, EditorState* pEditorState);
};

#endif // __TransformGizmo_H__
