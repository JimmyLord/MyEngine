//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorKeyBindings_H__
#define __EditorKeyBindings_H__

enum class HotkeyAction
{
    Global_Find,
    File_SaveScene,
    File_SaveAll,
    File_ExportBox2D,
    File_Preferences,
    Edit_Undo,
    Edit_Redo,
    View_ShowEditorCamProperties,
    View_ShowEditorIcons,
    View_ToggleEditorCamDeferred,
    View_Full,
    View_Tall,
    View_Square,
    View_Wide,
    Grid_Visible,
    Grid_SnapEnabled,
    Grid_Settings,
    Mode_TogglePlayStop,
    Mode_Pause,
    Mode_AdvanceOneFrame,
    Mode_AdvanceOneSecond,
    Mode_LaunchGame,
    Debug_DrawWireframe,
    Debug_ShowPhysicsShapes,
    Debug_ShowStats,
    Lua_RunLuaScript,
    Objects_MergeIntoFolder,
    Camera_Forward,
    Camera_Back,
    Camera_Left,
    Camera_Right,
    Camera_Up,
    Camera_Down,
    Camera_Focus,
    HeightmapEditor_FirstAction,
    HeightmapEditor_Tool_Raise = HeightmapEditor_FirstAction,
    HeightmapEditor_Tool_Lower,
    HeightmapEditor_Tool_Level,
    HeightmapEditor_LastAction = HeightmapEditor_Tool_Level,
    Num,
};

enum class HotkeyContext
{
    Global,
    HeightmapEditor,
};

class EditorKeyBindings
{
public:
    static const int MaxKeysPerAction = 2;

    enum Modifiers
    {
        Modifier_ControlOrCommand = 0x01, // Command on OSX, Control on everything else.
        Modifier_Alt              = 0x02,
        Modifier_Shift            = 0x04,
        Modifier_ControlOSX       = 0x08,
    };

    struct keySingle
    {
        // All members must be uint32.
        uint32 m_Modifiers;
        uint32 m_Key;

        inline bool operator ==(const keySingle& o) const
        {
            if( this->m_Modifiers != o.m_Modifiers ) return false;
            if( this->m_Key != o.m_Key ) return false;
            return true;
        }

        inline bool operator !=(const keySingle& o) const
        {
            if( this->m_Modifiers != o.m_Modifiers ) return true;
            if( this->m_Key != o.m_Key ) return true;
            return false;
        }
    };

    struct KeyBinding
    {
        // All members must be uint32.
        keySingle m_Keys[MaxKeysPerAction];

        KeyBinding::KeyBinding()
        {
            for( int i=0; i<MaxKeysPerAction; i++ )
            {
                m_Keys[i].m_Modifiers = 0;
                m_Keys[i].m_Key = 0;
            }
        }

        inline bool operator !=(const KeyBinding& o) const
        {
            for( int i=0; i<MaxKeysPerAction; i++ )
            {
                if( this->m_Keys[i] != o.m_Keys[i] ) return true;
            }
            return false;
        }
    };

protected:
    int m_CurrentPreset;
    const char** m_ppPresetNames;
    int m_NumPresets;
    uint32 m_ModifiersHeld;

    HotkeyContext m_ContextThisKeyIsActiveFor[HotkeyAction::Num];

    KeyBinding m_DefaultKeys[5][HotkeyAction::Num];
    KeyBinding m_Keys[5][HotkeyAction::Num];

    static const int m_MaxStringLength = 64; // Command-Ctrl-Alt-Shift-ScrollLock <- 33 + 1;
    char m_KeyStrings[5][HotkeyAction::Num][MaxKeysPerAction][m_MaxStringLength];

    // New key binding.
    bool m_RegisteringNewBinding;
    int m_NewBindingPreset;
    HotkeyAction m_NewBindingHotkeyAction;
    int m_NewBindingKeyIndex;

    // Filters.
    bool m_OnlyShowModifiedKeys;
    bool m_SetFilterBoxInFocus;
    char m_Filter[100];

public:
    EditorKeyBindings();
    ~EditorKeyBindings();

    void ResetCurrentPreset();

    void LoadPrefs(cJSON* jPrefs);
    void SavePrefs(cJSON* jPrefs);

    uint32 GetModifiersHeld();
    KeyBinding GetKey(HotkeyAction action);
    HotkeyContext GetContext(HotkeyAction action);
    bool KeyMatches(HotkeyAction action, uint32 modifiers, uint32 keyCode, HotkeyContext context = HotkeyContext::Global);
    const char* GetStringForKey(HotkeyAction action);

    // ImGui interface methods.
    void AddCustomizationTab();
    bool HandleInput(int KeyAction, int keyCode);
    void OnFocusLost();
    void SetFilterInFocus();
    
    void CancelBindingAction();

protected:
    void SetDefaultKey(int preset, HotkeyAction action, int keyIndex, uint32 modifiers, uint32 keyCode);
    void SetDefaultKeys(int preset, HotkeyAction action, uint32 modifiers0, uint32 keyCode0, uint32 modifiers1 = 0, uint32 keyCode1 = 0);
    void GenerateKeyStrings();
    bool HasConflict(HotkeyAction actionToFind, int keyIndexToFind);
};

#endif //__EditorKeyBindings_H__
