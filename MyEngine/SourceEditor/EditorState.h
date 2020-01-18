//
// Copyright (c) 2015-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorState_H__
#define __EditorState_H__

class ComponentBase;
class ComponentCamera;
class EditorDocument;
class GameObject;
class TransformGizmo;

#if MYFW_EDITOR
enum class EditorActionState
{
    None,
    TranslateX,
    TranslateY,
    TranslateZ,
    TranslateXY,
    TranslateXZ,
    TranslateYZ,
    ScaleX,
    ScaleY,
    ScaleZ,
    ScaleXYZ,
    RotateX,
    RotateY,
    RotateZ,
    GroupSelectingObjects,
    RotatingEditorCamera,
    NumStates,
};

enum class EditorCameraState
{
    Default,
    LockedToObject,
    NumStates,
};

enum class EditorIcon
{
    Light,
    Camera,
    NumIcons,
};

extern const char* EditorIconFilenames[EditorIcon::NumIcons];

class EditorState
{
public:
    EngineCore* m_pEngineCore;

    std::vector<EditorDocument*> m_pOpenDocuments;

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

#if MYFW_USING_BULLET
    // for physics "picker", to move physics around in editor view when gameplay is running.
    btRigidBody* m_MousePicker_PickedBody;
    btTypedConstraint* m_MousePicker_PickConstraint;
    btScalar m_MousePicker_OldPickingDist;
#endif //MYFW_USING_BULLET

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
    MySprite* m_pEditorIcons[EditorIcon::NumIcons];

public:
    EditorState(EngineCore* pEngineCore);
    ~EditorState();

    void Init();

    void SaveAllMiscFiles();
    void SaveAllOpenDocuments();
    bool DoAnyOpenDocumentsHaveUnsavedChanges();

    void OpenDocument(EditorDocument* pDocument);

    EngineCore* GetEngineCore() { return m_pEngineCore; }
    ComponentCamera* GetEditorCamera();

    void ClearEditorState(bool clearselectedobjectandcomponents = true);

    void OnFocusLost();
    void OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height);

    void SetModifierKeyStates(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure);
    void ClearModifierKeyStates(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure);

    void ClearConstraint();

    void SelectGameObject(GameObject* pObject);
    void UnselectGameObject(GameObject* pObject);
    bool IsGameObjectSelected(GameObject* pObject);
    bool IsGameObjectAParentOfASelectedObjectOrComponent(GameObject* pObject);
    void DeleteSelectedObjects();

    void SelectComponent(ComponentBase* pComponent);
    void UnselectComponent(ComponentBase* pComponent);
    bool IsComponentSelected(ComponentBase* pComponent);

    void ClearKeyAndActionStates();
    void ClearSelectedObjectsAndComponents();
    void ClearSelectedComponents();
    void ClearSelectedObjectsAndComponentsFromScene(SceneID sceneID);

    void LockCameraToGameObject(GameObject* pGameObject);
    void UpdateCamera(float deltaTime);

    bool HasMouseMovedSinceButtonPressed(int buttonid);

    //void SyncFromObjectPanelSelectedItems();
};
#endif //MYFW_EDITOR

#endif //__EditorState_H__
