//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorLayoutManager_ImGui_H__
#define __EditorLayoutManager_ImGui_H__

#include "EditorMainFrame_ImGui.h"

class EngineCore;
class EditorLogWindow_ImGui;

enum EditorLayoutTypes
{
    EditorLayout_CenterEditor,
    EditorLayout_CenterGame,
    EditorLayout_CenterSideBySide,
    EditorLayout_FullFrameGame,
    EditorLayout_NumLayouts,
};

extern const char* g_EditorLayoutMenuLabels[EditorLayout_NumLayouts];

class EditorLayout
{
public:
    // "Is window open" booleans.
    bool m_IsWindowOpen[EditorWindow_NumTypes];

    std::string m_ImGuiIniString;
};

class EditorLayoutManager_ImGui
{
protected:
    EditorMainFrame_ImGui* m_pEditorMainFrame_ImGui;

    EditorLayoutTypes m_CurrentLayoutIndex;
    EditorLayoutTypes m_RequestedLayoutIndex;

    EditorLayoutTypes m_SelectedLayout_EditorMode;
    EditorLayoutTypes m_SelectedLayout_GameMode;

    bool m_SwitchingToEditorLayout;
    bool m_SwitchingToGameLayout;

    EditorLayout m_DefaultLayouts[EditorLayout_NumLayouts];
    EditorLayout m_CustomLayouts[EditorLayout_NumLayouts];

public:
    EditorLayoutManager_ImGui(EditorMainFrame_ImGui* pEditorMainFrame_ImGui);
    ~EditorLayoutManager_ImGui();

    // Getters.
    EditorMainFrame_ImGui* GetEditorMainFrame_ImGui() { return m_pEditorMainFrame_ImGui; }
    EditorLayout* GetLayout(EditorLayoutTypes i) { return &m_CustomLayouts[i]; }
    EditorLayout* GetCurrentLayout();
    EditorLayoutTypes GetSelectedLayout_EditorMode() { return m_SelectedLayout_EditorMode; }
    EditorLayoutTypes GetSelectedLayout_GameMode() { return m_SelectedLayout_GameMode; }

    // Setters.
    void SetSelectedLayout_EditorMode(EditorLayoutTypes layout) { m_SelectedLayout_EditorMode = layout; }
    void SetSelectedLayout_GameMode(EditorLayoutTypes layout) { m_SelectedLayout_GameMode = layout; }

    // Load/Save.
    void LoadPrefs(cJSON* jPrefs);
    void SavePrefs(cJSON* jPrefs);
    void DumpCurrentLayoutToOutputWindow();
    void SyncCurrentImGuiIni();

    // Layout Change.
    void ResetCurrentLayout();
    void RequestLayoutChange(EditorLayoutTypes layout, bool discardModificationsAndForceChange);
    void RequestEditorLayout();
    void RequestGameLayout();

    void ApplyLayoutChange();
    void FinishFocusChangeIfNeeded();
};

#endif //__EditorLayoutManager_ImGui_H__
