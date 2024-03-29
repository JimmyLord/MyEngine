//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"
#include "../SourceCommon/GUI/ImGuiExtensions.h"
#include "../SourceEditor/Editor_ImGui/ImGuiStylePrefs.h"

const char* g_StylePrefsStrings[ImGuiStylePrefs::StylePref_Num] =
{
    "Text",
    "TextDisabled",
    "WindowBg",
    "ChildBg",
    "PopupBg",
    "Border",
    "BorderShadow",
    "FrameBg",
    "FrameBgHovered",
    "FrameBgActive",
    "TitleBg",
    "TitleBgActive",
    "TitleBgCollapsed",
    "MenuBarBg",
    "ScrollbarBg",
    "ScrollbarGrab",
    "ScrollbarGrabHovered",
    "ScrollbarGrabActive",
    "CheckMark",
    "SliderGrab",
    "SliderGrabActive",
    "Button",
    "ButtonHovered",
    "ButtonActive",
    "Header",
    "HeaderHovered",
    "HeaderActive",
    "Separator",
    "SeparatorHovered",
    "SeparatorActive",
    "ResizeGrip",
    "ResizeGripHovered",
    "ResizeGripActive",
    //"CloseButton",
    //"CloseButtonHovered",
    //"CloseButtonActive",
    "Tab",
    "TabHovered",
    "TabActive",
    "TabUnfocused",
    "TabUnfocusedActive",
    "DockingPreview",
    "DockingEmptyBg",
    "PlotLines",
    "PlotLinesHovered",
    "PlotHistogram",
    "PlotHistogramHovered",
    "TableHeaderBg",
    "TableBorderStrong",
    "TableBorderLight",
    "TableRowBg",
    "TableRowBgAlt",
    "TextSelectedBg",
    "DragDropTarget",
    "NavHighlight",
    "NavWindowingHighlight",
    "NavWindowingDimBg",
    "ModalWindowDimBg",

    "DivorcedText",
    "MultiSelectedVarDiffText",
    "UnsetObjectButton",
    "UnsetObjectText",
    "TransformAlphaMin",
    "TransformAlphaInUse",
    "TransformAlphaMax",
    "GameRunningMenuBarColor",
    "LogTextInfo",
    "LogTextError",
    "LogTextDebug",
};

