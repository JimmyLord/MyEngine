//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorPrefs_H__
#define __EditorPrefs_H__

class EditorPrefs;
class EngineCore;

extern EditorPrefs* g_pEditorPrefs;

class EditorPrefs
{
private:
    FILE* m_pSaveFile; // Stored temporarily between SaveStart and Save Finish.

protected:
    // Editor preferences JSON object
    cJSON* m_jEditorPrefs;

    // Editor preferences
    int m_WindowX;
    int m_WindowY;
    int m_WindowWidth;
    int m_WindowHeight;
    bool m_IsWindowMaximized;

public:
    EditorPrefs();
    ~EditorPrefs();

    void Init();
    void LoadWindowSizePrefs();
    void LoadPrefs();
    void LoadLastSceneLoaded();
    cJSON* SaveStart();
    void SaveFinish(cJSON* pPrefs);

    cJSON* GetEditorPrefsJSONString() { return m_jEditorPrefs; }

    // Preference Setters
    void SetWindowProperties(int x, int y, int w, int h, bool maximized)
    {
        m_WindowX = x;
        m_WindowY = y;
        m_WindowWidth = w;
        m_WindowHeight = h;
        m_IsWindowMaximized = false;
    }
};

#endif //__EditorPrefs_H__
