//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameMainFrame_H__
#define __GameMainFrame_H__

class GameMainFrame;

extern GameMainFrame* g_pGameMainFrame;

enum DefaultPerspectives
{
    Perspective_CenterEditor,
    Perspective_CenterGame,
    Perspective_CenterSideBySide,
    Perspective_NumPerspectives,
};

enum GameMenuIDs
{
    myIDGame_LoadScene = myID_NumIDs,
    myIDGame_SaveScene,
    myIDGame_SaveSceneAs,
    myIDGame_AddDatafile,
    myIDGame_RecordMacro,
    myIDGame_ExecuteMacro,
    myIDGame_EditorPerspectives,
    myIDGame_GameplayPerspectives,
    myIDGame_EditorPerspective,
    // Perspective_NumPerspectives more items here
    myIDGame_GameplayPerspective = myIDGame_EditorPerspective + Perspective_NumPerspectives,
    // Perspective_NumPerspectives more items here
    myIDGame_DebugShowMousePickerFBO = myIDGame_GameplayPerspective + Perspective_NumPerspectives,
    myIDGame_DebugShowSelectedAnimatedMesh,
};

class GameMainFrame : public MainFrame
{
public:
    MainGLCanvas* m_pGLCanvasEditor;
    wxTextCtrl* m_pLogPane;

    wxMenu* m_EditorPerspectives;
    wxMenu* m_GameplayPerspectives;

    wxMenuItem* m_EditorPerspectiveOptions[Perspective_NumPerspectives];
    wxMenuItem* m_GameplayPerspectiveOptions[Perspective_NumPerspectives];

    char m_CurrentSceneName[MAX_PATH];

    wxMenu* m_Data;
    wxMenu* m_Hackery;
    wxMenu* m_Debug;

    cJSON* m_pEditorPrefs;

    int m_Hackery_Record_StackDepth;

public:
    GameMainFrame();
    ~GameMainFrame();

    virtual void InitFrame();
    virtual void AddPanes();
    virtual void OnPostInit();
    virtual void OnClose();

    void OnGameMenu(wxCommandEvent& event);
    virtual void ResizeViewport();
    void SetWindowPerspectiveToDefault(bool forceswitch = false);
    int GetDefaultEditorPerspectiveIndex();
    int GetDefaultGameplayPerspectiveIndex();
    void SetDefaultEditorPerspectiveIndex(int index);
    void SetDefaultGameplayPerspectiveIndex(int index);

    void SaveScene();
    void SaveSceneAs();
    void LoadSceneDialog();
    void LoadScene(const char* scenename);

    void AddDatafileToScene();
};

#endif __GameMainFrame_H__
