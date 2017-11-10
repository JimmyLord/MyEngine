//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
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

void EngineMainFrame_DumpCachedMessagesToLogPane();

enum DefaultPerspectives
{
    Perspective_CenterEditor,
    Perspective_CenterGame,
    Perspective_CenterSideBySide,
    Perspective_FullFrameGame,
    Perspective_NumPerspectives,
};

enum EngineEditorWindowTypes
{
    EngineEditorWindow_Editor,
    EngineEditorWindow_PanelLog,
    EngineEditorWindow_NumTypes,
};

enum LaunchPlatforms
{
    LaunchPlatform_Win32,
    LaunchPlatform_Win64,
    LaunchPlatform_NaCl,
    LaunchPlatform_Android,
    LaunchPlatform_Emscripten,
    // AddNewLaunchPlatform
    LaunchPlatform_NumPlatforms,
};

enum EngineMenuIDs
{
    myIDEngine_File_NewScene = myID_LastID,
    myIDEngine_File_LoadScene,
    myIDEngine_File_CreateAdditionalScene,
    myIDEngine_File_LoadAdditionalScene,
    myIDEngine_File_SaveScene,
    myIDEngine_File_SaveSceneAs,
    myIDEngine_File_ExportBox2DScene,

    myIDEngine_View_EditorPerspectives,
    myIDEngine_View_GameplayPerspectives,
    myIDEngine_View_EditorPerspective,
        // Perspective_NumPerspectives more items here
    myIDEngine_View_GameplayPerspective = myIDEngine_View_EditorPerspective + Perspective_NumPerspectives,
        // Perspective_NumPerspectives more items here
    myIDEngine_View_ShowEditorIcons = myIDEngine_View_GameplayPerspective + Perspective_NumPerspectives,
    myIDEngine_View_SelectedObjects_ShowWireframe,
    myIDEngine_View_SelectedObjects_ShowEffect,
    myIDEngine_View_EditorCameraLayers,
    myIDEngine_View_EditorCameraLayer,
        // g_NumberOfVisibilityLayers more items here
    myIDEngine_View_FullScreenEditor = myIDEngine_View_EditorCameraLayer + g_NumberOfVisibilityLayers,
    myIDEngine_View_FullScreenGame,

    // Editor window toggles extended from m_EditorWindows in MainFrame, must have as many as EngineEditorWindow_NumTypes
    myIDEngine_View_EditorWindow_FirstWindow,
    myIDEngine_View_EditorWindow_Editor = myIDEngine_View_EditorWindow_FirstWindow,
    myIDEngine_View_EditorWindow_LogPane,

    myIDEngine_Grid_VisibleOnOff,
    myIDEngine_Grid_SnapOnOff,
    myIDEngine_Grid_Settings,

    myIDEngine_Mode_SwitchFocusOnPlayStop,
    myIDEngine_Mode_PlayStop,
    myIDEngine_Mode_Pause,
    myIDEngine_Mode_Advance1Frame,
    myIDEngine_Mode_Advance1Second,
    myIDEngine_Mode_LaunchPlatforms,
        // LaunchPlatform_NumPlatforms more items here
    myIDEngine_Mode_LaunchGame = myIDEngine_Mode_LaunchPlatforms + LaunchPlatform_NumPlatforms,

    myIDEngine_Data_AddDatafile,

    myIDEngine_Hackery_RecordMacro,
    myIDEngine_Hackery_ExecuteMacro,

    myIDEngine_Debug_ShowMousePickerFBO,
    myIDEngine_Debug_ShowSelectedAnimatedMesh,
    myIDEngine_Debug_ShowGLStats,
    myIDEngine_Debug_DrawWireframe,
    myIDEngine_Debug_ShowPhysicsShapes,
    myIDEngine_Debug_ShowProfilingInfo,

    myIDEngine_LastID,
};

struct GridSettings
{
    bool visible;
    bool snapenabled;
    Vector3 stepsize;
};

class FullScreenFrame : public wxFrame
{
public:
    MainGLCanvas* m_pCurrentCanvas;

public:
    FullScreenFrame(wxWindow* pParent);

    void OnCloseWindow(wxCloseEvent& event);
};

class EngineMainFrame : public MainFrame
{
protected:
    // Fullscreen frame used for using editor in full screen mode.
    // When fullscreen, the m_pGLCanvas will be parented to this.
    FullScreenFrame* m_pFullScreenFrame;

    // Editor windows
    MainGLCanvas* m_pGLCanvasEditor;
    wxNotebook* m_pLogPane;
    wxTextCtrl* m_pLogMain;
    wxTextCtrl* m_pLogInfo;
    wxTextCtrl* m_pLogErrors;

    // Engine specific windows tacked onto m_EditorWindows list.
    wxMenuItem* m_MenuItem_View_EngineEditorWindowOptions[EngineEditorWindow_NumTypes];

    wxMenu* m_SubMenu_View_EditorPerspectives;
    wxMenu* m_SubMenu_View_GameplayPerspectives;
    wxMenu* m_SubMenu_View_EditorCameraLayers;

