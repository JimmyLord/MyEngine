//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

//====================================================================================================
// Public methods
//====================================================================================================

pthread_mutex_t g_MessageLogMutex;

EditorLogWindow_ImGui* g_pGlobalLog;

void EditorLogWindow_ImGui_MessageLog(int logtype, const char* tag, const char* message)
{
    EditorLogWindow_ImGui::LogEntry logentry;
    logentry.logtype = logtype;
    logentry.tag = tag;
    logentry.message = message;

    pthread_mutex_lock( &g_MessageLogMutex );

    g_pGlobalLog->AddLog( logentry );

    pthread_mutex_unlock( &g_MessageLogMutex );
}

EditorLogWindow_ImGui::EditorLogWindow_ImGui(bool isGlobalLog)
{
    if( isGlobalLog )
        g_pGlobalLog = this;

    m_ScrollToBottom = false;
    m_Filter[0] = 0;

    pthread_mutex_init( &g_MessageLogMutex, 0 );
    g_pMessageLogCallbackFunction = EditorLogWindow_ImGui_MessageLog;
}

EditorLogWindow_ImGui::~EditorLogWindow_ImGui()
{
    g_pMessageLogCallbackFunction = 0;
    pthread_mutex_destroy( &g_MessageLogMutex );
}

void EditorLogWindow_ImGui::Clear()
{
    m_LoggedMessages.clear();
}

void EditorLogWindow_ImGui::AddLog(LogEntry logentry)
{
    m_LoggedMessages.push_back( logentry );

    m_ScrollToBottom = true;
}

void EditorLogWindow_ImGui::Draw(const char* title, bool* p_open)
{
    DrawStart( title, p_open );
    DrawMid();
    DrawEnd();
}

void EditorLogWindow_ImGui::DrawStart(const char* title, bool* p_open)
{
    ImGui::Begin( title, p_open );
}

void EditorLogWindow_ImGui::DrawMid()
{
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
    
    ImGui::BeginChild( "scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar );
    if( copy )
    {
        ImGui::LogToClipboard();
    }

    if( m_Filter[0] != 0 )
    {
        for( unsigned int i = 0; i < m_LoggedMessages.size(); i++ )
        {
            if( CheckIfMultipleSubstringsAreInString( m_LoggedMessages[i].message.c_str(), m_Filter ) )
            {
                DrawSingleLogEntry( i );
            }
        }
    }
    else
    {
        //for( unsigned int i = 0; i < m_LoggedMessages.size(); i++ )
        ImGuiListClipper clipper( (int)m_LoggedMessages.size() );
        while( clipper.Step() )
        {
            for( int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++ )
            {
                DrawSingleLogEntry( i );
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

void EditorLogWindow_ImGui::DrawEnd()
{
    ImGui::End();
}

//====================================================================================================
// Internal methods
//====================================================================================================

void EditorLogWindow_ImGui::DrawSingleLogEntry(unsigned int lineindex)
{
    LogEntry* pLogEntry = &m_LoggedMessages[lineindex];

    //ImGui::TextUnformatted( m_LoggedMessages[i].message.c_str() );

    if( pLogEntry->logtype == 0 )
    {
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f) );
        ImGui::Text( "Info  - " );
    }
    else if( pLogEntry->logtype == 1 )
    {
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f) );
        ImGui::Text( "Error - " );
    }
    else if( pLogEntry->logtype == 2 )
    {
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4(0.5f, 0.5f, 1.0f, 1.0f) );
        ImGui::Text( "Debug - " );
    }

    ImGui::SameLine();
    //ImGui::Text( pLogEntry->tag.c_str() );
    //ImGui::SameLine();
    if( ImGui::MenuItem( pLogEntry->message.c_str() ) )
    {
    }

    if( ImGui::IsItemHovered() )
    {
        if( ImGui::IsMouseDoubleClicked( 0 ) )
        {
            g_pEngineCore->GetEditorMainFrame()->ParseLogMessage( pLogEntry->message.c_str() );
        }
    }

    ImGui::PopStyleColor();
}