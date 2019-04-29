//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorMenuCommands.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/Core/PrefabManager.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/EditorPrefs.h"
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"

void LoadScene(const char* sceneName, bool unloadScenes)
{
    if( g_pEngineCore == 0 )
        return;

    MyAssert( sceneName != 0 );

    // clear out the old scene before loading.
    if( unloadScenes )
    {
        // Make sure gameplay is stopped before loading.
        g_pEngineCore->OnModeStop();

        // Reset the scene counter, so the new "first" scene loaded will be 0.
        g_pComponentSystemManager->ResetSceneIDCounter();

        // If we're unloading the old scene(s), clear all selected items along with the undo/redo stacks.
        g_pEngineCore->GetCommandStack()->ClearStacks();
        g_pEngineCore->GetEditorState()->ClearKeyAndActionStates();
        g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );

        g_pEngineCore->UnloadScene( SCENEID_AllScenes, false ); // don't unload editor objects.
    }

    // Load the scene from file.
    // This might cause some "undo" actions, so wipe them out once the load is complete.
    unsigned int numItemsInUndoStack = g_pEngineCore->GetCommandStack()->GetUndoStackSize();

    char fullpath[MAX_PATH];
    GetFullPath( sceneName, fullpath, MAX_PATH );
    SceneID sceneid = g_pEngineCore->LoadSceneFromFile( fullpath );

    g_pEngineCore->GetCommandStack()->ClearUndoStack( numItemsInUndoStack );

    //this->SetTitle( scenename ); //g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath );
}

void LoadMultipleDataFiles()
{
    bool openedMultipleFiles;
    const char* filenames = FileOpenDialog( "Data\\", "All Files\0*.*\0", &openedMultipleFiles );

    ComponentSystemManager* pComponentSystemManager = g_pEngineCore->GetComponentSystemManager();

    if( filenames[0] != 0 )
    {
        char fullPathToFile[MAX_PATH];

        if( openedMultipleFiles == false )
        {
            strcpy_s( fullPathToFile, MAX_PATH, filenames );
            const char* relativePath = GetRelativePath( fullPathToFile );

            if( relativePath == 0 )
            {
                LOGError( LOGTag, "Files must be in a path relative to the editor." );
                return;
            }

            pComponentSystemManager->LoadDataFile( relativePath, SCENEID_MainScene, filenames, true );
        }
        else
        {
            // filename will look like: "PathToFile0filename0filename0filename00"

            // First, copy out the path
            char path[MAX_PATH];
            strcpy_s( path, MAX_PATH, filenames );

            int count = 0;

            // Advance to next string.
            while( filenames[count] != 0 )
                count++;
            count++;

            // While there are strings.
            while( filenames[count] != 0 )
            {
                sprintf_s( fullPathToFile, MAX_PATH, "%s\\%s", path, &filenames[count] );
                const char* relativePath = GetRelativePath( fullPathToFile );

                if( relativePath == 0 )
                {
                    LOGError( LOGTag, "Files must be in a path relative to the editor." );
                    return;
                }

                pComponentSystemManager->LoadDataFile( relativePath, SCENEID_MainScene, fullPathToFile, true );

                // advance to next string.
                while( filenames[count] != 0 )
                    count++;
                count++;
            }
        }
    }
}

