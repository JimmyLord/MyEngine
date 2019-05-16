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
    enum KeyActions
    {
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

    Vector4Int m_DefaultKeys[5][KeyAction_Num];
    Vector4Int m_Keys[5][KeyAction_Num];

public:
    EditorKeyBindings();
    ~EditorKeyBindings();

    void ResetCurrentPreset();

    void LoadPrefs(cJSON* jPrefs);
    void SavePrefs(cJSON* jPrefs);

    Vector4Int GetKey(EditorKeyBindings::KeyActions index);

    void AddCustomizationTab();
};

#endif //__EditorKeyBindings_H__
