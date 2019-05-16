//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorKeyBindings.h"

const char* g_KeyBindingStrings[EditorKeyBindings::KeyAction_Num] =
{
    "Camera_Forward",
    "Camera_Back",
    "Camera_Left",
    "Camera_Right",
    "Camera_Up",
    "Camera_Down",
    "Camera_Focus",
};

EditorKeyBindings::EditorKeyBindings()
{
    m_DefaultKeys[0][KeyAction_Camera_Forward].x = 'W';
    m_DefaultKeys[0][KeyAction_Camera_Back   ].x = 'S';
    m_DefaultKeys[0][KeyAction_Camera_Left   ].x = 'A';
    m_DefaultKeys[0][KeyAction_Camera_Right  ].x = 'D';
    m_DefaultKeys[0][KeyAction_Camera_Up     ].x = 'Q';
    m_DefaultKeys[0][KeyAction_Camera_Down   ].x = 'E';
    m_DefaultKeys[0][KeyAction_Camera_Down   ].y = 'Z';
    m_DefaultKeys[0][KeyAction_Camera_Focus  ].x = 'F';

    // Copy preset 0 into other 4 presets.
    for( int i=1; i<5; i++ )
    {
        for( int j=0; j<KeyAction_Num; j++ )
        {
            m_DefaultKeys[i][j] = m_DefaultKeys[0][j];
        }
    }

    static const char* presets[] = { "MyDefaults", "Other1", "Other2", "Other3", "Other4" };
    m_ppPresetNames = presets;
    m_NumPresets = 5;

    for( int preset=0; preset<m_NumPresets; preset++ )
    {
        m_CurrentPreset = preset;
        ResetCurrentPreset();
    }

    m_CurrentPreset = 0; // Default to MyDefaults.
}

EditorKeyBindings::~EditorKeyBindings()
{
}

void EditorKeyBindings::ResetCurrentPreset()
{
    for( int i=0; i<KeyAction_Num; i++ )
    {
        m_Keys[m_CurrentPreset][i] = m_DefaultKeys[m_CurrentPreset][i];
    }
}

void EditorKeyBindings::LoadPrefs(cJSON* jPrefs)
{
    cJSONExt_GetInt( jPrefs, "KeyBindingCurrentPreset", &m_CurrentPreset );
    if( m_CurrentPreset < 0 || m_CurrentPreset >= m_NumPresets )
        m_CurrentPreset = 0;

    // Load all keys.
    cJSON* jPresetsArray = cJSON_GetObjectItem( jPrefs, "KeyBindingPresets" );
    for( int preset=0; preset<m_NumPresets; preset++ )
    {
        cJSON* jPreset = cJSON_GetArrayItem( jPresetsArray, preset );
        if( jPreset )
        {
            for( int i=0; i<KeyAction_Num; i++ )
            {
                Vector4Int key = m_DefaultKeys[preset][i];
                cJSONExt_GetIntArray( jPreset, g_KeyBindingStrings[i], &key.x, 4 );

                m_Keys[preset][i] = key;
            }
        }
    }
}

void EditorKeyBindings::SavePrefs(cJSON* jPrefs)
{
    // Save all key bindings.
    cJSON_AddNumberToObject( jPrefs, "KeyBindingCurrentPreset", m_CurrentPreset );

    cJSON* jPresetsArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jPrefs, "KeyBindingPresets", jPresetsArray );
    for( int preset=0; preset<m_NumPresets; preset++ )
    {
        cJSON* jPreset = cJSON_CreateObject();
        cJSON_AddItemToArray( jPresetsArray, jPreset );
        for( int i=0; i<KeyAction_Num; i++ )
        {
            if( m_Keys[preset][i] != m_DefaultKeys[preset][i] )
            {
                cJSONExt_AddIntArrayToObject( jPreset, g_KeyBindingStrings[i], &m_Keys[preset][i].x, 4 );
            }
        }
    }
}

Vector4Int EditorKeyBindings::GetKey(EditorKeyBindings::KeyActions index)
{
    return m_Keys[m_CurrentPreset][index];
}

void EditorKeyBindings::AddCustomizationTab()
{
    if( ImGui::BeginTabItem( "Key Bindings" ) )
    {
        ImGui::EndTabItem();

        ImGui::Combo( "Presets", &m_CurrentPreset, m_ppPresetNames, m_NumPresets );
        ImGui::SameLine();
        if( ImGui::Button( "Reset" ) ) { ResetCurrentPreset(); }

        int numColumns = 4;
        for( int i=0; i<KeyAction_Num; i++ )
        {
            if( i%numColumns != 0 )
                ImGui::SameLine( 180.0f * (i%numColumns) );

            ImGui::Text( g_KeyBindingStrings[i] );
            //ImGui::ColorEdit4( g_StylePrefsStrings[i], &m_Keys[m_CurrentPreset][i].x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar );
        }
    }
}
