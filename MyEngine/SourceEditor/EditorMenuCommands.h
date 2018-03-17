//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorMenuCommands_H__
#define __EditorMenuCommands_H__

enum EditorMenuCommands
{
    EditorMenuCommand_File_NewScene,
    EditorMenuCommand_File_LoadScene,
    EditorMenuCommand_File_SaveScene,
    EditorMenuCommand_File_SaveSceneAs,
    EditorMenuCommand_File_Export_Box2DScene,
    EditorMenuCommand_Edit_Undo,
    EditorMenuCommand_Edit_Redo,
    EditorMenuCommand_View_SelectedObjects_ShowWireframe,
    EditorMenuCommand_View_SelectedObjects_ShowEffect,
    EditorMenuCommand_View_ShowEditorIcons,
    EditorMenuCommand_Grid_Visible,
    EditorMenuCommand_Grid_SnapEnabled,
    EditorMenuCommand_Mode_SwitchFocusOnPlayStop,
    EditorMenuCommand_Mode_TogglePlayStop,
    EditorMenuCommand_Mode_Pause,
    EditorMenuCommand_Mode_AdvanceOneFrame,
    EditorMenuCommand_Mode_AdvanceOneSecond,
    EditorMenuCommand_Data_LoadDatafiles,
    EditorMenuCommand_Debug_DrawWireframe,
    EditorMenuCommand_Debug_ShowPhysicsShapes,
};

void EditorMenuCommand(EditorMenuCommands command);

#endif //__EditorMenuCommands_H__