ImGuiStylePrefs::ImGuiStylePrefs()
{
    // If this trips, imgui changed, so make fixes.
    MyAssert( StylePref_NumImGuiStyleColors == 55 );

    for( int i=0; i<StylePref_NumImGuiStyleColors; i++ )
    {
        // If this trips, imgui changed, so make fixes.
        MyAssert( strcmp( g_StylePrefsStrings[i], ImGui::GetStyleColorName(i) ) == 0 );

        ImGui::StyleColorsClassic();
        m_DefaultColors[0][i] = ImGui::GetStyleColorVec4( i ); // MyDefaults
        m_DefaultColors[1][i] = ImGui::GetStyleColorVec4( i ); // Custom
        m_DefaultColors[2][i] = ImGui::GetStyleColorVec4( i ); // Classic

        ImGui::StyleColorsDark();
        m_DefaultColors[3][i] = ImGui::GetStyleColorVec4( i ); // Dark

        ImGui::StyleColorsLight();
        m_DefaultColors[4][i] = ImGui::GetStyleColorVec4( i ); // Light

        ImGui::StyleColorsClassic();
    }

    // Set default values for non-ImGui colors.
    {
        // MyDefaults (Modified from Classic)
		m_DefaultColors[0][StylePref_Color_WindowBg]                .Set( 0, 0, 0, 1 );
		m_DefaultColors[0][StylePref_Color_Header]                  .Set( 0.260747f, 0.309392f, 0.222215f, 1 );
		m_DefaultColors[0][StylePref_Color_HeaderHovered]           .Set( 0.334156f, 0.397790f, 0.290101f, 1 );
		m_DefaultColors[0][StylePref_Color_HeaderActive]            .Set( 0.441434f, 0.535912f, 0.376026f, 1 );
        m_DefaultColors[0][StylePref_Color_DivorcedVarText]         .Set( 1, 0.5f, 0, 1 );
		m_DefaultColors[0][StylePref_Color_MultiSelectedVarDiffText].Set( 0.806630f, 1, 0, 1 );
        m_DefaultColors[0][StylePref_Color_UnsetObjectButton]        = m_DefaultColors[0][StylePref_Color_Button] * 0.5f;
        m_DefaultColors[0][StylePref_Color_UnsetObjectText]          = m_DefaultColors[0][StylePref_Color_Text] * 0.5f;
        m_DefaultColors[0][StylePref_Color_TransformGizmoAlphaMin]  .Set( 1, 1, 1, 0.05f );
        m_DefaultColors[0][StylePref_Color_TransformGizmoAlphaInUse].Set( 1, 1, 1, 0.2f );
        m_DefaultColors[0][StylePref_Color_TransformGizmoAlphaMax]  .Set( 1, 1, 1, 1.0f );
        m_DefaultColors[0][StylePref_Color_GameRunningMenuBarColor] .Set( 0.042f, 0.481f, 0.067f, 1.000f );
        m_DefaultColors[0][StylePref_Color_LogTextInfo]             .Set( 1.0f, 1.0f, 1.0f, 1.0f );
        m_DefaultColors[0][StylePref_Color_LogTextError]            .Set( 1.0f, 0.5f, 0.5f, 1.0f );
        m_DefaultColors[0][StylePref_Color_LogTextDebug]            .Set( 0.5f, 0.5f, 1.0f, 1.0f );

        // Copy some of style 0 into other 4 styles.
        for( int i=1; i<5; i++ )
        {
            // Copy all the extended (non-builtin imgui) colors.
            for( int j=StylePref_NumImGuiStyleColors; j<StylePref_Num; j++ )
            {
                m_DefaultColors[i][j] = m_DefaultColors[0][j];
            }
        }

        // Custom
        m_DefaultColors[1][StylePref_Color_MultiSelectedVarDiffText].Set( 0, 0.5f, 1, 1 );
        m_DefaultColors[1][StylePref_Color_UnsetObjectButton]        = m_DefaultColors[1][StylePref_Color_Button] * 0.5f;
        m_DefaultColors[1][StylePref_Color_UnsetObjectText]          = m_DefaultColors[1][StylePref_Color_Text] * 0.5f;

        // Classic (Same as Custom)
        m_DefaultColors[2][StylePref_Color_MultiSelectedVarDiffText] = m_DefaultColors[1][StylePref_Color_MultiSelectedVarDiffText];
        m_DefaultColors[2][StylePref_Color_UnsetObjectButton]        = m_DefaultColors[2][StylePref_Color_Button] * 0.5f;
        m_DefaultColors[2][StylePref_Color_UnsetObjectText]          = m_DefaultColors[2][StylePref_Color_Text] * 0.5f;

        // Dark
        m_DefaultColors[3][StylePref_Color_MultiSelectedVarDiffText] = m_DefaultColors[1][StylePref_Color_MultiSelectedVarDiffText];
        m_DefaultColors[3][StylePref_Color_UnsetObjectButton]        = m_DefaultColors[3][StylePref_Color_Button] * 0.5f;
        m_DefaultColors[3][StylePref_Color_UnsetObjectText]          = m_DefaultColors[3][StylePref_Color_Text] * 0.5f;
        m_DefaultColors[3][StylePref_Color_GameRunningMenuBarColor] .Set( 0.600f, 0.000f, 0.000f, 1.000f );

        // Light
        m_DefaultColors[4][StylePref_Color_MultiSelectedVarDiffText] = m_DefaultColors[1][StylePref_Color_MultiSelectedVarDiffText];
        m_DefaultColors[4][StylePref_Color_UnsetObjectButton]        = m_DefaultColors[4][StylePref_Color_Button] * 0.5f;
        m_DefaultColors[4][StylePref_Color_UnsetObjectText]          = m_DefaultColors[4][StylePref_Color_Text] * 0.5f;
        m_DefaultColors[4][StylePref_Color_GameRunningMenuBarColor]  .Set(0.284f, 0.934f, 0.794f, 1.000f);
        m_DefaultColors[4][StylePref_Color_LogTextInfo]              = m_DefaultColors[4][StylePref_Color_Text];
        m_DefaultColors[4][StylePref_Color_LogTextError]             = m_DefaultColors[4][StylePref_Color_Text] + Vector4(0.6f, 0.2f, 0.2f, 0.0f);
        m_DefaultColors[4][StylePref_Color_LogTextDebug]             = m_DefaultColors[4][StylePref_Color_Text] + Vector4(0.0f, 0.0f, 0.5f, 0.0f);
    }

    static const char* presets[] = { "MyDefaults", "Custom", "Classic", "Dark", "Light" };
    m_ppPresetNames = presets;
    m_NumPresets = 5;

    for( int preset=0; preset<m_NumPresets; preset++ )
    {
        m_CurrentPreset = preset;
        ResetCurrentPreset();
    }

    m_CurrentPreset = 0; // Default to MyDefaults.
}

