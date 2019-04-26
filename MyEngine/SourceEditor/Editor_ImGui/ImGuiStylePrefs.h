//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ImGuiStylePrefs_H__
#define __ImGuiStylePrefs_H__

class ImGuiStylePrefs
{
public:
    enum StylePrefs
    {
        StylePref_Color_Text,
        StylePref_Color_TextDisabled,
        StylePref_Color_WindowBg,
        StylePref_Color_ChildBg,
        StylePref_Color_PopupBg,
        StylePref_Color_Border,
        StylePref_Color_BorderShadow,
        StylePref_Color_FrameBg,
        StylePref_Color_FrameBgHovered,
        StylePref_Color_FrameBgActive,
        StylePref_Color_TitleBg,
        StylePref_Color_TitleBgActive,
        StylePref_Color_TitleBgCollapsed,
        StylePref_Color_MenuBarBg,
        StylePref_Color_ScrollbarBg,
        StylePref_Color_ScrollbarGrab,
        StylePref_Color_ScrollbarGrabHovered,
        StylePref_Color_ScrollbarGrabActive,
        StylePref_Color_CheckMark,
        StylePref_Color_SliderGrab,
        StylePref_Color_SliderGrabActive,
        StylePref_Color_Button,
        StylePref_Color_ButtonHovered,
        StylePref_Color_ButtonActive,
        StylePref_Color_Header,
        StylePref_Color_HeaderHovered,
        StylePref_Color_HeaderActive,
        StylePref_Color_Separator,
        StylePref_Color_SeparatorHovered,
        StylePref_Color_SeparatorActive,
        StylePref_Color_ResizeGrip,
        StylePref_Color_ResizeGripHovered,
        StylePref_Color_ResizeGripActive,
        //StylePref_Color_CloseButton,
        //StylePref_Color_CloseButtonHovered,
        //StylePref_Color_CloseButtonActive,
        StylePref_Color_Tab,
        StylePref_Color_TabHovered,
        StylePref_Color_TabActive,
        StylePref_Color_TabUnfocused,
        StylePref_Color_TabUnfocusedActive,
        StylePref_Color_DockingPreview,
        StylePref_Color_DockingEmptyBg,
        StylePref_Color_PlotLines,
        StylePref_Color_PlotLinesHovered,
        StylePref_Color_PlotHistogram,
        StylePref_Color_PlotHistogramHovered,
        StylePref_Color_TextSelectedBg,
        StylePref_Color_DragDropTarget,
        StylePref_Color_NavHighlight,
        StylePref_Color_NavWindowingHighlight,
        StylePref_Color_NavWindowingDimBg,
        StylePref_Color_ModalWindowDimBg,
        StylePref_NumImGuiStyleColors,

        StylePref_Color_DivorcedVarText = StylePref_NumImGuiStyleColors,
        StylePref_Color_MultiSelectedVarDiffText,
        StylePref_Color_UnsetObjectButton,
        StylePref_Color_UnsetObjectText,
        StylePref_Color_TransformGizmoAlphaMin,
        StylePref_Color_TransformGizmoAlphaInUse,
        StylePref_Color_TransformGizmoAlphaMax,
        StylePref_Color_GameRunningMenuBarColor,
        StylePref_Color_LogTextInfo,
        StylePref_Color_LogTextError,
        StylePref_Color_LogTextDebug,
        StylePref_Num,
    };

protected:
    bool m_Visible;

    int m_CurrentPreset;
    const char** m_ppPresetNames;
    int m_NumPresets;

    Vector4 m_DefaultColors[5][StylePref_Num];
    Vector4 m_Styles[5][StylePref_Num];

public:
    ImGuiStylePrefs();
    ~ImGuiStylePrefs();

    void ResetCurrentPreset();
    void ReapplyCurrentPreset();

    void LoadPrefs(cJSON* jPrefs);
    void SavePrefs(cJSON* jPrefs);

    Vector4 GetColor(StylePrefs index);

    void Display();
    void AddCustomizationDialog();
};

#endif //__ImGuiStylePrefs_H__
