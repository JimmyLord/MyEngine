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
                if( this->m_Keys[i].m_Modifiers != o.m_Keys[i].m_Modifiers ) return true;
                if( this->m_Keys[i].m_Key != o.m_Keys[i].m_Key ) return true;
            }
            return false;
        }
    };

    enum KeyActions
    {
        KeyAction_Global_Find,
        KeyAction_File_SaveScene,
        KeyAction_File_SaveAll,
        KeyAction_File_ExportBox2D,
        KeyAction_File_Preferences,
        KeyAction_Edit_Undo,
        KeyAction_Edit_Redo,
        KeyAction_Camera_Forward,
        KeyAction_Camera_Back,
        KeyAction_Camera_Left,
        KeyAction_Camera_Right,
        KeyAction_Camera_Up,
        KeyAction_Camera_Down,
        KeyAction_Camera_Focus,
        KeyAction_Num,
    };

protected:
    int m_CurrentPreset;
    const char** m_ppPresetNames;
    int m_NumPresets;

    KeyBinding m_DefaultKeys[5][KeyAction_Num];
    KeyBinding m_Keys[5][KeyAction_Num];

    static const int m_MaxStringLength = 64; // Command-Ctrl-Alt-Shift-ScrollLock <- 33 + 1;
    char m_KeyStrings[5][KeyAction_Num][MaxKeysPerAction][m_MaxStringLength];

    // New key binding.
    bool m_RegisteringNewBinding;
    int m_NewBindingPreset;
    KeyActions m_NewBindingKeyAction;
    int m_NewBindingKeyIndex;
    uint32 m_NewBindingModifiers;

public:
    EditorKeyBindings();
    ~EditorKeyBindings();

    void ResetCurrentPreset();

    void LoadPrefs(cJSON* jPrefs);
    void SavePrefs(cJSON* jPrefs);

    KeyBinding GetKey(EditorKeyBindings::KeyActions index);
    bool KeyMatches(EditorKeyBindings::KeyActions index, uint8 modifiers, uint32 keyCode);
    const char* GetStringForKey(EditorKeyBindings::KeyActions index);

    // ImGui interface methods.
    void AddCustomizationTab();
    bool HandleInput(int keyAction, int keyCode);
    
    void CancelBindingAction();

protected:
    void SetDefaultKey(int preset, EditorKeyBindings::KeyActions index, int keyIndex, uint32 modifiers, uint32 keyCode);
    void SetDefaultKeys(int preset, EditorKeyBindings::KeyActions index, uint32 modifiers0, uint32 keyCode0, uint32 modifiers1 = 0, uint32 keyCode1 = 0);
    void GenerateKeyStrings();
};

#endif //__EditorKeyBindings_H__
