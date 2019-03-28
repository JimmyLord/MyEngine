//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorDocument.h"

#include "Core/EngineCore.h"
#include "../SourceEditor/EditorPrefs.h"
#include "../SourceEditor/NodeGraph/VisualScriptNodes.h"
#include "../SourceEditor/NodeGraph/VisualScriptNodeTypeManager.h"
#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"

static VisualScriptNodeTypeManager g_VisualScriptNodeTypeManager;

EditorDocument::EditorDocument(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    m_pCommandStack = nullptr;
    m_UndoStackDepthAtLastSave = 0;
    m_SaveRequested = false;

     m_RelativePath[0] = '\0';
     m_Filename = m_RelativePath;
}

EditorDocument::~EditorDocument()
{
}

EditorDocument* EditorDocument::EditorDocumentMenuCommand(EditorDocumentMenuCommands command)
{
    switch( command )
    {
    case EditorDocumentMenuCommand_Undo:
        {
            if( m_pCommandStack->GetUndoStackSize() > 0 )
                m_pCommandStack->Undo( 1 );
        }
        break;

    case EditorDocumentMenuCommand_Redo:
        {
            if( m_pCommandStack->GetRedoStackSize() > 0 )
                m_pCommandStack->Redo( 1 );
        }
        break;

    case EditorDocumentMenuCommand_Load:
        {
            const char* filename = FileOpenDialog( "DataSource\\VisualScripts\\", "VisualScript Files\0*.myvisualscript\0All\0*.*\0" );
            if( filename[0] != '\0' )
            {
                char path[MAX_PATH];
                strcpy_s( path, MAX_PATH, filename );
                const char* relativePath = ::GetRelativePath( path );

                EditorDocument* pNewDocument = MyNew MyNodeGraph( m_pEngineCore, &g_VisualScriptNodeTypeManager );
                pNewDocument->SetRelativePath( relativePath );
                pNewDocument->Load();

                g_pEditorPrefs->AddRecentDocument( relativePath );

                return pNewDocument;
            }
        }
        break;

    case EditorDocumentMenuCommand_Save:
        {
            // If a filename is set, save.  Otherwise, drop down into EditorDocumentMenuCommand_SaveAs and ask for a name.
            if( m_RelativePath[0] != '\0' )
            {
                Save();
                break;
            }
        }
        // no break

    case EditorDocumentMenuCommand_SaveAs:
        {
            // Request a file path from the OS.
            const char* path = FileSaveDialog( "DataSource\\VisualScripts\\", "VisualScript Files\0*.myvisualscript\0All\0*.*\0" );
            if( path[0] != 0 )
            {
                int len = (int)strlen( path );

                // Append '.myvisualscript' to end of filename if it wasn't already there.
                char fullPath[MAX_PATH];
                if( strcmp( &path[len-15], ".myvisualscript" ) == 0 )
                {
                    strcpy_s( fullPath, MAX_PATH, path );
                }
                else
                {
                    sprintf_s( fullPath, MAX_PATH, "%s.myvisualscript", path );
                }

                // Only set the filename and save if the path is relative.
                const char* relativePath = ::GetRelativePath( path );
                if( relativePath )
                {
                    SetRelativePath( relativePath );
                }
                else
                {
                    LOGError( LOGTag, "Document not saved, path must be relative to the editor." );
                }
            }

            if( m_RelativePath[0] != '\0' )
            {
                Save();
                g_pEditorPrefs->AddRecentDocument( m_RelativePath );
            }
        }
        break;

    //case EditorDocumentMenuCommand_SaveAll:
    //    {
    //    }
    //    break;
    }

    return nullptr;
}

EditorDocument* EditorDocument::EditorDocumentMenuCommand(EditorDocumentMenuCommands command, std::string value)
{
    return nullptr;
}

