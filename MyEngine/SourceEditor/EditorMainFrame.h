//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorMainFrame_H__
#define __EditorMainFrame_H__

class EngineCore;

enum DefaultPerspectives
{
    Perspective_CenterEditor,
    Perspective_CenterGame,
    Perspective_CenterSideBySide,
    Perspective_FullFrameGame,
    Perspective_NumPerspectives,
};

enum EngineEditorWindowTypes
{
    EngineEditorWindow_Editor,
    EngineEditorWindow_PanelLog,
    EngineEditorWindow_NumTypes,
};

enum LaunchPlatforms
{
#if MYFW_WINDOWS
    LaunchPlatform_Win32,
    LaunchPlatform_Win64,
    LaunchPlatform_NaCl,
    LaunchPlatform_Android,
    LaunchPlatform_Emscripten,
#elif MYFW_OSX
    LaunchPlatform_OSX,
    LaunchPlatform_iOSSimulator,
    LaunchPlatform_iOSDevice,
    LaunchPlatform_iOSDevice_iOS6,
#endif
    // AddNewLaunchPlatform
    LaunchPlatform_NumPlatforms,
};

extern const char* g_DefaultPerspectiveMenuLabels[Perspective_NumPerspectives];
extern const char* g_DefaultEngineEditorWindowTypeMenuLabels[EngineEditorWindow_NumTypes];
extern const char* g_LaunchPlatformsMenuLabels[LaunchPlatform_NumPlatforms];

class EditorMainFrame
{
protected:

public:
    EditorMainFrame();
    ~EditorMainFrame();
};

#endif //__EditorMainFrame_H__
