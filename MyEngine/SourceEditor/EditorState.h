//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorState_H__
#define __EditorState_H__

class TransformGizmo;

#if MYFW_EDITOR
enum EditorActionState
{
    EDITORACTIONSTATE_None,
    EDITORACTIONSTATE_TranslateX,
    EDITORACTIONSTATE_TranslateY,
    EDITORACTIONSTATE_TranslateZ,
    EDITORACTIONSTATE_TranslateXY,
    EDITORACTIONSTATE_TranslateXZ,
    EDITORACTIONSTATE_TranslateYZ,
    EDITORACTIONSTATE_ScaleX,
    EDITORACTIONSTATE_ScaleY,
    EDITORACTIONSTATE_ScaleZ,
    EDITORACTIONSTATE_ScaleXYZ,
    EDITORACTIONSTATE_RotateX,
    EDITORACTIONSTATE_RotateY,
    EDITORACTIONSTATE_RotateZ,
    EDITORACTIONSTATE_GroupSelectingObjects,
    EDITORACTIONSTATE_RotatingEditorCamera,
    EDITORACTIONSTATE_NumStates,
};

enum EditorCameraState
{
    EditorCameraState_Default,
    EditorCameraState_LockedToObject,
    EditorCameraState_NumStates,
};

enum EditorIcons
{
    EditorIcon_Light,
    EditorIcon_Camera,
    EditorIcon_NumIcons,
};

extern const char* EditorIconFilenames[EditorIcon_NumIcons];

struct EditorState
{
    MyRect m_EditorWindowRect;

    unsigned int m_ModifierKeyStates;
    EditorActionState m_EditorActionState;
    Vector2 m_MouseDownLocation[3]; // 0 is left button, 1 is right button, 2 is middle button
    bool m_HasMouseMovedSinceButtonPressed[3]; // 0 is left button, 1 is right button, 2 is middle button
    Vector2 m_LastMousePosition;
    Vector2 m_CurrentMousePosition;

    FBODefinition* m_pDebugViewFBO; // used to draw the selected animated mesh, more later?
    FBODefinition* m_pMousePickerFBO;

    std::vector<GameObject*> m_pSelectedObjects;
    std::vector<ComponentBase*> m_pSelectedComponents;

    TransformGizmo* m_pTransformGizmo;
    GameObject* m_p3DGridPlane;
    GameObject* m_pEditorCamera;

    // for physics "picker", to move physics around in editor view when gameplay is running.
    btRigidBody* m_MousePicker_PickedBody;
    btTypedConstraint* m_MousePicker_PickConstraint;
    btScalar m_MousePicker_OldPickingDist;

    // transform gizmo info, keep track of total distance translated/scaled for undo/redo
    Vector3 m_DistanceTranslated;
    Vector3 m_AmountScaled;
    Vector3 m_DistanceRotated;
    bool m_TransformedInLocalSpace;
    Vector3 m_WorldSpacePivot;

    // camera state
    EditorCameraState m_CameraState;
    GameObject* m_pGameObjectCameraIsFollowing;
    MyMatrix m_OffsetFromObject;

    // icons for various editor objects.  lights, cameras, etc...
    MySprite* m_pEditorIcons[EditorIcon_NumIcons];

public:
    EditorState();
    ~EditorState();

    ComponentCamera* GetEditorCamera();

    void ClearEditorState(bool clearselectedobjectandcomponents = true);

    void OnFocusLost();
    void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);

    void ClearConstraint();

    void SelectGameObject(GameObject* pObject);
    void UnselectGameObject(GameObject* pObject);
    bool IsGameObjectSelected(GameObject* pObject);
    bool IsGameObjectAParentOfASelectedObject(GameObject* pObject);
    void DeleteSelectedObjects();

    void SelectComponent(ComponentBase* pComponent);
    void UnselectComponent(ComponentBase* pComponent);
    bool IsComponentSelected(ComponentBase* pComponent);

    void ClearKeyAndActionStates();
    void ClearSelectedObjectsAndComponents();
    void ClearSelectedObjectsAndComponentsFromScene(SceneID sceneID);

    void LockCameraToGameObject(GameObject* pGameObject);
    void UpdateCamera(float deltaTime);

    bool HasMouseMovedSinceButtonPressed(int buttonid);

    //void SyncFromObjectPanelSelectedItems();
};
#endif //MYFW_EDITOR

#endif //__EditorState_H__
