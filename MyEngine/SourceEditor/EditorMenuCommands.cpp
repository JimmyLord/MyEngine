//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "PlatformSpecific/FileOpenDialog.h"
#include "EditorMenuCommands.h"

void LoadScene(const char* scenename, bool unloadscenes)
{
    if( g_pEngineCore == 0 )
        return;

    MyAssert( scenename != 0 );

    // clear out the old scene before loading.
    if( unloadscenes )
    {
        // Make sure gameplay is stopped before loading
        g_pEngineCore->OnModeStop();

        // Reset the scene counter, so the new "first" scene loaded will be 1.
        g_pComponentSystemManager->ResetSceneIDCounter();

        // if we're unloading the old scene(s), clear all selected items.
        g_pEngineCore->GetEditorState()->ClearKeyAndActionStates();
        g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
        g_pEngineCore->SetEditorInterface( EditorInterfaceType_SceneManagement );

        g_pEngineCore->UnloadScene( SCENEID_AllScenes, false ); // don't unload editor objects.
    }

    // Load the scene from file.
    // This might cause some "undo" actions, so wipe them out once the load is complete.
    unsigned int numItemsInUndoStack = g_pEngineCore->GetCommandStack()->GetUndoStackSize();

    char fullpath[MAX_PATH];
    GetFullPath( scenename, fullpath, MAX_PATH );
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
        char fullpathtofile[MAX_PATH];

        if( openedMultipleFiles == false )
        {
            strcpy_s( fullpathtofile, MAX_PATH, filenames );
            const char* relativepath = GetRelativePath( fullpathtofile );

            if( relativepath == 0 )
            {
                LOGError( LOGTag, "Files must be in a path relative to the editor." );
                return;
            }

            pComponentSystemManager->LoadDataFile( relativepath, SCENEID_MainScene, filenames, true );
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
                sprintf_s( fullpathtofile, MAX_PATH, "%s\\%s", path, &filenames[count] );
                const char* relativepath = GetRelativePath( fullpathtofile );

                if( relativepath == 0 )
                {
                    LOGError( LOGTag, "Files must be in a path relative to the editor." );
                    return;
                }

                pComponentSystemManager->LoadDataFile( relativepath, SCENEID_MainScene, fullpathtofile, true );

                // advance to next string.
                while( filenames[count] != 0 )
                    count++;
                count++;
            }
        }
    }
}

void EditorMenuCommand(EditorMenuCommands command)
{
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
            if( filename[0] != 0 )
            {
                char path[MAX_PATH];
                strcpy_s( path, MAX_PATH, filename );
                const char* relativepath = GetRelativePath( path );
                LoadScene( relativepath, true );
            }
        }
        break;

    case EditorMenuCommand_File_SaveScene:
        {
            if( g_pEngineCore->IsInEditorMode() == false )
            {
                LOGInfo( LOGTag, "Can't save when gameplay is active... use \"Save As\"\n" );
            }
            else
            {
                for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
                {
                    if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                        continue;

                    SceneID sceneid = (SceneID)i;
                    SceneInfo* pSceneInfo = &g_pComponentSystemManager->m_pSceneInfoMap[i];

                    if( sceneid != SCENEID_Unmanaged && sceneid != SCENEID_EngineObjects )
                    {
                        if( g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath[0] == 0 )
                        {
                            EditorMenuCommand( EditorMenuCommand_File_SaveSceneAs );
                        }
                        else
                        {
                            LOGInfo( LOGTag, "Saving scene... %s\n", pSceneInfo->m_FullPath );

#if MYFW_USING_IMGUI
                            g_pEngineCore->GetEditorMainFrame_ImGui()->StoreCurrentUndoStackSize();
#endif
                            g_pMaterialManager->SaveAllMaterials();
                            //g_pComponentSystemManager->m_pPrefabManager->SaveAllPrefabs(); // TODO:
                            g_pGameCore->GetSoundManager()->SaveAllCues();

                            g_pEngineCore->SaveScene( pSceneInfo->m_FullPath, sceneid );
                        }
                    }
                }
            }
        }
        break;

    case EditorMenuCommand_File_SaveSceneAs:
        {
            const char* filename = FileSaveDialog( "Data\\Scenes\\", "Scene Files\0*.scene\0All\0*.*\0" );
            if( filename[0] != 0 )
            {
                int len = strlen( filename );

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
                
                const char* relativepath = GetRelativePath( path );

                g_pComponentSystemManager->GetSceneInfo( SCENEID_MainScene )->ChangePath( relativepath );

#if MYFW_USING_IMGUI
                g_pEngineCore->GetEditorMainFrame_ImGui()->StoreCurrentUndoStackSize();
#endif
                g_pMaterialManager->SaveAllMaterials();
                //g_pComponentSystemManager->m_pPrefabManager->SaveAllPrefabs(); // TODO:
                g_pGameCore->GetSoundManager()->SaveAllCues();

                g_pEngineCore->SaveScene( relativepath, SCENEID_MainScene );
            }
        }
        break;

    case EditorMenuCommand_File_Export_Box2DScene:
        {
            const char* filename = FileSaveDialog( "", "Box2D Scene Files\0*.box2dscene\0All\0*.*\0" );
            if( filename[0] != 0 )
            {
                char filenameWithExtension[MAX_PATH];

                int len = strlen( filename );
                const char* ext = ".box2dscene";
                int extlen = strlen( ext );

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
            g_pEngineCore->GetEditorPrefs()->Toggle_View_SelectedObjects_ShowWireframe();
        }
        break;

    case EditorMenuCommand_View_SelectedObjects_ShowEffect:
        {
            g_pEngineCore->GetEditorPrefs()->Toggle_View_SelectedObjects_ShowEffect();
        }
        break;        

    case EditorMenuCommand_View_ShowEditorIcons:
        {
            g_pEngineCore->GetEditorPrefs()->Toggle_View_ShowEditorIcons();
        }
        break;

    case EditorMenuCommand_Grid_Visible:
        {
            g_pEngineCore->GetEditorPrefs()->Toggle_Grid_Visible();
        }
        break;

    case EditorMenuCommand_Grid_SnapEnabled:
        {
            g_pEngineCore->GetEditorPrefs()->Toggle_Grid_SnapEnabled();
        }
        break;

    case EditorMenuCommand_Mode_SwitchFocusOnPlayStop:
        {
            g_pEngineCore->GetEditorPrefs()->Toggle_Mode_SwitchFocusOnPlayStop();
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
            g_pEngineCore->GetEditorPrefs()->Toggle_Debug_DrawPhysicsDebugShapes();
            break;
        }

    default:
        MyAssert( false );
        break;
    }
}