// Static
EditorDocument* EditorDocument::AddDocumentMenu(EngineCore* pEngineCore, EditorDocument* pDocument)
{
    EditorDocument* pNewDocument = nullptr;

    if( ImGui::BeginMenu( "Document" ) )
    {
        if( ImGui::BeginMenu( "New Document..." ) )
        {
            if( ImGui::MenuItem( "Visual Script" ) )
            {
                pNewDocument = MyNew MyNodeGraph( pEngineCore, &g_VisualScriptNodeTypeManager );
            }
            ImGui::EndMenu(); // "New Document..."
        }

        if( ImGui::MenuItem( "Load Document..." ) )
        {
            pNewDocument = pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_Load );
        }

        if( ImGui::BeginMenu( "Load Recent Document..." ) )
        {
            uint32 numRecentDocuments = pEngineCore->GetEditorPrefs()->Get_Document_NumRecentDocuments();
            if( numRecentDocuments == 0 )
            {
                ImGui::Text( "no recent documents..." );
            }

            for( uint32 i=0; i<numRecentDocuments; i++ )
            {
                std::string relativePath = pEngineCore->GetEditorPrefs()->Get_Document_RecentDocument( i );

                if( ImGui::MenuItem( relativePath.c_str() ) )
                {
                    pNewDocument = MyNew MyNodeGraph( pEngineCore, &g_VisualScriptNodeTypeManager );
                    pNewDocument->SetRelativePath( relativePath.c_str() );
                    pNewDocument->Load();

                    g_pEditorPrefs->AddRecentDocument( relativePath.c_str() );
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        // Save.
        {
            char tempstr[MAX_PATH + 10];
            if( pDocument )
            {
                if( pDocument->GetFilename()[0] == '\0' )
                {
                    sprintf_s( tempstr, MAX_PATH + 10, "Save Untitled as..." );
                }
                else
                {
                    sprintf_s( tempstr, MAX_PATH + 10, "Save %s", pDocument->GetFilename() );
                }
            }
            else
            {
                sprintf_s( tempstr, MAX_PATH + 10, "Save" );
            }

            if( ImGui::MenuItem( tempstr, "Ctrl-S", false, pDocument != nullptr ) )
            {
                pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_Save );
            }
        }

        if( ImGui::MenuItem( "Save As...", nullptr, false, pDocument != nullptr ) )
        {
            pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_SaveAs );
        }

        //if( ImGui::MenuItem( "Save All", "Ctrl-Shift-S", false, pDocument != nullptr ) )
        //{
        //    pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_SaveAll );
        //}

        ImGui::EndMenu(); // "Document"
    }

    return pNewDocument;
}

bool EditorDocument::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    ImGuiIO& io = ImGui::GetIO();

    if( keyAction == GCBA_Down )
    {
        bool N  = !io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper; // No modifiers held
        bool C  =  io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper; // Ctrl
        bool A  = !io.KeyCtrl &&  io.KeyAlt && !io.KeyShift && !io.KeySuper; // Alt
        bool S  = !io.KeyCtrl && !io.KeyAlt &&  io.KeyShift && !io.KeySuper; // Shift
        bool CS =  io.KeyCtrl && !io.KeyAlt &&  io.KeyShift && !io.KeySuper; // Ctrl-Shift

        if( C  && keyCode == 'Z' ) { EditorDocumentMenuCommand( EditorDocumentMenuCommand_Undo ); return true; }
        if( C  && keyCode == 'Y' ) { EditorDocumentMenuCommand( EditorDocumentMenuCommand_Redo ); return true; }
        if( CS && keyCode == 'Z' ) { EditorDocumentMenuCommand( EditorDocumentMenuCommand_Redo ); return true; }
        if( C  && keyCode == 'S' ) { EditorDocumentMenuCommand( EditorDocumentMenuCommand_Save ); return true; }
        //if( CS && keyCode == 'S' ) { EditorDocumentMenuCommand( EditorDocumentMenuCommand_SaveAll ); return true; }
    }

    return false;
}

void EditorDocument::Save()
{
    m_UndoStackDepthAtLastSave = m_pCommandStack->GetUndoStackSize();
}

void EditorDocument::Load()
{
    m_pCommandStack->ClearStacks();
    m_UndoStackDepthAtLastSave = 0;
}

void EditorDocument::SetRelativePath(const char* relativePath)
{
    strcpy_s( m_RelativePath, MAX_PATH, relativePath );

    // Get the filename from the relative path.
    if( m_RelativePath[0] == '\0' )
    {
        m_Filename = m_RelativePath;
    }

    for( int i=(int)strlen( m_RelativePath ); i>=0; i-- )
    {
        if( m_RelativePath[i] == '\\' || m_RelativePath[i] == '/' )
        {
            m_Filename = &m_RelativePath[i+1];
            break;
        }

        if( i == 0 )
        {
            m_Filename = m_RelativePath;
        }
    }
}

const char* EditorDocument::GetRelativePath()
{
    return m_RelativePath;
}

const char* EditorDocument::GetFilename()
{
    return m_Filename;
}

bool EditorDocument::HasUnsavedChanges()
{
    if( m_pCommandStack->GetUndoStackSize() != m_UndoStackDepthAtLastSave )
        return true;

    return false;
}
