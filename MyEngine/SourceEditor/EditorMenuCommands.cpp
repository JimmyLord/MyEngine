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

        g_pEngineCore->UnloadScene( UINT_MAX, false ); // don't unload editor objects.
    }

    // Load the scene from file.
    // This might cause some "undo" actions, so wipe them out once the load is complete.
    //unsigned int numItemsInUndoStack = g_pEngineMainFrame->m_pCommandStack->GetUndoStackSize();

    char fullpath[MAX_PATH];
    GetFullPath( scenename, fullpath, MAX_PATH );
    unsigned int sceneid = g_pEngineCore->LoadSceneFromFile( fullpath );

    //g_pEngineMainFrame->m_pCommandStack->ClearUndoStack( numItemsInUndoStack );

    //this->SetTitle( scenename ); //g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath );
}

void EditorMenuCommand(EditorMenuCommands command)
{
    switch( command )
    {
    case EditorMenuCommand_File_SaveScene:
        {
            if( g_pEngineCore->IsInEditorMode() == false )
            {
                LOGInfo( LOGTag, "Can't save when gameplay is active... use \"Save As\"\n" );
            }
            else
            {
                for( unsigned int i=0; i<ComponentSystemManager::MAX_SCENES_LOADED; i++ )
                {
                    if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                        continue;

                    unsigned int sceneid = i;
                    SceneInfo* pSceneInfo = &g_pComponentSystemManager->m_pSceneInfoMap[i];

                    if( sceneid != EngineCore::UNMANAGED_SCENE_ID && sceneid != EngineCore::ENGINE_SCENE_ID )
                    {
                        if( g_pComponentSystemManager->GetSceneInfo( sceneid )->m_FullPath[0] == 0 )
                        {
                            // TODO:
                            //SaveSceneAs( sceneid );
                        }
                        else
                        {
                            LOGInfo( LOGTag, "Saving scene... %s\n", pSceneInfo->m_FullPath );
                            g_pEngineCore->SaveScene( pSceneInfo->m_FullPath, sceneid );
                        }
                    }
                }
            }
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

                g_pEngineCore->ExportBox2DScene( filenameWithExtension, 1 );
            }
        }
        break;

    case EditorMenuCommand_Mode_TogglePlayStop:
        {
            g_pEngineCore->OnModeTogglePlayStop();
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

    default:
        MyAssert( false );
        break;
    }
}