void LaunchGame(LaunchPlatforms platform)
{
    switch( platform )
    {
#if MYFW_WINDOWS
    case LaunchPlatform_Win32:
        {
            LaunchApplication( "MyEngine_Game.exe", g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->m_FullPath );
        }
        break;

    case LaunchPlatform_Win64:
        {
            LaunchApplication( "MyEngine_Game_x64.exe", g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->m_FullPath );
        }
        break;

    case LaunchPlatform_NaCl:
        {
            LaunchApplication( "cmd.exe", "/C cd Web & RunWebServer.bat" );
            LaunchApplication( "Chrome", "http://localhost:5103/game.html" );
        }
        break;

    case LaunchPlatform_Android:
        {
            char tempstr[255];
            sprintf_s( tempstr, 255, "/C cd Android & BuildAndLaunch.bat %s", g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->m_FullPath );
            for( unsigned int i=0; i<strlen(tempstr); i++ )
            {
                if( tempstr[i] == '\\' )
                    tempstr[i] = '/';
            }
            LaunchApplication( "cmd.exe", tempstr );
        }
        break;

    case LaunchPlatform_Emscripten:
        {
            LaunchApplication( "cmd.exe", "/C cd Emscripten & BuildAndLaunch.bat" );
        }
        break;
#elif MYFW_OSX
    case LaunchPlatform_OSX:
        {
            LaunchApplication( "MyEngine_Game.app", g_pComponentSystemManager->GetSceneInfo( 1 )->m_FullPath );
        }
        break;

    case LaunchPlatform_iOSSimulator:
        {
            LaunchApplication( "cd iOS && ./BuildAndLaunch-Simulator.sh", 0 );
        }
        break;

    case LaunchPlatform_iOSDevice:
        {
            LaunchApplication( "cd iOS && ./BuildAndLaunch-Device.sh", 0 );
        }
        break;

    case LaunchPlatform_iOSDevice_iOS6:
        {
            LaunchApplication( "cd iOS && ./BuildAndLaunch-DeviceiOS6.sh", 0 );
        }
        break;
#endif

    // AddNewLaunchPlatform
    
    case LaunchPlatform_NumPlatforms:
        MyAssert( false );
        break;
    }
}

