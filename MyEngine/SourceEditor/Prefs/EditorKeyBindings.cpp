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
#include "Core/EngineCore.h"

const char* g_KeyBindingStrings[HotKeyAction::Num] =
{
    "Global_Find",
    "File_SaveScene",
    "File_SaveAll",
    "File_ExportBox2D",
    "File_Preferences",
    "Edit_Undo",
    "Edit_Redo",
    "View_ShowEditorCamProperties",
    "View_ShowEditorIcons",
    "View_ToggleEditorCamDeferred",
    "View_Full",
    "View_Tall",
    "View_Square",
    "View_Wide",
    "Grid_Visible",
    "Grid_SnapEnabled",
    "Grid_Settings",
    "Mode_TogglePlayStop",
    "Mode_Pause",
    "Mode_AdvanceOneFrame",
    "Mode_AdvanceOneSecond",
    "Mode_LaunchGame",
    "Debug_DrawWireframe",
    "Debug_ShowPhysicsShapes",
    "Debug_ShowStats",
    "Lua_RunLuaScript",
    "Objects_MergeIntoFolder",
    "Camera_Forward",
    "Camera_Back",
    "Camera_Left",
    "Camera_Right",
    "Camera_Up",
    "Camera_Down",
    "Camera_Focus",
    "HeightmapEditor_Tool_Raise",
    "HeightmapEditor_Tool_Lower",
    "HeightmapEditor_Tool_Level",
};

