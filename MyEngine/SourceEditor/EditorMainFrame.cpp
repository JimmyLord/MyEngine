//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorMainFrame.h"
#include "Commands/EditorMenuCommands.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"

const char* g_DefaultEngineEditorWindowTypeMenuLabels[EngineEditorWindow_NumTypes] =
{
    "&Editor View",
    "&Log Panel",
};

const char* g_LaunchPlatformsMenuLabels[LaunchPlatform_NumPlatforms] =
{
#if MYFW_WINDOWS
    "Win32",
    "Win64",
    "NaCl",
    "Android",
    "Emscripten",
#elif MYFW_OSX
    "OSX",
    "iOS Simulator",
    "iOS Device",
    "iOS6 Device (Needs Xcode 7.3.1)"
#endif
    // AddNewLaunchPlatform
};

EditorMainFrame::EditorMainFrame()
{
}

EditorMainFrame::~EditorMainFrame()
{
}

void EditorMainFrame::ParseLogMessage(const char* message)
{
    // Parse the line and select the gameobject/material.
    {
        // Check if the line is a GameObject or Prefab.
        GameObject* pGameObject = g_pComponentSystemManager->ParseLog_GameObject( message );
        if( pGameObject )
        {
            g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
            g_pEngineCore->GetEditorState()->SelectGameObject( pGameObject );

#if MYFW_USING_WX
            // Select the object in the object tree.
            if( pGameObject->IsPrefabInstance() )
                g_pPanelObjectList->SelectObject( pGameObject->GetPrefabRef()->GetGameObject() );
            else
                g_pPanelObjectList->SelectObject( pGameObject );
#endif
            return;
        }

        // Check if the line is a Material.
        MaterialDefinition* pMaterial = g_pComponentSystemManager->ParseLog_Material( message );
        if( pMaterial )
        {
#if MYFW_USING_WX
            pMaterial->AddToWatchPanel( true, true, true );
#elif MYFW_USING_IMGUI
            ((EditorMainFrame_ImGui*)this)->EditMaterial( pMaterial );
#endif

            // TODO: MAYBE? select the material in the memory panel.
            return;
        }

        // Check if the line is a error or warning from a compiler in this format:
        //      relative path/filename.ext(line#, char#): error error#
        {
            // Simply launch any filename/path followed by a bracket.
            // TODO: Figure out how to send the line number to the associated app.
            //       This will be different per app, so maybe start with an app selector with VSCode, Notepad++, etc.
            const char* posOpenBracket = strstr( message, "(" );
            if( posOpenBracket != nullptr )
            {
                int pathLen = (int)(posOpenBracket - message);
                if( pathLen > 0 && pathLen < MAX_PATH )
                {
                    char relativePath[MAX_PATH];
                    strncpy_s( relativePath, MAX_PATH, message, pathLen );
                    relativePath[pathLen] = '\0';
                    LaunchFileInDefaultApp( relativePath );

                    return;
                }
            }
        }
    }
}
