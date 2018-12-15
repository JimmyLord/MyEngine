//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
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

void EditorMemoryWindow_ImGui::AddEntry(char* file, uint32 line, uint32 size)
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

bool sortBySize(EditorMemoryWindow_ImGui::Entry& e1, EditorMemoryWindow_ImGui::Entry& e2)
{
    return( e1.size < e2.size );
}

bool sortBySizeFlip(EditorMemoryWindow_ImGui::Entry& e1, EditorMemoryWindow_ImGui::Entry& e2)
{
    return( e1.size > e2.size );
}

void EditorMemoryWindow_ImGui::DrawMid()
{
    ImGui::Text( "Unique: %d", m_Count );

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

    if( ImGui::Button( "Size" ) && m_Count > 1 )
    {
        uint32 first = m_Entries[0].size;
        uint32 last = m_Entries[m_Count-1].size;
        if( first > last )
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count, sortBySize );
        else
            std::sort( m_Entries.begin(), m_Entries.begin()+m_Count, sortBySizeFlip );
    }

    ImGui::Separator();
    
    ImGui::BeginChild( "scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar );
    if( copy )
    {
        ImGui::LogToClipboard();
    }

    if( m_Filter[0] != 0 )
    {
        for( unsigned int i = 0; i < m_Count; i++ )
        {
            if( CheckIfMultipleSubstringsAreInString( m_Entries[i].file, m_Filter ) )
            {
                DrawSingleEntry( i );
            }
        }
    }
    else
    {
        //for( unsigned int i = 0; i < m_LoggedMessages.size(); i++ )
        ImGuiListClipper clipper( (int)m_Count );
        while( clipper.Step() )
        {
            for( int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++ )
            {
                DrawSingleEntry( i );
            }
        }
    }

    if( m_ScrollToBottom )
    {
        ImGui::SetScrollHere();
        m_ScrollToBottom = false;
    }
    
    ImGui::EndChild();
}

void EditorMemoryWindow_ImGui::DrawEnd()
{
    ImGui::End();
}

//====================================================================================================
// Internal methods
//====================================================================================================

void EditorMemoryWindow_ImGui::DrawSingleEntry(unsigned int lineindex)
{
    Entry* pEntry = &m_Entries[lineindex];

    //ImGui::TextUnformatted( m_Entries[i].message.c_str() );

    //if( pEntry->logtype == 0 )
    //{
    //    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f) );
    //    ImGui::Text( "Info  - " );
    //}
    //else if( pEntry->logtype == 1 )
    //{
    //    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f) );
    //    ImGui::Text( "Error - " );
    //}
    //else if( pEntry->logtype == 2 )
    //{
    //    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(0.5f, 0.5f, 1.0f, 1.0f) );
    //    ImGui::Text( "Debug - " );
    //}

    //ImGui::SameLine();
    //ImGui::Text( pLogEntry->tag.c_str() );
    //ImGui::SameLine();
    if( pEntry->file )
    {
        char string[400];
        sprintf_s( string, 400, "%d - %d - %s(%d)", pEntry->count, pEntry->size, pEntry->file, pEntry->line );
        if( ImGui::MenuItem( string ) )
        {
        }
    }
    else
    {
        char string[400];
        sprintf_s( string, 400, "%d - %d - %s(%d)", pEntry->count, pEntry->size, "no file", pEntry->line );
        if( ImGui::MenuItem( string ) )
        {
        }
    }

    if( ImGui::IsItemHovered() )
    {
        if( ImGui::IsMouseDoubleClicked( 0 ) )
        {
            //g_pEngineCore->GetEditorMainFrame()->ParseMessage( pLogEntry->message.c_str() );
        }
    }

    //ImGui::PopStyleColor();
}