EditorKeyBindings::EditorKeyBindings()
{
    uint32 C  = Modifier_ControlOrCommand;
    uint32 A  = Modifier_Alt;
    uint32 S  = Modifier_Shift;
    uint32 CA = Modifier_ControlOrCommand | Modifier_Alt;
    uint32 CS = Modifier_ControlOrCommand | Modifier_Shift;

    // Global.
    SetDefaultKeys( 0, HotKeyAction::Global_Find,                   C,  'F',   0,  114 ); // F3
    // File menu.
    SetDefaultKeys( 0, HotKeyAction::File_SaveScene,                C,  'S' );
    SetDefaultKeys( 0, HotKeyAction::File_SaveAll,                  CS, 'S' );
    SetDefaultKeys( 0, HotKeyAction::File_ExportBox2D,              CS, 'E' );
    SetDefaultKeys( 0, HotKeyAction::File_Preferences,              CS, 'P' );
    // Edit menu.
    SetDefaultKeys( 0, HotKeyAction::Edit_Undo,                     C,  'Z' );
    SetDefaultKeys( 0, HotKeyAction::Edit_Redo,                     C,  'Y',   CS, 'Z' );
    // View menu.
    SetDefaultKeys( 0, HotKeyAction::View_ShowEditorCamProperties,  CA, 118 ); // F7
    SetDefaultKeys( 0, HotKeyAction::View_ShowEditorIcons,          S,  118 ); // F7
    SetDefaultKeys( 0, HotKeyAction::View_ToggleEditorCamDeferred,  CS, 118 ); // F7
    SetDefaultKeys( 0, HotKeyAction::View_Full,                     A,  '1' );
    SetDefaultKeys( 0, HotKeyAction::View_Tall,                     A,  '2' );
    SetDefaultKeys( 0, HotKeyAction::View_Square,                   A,  '3' );
    SetDefaultKeys( 0, HotKeyAction::View_Wide,                     A,  '4' );
    // Grid menu.
    SetDefaultKeys( 0, HotKeyAction::Grid_Visible,                  CS, 'V' );
    SetDefaultKeys( 0, HotKeyAction::Grid_SnapEnabled,              C,  'G' );
    SetDefaultKeys( 0, HotKeyAction::Grid_Settings,                 CS, 'G' );
    // Mode menu.
    SetDefaultKeys( 0, HotKeyAction::Mode_TogglePlayStop,           C,  ' ' );
    SetDefaultKeys( 0, HotKeyAction::Mode_Pause,                    C,  '.' );
    SetDefaultKeys( 0, HotKeyAction::Mode_AdvanceOneFrame,          C,  ']' );
    SetDefaultKeys( 0, HotKeyAction::Mode_AdvanceOneSecond,         C,  '[' );
    SetDefaultKeys( 0, HotKeyAction::Mode_LaunchGame,               C,  116 ); // F5
    // Debug menu.
    SetDefaultKeys( 0, HotKeyAction::Debug_DrawWireframe,           C,  120 ); // F9
    SetDefaultKeys( 0, HotKeyAction::Debug_ShowPhysicsShapes,       S,  119 ); // F8
    SetDefaultKeys( 0, HotKeyAction::Debug_ShowStats,               CS, 119 ); // F8
    // Lua menu.
    SetDefaultKeys( 0, HotKeyAction::Lua_RunLuaScript,              CS, 'L' );
    // Misc.
    SetDefaultKeys( 0, HotKeyAction::Objects_MergeIntoFolder,       CS, 'K' );
    // Editor camera.
    SetDefaultKeys( 0, HotKeyAction::Camera_Forward,                0,  'W' );
    SetDefaultKeys( 0, HotKeyAction::Camera_Back,                   0,  'S' );
    SetDefaultKeys( 0, HotKeyAction::Camera_Left,                   0,  'A' );
    SetDefaultKeys( 0, HotKeyAction::Camera_Right,                  0,  'D' );
    SetDefaultKeys( 0, HotKeyAction::Camera_Up,                     0,  'Q' );
    SetDefaultKeys( 0, HotKeyAction::Camera_Down,                   0,  'E',   0,  'Z' );
    SetDefaultKeys( 0, HotKeyAction::Camera_Focus,                  0,  'F' );
    // Heightmap editor tools.
    SetDefaultKeys( 0, HotKeyAction::HeightmapEditor_Tool_Raise,    0,  '1' );
    SetDefaultKeys( 0, HotKeyAction::HeightmapEditor_Tool_Lower,    0,  '2' );
    SetDefaultKeys( 0, HotKeyAction::HeightmapEditor_Tool_Level,    0,  '3' );

    for( int j=0; j<(int)HotKeyAction::Num; j++ )
    {
        m_EditorInterfaceThisKeyIsActiveFor[j] = EditorInterfaceType::NumInterfaces;

        if( j >= (int)HotKeyAction::HeightmapEditor_FirstAction && j <= (int)HotKeyAction::HeightmapEditor_LastAction )
        {
            m_EditorInterfaceThisKeyIsActiveFor[j] = EditorInterfaceType::HeightmapEditor; 
        }
    }

    // Copy preset 0 into other 4 presets.
    for( int i=1; i<5; i++ )
    {
        for( int j=0; j<(int)HotKeyAction::Num; j++ )
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
    m_NewBindingHotKeyAction = HotKeyAction::Num;
    m_NewBindingKeyIndex = 0;
    m_ModifiersHeld = 0;

    // Filters.
    m_OnlyShowModifiedKeys = false;
    m_SetFilterBoxInFocus = false;
    m_Filter[0] = '\0';

    GenerateKeyStrings();
}

EditorKeyBindings::~EditorKeyBindings()
{
}

void EditorKeyBindings::ResetCurrentPreset()
{
    for( int i=0; i<(int)HotKeyAction::Num; i++ )
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
            for( int i=0; i<(int)HotKeyAction::Num; i++ )
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
        for( int i=0; i<(int)HotKeyAction::Num; i++ )
        {
            if( m_Keys[preset][i] != m_DefaultKeys[preset][i] )
            {
                int numUnsignedInts = MaxKeysPerAction * 2;

                cJSONExt_AddUnsignedIntArrayToObject( jPreset, g_KeyBindingStrings[i], reinterpret_cast<unsigned int*>( &m_Keys[preset][i] ), numUnsignedInts );
            }
        }
    }
}

