//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorMainFrame_ImGui_H__
#define __EditorMainFrame_ImGui_H__

class EngineCore;

class EditorMainFrame_ImGui : public EditorMainFrame
{
protected:
    FBODefinition* m_pGameFBO;
    FBODefinition* m_pEditorFBO;
    FBODefinition* m_pMaterialPreviewFBO;
    MaterialDefinition* m_pMaterialToPreview;

    MaterialDefinition* m_pMaterialBeingEdited;
    bool m_IsMaterialEditorOpen;

    Vector2 m_GameWindowPos;
    Vector2 m_EditorWindowPos;
    Vector2 m_GameWindowSize;
    Vector2 m_EditorWindowSize;

    bool m_GameWindowFocused;
    bool m_EditorWindowHovered;
    bool m_EditorWindowFocused;

    int m_CurrentMemoryPanelPage;

    // Modifier key states
    bool m_KeyDownCtrl;
    bool m_KeyDownAlt;
    bool m_KeyDownShift;
    bool m_KeyDownCommand;

    // Master Undo/Redo Stack for ImGui editor builds.
    CommandStack* m_pCommandStack;

public:
    EditorMainFrame_ImGui();
    ~EditorMainFrame_ImGui();

    Vector2 GetEditorWindowCenterPosition();

    bool HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);
    bool CheckForHotkeys(int keyaction, int keycode);

    void AddEverything();
    void AddMainMenuBar();
    void AddGameAndEditorWindows();
    void AddObjectList();
    void AddGameObjectToObjectList(GameObject* pGameObject);
    void AddWatchPanel();
    void AddMemoryPanel();

    void AddMemoryPanel_Materials();
    void AddMemoryPanel_Textures();

    void AddMaterialEditor();

    void AddMaterialPreview(bool createWindow, ImVec2 requestedSize, ImVec4 tint);
    void AddTexturePreview(bool createWindow, TextureDefinition* pTex, ImVec2 requestedSize, ImVec4 tint);
    void AddDebug_MousePicker();

    void DrawGameAndEditorWindows(EngineCore* pEngineCore);
};

#endif //__EditorMainFrame_ImGui_H__