ImGuiStylePrefs::~ImGuiStylePrefs()
{
}

void ImGuiStylePrefs::ResetCurrentPreset()
{
    for( int i=0; i<StylePref_Num; i++ )
    {
        m_Styles[m_CurrentPreset][i] = m_DefaultColors[m_CurrentPreset][i];
    }
}

void ImGuiStylePrefs::ReapplyCurrentPreset()
{
    // Apply the current preset for all imgui style colors.
    for( int i=0; i<StylePref_NumImGuiStyleColors; i++ )
    {
        ImVec4 color = m_Styles[m_CurrentPreset][i];
        ImGuiExt::SetStyleColorVec4( i, color );
    }
}

void ImGuiStylePrefs::LoadPrefs(cJSON* jPrefs)
{
    cJSONExt_GetInt( jPrefs, "StyleCurrentPreset", &m_CurrentPreset );
    if( m_CurrentPreset < 0 || m_CurrentPreset >= m_NumPresets )
        m_CurrentPreset = 0;

    // Load all style preset colors.
    cJSON* jPresetsArray = cJSON_GetObjectItem( jPrefs, "StylePresets" );
    for( int preset=0; preset<m_NumPresets; preset++ )
    {
        cJSON* jPreset = cJSON_GetArrayItem( jPresetsArray, preset );
        if( jPreset )
        {
            for( int i=0; i<StylePref_Num; i++ )
            {
                ImVec4 color = m_DefaultColors[preset][i];
                cJSONExt_GetFloatArray( jPreset, g_StylePrefsStrings[i], &color.x, 4 );

                m_Styles[preset][i] = color;
            }
        }
    }

    ReapplyCurrentPreset();
}

void ImGuiStylePrefs::SavePrefs(cJSON* jPrefs)
{
    // Save all style colors.
    cJSON_AddNumberToObject( jPrefs, "StyleCurrentPreset", m_CurrentPreset );

    cJSON* jPresetsArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jPrefs, "StylePresets", jPresetsArray );
    for( int preset=0; preset<m_NumPresets; preset++ )
    {
        cJSON* jPreset = cJSON_CreateObject();
        cJSON_AddItemToArray( jPresetsArray, jPreset );
        for( int i=0; i<StylePref_Num; i++ )
        {
            if( m_Styles[preset][i] != m_DefaultColors[preset][i] )
            {
                cJSONExt_AddFloatArrayToObject( jPreset, g_StylePrefsStrings[i], &m_Styles[preset][i].x, 4 );
            }
        }
    }
}

Vector4 ImGuiStylePrefs::GetColor(ImGuiStylePrefs::StylePrefs index)
{
    return m_Styles[m_CurrentPreset][index];
}

void ImGuiStylePrefs::AddCustomizationTab()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    if( ImGui::BeginTabItem( "Style" ) )
    {
        ImGui::EndTabItem();

        ImGui::Combo( "Presets", &m_CurrentPreset, m_ppPresetNames, m_NumPresets );
        ImGui::SameLine();
        if( ImGui::Button( "Reset" ) ) { ResetCurrentPreset(); }

        int numColumns = 4;
        for( int i=0; i<StylePref_Num; i++ )
        {
            if( i%numColumns != 0 )
                ImGui::SameLine( 180.0f * (i%numColumns) );

            ImGui::ColorEdit4( g_StylePrefsStrings[i], &m_Styles[m_CurrentPreset][i].x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar );

            // Copy the color to the ImGui state.
            if( i < StylePref_NumImGuiStyleColors )
                colors[i] = m_Styles[m_CurrentPreset][i];
        }
    }
}
