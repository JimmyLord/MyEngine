//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorMainFrame_ImGui_H__
#define __EditorMainFrame_ImGui_H__

#include "../SourceEditor/EditorMainFrame.h"

class ComponentAnimationPlayer2D;
class EditorDocument;
class EditorLogWindow_ImGui;
class EditorMemoryWindow_ImGui;
class EditorLayoutManager_ImGui;
class EditorLayout;
class EngineCore;
class GameObject;
class PrefabObject;

enum EditorWindowTypes
{
    EditorWindow_Game,
    EditorWindow_Editor,
    EditorWindow_ObjectList,
    EditorWindow_Watch,
    EditorWindow_Resources,
    EditorWindow_Log,
    EditorWindow_GridSettings,
    EditorWindow_MaterialEditor,
    EditorWindow_2DAnimationEditor,
    EditorWindow_Debug_MousePicker,
    EditorWindow_Debug_Stuff,
    EditorWindow_Debug_MemoryAllocations,
    EditorWindow_Debug_ImGuiDemo,
    EditorWindow_NumTypes,
};

class EditorMainFrame_ImGui : public EditorMainFrame
{
protected:
    // Layouts.
    EditorLayoutManager_ImGui* m_pLayoutManager;
    EditorLayout* m_pCurrentLayout;

    // Warnings.
    bool m_ShowNewSceneWarning;
    bool m_ShowLoadSceneWarning;
    std::string m_SceneToLoadAfterWarning;
    bool m_ShowCloseEditorWarning;

    // Render surfaces.
    FBODefinition* m_pGameFBO;
    FBODefinition* m_pEditorFBO;
    FBODefinition* m_pMaterialPreviewFBO; // TODO: allow for more of these, one for material editor and one for rollover preview.
    
    // Material Preview and Editor.
    MaterialDefinition* m_pMaterialToPreview;
    MaterialDefinition* m_pMaterialBeingEdited;

    // 2D Animation Editor.
    char m_FullPathToLast2DAnimInfoBeingEdited[MAX_PATH];
    My2DAnimInfo* m_p2DAnimInfoBeingEdited;
    unsigned int m_Current2DAnimationIndex;
    ComponentAnimationPlayer2D* m_pAnimPlayerComponent;

    // Log Window.
    EditorLogWindow_ImGui* m_pLogWindow;
    EditorMemoryWindow_ImGui* m_pMemoryWindow;

    // Object list.
    GameObject* m_pGameObjectToDrawReorderLineAfter;
    bool m_SetObjectListFilterBoxInFocus;
    char m_ObjectListFilter[100];

    // Memory panel.
    int m_CurrentMemoryPanelPage;
    bool m_SetMemoryPanelFilterBoxInFocus;
    char m_MemoryPanelFilter[100];

    // For renaming things.
    bool m_RenamePressedThisFrame;
    bool m_ConfirmCurrentRenameOp;
    float m_RenameTimerForSlowDoubleClick;
    void* m_RenameOp_LastObjectClicked;
    GameObject* m_pGameObjectWhoseNameIsBeingEdited;
    MaterialDefinition* m_pMaterialWhoseNameIsBeingEdited;
    char m_NameBeingEdited[100];

    // Draw call debugger.
    int m_SelectedDrawCallCanvas;
    int m_SelectedDrawCallIndex;

    // Game and Editor windows.
    Vector2 m_GameWindowPos;
    Vector2 m_EditorWindowPos;
    Vector2 m_GameWindowSize;
    Vector2 m_EditorWindowSize;

    EditorDocument* m_pActiveDocument;
    bool m_GameWindowFocused;
    bool m_GameWindowVisible;
    bool m_EditorWindowHovered;
    bool m_EditorWindowFocused;
    bool m_EditorWindowVisible;

    unsigned int m_CurrentMouseInEditorWindow_X;
    unsigned int m_CurrentMouseInEditorWindow_Y;

    // Misc.
    GameObject* m_pLastGameObjectInteractedWithInObjectPanel;
    unsigned int m_UndoStackDepthAtLastSave;

    // Modifier key states.
    bool m_KeyDownCtrl;
    bool m_KeyDownAlt;
    bool m_KeyDownShift;
    bool m_KeyDownCommand;

    // Master Undo/Redo Stack for ImGui editor builds.
    CommandStack* m_pCommandStack;

public:
    EditorMainFrame_ImGui();
    ~EditorMainFrame_ImGui();

    EditorLayoutManager_ImGui* GetLayoutManager() { return m_pLayoutManager; }

    Vector2 GetEditorWindowCenterPosition();
    bool IsGameWindowFocused() { return m_GameWindowFocused; }

    void StoreCurrentUndoStackSize();
    unsigned int GetUndoStackDepthAtLastSave() { return m_UndoStackDepthAtLastSave; }

    bool HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure);
    bool CheckForHotkeys(int keyAction, int keyCode);

    void RequestCloseWindow();

    void OnModeTogglePlayStop(bool nowInEditorMode);
    void Update(float deltaTime);

    void AddEverything();

    void DrawGameAndEditorWindows(EngineCore* pEngineCore);

    void EditMaterial(MaterialDefinition* pMaterial);
    void Edit2DAnimInfo(My2DAnimInfo* pAnimInfo);
    void AddInlineMaterial(MaterialDefinition* pMaterial);

    My2DAnimInfo* Get2DAnimInfoBeingEdited();
    void SetFullPathToLast2DAnimInfoBeingEdited(const char* fullPath);

protected:
    void StartRenameOp(GameObject* pGameObject, MaterialDefinition* pMaterial, const char* name);
    bool WasItemSlowDoubleClicked(void* pObjectClicked);

    void AddMainMenuBar();
    void AddLoseChangesWarningPopups();
    void AddGameAndEditorWindows();
    void AddObjectList();
    void AddPrefabFiles(bool forceOpen);
    void AddGameObjectToObjectList(GameObject* pGameObject, PrefabObject* pPrefab);
    void AddWatchPanel();
    void AddLogWindow();
    void AddMemoryWindow();
    void AddMemoryPanel();

    void AddContextMenuOptionsForAddingComponents(GameObject* pGameObject);
    void AddContextMenuOptionsForCreatingGameObjects(GameObject* pParentGameObject, SceneID sceneID);

public:
    void AddContextMenuItemsForFiles(MyFileObject* pFile, void* pSelectedObject = nullptr);
    void AddMaterialPreview(MaterialDefinition* pMaterial, bool createWindow, ImVec2 requestedSize, ImVec4 tint);
    void AddMaterialColorTexturePreview(MaterialDefinition* pMaterial, bool createWindow, ImVec2 requestedSize, ImVec4 tint);
    void AddTexturePreview(TextureDefinition* pTexture, bool createWindow, ImVec2 requestedSize, ImVec4 tint, ImVec2 startUV = ImVec2(0,0), ImVec2 endUV = ImVec2(1,1));

protected:
    void AddMemoryPanel_Materials();
    void AddMemoryPanel_Textures();
    void AddMemoryPanel_ShaderGroups();
    void AddMemoryPanel_SoundCues();
    void AddMemoryPanel_Files();
    void AddMemoryPanel_DrawCalls();

    void AddMaterialEditor();
    void Add2DAnimationEditor();

    void AddDebug_MousePicker();

    bool OnDropObjectList(GameObject* pGameObject, bool forceReorder);
    void OnDropEditorWindow();
};

#endif //__EditorMainFrame_ImGui_H__
