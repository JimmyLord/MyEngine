//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorMemoryWindow_ImGui_H__
#define __EditorMemoryWindow_ImGui_H__

class EditorMemoryWindow_ImGui
{
public:
    struct Entry
    {
        char* file;
        uint32 line;
        uint32 size;
        uint32 count;
    };

protected:
    static const uint32 MAX_ENTRIES = 100000;

    std::vector<Entry> m_Entries;
    uint32 m_Count;

    bool m_ScrollToBottom;
    char m_Filter[100];

    void DrawSingleEntry(unsigned int index);

public:
    EditorMemoryWindow_ImGui();
    ~EditorMemoryWindow_ImGui();

    void Clear();
    void AddEntry(char* file, uint32 line, uint32 size);
    void Draw(const char* title, bool* p_open = 0);

    void DrawStart(const char* title, bool* p_open);
    void DrawMid();
    void DrawEnd();
};

#endif //__EditorLogWindow_ImGui_H__
