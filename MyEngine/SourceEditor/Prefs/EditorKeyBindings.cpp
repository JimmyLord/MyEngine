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
    "File_SaveScene",
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
    m_DefaultKeys[0][KeyAction_File_SaveScene].m_Keys[0].m_Flags = 1;
    m_DefaultKeys[0][KeyAction_File_SaveScene].m_Keys[0].m_Key = 'S';
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

    // Key binding.
    m_RegisteringNewBinding = false;
    m_NewBindingPreset = 0;
    m_NewBindingKeyAction = KeyAction_Num;
    m_NewBindingKeyIndex = 0;
    m_NewBindingModifiers = 0;

    GenerateKeyStrings();
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
                int numUnsignedInts = MaxKeysPerAction * 2;

                KeyBinding key = m_DefaultKeys[preset][i];
                cJSONExt_GetUnsignedIntArray( jPreset, g_KeyBindingStrings[i], reinterpret_cast<unsigned int*>( &key ), numUnsignedInts );
                m_Keys[preset][i] = key;
            }
        }
    }

    GenerateKeyStrings();
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
                int numUnsignedInts = MaxKeysPerAction * 2;

                cJSONExt_AddUnsignedIntArrayToObject( jPreset, g_KeyBindingStrings[i], reinterpret_cast<unsigned int*>( &m_Keys[preset][i] ), numUnsignedInts );
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

const char* EditorKeyBindings::GetStringForKey(EditorKeyBindings::KeyActions index)
{
    // Only return the string for the first key.
    return m_KeyStrings[m_CurrentPreset][index][0];
}

void EditorKeyBindings::AddCustomizationTab()
{
    if( ImGui::BeginTabItem( "Key Bindings" ) )
    {
        ImGui::EndTabItem();

        if( ImGui::Combo( "Presets", &m_CurrentPreset, m_ppPresetNames, m_NumPresets ) )
        {
            CancelBindingAction();
        }
        ImGui::SameLine();
        if( ImGui::Button( "Reset" ) )
        {
            CancelBindingAction();
            ResetCurrentPreset();
        }

        ImGui::Columns( 3 );

        for( uint32 action=0; action<KeyAction_Num; action++ )
        {
            ImGui::Text( g_KeyBindingStrings[action] );

            ImGui::NextColumn();

            for( int keyIndex=0; keyIndex<MaxKeysPerAction; keyIndex++ )
            {
                ImGui::PushID( action*MaxKeysPerAction + keyIndex );

                if( m_RegisteringNewBinding &&
                    m_CurrentPreset == m_NewBindingPreset &&
                    m_NewBindingKeyAction == static_cast<KeyActions>( action ) &&
                    m_NewBindingKeyIndex == keyIndex )
                {
                    ImGui::Text( "Waiting for key press..." );
                }
                else if( ImGui::Button( m_KeyStrings[m_CurrentPreset][action][keyIndex] ) )
                {
                    m_RegisteringNewBinding = true;
                    m_NewBindingPreset = m_CurrentPreset;
                    m_NewBindingKeyAction = static_cast<KeyActions>( action );
                    m_NewBindingKeyIndex = keyIndex;
                    m_NewBindingModifiers = 0;
                }

                ImGui::NextColumn();

                ImGui::PopID();
            }
        }

        ImGui::Columns( 1 );
    }
    else
    {
        CancelBindingAction();
    }
}

bool EditorKeyBindings::HandleInput(int keyAction, int keyCode)
{
    if( m_RegisteringNewBinding )
    {
        if( keyAction == GCBA_Up )
        {
            if( keyCode == MYKEYCODE_LCTRL  || keyCode == MYKEYCODE_RCTRL  ) { m_NewBindingModifiers &= ~1; return true; }
            if( keyCode == MYKEYCODE_LALT   || keyCode == MYKEYCODE_RALT   ) { m_NewBindingModifiers &= ~2; return true; }
            if( keyCode == MYKEYCODE_LSHIFT || keyCode == MYKEYCODE_RSHIFT ) { m_NewBindingModifiers &= ~4; return true; }
        }

        if( keyAction == GCBA_Down )
        {
            if( keyCode == MYKEYCODE_LCTRL  || keyCode == MYKEYCODE_RCTRL  ) { m_NewBindingModifiers |= 1; return true; }
            if( keyCode == MYKEYCODE_LALT   || keyCode == MYKEYCODE_RALT   ) { m_NewBindingModifiers |= 2; return true; }
            if( keyCode == MYKEYCODE_LSHIFT || keyCode == MYKEYCODE_RSHIFT ) { m_NewBindingModifiers |= 4; return true; }

            if( keyCode == MYKEYCODE_DELETE )
            {
                // Clear the key binding in delete is pressed.
                keyCode = 0;
                m_NewBindingModifiers = 0;
            }

            //LOGInfo( LOGTag, "New key bound: %d - %d", m_NewBindingModifiers, keyCode );
            m_RegisteringNewBinding = false;

            m_Keys[m_NewBindingPreset][m_NewBindingKeyAction].m_Keys[m_NewBindingKeyIndex].m_Key = keyCode;
            m_Keys[m_NewBindingPreset][m_NewBindingKeyAction].m_Keys[m_NewBindingKeyIndex].m_Flags = m_NewBindingModifiers;

            GenerateKeyStrings();

            return true;
        }
    }

    return false;
}