uint32 EditorKeyBindings::GetModifiersHeld()
{
    return m_ModifiersHeld;
}

EditorKeyBindings::KeyBinding EditorKeyBindings::GetKey(HotKeyAction action)
{
    return m_Keys[m_CurrentPreset][(int)action];
}

EditorInterfaceType EditorKeyBindings::GetEditorInterfaceType(HotKeyAction action)
{
    return m_EditorInterfaceThisKeyIsActiveFor[(int)action];
}

bool EditorKeyBindings::KeyMatches(HotKeyAction action, uint32 modifiers, uint32 keyCode, EditorInterfaceType currentEditorInterfaceType)
{
    // Only keep the first 4 bits of the modifiers.
    modifiers &= (Modifier_ControlOrCommand | Modifier_Alt | Modifier_Shift | Modifier_ControlOSX);

    KeyBinding* pHotKey = &m_Keys[m_CurrentPreset][(int)action];
    if( m_EditorInterfaceThisKeyIsActiveFor[(int)action] == EditorInterfaceType::NumInterfaces ||
        m_EditorInterfaceThisKeyIsActiveFor[(int)action] == currentEditorInterfaceType )
    {
        for( int k=0; k<MaxKeysPerAction; k++ )
        {
            if( pHotKey->m_Keys[k].m_Modifiers == modifiers &&
                pHotKey->m_Keys[k].m_Key == keyCode )
                return true;
        }
    }

    return false;
}

const char* EditorKeyBindings::GetStringForKey(HotKeyAction action)
{
    // Only return the string for the first key.
    return m_KeyStrings[m_CurrentPreset][(int)action][0];
}

