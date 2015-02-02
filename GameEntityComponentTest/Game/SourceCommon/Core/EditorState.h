//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorState_H__
#define __EditorState_H__

#if MYFW_USING_WX
enum EditorActionState
{
    EDITORACTIONSTATE_None,
    EDITORACTIONSTATE_TranslateX,
    EDITORACTIONSTATE_TranslateY,
    EDITORACTIONSTATE_TranslateZ,
    EDITORACTIONSTATE_TranslateXY,
    EDITORACTIONSTATE_TranslateXZ,
    EDITORACTIONSTATE_TranslateYZ,
    EDITORACTIONSTATE_NumStates,
};

struct EditorState
{
    unsigned int m_ModifierKeyStates;
    EditorActionState m_EditorActionState;
    Vector2 m_MouseLeftDownLocation;
    Vector2 m_MouseRightDownLocation;
    Vector2 m_MouseMiddleDownLocation;
    Vector2 m_LastMousePosition;
    Vector2 m_CurrentMousePosition;

    FBODefinition* m_pMousePickerFBO;

    std::vector<GameObject*> m_pSelectedObjects;
    std::vector<ComponentBase*> m_pSelectedComponents;

    GameObject* m_p3DGridPlane;
    GameObject* m_pTransformGizmos[3];
    GameObject* m_pEditorCamera;

    // for physics "picker", to move physics around in editor view when gameplay is running.
    btRigidBody* m_MousePicker_PickedBody;
    btTypedConstraint* m_MousePicker_PickConstraint;
    btScalar m_MousePicker_OldPickingDist;

    // transform gizmo info, keep track of total distance translated for undo/redo.
    Vector3 m_DistanceTranslated;

public:
    EditorState();
    ~EditorState();

    ComponentCamera* GetEditorCamera();

    void UnloadScene();

    void ClearConstraint();

    bool IsObjectSelected(GameObject* pObject);
    bool IsComponentSelected(ComponentBase* pComponent);

    void ClearSelectedObjectsAndComponents();
};
#endif //MYFW_USING_WX

#endif //__EditorState_H__
