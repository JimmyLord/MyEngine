//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "../SourceCommon/GUI/ImGuiExtensions.h"
#include "../SourceEditor/Editor_ImGui/ImGuiStylePrefs.h"

char* g_StylePrefStrings[ImGuiStylePrefs::StylePref_Num] =
{
    "ColorText",
    "ColorTextDisabled",
    "ColorWindowBg",
    "ColorChildBg",
    "ColorPopupBg",
    "ColorBorder",
    "ColorBorderShadow",
    "ColorFrameBg",
    "ColorFrameBgHovered",
    "ColorFrameBgActive",
    "ColorTitleBg",
    "ColorTitleBgActive",
    "ColorTitleBgCollapsed",
    "ColorMenuBarBg",
    "ColorScrollbarBg",
    "ColorScrollbarGrab",
    "ColorScrollbarGrabHovered",
    "ColorScrollbarGrabActive",
    "ColorCheckMark",
    "ColorSliderGrab",
    "ColorSliderGrabActive",
    "ColorButton",
    "ColorButtonHovered",
    "ColorButtonActive",
    "ColorHeader",
    "ColorHeaderHovered",
    "ColorHeaderActive",
    "ColorSeparator",
    "ColorSeparatorHovered",
    "ColorSeparatorActive",
    "ColorResizeGrip",
    "ColorResizeGripHovered",
    "ColorResizeGripActive",
    "ColorCloseButton",
    "ColorCloseButtonHovered",
    "ColorCloseButtonActive",
    "ColorPlotLines",
    "ColorPlotLinesHovered",
    "ColorPlotHistogram",
    "ColorPlotHistogramHovered",
    "ColorTextSelectedBg",
    "ColorModalWindowDarkening",
    "ColorDragDropTarget",
    "ColorNavHighlight",
    "ColorNavWindowingHighlight",
};

ImGuiStylePrefs::ImGuiStylePrefs()
{
    for( int i=0; i<StylePref_Num; i++ )
    {
        m_DefaultColors[i] = ImGui::GetStyleColorVec4( i );
    }
}

ImGuiStylePrefs::~ImGuiStylePrefs()
{
}

void ImGuiStylePrefs::LoadPrefs(cJSON* jPrefs)
{
    // Load all style colors.
    cJSON* jColorArray = cJSON_GetObjectItem( jPrefs, "Styles" );
    if( jColorArray )
    {
        for( int i=0; i<StylePref_Num; i++ )
        {
            ImVec4 color = ImGui::GetStyleColorVec4( i );
            //cJSONExt_GetFloatArrayFromArray( jColorArray, i, &color.x, 4 );
            cJSONExt_GetFloatArray( jColorArray, g_StylePrefStrings[i], &color.x, 4 );
            ImGuiExt::SetStyleColorVec4( i, color );
        }
    }
}

void ImGuiStylePrefs::SavePrefs(cJSON* jPrefs)
{
    // Save all style colors.
    //cJSON* jColorArray = cJSON_CreateArray();
    cJSON* jColorArray = cJSON_CreateObject();
    cJSON_AddItemToObject( jPrefs, "Styles", jColorArray );
    for( int i=0; i<StylePref_Num; i++ )
    {
        Vector4 color = ImGui::GetStyleColorVec4( i );

        if( color != m_DefaultColors[i] )
        {
            //cJSONExt_AddFloatArrayToArray( jColorArray, &color.x, 4 );
            cJSONExt_AddFloatArrayToObject( jColorArray, g_StylePrefStrings[i], &color.x, 4 );
        }
    }
}