void EditorKeyBindings::AddCustomizationTab()
{
    if( ImGui::BeginTabItem( "Key Bindings" ) )
    {
        ImGui::EndTabItem();

        // Add Preset selector.
        if( ImGui::Combo( "Presets", &m_CurrentPreset, m_ppPresetNames, m_NumPresets ) )
        {
            CancelBindingAction();
        }

        ImGui::SameLine();

        // Button to reset to the default for this preset.
        if( ImGui::Button( "Reset" ) )
        {
            CancelBindingAction();
            ResetCurrentPreset();
            GenerateKeyStrings();
        }

        // Add an input box for filter.
        {
            // For now, it will always auto-select the text when given focus.
            ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_AutoSelectAll;
            if( m_SetFilterBoxInFocus )
            {
                ImGui::SetKeyboardFocusHere();
                m_SetFilterBoxInFocus = false;
            }
            ImGui::PushItemWidth( 100 );
            ImGui::InputText( "Filter", m_Filter, 100, inputTextFlags );
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if( ImGui::Button( "X" ) )
            {
                m_Filter[0] = '\0';
            }

            ImGui::SameLine();
            ImGui::Checkbox( "Only show modified keys", &m_OnlyShowModifiedKeys );
        }

        // Only show the headers if the filter is blank.
        bool showHeaders = ( m_Filter[0] == '\0' ) && ( m_OnlyShowModifiedKeys == false );

        char currentHeader[32] = "noCategory";
        bool currentHeaderIsCollapsed = false;

        for( uint32 action=0; action<(int)HotKeyAction::Num; action++ )
        {
            // Compare string before _ to create new header blocks.
            if( showHeaders )
            {
                if( strncmp( currentHeader, g_KeyBindingStrings[action], strlen( currentHeader ) ) )
                {
                    const char* underscoreLocation = reinterpret_cast<const char*>( memchr( g_KeyBindingStrings[action], '_', strlen( g_KeyBindingStrings[action] ) ) );
                    int underscoreOffset = underscoreLocation ? underscoreLocation - g_KeyBindingStrings[action] : 0;

                    ImGui::Columns( 1 );
                    strncpy_s( currentHeader, 32, g_KeyBindingStrings[action], underscoreOffset );
                    currentHeaderIsCollapsed = ImGui::CollapsingHeader( currentHeader );
                }
            }

            // Skip this key if the header is collapsed.
            if( currentHeaderIsCollapsed )
                continue;

            bool showThisItem = true;
            if( m_Filter[0] != '\0' )
            {
                if( CheckIfMultipleSubstringsAreInString( g_KeyBindingStrings[action], m_Filter ) == false )
                {
                    showThisItem = false;
                }
            }

            bool modified = ( m_Keys[m_CurrentPreset][action] != m_DefaultKeys[m_CurrentPreset][action] );

            if( m_OnlyShowModifiedKeys )
            {
                if( modified == false )
                {
                    showThisItem = false;
                }
            }

            if( showThisItem )
            {
                ImGui::Columns( 3 );

                ImGui::Text( g_KeyBindingStrings[action] );
                ImGui::NextColumn();

                for( int keyIndex=0; keyIndex<MaxKeysPerAction; keyIndex++ )
                {
                    ImGui::PushID( action*MaxKeysPerAction + keyIndex );

                    if( m_RegisteringNewBinding &&
                        m_CurrentPreset == m_NewBindingPreset &&
                        m_NewBindingHotKeyAction == static_cast<HotKeyAction>( action ) &&
                        m_NewBindingKeyIndex == keyIndex )
                    {
                        ImGui::Text( "Waiting for key press..." );
                    }
                    else if( ImGui::Button( m_KeyStrings[m_CurrentPreset][action][keyIndex] ) )
                    {
                        m_RegisteringNewBinding = true;
                        m_NewBindingPreset = m_CurrentPreset;
                        m_NewBindingHotKeyAction = static_cast<HotKeyAction>( action );
                        m_NewBindingKeyIndex = keyIndex;
                    }

                    // Reset modified flag for just one of the keys.
                    modified = ( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex] !=
                                 m_DefaultKeys[m_CurrentPreset][action].m_Keys[keyIndex] );
                    if( modified )
                    {
                        ImGui::SameLine();
                        ImGui::Text( "*" );
                    }

                    if( HasConflict( static_cast<HotKeyAction>( action ), keyIndex ) )
                    {
                        ImGui::SameLine();
                        ImGui::Text( "CONFLICT" );
                    }

                    ImGui::NextColumn();

                    ImGui::PopID();
                }

                ImGui::Columns( 1 );
            }
        }
    }
    else
    {
        CancelBindingAction();
    }
}

bool EditorKeyBindings::HandleInput(int keyAction, int keyCode)
{
    if( keyAction == GCBA_Up )
    {
        if( keyCode == MYKEYCODE_LCTRL  || keyCode == MYKEYCODE_RCTRL  ) { m_ModifiersHeld &= ~Modifier_ControlOrCommand; return true; }
        if( keyCode == MYKEYCODE_LALT   || keyCode == MYKEYCODE_RALT   ) { m_ModifiersHeld &= ~Modifier_Alt;              return true; }
        if( keyCode == MYKEYCODE_LSHIFT || keyCode == MYKEYCODE_RSHIFT ) { m_ModifiersHeld &= ~Modifier_Shift;            return true; }
    }

    if( keyAction == GCBA_Down )
    {
        if( keyCode == MYKEYCODE_LCTRL  || keyCode == MYKEYCODE_RCTRL  ) { m_ModifiersHeld |= Modifier_ControlOrCommand;  return true; }
        if( keyCode == MYKEYCODE_LALT   || keyCode == MYKEYCODE_RALT   ) { m_ModifiersHeld |= Modifier_Alt;               return true; }
        if( keyCode == MYKEYCODE_LSHIFT || keyCode == MYKEYCODE_RSHIFT ) { m_ModifiersHeld |= Modifier_Shift;             return true; }
    }

    if( m_RegisteringNewBinding )
    {
        if( keyAction == GCBA_Down )
        {
            if( keyCode == MYKEYCODE_DELETE )
            {
                // Clear the key binding in delete is pressed.
                keyCode = 0;
            }

            //LOGInfo( LOGTag, "New key bound: %d - %d", m_ModifiersHeld, keyCode );
            m_RegisteringNewBinding = false;

            m_Keys[m_NewBindingPreset][(int)m_NewBindingHotKeyAction].m_Keys[m_NewBindingKeyIndex].m_Key = keyCode;
            m_Keys[m_NewBindingPreset][(int)m_NewBindingHotKeyAction].m_Keys[m_NewBindingKeyIndex].m_Modifiers = m_ModifiersHeld;

            GenerateKeyStrings();

            return true;
        }
    }

    return false;
}

