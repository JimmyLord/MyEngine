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

class EngineCore;
class EditorLogWindow_ImGui;

enum EditorLayouts
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
    EditorLayouts m_CurrentLayoutIndex;
    EditorLayouts m_RequestedLayoutIndex;

    EditorLayout m_CustomLayouts[EditorLayout_NumLayouts];

public:
    EditorLayoutManager_ImGui();
    ~EditorLayoutManager_ImGui();

    EditorLayout* GetCurrentLayout() { return &m_CustomLayouts[m_CurrentLayoutIndex]; }

    void DumpCurrentLayoutToOutputWindow();

    void RequestLayoutChange(EditorLayouts layout);
    void ApplyLayoutChange();

    EditorLayout* GetLayout(EditorLayouts i) { return &m_CustomLayouts[i]; }
};

#endif //__EditorLayoutManager_ImGui_H__
