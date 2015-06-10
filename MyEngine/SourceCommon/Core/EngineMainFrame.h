//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineMainFrame_H__
#define __EngineMainFrame_H__

class EngineMainFrame;

extern EngineMainFrame* g_pEngineMainFrame;

enum DefaultPerspectives
{
    Perspective_CenterEditor,
    Perspective_CenterGame,
    Perspective_CenterSideBySide,
    Perspective_FullFrameGame,
    Perspective_NumPerspectives,
};

enum GameMenuIDs
{
    myIDGame_NewScene = myID_NumIDs,
    myIDGame_LoadScene,
    myIDGame_SaveScene,
    myIDGame_SaveSceneAs,
    myIDGame_AddDatafile,
    myIDGame_Grid_SnapOnOff,
    myIDGame_Grid_Settings,
    myIDGame_Mode_PlayStop,
    myIDGame_Mode_Pause,
    myIDGame_Mode_Advance1Frame,
    myIDGame_Mode_Advance1Second,
    //myIDGame_Mode_Stop,
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

struct GridSettings
{
    bool snapenabled;
    Vector3 stepsize;
};

class EngineMainFrame : public MainFrame
{
public:
    MainGLCanvas* m_pGLCanvasEditor;
    wxTextCtrl* m_pLogPane;

    unsigned int m_StackDepthAtLastSave;

    wxMenu* m_EditorPerspectives;
    wxMenu* m_GameplayPerspectives;

    wxMenuItem* m_EditorPerspectiveOptions[Perspective_NumPerspectives];
    wxMenuItem* m_GameplayPerspectiveOptions[Perspective_NumPerspectives];

    //char m_CurrentSceneName[MAX_PATH];

    wxMenu* m_Grid;
    wxMenu* m_PlayPauseStop;
    wxMenu* m_Data;
    wxMenu* m_Hackery;
    wxMenu* m_Debug;

    wxMenuItem* m_MenuItem_GridSnapEnabled;

    cJSON* m_pEditorPrefs;

    GridSettings m_GridSettings;

    int m_Hackery_Record_StackDepth;

public:
    EngineMainFrame();
    ~EngineMainFrame();

    virtual void InitFrame();
    virtual void AddPanes();
    virtual bool UpdateAUIManagerAndLoadPerspective();
    virtual void OnPostInit();
    virtual bool OnClose();

    virtual void ResizeViewport();

    virtual void UpdateMenuItemStates();

    void OnGameMenu(wxCommandEvent& event);
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

    static void StaticOnDrop(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((EngineMainFrame*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);
};

#endif //__EngineMainFrame_H__
