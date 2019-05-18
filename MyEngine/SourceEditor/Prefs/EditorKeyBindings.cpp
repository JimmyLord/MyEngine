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
    m_DefaultKeys[0][KeyAction_Camera_Forward].m_Keys[0].m_Key = 'W';
    m_DefaultKeys[0][KeyAction_Camera_Back   ].m_Keys[0].m_Key = 'S';
    m_DefaultKeys[0][KeyAction_Camera_Left   ].m_Keys[0].m_Key = 'A';
    m_DefaultKeys[0][KeyAction_Camera_Right  ].m_Keys[0].m_Key = 'D';
    m_DefaultKeys[0][KeyAction_Camera_Up     ].m_Keys[0].m_Key = 'Q';
    m_DefaultKeys[0][KeyAction_Camera_Down   ].m_Keys[0].m_Key = 'E';
    m_DefaultKeys[0][KeyAction_Camera_Down   ].m_Keys[1].m_Key = 'Z';
    m_DefaultKeys[0][KeyAction_Camera_Focus  ].m_Keys[0].m_Key = 'F';

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
                int numUnsignedChars = MaxKeysPerAction * 2;

                KeyBinding key = m_DefaultKeys[preset][i];
                cJSONExt_GetUnsignedCharArray( jPreset, g_KeyBindingStrings[i], reinterpret_cast<unsigned char*>( &key ), numUnsignedChars );
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
                int numUnsignedChars = MaxKeysPerAction * 2;

                cJSONExt_AddUnsignedCharArrayToObject( jPreset, g_KeyBindingStrings[i], reinterpret_cast<unsigned char*>( &m_Keys[preset][i] ), numUnsignedChars );
            }
        }
    }
}

EditorKeyBindings::KeyBinding EditorKeyBindings::GetKey(EditorKeyBindings::KeyActions index)
{
    return m_Keys[m_CurrentPreset][index];
}

bool EditorKeyBindings::KeyMatches(EditorKeyBindings::KeyActions index, uint8 modifiers, uint8 keyCode)
{
    // Only keep the first 3 bits of the modifiers.
    modifiers &= (1+2+4);

    for( int k=0; k<MaxKeysPerAction; k++ )
    {
        if( m_Keys[m_CurrentPreset][index].m_Keys[k].m_Flags == modifiers &&
            m_Keys[m_CurrentPreset][index].m_Keys[k].m_Key == keyCode )
            return true;
    }

    return false;
}

void EditorKeyBindings::AddCustomizationTab()
{
    if( ImGui::BeginTabItem( "Key Bindings" ) )
    {
        ImGui::EndTabItem();

        ImGui::Combo( "Presets", &m_CurrentPreset, m_ppPresetNames, m_NumPresets );
        ImGui::SameLine();
        if( ImGui::Button( "Reset" ) ) { ResetCurrentPreset(); }

        ImGui::Columns( 3 );

        for( int i=0; i<KeyAction_Num; i++ )
        {
            ImGui::Text( g_KeyBindingStrings[i] );

            ImGui::NextColumn();

            for( int k=0; k<MaxKeysPerAction; k++ )
            {
                ImGui::PushID( i*MaxKeysPerAction + k );

                bool flagControl = m_Keys[m_CurrentPreset][i].m_Keys[k].m_Flags & 1;
                bool flagAlt     = m_Keys[m_CurrentPreset][i].m_Keys[k].m_Flags & 2;
                bool flagShift   = m_Keys[m_CurrentPreset][i].m_Keys[k].m_Flags & 4;
                char tempString[2];
                tempString[0] = m_Keys[m_CurrentPreset][i].m_Keys[k].m_Key;
                tempString[1] = '\0';

                ImGui::Checkbox( "C", &flagControl );           ImGui::SameLine();
                ImGui::Checkbox( "A", &flagAlt );               ImGui::SameLine();
                ImGui::Checkbox( "S", &flagShift );             ImGui::SameLine();
                ImGui::PushItemWidth( 20 );
                ImGui::InputText( "Key", tempString, 2, ImGuiInputTextFlags_AutoSelectAll );
                ImGui::PopItemWidth();

                m_Keys[m_CurrentPreset][i].m_Keys[k].m_Flags = flagControl << 0 | flagAlt << 1 | flagShift << 2;
                if( tempString[0] < 32 && tempString[0] != 0 )
                    tempString[0] = 33;
                if( tempString[0] >= 97 && tempString[0] <= 122 )
                    tempString[0] -= 'a' - 'A';
                m_Keys[m_CurrentPreset][i].m_Keys[k].m_Key = static_cast<unsigned char>( tempString[0] );

                ImGui::NextColumn();

                ImGui::PopID();
            }
        }

        ImGui::Columns( 1 );
    }
}