void EditorKeyBindings::OnFocusLost()
{
    // Reset modifiers if window loses focus.
    m_ModifiersHeld = 0;
}

void EditorKeyBindings::SetFilterInFocus()
{
    m_SetFilterBoxInFocus = true;
}

void EditorKeyBindings::CancelBindingAction()
{
    // Cancel the last binding action.
    m_RegisteringNewBinding = false;
}

//====================================================================================================
// Protected methods.
//====================================================================================================

void EditorKeyBindings::SetDefaultKey(int preset, HotKeyAction action, int keyIndex, uint32 modifiers, uint32 keyCode)
{
    m_DefaultKeys[preset][(int)action].m_Keys[keyIndex].m_Modifiers = modifiers;
    m_DefaultKeys[preset][(int)action].m_Keys[keyIndex].m_Key = keyCode;
}

void EditorKeyBindings::SetDefaultKeys(int preset, HotKeyAction action, uint32 modifiers0, uint32 keyCode0, uint32 modifiers1, uint32 keyCode1)
{
    m_DefaultKeys[preset][(int)action].m_Keys[0].m_Modifiers = modifiers0;
    m_DefaultKeys[preset][(int)action].m_Keys[0].m_Key = keyCode0;

    m_DefaultKeys[preset][(int)action].m_Keys[1].m_Modifiers = modifiers1;
    m_DefaultKeys[preset][(int)action].m_Keys[1].m_Key = keyCode1;
}

void EditorKeyBindings::GenerateKeyStrings()
{
    memset( m_KeyStrings, 0, sizeof( m_KeyStrings ) );

    for( int preset=0; preset<5; preset++ )
    {
        for( int action=0; action<(int)HotKeyAction::Num; action++ )
        {
            for( int keyIndex=0; keyIndex<MaxKeysPerAction; keyIndex++ )
            {
                if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Modifiers & Modifier_ControlOrCommand )
#if MYFW_OSX
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Command-" );
#else
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Ctrl-" );
#endif
                if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Modifiers & Modifier_Alt )
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Alt-" );
                if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Modifiers & Modifier_Shift )
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Shift-" );
                if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex].m_Modifiers & Modifier_ControlOSX )
                    strcat_s( m_KeyStrings[preset][action][keyIndex], m_MaxStringLength, "Ctrl-" );

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

bool EditorKeyBindings::HasConflict(HotKeyAction actionToFind, int keyIndexToFind)
{
    if( m_Keys[m_CurrentPreset][(int)actionToFind].m_Keys[keyIndexToFind].m_Key == 0 )
        return false;

    for( int action=0; action<(int)HotKeyAction::Num; action++ )
    {
        for( int keyIndex=0; keyIndex<MaxKeysPerAction; keyIndex++ )
        {
            if( action == (int)actionToFind && keyIndex == keyIndexToFind )
                continue;

            if( m_Keys[m_CurrentPreset][action].m_Keys[keyIndex] == m_Keys[m_CurrentPreset][(int)actionToFind].m_Keys[keyIndexToFind] )
            {
                return true;
            }
        }
    }

    return false;
}
