//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
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

enum EngineMenuIDs
{
    myIDEngine_NewScene = myID_LastID,
    myIDEngine_LoadScene,
    myIDEngine_CreateAdditionalScene,
    myIDEngine_LoadAdditionalScene,
    myIDEngine_SaveScene,
    myIDEngine_SaveSceneAs,
    myIDEngine_ExportBox2DScene,
    myIDEngine_AddDatafile,
    myIDEngine_Grid_SnapOnOff,
    myIDEngine_Grid_Settings,
    myIDEngine_Mode_PlayStop,
    myIDEngine_Mode_Pause,
    myIDEngine_Mode_Advance1Frame,
    myIDEngine_Mode_Advance1Second,
    //myIDEngine_Mode_Stop,
    myIDEngine_RecordMacro,
    myIDEngine_ExecuteMacro,
    myIDEngine_View_EditorPerspectives,
    myIDEngine_View_GameplayPerspectives,
    myIDEngine_View_EditorPerspective,
        // Perspective_NumPerspectives more items here
    myIDEngine_View_GameplayPerspective = myIDEngine_View_EditorPerspective + Perspective_NumPerspectives,
        // Perspective_NumPerspectives more items here
    myIDEngine_View_ShowEditorIcons = myIDEngine_View_GameplayPerspective + Perspective_NumPerspectives,
    myIDEngine_View_EditorCameraLayers,
    myIDEngine_View_EditorCameraLayer,
        // g_NumberOfVisibilityLayers more items here
    myIDEngine_DebugShowMousePickerFBO = myIDEngine_View_EditorCameraLayer + g_NumberOfVisibilityLayers,
    myIDEngine_DebugShowSelectedAnimatedMesh,
    myIDEngine_DebugShowGLStats,
    myIDEngine_DebugDrawWireframe,
    myIDEngine_DebugShowPhysicsShapes,
    myIDEngine_DebugShowProfilingInfo,
    myIDEngine_LastID,
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
    wxNotebook* m_pLogPane;
    wxTextCtrl* m_pLogMain;
    wxTextCtrl* m_pLogInfo;
    wxTextCtrl* m_pLogErrors;

    unsigned int m_StackDepthAtLastSave;

    wxMenu* m_EditorPerspectives;
    wxMenu* m_GameplayPerspectives;
    wxMenu* m_EditorCameraLayers;

    wxMenuItem* m_EditorPerspectiveOptions[Perspective_NumPerspectives];
    wxMenuItem* m_GameplayPerspectiveOptions[Perspective_NumPerspectives];
    wxMenuItem* m_EditorCameraLayerOptions[g_NumberOfVisibilityLayers];

    //char m_CurrentSceneName[MAX_PATH];

    wxMenu* m_Grid;
    wxMenu* m_PlayPauseStop;
    wxMenu* m_Data;
    wxMenu* m_Hackery;
    wxMenu* m_Debug;

    wxMenuItem* m_MenuItem_GridSnapEnabled;
    wxMenuItem* m_MenuItem_ShowEditorIcons;

    wxMenuItem* m_MenuItem_Debug_DrawMousePickerFBO;
    wxMenuItem* m_MenuItem_Debug_DrawSelectedAnimatedMesh;
    wxMenuItem* m_MenuItem_Debug_DrawGLStats;
    wxMenuItem* m_MenuItem_Debug_DrawWireframe;
    wxMenuItem* m_MenuItem_Debug_DrawPhysicsDebugShapes;
    wxMenuItem* m_MenuItem_Debug_ShowProfilingInfo;

    cJSON* m_pEditorPrefs;

    // Editor preferences
    bool m_ShowEditorIcons;
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

    virtual bool FilterGlobalEvents(wxEvent& event);

    void OnDropFiles(wxDropFilesEvent& event);

    virtual void ResizeViewport();

    virtual void UpdateMenuItemStates();

    virtual void ProcessAllGLCanvasInputEventQueues();

    void OnMenu_Engine(wxCommandEvent& event);
    void SetWindowPerspectiveToDefault(bool forceswitch = false);
    int GetCurrentPerspectiveIndex();
    int GetDefaultEditorPerspectiveIndex();
    int GetDefaultGameplayPerspectiveIndex();
    void SetDefaultEditorPerspectiveIndex(int index);
    void SetDefaultGameplayPerspectiveIndex(int index);

    void SaveScene();
    void SaveSceneAs(unsigned int sceneid);
    void ExportBox2DScene(unsigned int sceneid);
    void LoadSceneDialog(bool unloadscenes);
    void LoadScene(const char* scenename, bool unloadscenes);

    void AddDatafilesToScene();
    void LoadDatafile(wxString filename);

    static void StaticOnDrop(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((EngineMainFrame*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);
};

#endif //__EngineMainFrame_H__
