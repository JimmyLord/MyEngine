//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorLogWindow_ImGui_H__
#define __EditorLogWindow_ImGui_H__

class EditorLogWindow_ImGui
{
public:
    struct LogEntry
    {
        int logtype;
        std::string tag;
        std::string message;
    };

protected:
    EngineCore* m_pEngineCore;

    pthread_mutex_t m_MessageLogMutex;

    std::vector<LogEntry> m_LoggedMessages;
    //ImGuiTextBuffer     m_TextBuffer;
    //ImGuiTextFilter     m_Filter;
    //ImVector<int>       m_LineOffsets;        // Index to lines offset.
    bool m_ScrollToBottom;
    char m_Filter[100];

    void DrawSingleLogEntry(unsigned int lineindex);

public:
    EditorLogWindow_ImGui(EngineCore* pEngineCore, bool isGlobalLog);
    ~EditorLogWindow_ImGui();

    void Clear();
    void AddLog(LogEntry logentry);
    void Draw(const char* title, bool* p_open = nullptr);

    void DrawStart(const char* title, bool* p_open);
    void DrawMid();
    void DrawEnd();

    void SetScrollToBottom() { m_ScrollToBottom = true; }
};

#endif //__EditorLogWindow_ImGui_H__
