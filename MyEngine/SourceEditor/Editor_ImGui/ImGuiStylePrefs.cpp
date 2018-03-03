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

ImGuiStylePrefs::ImGuiStylePrefs()
{
    m_Visible = false;

    for( int i=0; i<StylePref_Num; i++ )
    {
        ImGui::StyleColorsClassic();
        m_DefaultColors[0][i] = ImGui::GetStyleColorVec4( i ); // Custom
        m_DefaultColors[1][i] = ImGui::GetStyleColorVec4( i ); // Classic

        ImGui::StyleColorsDark();
        m_DefaultColors[2][i] = ImGui::GetStyleColorVec4( i ); // Dark

        ImGui::StyleColorsLight();
        m_DefaultColors[3][i] = ImGui::GetStyleColorVec4( i ); // Light

        ImGui::StyleColorsClassic();
    }

    static const char* presets[] = { "Custom", "Classic", "Dark", "Light" };
    m_ppPresetNames = presets;
    m_NumPresets = 4;

    m_CurrentPreset = 0; // Default to custom, which should match Classic.

    for( int preset=0; preset<m_NumPresets; preset++ )
    {
        for( int i=0; i<StylePref_Num; i++ )
        {
            m_Styles[preset][i] = m_DefaultColors[preset][i];
        }
    }
}

ImGuiStylePrefs::~ImGuiStylePrefs()
{
}

void ImGuiStylePrefs::LoadPrefs(cJSON* jPrefs)
{
    cJSONExt_GetInt( jPrefs, "CurrentPreset", &m_CurrentPreset );
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
                cJSONExt_GetFloatArray( jPreset, ImGui::GetStyleColorName(i), &color.x, 4 );

                m_Styles[preset][i] = color;
            }
        }
    }

    // Apply the current preset.
    for( int i=0; i<StylePref_Num; i++ )
    {
        ImVec4 color = m_Styles[m_CurrentPreset][i];
        ImGuiExt::SetStyleColorVec4( i, color );
    }
}

void ImGuiStylePrefs::SavePrefs(cJSON* jPrefs)
{
    // Save all style colors.
    cJSON_AddNumberToObject( jPrefs, "CurrentPreset", m_CurrentPreset );

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
                //cJSONExt_AddFloatArrayToArray( jColorArray, &color.x, 4 );
                cJSONExt_AddFloatArrayToObject( jPreset, ImGui::GetStyleColorName(i), &m_Styles[preset][i].x, 4 );
            }
        }
    }
}

void ImGuiStylePrefs::Display()
{
    m_Visible = true;
}

void ImGuiStylePrefs::AddCustomizationDialog()
{
    if( m_Visible == false )
        return;

    //ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.1f, 0.1f, 0.1f, 0.9f ) );
    
    ImGui::SetNextWindowPos( ImVec2(208, 61), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowSize( ImVec2(745, 530), ImGuiCond_FirstUseEver );

    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    if( ImGui::Begin( "Editor Style Prefs", &m_Visible ) )
    {
        if( ImGui::Combo( "Presets", &m_CurrentPreset, m_ppPresetNames, m_NumPresets ) )
        {
            for( int i=0; i<StylePref_Num; i++ )
            {
                colors[i] = m_Styles[m_CurrentPreset][i];
            }
        }
        ImGui::SameLine();
        if( ImGui::Button( "Reset" ) )
        {
            for( int i=0; i<StylePref_Num; i++ )
            {
                colors[i] = m_DefaultColors[m_CurrentPreset][i];
            }
        }

        int numcols = 4;
        for( int i=0; i<StylePref_Num; i++ )
        {
            if( i%numcols != 0 )
                ImGui::SameLine( 180.0f * (i%numcols) );

            ImGui::ColorEdit4( ImGui::GetStyleColorName(i), &colors[i].x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar );
            
            m_Styles[m_CurrentPreset][i] = colors[i];
        }
    }
    ImGui::End();

    //ImGui::PopStyleColor();
}