    wxMenuItem* m_MenuItem_View_EditorPerspectiveOptions[Perspective_NumPerspectives];
    wxMenuItem* m_MenuItem_View_GameplayPerspectiveOptions[Perspective_NumPerspectives];
    wxMenuItem* m_MenuItem_View_EditorCameraLayerOptions[g_NumberOfVisibilityLayers];

    // Engine specific menus
    wxMenu* m_Menu_Grid;
    wxMenu* m_Menu_Mode;
    wxMenu* m_Menu_Data;
    wxMenu* m_Menu_Hackery;
    wxMenu* m_Menu_Debug;

    // Engine specific menu items
    wxMenuItem* m_MenuItem_View_ShowEditorIcons;
    wxMenuItem* m_MenuItem_View_SelectedObjects_ShowWireframe;
    wxMenuItem* m_MenuItem_View_SelectedObjects_ShowEffect;

    wxMenuItem* m_MenuItem_Grid_Visible;
    wxMenuItem* m_MenuItem_Grid_SnapEnabled;

    wxMenuItem* m_MenuItem_Mode_SwitchFocusOnPlayStop;
    wxMenu* m_SubMenu_Mode_LaunchPlatform;
    wxMenuItem* m_MenuItem_Mode_LaunchPlatformOptions[LaunchPlatform_NumPlatforms];

    wxMenuItem* m_MenuItem_Debug_DrawMousePickerFBO;
    wxMenuItem* m_MenuItem_Debug_DrawSelectedAnimatedMesh;
    wxMenuItem* m_MenuItem_Debug_DrawGLStats;
    wxMenuItem* m_MenuItem_Debug_DrawWireframe;
    wxMenuItem* m_MenuItem_Debug_DrawPhysicsDebugShapes;
    wxMenuItem* m_MenuItem_Debug_ShowProfilingInfo;

    // Editor preferences
    cJSON* m_pEditorPrefs;

    bool m_ShowEditorIcons;
    bool m_SelectedObjects_ShowWireframe;
    bool m_SelectedObjects_ShowEffect;
    GridSettings m_GridSettings;
    bool m_Mode_SwitchFocusOnPlayStop;

    // Editor state values we don't save to disk
    unsigned int m_UndoStackDepthAtLastSave;
    int m_Hackery_Record_StackDepth;

public:
    EngineMainFrame();
    ~EngineMainFrame();

    // EngineMainFrame Getters
    FullScreenFrame* GetFullScreenFrame() { return m_pFullScreenFrame; }

    MainGLCanvas* GetGLCanvasEditor() { return m_pGLCanvasEditor; }
    wxNotebook* GetLogPane() { return m_pLogPane; }
    wxTextCtrl* GetLogMain() { return m_pLogMain; }
    wxTextCtrl* GetLogInfo() { return m_pLogInfo; }
    wxTextCtrl* GetLogErrors() { return m_pLogErrors; }

    unsigned int GetUndoStackDepthAtLastSave() { return m_UndoStackDepthAtLastSave; }

    bool ShowEditorIcons() { return m_ShowEditorIcons; }
    bool SelectedObjects_ShowWireframe() { return m_SelectedObjects_ShowWireframe; }
    bool SelectedObjects_ShowEffect() { return m_SelectedObjects_ShowEffect; }
    bool Mode_SwitchFocusOnPlayStop() { return m_Mode_SwitchFocusOnPlayStop; }
    GridSettings* GetGridSettings() { return &m_GridSettings; }

    // EngineMainFrame Methods
    virtual void InitFrame();
    virtual void AddPanes();
    virtual bool UpdateAUIManagerAndLoadPerspective();
    virtual void OnPostInit();
    virtual bool OnClose();

    virtual bool FilterGlobalEvents(wxEvent& event);
    virtual void OnGLCanvasShownOrHidden(bool shown);

    void OnDropFiles(wxDropFilesEvent& event);

    virtual void ResizeViewport();

    virtual void UpdateMenuItemStates();

    virtual void ProcessAllGLCanvasInputEventQueues();

    void StoreCurrentUndoStackSize();

    void OnMenu_Engine(wxCommandEvent& event);

    void SetGLCanvasFullScreenMode(MainGLCanvas* canvas, bool show);
    void SetWindowPerspectiveToDefault(bool forceswitch = false);
    int GetCurrentPerspectiveIndex();
    int GetDefaultEditorPerspectiveIndex();
    int GetDefaultGameplayPerspectiveIndex();
    void SetDefaultEditorPerspectiveIndex(int index);
    void SetDefaultGameplayPerspectiveIndex(int index);

    int GetLaunchPlatformIndex();
    void SetLaunchPlatformIndex(int index);

    void SaveScene();
    void SaveSceneAs(unsigned int sceneid);
    void ExportBox2DScene(unsigned int sceneid);
    void LoadSceneDialog(bool unloadscenes);
    void LoadScene(const char* scenename, bool unloadscenes);

    void AddDatafilesToScene();
    void LoadDatafile(wxString filename);

    static void StaticOnDrop(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((EngineMainFrame*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);

    // Internal event handling functions
    void OnTextCtrlLeftDoubleClick(wxMouseEvent& evt);

protected:
    void LaunchGame();
};

#endif //__EngineMainFrame_H__
