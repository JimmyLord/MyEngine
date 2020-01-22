//
// Copyright (c) 2019-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorDocument.h"

#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/NodeGraph/VisualScriptNodes.h"
#include "../SourceEditor/NodeGraph/VisualScriptNodeTypeManager.h"
#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"

static VisualScriptNodeTypeManager g_VisualScriptNodeTypeManager;

EditorDocument::EditorDocument(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    m_pCommandStack = MyNew CommandStack();
    m_UndoStackDepthAtLastSave = 0;
    m_SaveRequested = false;

     m_RelativePath[0] = '\0';
     m_Filename = m_RelativePath;

     // Windowing system stuff.
     m_pCamera = nullptr;
     m_pCameraTransform = nullptr;
     m_pFBO = nullptr;
     m_WindowPos.Set( -1, -1 );
     m_WindowSize.Set( -1, -1 );
     m_WindowHovered = false;
     m_WindowFocused = false;
     m_WindowVisible = false;
}

EditorDocument::~EditorDocument()
{
    delete m_pCommandStack;
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
            char tempFilter[256];
            sprintf_s( tempFilter, 256, "%s=All=*.*=", GetDefaultFileSaveFilter() );
            uint32 tempFilterLen = (int)strlen( tempFilter );
            for( uint32 i=0; i<tempFilterLen; i++ )
            {
                if( tempFilter[i] == '=' )
                {
                    tempFilter[i] = '\0';
                }
            }

            const char* path = FileSaveDialog( GetDefaultDataFolder(), tempFilter );
            if( path[0] != 0 )
            {
                int len = (int)strlen( path );
                int defaultFileExtensionLength = (int)strlen( GetFileExtension() );

                // Append extension to end of filename if it wasn't already there.
                char fullPath[MAX_PATH];
                if( strcmp( &path[len-defaultFileExtensionLength], GetFileExtension() ) == 0 )
                {
                    strcpy_s( fullPath, MAX_PATH, path );
                }
                else
                {
                    sprintf_s( fullPath, MAX_PATH, "%s%s", path, GetFileExtension() );
                }

                // Only set the filename and save if the path is relative.
                const char* relativePath = ::GetRelativePath( fullPath );
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

                if( m_pEngineCore )
                {
                    m_pEngineCore->GetEditorPrefs()->AddRecentDocument( m_RelativePath );
                }
            }
        }
        break;

    case EditorDocumentMenuCommand_Run:
        Run();
        break;
    }

    return nullptr;
}

EditorDocument* EditorDocument::EditorDocumentMenuCommand(EditorDocumentMenuCommands command, std::string value)
{
    return nullptr;
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
        //if( N  && keyCode == VK_F5) { EditorDocumentMenuCommand( EditorDocumentMenuCommand_Run ); return true; }
    }

    return false;
}

bool EditorDocument::ExecuteHotkeyAction(HotkeyAction action)
{
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

void EditorDocument::Run()
{
}

void EditorDocument::GetWindowTitle(char* pTitle, const uint32 titleAllocationSize)
{
    const char* filename = GetFilename();
    if( filename[0] == '\0' )
        filename = "Untitled";

    if( HasUnsavedChanges() )
    {
        sprintf_s( pTitle, titleAllocationSize, "%s*###%p", filename, this );
    }
    else
    {
        sprintf_s( pTitle, titleAllocationSize, "%s###%p", filename, this );
    }
};

void EditorDocument::CreateWindowAndUpdate(bool* pDocumentStillOpen)
{
    const uint32 tempTitleAllocationSize = MAX_PATH*2+5;
    static char tempTitle[tempTitleAllocationSize];

    GetWindowTitle( tempTitle, tempTitleAllocationSize );

    m_WindowVisible = false;

    if( ImGui::Begin( tempTitle, pDocumentStillOpen ) )
    {
        m_WindowVisible = true;

        if( ImGui::BeginPopupContextItem() )
        {
            if( ImGui::MenuItem( "Close" ) )
            {
                *pDocumentStillOpen = false;
            }
            ImGui::EndPopup();
        }

        Update();

        m_WindowFocused = ImGui::IsWindowFocused( ImGuiFocusedFlags_RootAndChildWindows );
        m_WindowHovered = ImGui::IsWindowHovered( ImGuiHoveredFlags_AllowWhenBlockedByActiveItem );
    }
    ImGui::End();
}

void EditorDocument::Update()
{
    m_pCommandStack->IncrementFrameCount();
}

void EditorDocument::OnDrawFrame()
{
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
