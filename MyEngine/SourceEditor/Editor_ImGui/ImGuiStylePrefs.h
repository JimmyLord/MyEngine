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
        StylePref_ColorText,
        StylePref_ColorTextDisabled,
        StylePref_ColorWindowBg,
        StylePref_ColorChildBg,
        StylePref_ColorPopupBg,
        StylePref_ColorBorder,
        StylePref_ColorBorderShadow,
        StylePref_ColorFrameBg,
        StylePref_ColorFrameBgHovered,
        StylePref_ColorFrameBgActive,
        StylePref_ColorTitleBg,
        StylePref_ColorTitleBgActive,
        StylePref_ColorTitleBgCollapsed,
        StylePref_ColorMenuBarBg,
        StylePref_ColorScrollbarBg,
        StylePref_ColorScrollbarGrab,
        StylePref_ColorScrollbarGrabHovered,
        StylePref_ColorScrollbarGrabActive,
        StylePref_ColorCheckMark,
        StylePref_ColorSliderGrab,
        StylePref_ColorSliderGrabActive,
        StylePref_ColorButton,
        StylePref_ColorButtonHovered,
        StylePref_ColorButtonActive,
        StylePref_ColorHeader,
        StylePref_ColorHeaderHovered,
        StylePref_ColorHeaderActive,
        StylePref_ColorSeparator,
        StylePref_ColorSeparatorHovered,
        StylePref_ColorSeparatorActive,
        StylePref_ColorResizeGrip,
        StylePref_ColorResizeGripHovered,
        StylePref_ColorResizeGripActive,
        StylePref_ColorCloseButton,
        StylePref_ColorCloseButtonHovered,
        StylePref_ColorCloseButtonActive,
        StylePref_ColorPlotLines,
        StylePref_ColorPlotLinesHovered,
        StylePref_ColorPlotHistogram,
        StylePref_ColorPlotHistogramHovered,
        StylePref_ColorTextSelectedBg,
        StylePref_ColorModalWindowDarkening,
        StylePref_ColorDragDropTarget,
        StylePref_ColorNavHighlight,
        StylePref_ColorNavWindowingHighlight,
        StylePref_Num,
    };

protected:
    bool m_Visible;

    int m_CurrentPreset;
    const char** m_ppPresetNames;
    int m_NumPresets;

    Vector4 m_DefaultColors[4][StylePref_Num];
    Vector4 m_Styles[4][StylePref_Num];

public:
    ImGuiStylePrefs();
    ~ImGuiStylePrefs();

    void LoadPrefs(cJSON* jPrefs);
    void SavePrefs(cJSON* jPrefs);

    void Display();
    void AddCustomizationDialog();
};

#endif //__ImGuiStylePrefs_H__
