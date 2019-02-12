//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"
#include "EditorMemoryWindow_ImGui.h"

//====================================================================================================
// Public methods
//====================================================================================================

EditorMemoryWindow_ImGui::EditorMemoryWindow_ImGui()
{
    m_Entries.resize( MAX_ENTRIES );
    m_Count = 0;

    m_ScrollToBottom = false;
    m_Filter[0] = 0;
}

EditorMemoryWindow_ImGui::~EditorMemoryWindow_ImGui()
{
}

void EditorMemoryWindow_ImGui::Clear()
{
    m_Count = 0;
}

void EditorMemoryWindow_ImGui::AddEntry(const char* file, uint32 line, uint32 size)
{
    for( uint32 i = 0; i < m_Count; i++ )
    {
        if( m_Entries[i].file == file && m_Entries[i].line == line && m_Entries[i].size == size )
        {
            m_Entries[i].count++;
            return;
        }
    }

    m_Entries[m_Count].file = file;
    m_Entries[m_Count].line = line;
    m_Entries[m_Count].size = size;
    m_Entries[m_Count].count = 1;

    m_Count++;

    //m_ScrollToBottom = true;
}

void EditorMemoryWindow_ImGui::Draw(const char* title, bool* p_open)
{
    DrawStart( title, p_open );
    DrawMid();
    DrawEnd();
}

void EditorMemoryWindow_ImGui::DrawStart(const char* title, bool* p_open)
{
    ImGui::Begin( title, p_open );
}

void EditorMemoryWindow_ImGui::DrawMid()
{
    ImGui::Text( "Unique Entries: %d", m_Count );

    // Add filter and copy to clipboard buttons.
    if( ImGui::Button( "Clear" ) )
    {
        Clear();
    }
    ImGui::SameLine();
    bool copy = ImGui::Button( "Copy" );
    ImGui::SameLine();
    ImGui::InputText( "Filter", m_Filter, 100, ImGuiInputTextFlags_AutoSelectAll );
    ImGui::SameLine();
    if( ImGui::Button( "X" ) )
    {
        m_Filter[0] = 0;
    }

    ImGui::Separator();
    
    // Start "scrolling" region... TODO: find way to keep column headers in non-scrolling region.
    ImGui::BeginChild( "scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar );

    // Start logging to clipboard if "Copy" was pressed.
    if( copy )
    {
        ImGui::LogToClipboard();
    }

    // Split to 4 columns.
    ImGui::Columns( 4, 0, true );

    // Draw headings and handle sorting, hardcoded to 4 columns.
    DrawColumnHeadings();

    // Add entries to list, either filtered or not.
    if( m_Filter[0] != 0 )
    {
        for( unsigned int i = 0; i < m_Count; i++ )
        {
            if( (m_Entries[i].file != 0 && CheckIfMultipleSubstringsAreInString( m_Entries[i].file, m_Filter )) ||
                (m_Entries[i].file == 0 && CheckIfMultipleSubstringsAreInString( "unknown file", m_Filter )) )
            {
                DrawSingleEntry( i );
            }
        }
    }
    else
    {
        ImGuiListClipper clipper( (int)m_Count );
        while( clipper.Step() )
        {
            for( int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++ )
            {
                DrawSingleEntry( i );
            }
        }
    }

    // Reset to 1 column.
    ImGui::Columns( 1 );

    // Finish writing to log if "Copy" was pressed.
    if( copy )
    {
        ImGui::LogFinish();
    }

    // Not used, but force scroll to bottom if flag is set.
    if( m_ScrollToBottom )
    {
        ImGui::SetScrollHere();
        m_ScrollToBottom = false;
    }
    
    // End "scrolling" region.
    ImGui::EndChild();
}

void EditorMemoryWindow_ImGui::DrawEnd()
{
    ImGui::End();
}

//====================================================================================================
// Internal methods
//====================================================================================================

void EditorMemoryWindow_ImGui::DrawColumnHeadings()
{
    if( m_Count == 0 )
        return;

    if( ImGui::MenuItem( "Count" ) )
    {
        int order = m_Entries[0].count - m_Entries[m_Count-1].count;
        if( order > 0 )
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                        [](const Entry &e1, const Entry &e2) -> bool { return e1.count < e2.count; } );
        }
        else
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                        [](const Entry &e1, const Entry &e2) -> bool { return e1.count > e2.count; } );
        }
    }

    ImGui::NextColumn();

    if( ImGui::MenuItem( "Bytes" ) )
    {
        int order = m_Entries[0].size - m_Entries[m_Count-1].size;
        if( order > 0 )
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                       [](const Entry &e1, const Entry &e2) -> bool { return e1.size < e2.size; } );
        }
        else
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                       [](const Entry &e1, const Entry &e2) -> bool { return e1.size > e2.size; } );
        }
    }

    ImGui::NextColumn();

    if( ImGui::MenuItem( "Filename" ) )
    {
        int order;

        if( m_Entries[0].file == 0 )
            order = -1;
        else if( m_Entries[m_Count-1].file == 0 )
            order = 1;
        else
            order = strcmp( m_Entries[0].file, m_Entries[m_Count-1].file );

        if( order > 0 )
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                       [](const Entry &e1, const Entry &e2) -> bool
                       {
                           if( e2.file == 0 ) return false;
                           if( e1.file == 0 ) return true;
                           return strcmp( e2.file, e1.file ) > 0 ? true : false;
                       } );
        }
        else
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                       [](const Entry &e1, const Entry &e2) -> bool
                       {
                           if( e1.file == 0 ) return false;
                           if( e2.file == 0 ) return true;
                           return strcmp( e1.file, e2.file ) > 0 ? true : false;
                       } );
        }
    }

    ImGui::NextColumn();

    if( ImGui::MenuItem( "Line" ) )
    {
        int order = m_Entries[0].line - m_Entries[m_Count-1].line;
        if( order > 0 )
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                       [](const Entry &e1, const Entry &e2) -> bool { return e1.line < e2.line; } );
        }
        else
        {
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count,
                       [](const Entry &e1, const Entry &e2) -> bool { return e1.line > e2.line; } );
        }
    }

    ImGui::NextColumn();

    // Separator spans all 4 columns.
    ImGui::Separator();
}

