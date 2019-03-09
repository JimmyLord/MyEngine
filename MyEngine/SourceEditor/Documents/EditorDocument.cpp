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

#include "../SourceEditor/NodeGraph/VisualScriptNodes.h"
#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"

EditorDocument::EditorDocument()
{
    m_pCommandStack = nullptr;
    m_UndoStackDepthAtLastSave = 0;
    m_SaveRequested = false;

     m_Filename[0] = '\0';
}

EditorDocument::~EditorDocument()
{
}

void EditorDocument::EditorDocumentMenuCommand(EditorDocumentMenuCommands command)
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

    case EditorDocumentMenuCommand_Save:
        {
            if( m_Filename[0] == '\0' )
            {
                const char* filename = FileSaveDialog( "DataSource\\VisualScripts\\", "VisualScript Files\0*.myvisualscript\0All\0*.*\0" );
                if( filename[0] != 0 )
                {
                    int len = (int)strlen( filename );

                    // Append '.myvisualscript' to end of filename if it wasn't already there.
                    char path[MAX_PATH];
                    if( strcmp( &filename[len-15], ".myvisualscript" ) == 0 )
                    {
                        strcpy_s( path, MAX_PATH, filename );
                    }
                    else
                    {
                        sprintf_s( path, MAX_PATH, "%s.myvisualscript", filename );
                    }

                    strcpy_s( m_Filename, MAX_PATH, path );
                }
            }

            if( m_Filename[0] != '\0' )
            {
                Save();
            }
        }
        break;

    case EditorDocumentMenuCommand_SaveAs:
        {
        }
        break;

    case EditorDocumentMenuCommand_SaveAll:
        {
        }
        break;
    }
}

void EditorDocument::EditorDocumentMenuCommand(EditorDocumentMenuCommands command, std::string value)
{
}

// Static
EditorDocument* EditorDocument::AddDocumentMenu(EditorDocument* pDocument)
{
    EditorDocument* pNewDocument = nullptr;

    if( ImGui::BeginMenu( "Document" ) )
    {
        if( ImGui::BeginMenu( "New Document..." ) )
        {
            if( ImGui::MenuItem( "Visual Script" ) )
            {
                static VisualScriptNodeTypeManager nodeTypeManager;
                pNewDocument = MyNew MyNodeGraph( &nodeTypeManager );
            }
            ImGui::EndMenu(); // "New Document..."
        }

        ImGui::Separator();

        char tempstr[MAX_PATH + 10];
        if( pDocument )
        {
            if( pDocument->GetFilename()[0] == '\0' )
            {
                sprintf_s( tempstr, MAX_PATH + 10, "Save Untitled as...", "Untitled" );
            }
            else
            {
                sprintf_s( tempstr, MAX_PATH + 10, "Save %s", pDocument->GetFilename() );
            }
        }
        else
        {
            sprintf_s( tempstr, MAX_PATH + 10, "Save: Nothing to save" );
        }
        if( ImGui::MenuItem( tempstr, "Ctrl-S", false, pDocument != nullptr ) )
        {
            pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_Save );
        }

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
        if( CS && keyCode == 'S' ) { EditorDocumentMenuCommand( EditorDocumentMenuCommand_SaveAll ); return true; }
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

void EditorDocument::SetFilename(const char* filename)
{
    strcpy_s( m_Filename, MAX_PATH, filename );
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