void EditorKeyBindings::CancelBindingAction()
{
    // Cancel the last binding action.
    m_RegisteringNewBinding = false;
    m_NewBindingModifiers = 0;
}

void EditorKeyBindings::GenerateKeyStrings()
{
    memset( m_KeyStrings, 0, sizeof( m_KeyStrings ) );

    for( int preset=0; preset<5; preset++ )
    {
        for( int action=0; action<KeyAction_Num; action++ )
        {
            for( int keyIndex=0; keyIndex<MaxKeysPerAction; keyIndex++ )
            {
                if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Flags & 1 )
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Ctrl-" );
                if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Flags & 2 )
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Alt-" );
                if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Flags & 4 )
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Shift-" );

                static const int bufferSize = 32;
                char keyName[bufferSize];
                uint32 keyCode = m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Key;
                
                if( keyCode == 0 )                          sprintf_s( keyName, bufferSize, "" );
                else if( keyCode == 8 )                     sprintf_s( keyName, bufferSize, "BackSpace" );
                else if( keyCode == 9 )                     sprintf_s( keyName, bufferSize, "Tab" );
                else if( keyCode == 13 )                    sprintf_s( keyName, bufferSize, "Enter" );
                else if( keyCode == 19 )                    sprintf_s( keyName, bufferSize, "Pause" );
                else if( keyCode == 20 )                    sprintf_s( keyName, bufferSize, "CapsLock" );
                else if( keyCode == 27 )                    sprintf_s( keyName, bufferSize, "Escape" );
                else if( keyCode == 32 )                    sprintf_s( keyName, bufferSize, "Spacebar" );
                else if( keyCode == 33 )                    sprintf_s( keyName, bufferSize, "PageUp" );
                else if( keyCode == 34 )                    sprintf_s( keyName, bufferSize, "PageDown" );
                else if( keyCode == 35 )                    sprintf_s( keyName, bufferSize, "End" );
                else if( keyCode == 36 )                    sprintf_s( keyName, bufferSize, "Home" );
                else if( keyCode == 37 )                    sprintf_s( keyName, bufferSize, "Left" );
                else if( keyCode == 38 )                    sprintf_s( keyName, bufferSize, "Up" );
                else if( keyCode == 39 )                    sprintf_s( keyName, bufferSize, "Right" );
                else if( keyCode == 40 )                    sprintf_s( keyName, bufferSize, "Down" );
                else if( keyCode == 44 )                    sprintf_s( keyName, bufferSize, "," );
                else if( keyCode == 45 )                    sprintf_s( keyName, bufferSize, "Insert" );
                else if( keyCode == 46 )                    sprintf_s( keyName, bufferSize, "." );
                else if( keyCode >= 48 && keyCode <= 57 )   sprintf_s( keyName, bufferSize, "%c", keyCode ); // 0-9
                else if( keyCode >= 65 && keyCode <= 90 )   sprintf_s( keyName, bufferSize, "%c", keyCode ); // A-Z
                else if( keyCode >= 112 && keyCode <= 123 ) sprintf_s( keyName, bufferSize, "F%d", keyCode - 112 + 1 ); // F1-F12
                else if( keyCode == 91 )                    sprintf_s( keyName, bufferSize, "[" );
                else if( keyCode == 93 )                    sprintf_s( keyName, bufferSize, "]" );
                else if( keyCode == 127 )                   sprintf_s( keyName, bufferSize, "Delete" );
                else if( keyCode == 144 )                   sprintf_s( keyName, bufferSize, "NumLock" );
                else if( keyCode == 145 )                   sprintf_s( keyName, bufferSize, "ScrollLock" );
                else if( keyCode == 186 )                   sprintf_s( keyName, bufferSize, ";" );
                else if( keyCode == 187 )                   sprintf_s( keyName, bufferSize, "=" );
                else if( keyCode == 188 )                   sprintf_s( keyName, bufferSize, "," );
                else if( keyCode == 189 )                   sprintf_s( keyName, bufferSize, "-" );
                else if( keyCode == 191 )                   sprintf_s( keyName, bufferSize, "/" );
                else if( keyCode == 192 )                   sprintf_s( keyName, bufferSize, "~" );
                else if( keyCode == 220 )                   sprintf_s( keyName, bufferSize, "\\" );
                else if( keyCode == 222 )                   sprintf_s( keyName, bufferSize, "'" );
                else if( keyCode == 226 )                   sprintf_s( keyName, bufferSize, "\\" );
                else                                        
                    sprintf_s( keyName, bufferSize, "FixMe - %d - %c", keyCode, keyCode );

                strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, keyName );
            }
        }
    }
}