void EditorMemoryWindow_ImGui::DrawSingleEntry(unsigned int lineindex)
{
    Entry* pEntry = &m_Entries[lineindex];

    char string[32];
    bool selected = false;

    // Add "Count" column.
    {
        sprintf_s( string, 32, "%d", pEntry->count );
        if( ImGui::MenuItem( string ) ) { selected = true; }
    }

    ImGui::NextColumn();

    // Add "Bytes" column.
    {
        sprintf_s( string, 32, "%d", pEntry->size );
        if( ImGui::MenuItem( string ) ) { selected = true; }
    }

    ImGui::NextColumn();

    // Add "Filename" column.
    {
        const char* filename = pEntry->file;
        if( filename == 0 )
        {
            filename = "unknown file";
        }
        else
        {
            int len = (int)strlen( filename );
            for( ; len >= 0; len-- )
                if( filename[len] == '\\' || filename[len] == '/' )
                    break;
        
            if( len > 0 )
                filename = &filename[len + 1];
        }
        
        if( ImGui::MenuItem( filename ) ) { selected = true; }
        
        if( ImGui::IsItemHovered() )
        {
            if( pEntry->file )
            {
                ImGui::BeginTooltip();
                ImGui::Text( pEntry->file );
                ImGui::EndTooltip();
            }

            if( ImGui::IsMouseDoubleClicked( 0 ) )
            {
                //g_pEngineCore->GetEditorMainFrame()->ParseMessage( pLogEntry->message.c_str() );
            }
        }
    }

    ImGui::NextColumn();

    // Add "Line" column.
    {
        sprintf_s( string, 32, "%d", pEntry->line );
        if( ImGui::MenuItem( string ) ) { selected = true; }
        ImGui::NextColumn();
    }

    //if( selected )
    //{
    //    int bp = 1;
    //}
}
