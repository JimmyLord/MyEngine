//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "EditorMenuCommands.h"

const char* g_DefaultPerspectiveMenuLabels[Perspective_NumPerspectives] =
{
    "Center &Editor",
    "Center &Game",
    "&Side by Side",
    "&Full Frame Game",
};

const char* g_DefaultEngineEditorWindowTypeMenuLabels[EngineEditorWindow_NumTypes] =
{
    "&Editor View",
    "&Log Panel",
};

const char* g_LaunchPlatformsMenuLabels[LaunchPlatform_NumPlatforms] =
{
#if MYFW_WINDOWS
    "&Win32",
    "Win&64",
    "&NaCl",
    "&Android",
    "&Emscripten",
#elif MYFW_OSX
    "&OSX",
    "iOS &Simulator",
    "&iOS Device",
    "iOS&6 Device (Needs Xcode 7.3.1)"
#endif
    // AddNewLaunchPlatform
};

EditorMainFrame::EditorMainFrame()
{
    m_ShowEditorIcons = true;
    m_SelectedObjects_ShowWireframe = true;
    m_SelectedObjects_ShowEffect = true;
    m_Mode_SwitchFocusOnPlayStop = true;

    m_GridSettings.visible = true;
    m_GridSettings.snapenabled = false;
    m_GridSettings.stepsize.Set( 5, 5, 5 );
}

EditorMainFrame::~EditorMainFrame()
{
}