void EditorMenuCommand(EditorMenuCommands command)
{
    EditorPrefs* pEditorPrefs = g_pEngineCore->GetEditorPrefs();

    switch( command )
    {
    case EditorMenuCommand_File_NewScene:
        {
            g_pEngineCore->GetEditorState()->ClearKeyAndActionStates();
            g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
            g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );

            //this->SetTitle( "New scene" );
            g_pEngineCore->UnloadScene( SCENEID_AllScenes, false );
            g_pComponentSystemManager->CreateNewScene( "Unsaved.scene", SCENEID_MainScene );
            g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->m_FullPath[0] = 0;
            g_pEngineCore->CreateDefaultSceneObjects();
            //ResizeViewport();
        }
        break;

    case EditorMenuCommand_File_LoadScene:
        {
            const char* filename = FileOpenDialog( "Data\\Scenes\\", "Scene Files\0*.scene\0All\0*.*\0" );
            if( filename[0] != '\0' )
            {
                char path[MAX_PATH];
                strcpy_s( path, MAX_PATH, filename );
                const char* relativePath = GetRelativePath( path );
                LoadScene( relativePath, true );

                pEditorPrefs->AddRecentScene( relativePath );
            }
        }
        break;

    case EditorMenuCommand_File_LoadPreselectedScene:
        MyAssert( false ); // Handled by an override of EditorMenuCommand.
        break;

    case EditorMenuCommand_File_CreateAdditionalScene:
        {
            SceneID sceneid = g_pComponentSystemManager->GetNextSceneID();
            g_pComponentSystemManager->CreateNewScene( "Unsaved.scene", sceneid );
            g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath[0] = 0;
        }
        break;

    case EditorMenuCommand_File_LoadAdditionalScene:
        {
            const char* filename = FileOpenDialog( "Data\\Scenes\\", "Scene Files\0*.scene\0All\0*.*\0" );
            if( filename[0] != '\0' )
            {
                char path[MAX_PATH];
                strcpy_s( path, MAX_PATH, filename );
                const char* relativePath = GetRelativePath( path );
                LoadScene( relativePath, false );

                pEditorPrefs->AddRecentScene( relativePath );
            }
        }
        break;

    case EditorMenuCommand_File_SaveScene:
        {
            g_pEngineCore->GetEditorState()->SaveAllMiscFiles();
            g_pEngineCore->SaveAllScenes();
        }
        break;

    case EditorMenuCommand_File_SaveSceneAs:
        {
            g_pEngineCore->GetEditorState()->SaveAllMiscFiles();

            const char* filename = FileSaveDialog( "Data\\Scenes\\", "Scene Files\0*.scene\0All\0*.*\0" );
            if( filename[0] != 0 )
            {
                int len = (int)strlen( filename );

                // Append '.scene' to end of filename if it wasn't already there.
                char path[MAX_PATH];
                if( strcmp( &filename[len-6], ".scene" ) == 0 )
                {
                    strcpy_s( path, MAX_PATH, filename );
                }
                else
                {
                    sprintf_s( path, MAX_PATH, "%s.scene", filename );
                }
                
                const char* relativePath = GetRelativePath( path );

                g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->ChangePath( relativePath );

#if MYFW_USING_IMGUI
                g_pEngineCore->GetEditorMainFrame_ImGui()->StoreCurrentUndoStackSize();
#endif
                g_pEngineCore->SaveScene( relativePath, SCENEID_MainScene );

                pEditorPrefs->AddRecentScene( relativePath );
            }
        }
        break;

    case EditorMenuCommand_File_SaveAll:
        {
            g_pEngineCore->SaveAllScenes();
            g_pEngineCore->GetEditorState()->SaveAllOpenDocuments();
        }
        break;

    case EditorMenuCommand_File_Export_Box2DScene:
        {
            const char* filename = FileSaveDialog( "", "Box2D Scene Files\0*.box2dscene\0All\0*.*\0" );
            if( filename[0] != 0 )
            {
                char filenameWithExtension[MAX_PATH];

                int len = (int)strlen( filename );
                const char* ext = ".box2dscene";
                int extlen = (int)strlen( ext );

                if( len > extlen && strcmp( &filename[len-extlen], ext ) == 0 )
                {
                    sprintf_s( filenameWithExtension, MAX_PATH, "%s", filename );
                }
                else
                {
                    sprintf_s( filenameWithExtension, MAX_PATH, "%s%s", filename, ext );
                }

                g_pEngineCore->ExportBox2DScene( filenameWithExtension, SCENEID_MainScene );
            }
        }
        break;

    case EditorMenuCommand_Edit_Undo:
        {
            if( g_pEngineCore->GetCommandStack()->GetUndoStackSize() > 0 )
                g_pEngineCore->GetCommandStack()->Undo( 1 );
        }
        break;

    case EditorMenuCommand_Edit_Redo:
        {
            if( g_pEngineCore->GetCommandStack()->GetRedoStackSize() > 0 )
                g_pEngineCore->GetCommandStack()->Redo( 1 );
        }
        break;

    case EditorMenuCommand_View_SelectedObjects_ShowWireframe:
        {
            pEditorPrefs->Toggle_View_SelectedObjects_ShowWireframe();
        }
        break;

    case EditorMenuCommand_View_SelectedObjects_ShowEffect:
        {
            pEditorPrefs->Toggle_View_SelectedObjects_ShowEffect();
        }
        break;        

    case EditorMenuCommand_View_ShowEditorIcons:
        {
            pEditorPrefs->Toggle_View_ShowEditorIcons();
        }
        break;

    case EditorMenuCommand_View_ToggleEditorCamDeferred:
        {
            pEditorPrefs->Toggle_View_EditorCamDeferred();
        }
        break;

    case EditorMenuCommand_View_ShowEditorCamDeferredGBuffer:
        {
            g_pEngineCore->GetEditorState()->GetEditorCamera()->ShowDeferredGBuffer();
        }
        break;        

    case EditorMenuCommand_Grid_Visible:
        {
            pEditorPrefs->Toggle_Grid_Visible();
        }
        break;

    case EditorMenuCommand_Grid_SnapEnabled:
        {
            pEditorPrefs->Toggle_Grid_SnapEnabled();
        }
        break;

    case EditorMenuCommand_Mode_SwitchFocusOnPlayStop:
        {
            pEditorPrefs->Toggle_Mode_SwitchFocusOnPlayStop();
        }
        break;

    case EditorMenuCommand_Mode_TogglePlayStop:
        {
            g_pEngineCore->OnModeTogglePlayStop();
        }
        break;

    case EditorMenuCommand_Mode_Pause:
        {
            g_pEngineCore->OnModePause();
        }
        break;

    case EditorMenuCommand_Mode_AdvanceOneFrame:
        {
            g_pEngineCore->OnModeAdvanceTime( 1/60.0f );
        }
        break;

    case EditorMenuCommand_Mode_AdvanceOneSecond:
        {
            g_pEngineCore->OnModeAdvanceTime( 1.0f );
        }
        break;

    case EditorMenuCommand_Mode_LaunchPlatforms:
        // Handled after switch statement.
        break;

    case EditorMenuCommand_Mode_LaunchGame:
        {
            LaunchGame( pEditorPrefs->Get_Mode_LaunchPlatform() );
        }
        break;

    case EditorMenuCommand_Data_LoadDatafiles:
        {
            LoadMultipleDataFiles();
        }
        break;

    case EditorMenuCommand_Debug_DrawWireframe:
        {
            g_pEngineCore->ToggleDebug_DrawWireframe();
        }
        break;

    case EditorMenuCommand_Debug_ShowPhysicsShapes:
        {
            pEditorPrefs->Toggle_Debug_DrawPhysicsDebugShapes();
        }
        break;

    case EditorMenuCommand_Lua_RunLuaScript:
        {
            const char* filename = FileOpenDialog( "DataEditor\\", "Lua Files\0*.lua\0All\0*.*\0" );
            if( filename[0] != 0 )
            {
                char path[MAX_PATH];
                strcpy_s( path, MAX_PATH, filename );
                const char* relativepath = GetRelativePath( path );
                
                g_pEngineCore->GetLuaGameState()->RunFile( relativepath );

                pEditorPrefs->AddRecentLuaScript( relativepath );
            }
        }
        break;

    case EditorMenuCommand_Lua_RunRecentLuaScript:
        // Handled after switch statement.
        break;

    case EditorMenuCommand_Objects_MergeIntoFolder:
        {
            LOGError( LOGTag, "(TODO) Moving selected objects into folder has no undo!\n" );

            int numSelected = (int)g_pEngineCore->GetEditorState()->m_pSelectedObjects.size();

            if( numSelected > 0 )
            {
                GameObject* pFirstGO = g_pEngineCore->GetEditorState()->m_pSelectedObjects[0];
                SceneID sceneID = pFirstGO->GetSceneID();
                GameObject* pParentGO = pFirstGO->GetParentGameObject();

                // Create a folder
                GameObject* pFolder = g_pComponentSystemManager->CreateGameObject( true, sceneID, true, false, 0 );
                pFolder->SetName( "New Merged Object Folder" );
                pFolder->SetParentGameObject( pParentGO );

                // Move all selected GOs into this folder.
                for( int i=0; i<numSelected; i++ )
                {
                    GameObject* selectedGO = g_pEngineCore->GetEditorState()->m_pSelectedObjects[i];

                    if( selectedGO->GetSceneID() != sceneID )
                    {
                        LOGInfo( LOGTag, "Selected object not placed in folder since it was part of a different scene: %s\n", selectedGO->GetName() );
                        continue; // skip this GO.
                    }
                    
                    selectedGO->SetParentGameObject( pFolder );
                }
            }
        }
        break;

    default:
        {
            // The only cases not handled above are launch platform cases, since there's a dynamic amount of them.
            MyAssert( command >= EditorMenuCommand_Mode_LaunchPlatforms && command < EditorMenuCommand_Mode_LaunchPlatforms + LaunchPlatform_NumPlatforms );
        }
        break;
    }

    if( command >= EditorMenuCommand_Mode_LaunchPlatforms && command < EditorMenuCommand_Mode_LaunchPlatforms + LaunchPlatform_NumPlatforms )
    {
        LaunchPlatforms platformIndex = (LaunchPlatforms)(command - EditorMenuCommand_Mode_LaunchPlatforms);

        pEditorPrefs->Set_Mode_LaunchPlatform( platformIndex );
    }

    if( command >= EditorMenuCommand_Lua_RunRecentLuaScript && command < EditorMenuCommand_Lua_RunRecentLuaScript + EditorPrefs::MAX_RECENT_LUA_SCRIPTS )
    {
        int fileIndex = command - EditorMenuCommand_Lua_RunRecentLuaScript;

        std::string relativePathStr = pEditorPrefs->Get_Lua_RecentScript( fileIndex );
        const char* relativePath = relativePathStr.c_str();
        g_pEngineCore->GetLuaGameState()->RunFile( relativePath );

        pEditorPrefs->AddRecentLuaScript( relativePath );
    }
}

void EditorMenuCommand(EditorMenuCommands command, std::string value)
{
    EditorPrefs* pEditorPrefs = g_pEngineCore->GetEditorPrefs();

    if( command == EditorMenuCommand_File_LoadPreselectedScene )
    {
        LoadScene( value.c_str(), true );

        pEditorPrefs->AddRecentScene( value.c_str() );
    }
}
