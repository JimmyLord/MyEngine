//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorLogWindow_ImGui.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorMainFrame.h"
#include "../SourceEditor/Editor_ImGui/ImGuiStylePrefs.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"

//====================================================================================================
// Public methods
//====================================================================================================

EditorLogWindow_ImGui* g_pGlobalLog;

void EditorLogWindow_ImGui_MessageLog(int logtype, const char* tag, const char* message)
{
    EditorLogWindow_ImGui::LogEntry logentry;
    logentry.logtype = logtype;
    logentry.tag = tag;
    logentry.message = message;

    g_pGlobalLog->AddLog( logentry );
}

EditorLogWindow_ImGui::EditorLogWindow_ImGui(EngineCore* pEngineCore, bool isGlobalLog)
{
    m_pEngineCore = pEngineCore;

    if( isGlobalLog )
        g_pGlobalLog = this;

    m_ScrollToBottom = false;
    m_Filter[0] = '\0';

#if USE_PTHREAD
    pthread_mutex_init( &m_MessageLogMutex, nullptr );
#endif //USE_PTHREAD
    g_pMessageLogCallbackFunction = EditorLogWindow_ImGui_MessageLog;
}

EditorLogWindow_ImGui::~EditorLogWindow_ImGui()
{
    g_pMessageLogCallbackFunction = nullptr;
#if USE_PTHREAD
    pthread_mutex_destroy( &m_MessageLogMutex );
#endif //USE_PTHREAD
}

void EditorLogWindow_ImGui::Clear()
{
    m_LoggedMessages.clear();
}

void EditorLogWindow_ImGui::AddLog(LogEntry logentry)
{
#if USE_PTHREAD
    pthread_mutex_lock( &m_MessageLogMutex );
#endif //USE_PTHREAD
    m_LoggedMessages.push_back( logentry );
#if USE_PTHREAD
    pthread_mutex_unlock( &m_MessageLogMutex );
#endif //USE_PTHREAD

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
        m_Filter[0] = '\0';
    }
    
    ImGui::Separator();
    
    ImGui::BeginChild( "scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar );
    if( copy )
    {
        ImGui::LogToClipboard();
    }

    float totalHeight = -1;

    if( m_Filter[0] != '\0' )
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
        ImGuiListClipper clipper( (int)m_LoggedMessages.size() );
        while( clipper.Step() )
        {
            for( int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++ )
            {
                DrawSingleLogEntry( i );
            }
        }

        totalHeight = m_LoggedMessages.size() * clipper.ItemsHeight;
    }

    if( copy )
    {
        ImGui::LogFinish();
    }

    if( m_ScrollToBottom && totalHeight > 0 )
    {
        ImGui::SetScrollFromPosY( totalHeight );
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

    if( pLogEntry->logtype == 0 )
    {
        Vector4 color = m_pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_LogTextInfo );
        ImGui::PushStyleColor( ImGuiCol_Text, color );
        ImGui::Text( "Info  - " );
    }
    else if( pLogEntry->logtype == 1 )
    {
        Vector4 color = m_pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_LogTextError );
        ImGui::PushStyleColor( ImGuiCol_Text, color );
        ImGui::Text( "Error - " );
    }
    else if( pLogEntry->logtype == 2 )
    {
        Vector4 color = m_pEngineCore->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_LogTextDebug );
        ImGui::PushStyleColor( ImGuiCol_Text, color );
        ImGui::Text( "Debug - " );
    }

    ImGui::SameLine();
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