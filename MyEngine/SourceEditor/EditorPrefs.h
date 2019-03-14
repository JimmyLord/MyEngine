//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
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
class ImGuiStylePrefs;

extern EditorPrefs* g_pEditorPrefs;

enum EngineEditorWindowTypes
{
    EngineEditorWindow_Editor,
    EngineEditorWindow_PanelLog,
    EngineEditorWindow_NumTypes,
};

enum LaunchPlatforms
{
#if MYFW_WINDOWS
    LaunchPlatform_Win32,
    LaunchPlatform_Win64,
    LaunchPlatform_NaCl,
    LaunchPlatform_Android,
    LaunchPlatform_Emscripten,
#elif MYFW_OSX
    LaunchPlatform_OSX,
    LaunchPlatform_iOSSimulator,
    LaunchPlatform_iOSDevice,
    LaunchPlatform_iOSDevice_iOS6,
#endif
    // AddNewLaunchPlatform
    LaunchPlatform_NumPlatforms,
};

extern const char* g_DefaultEngineEditorWindowTypeMenuLabels[EngineEditorWindow_NumTypes];
extern const char* g_LaunchPlatformsMenuLabels[LaunchPlatform_NumPlatforms];

struct GridSettings
{
    bool visible;
    bool snapEnabled;
    Vector3 stepSize;
};

class EditorPrefs
{
public:
    const static int MAX_RECENT_LUA_SCRIPTS = 10;
    const static int MAX_RECENT_SCENES = 10;
    const static int MAX_RECENT_DOCUMENTS = 10;

private:
    FILE* m_pSaveFile; // Stored temporarily between SaveStart and Save Finish.

protected:
    // Editor preferences JSON object.
    cJSON* m_jEditorPrefs;

    // Editor preferences.
    int m_WindowX;
    int m_WindowY;
    int m_WindowWidth;
    int m_WindowHeight;
    bool m_IsWindowMaximized;

    bool m_View_ShowEditorIcons;
    bool m_View_EditorCamDeferred;
    bool m_View_SelectedObjects_ShowWireframe;
    bool m_View_SelectedObjects_ShowEffect;
    GLViewTypes m_Aspect_CurrentGameWindowAspectRatio;
    GridSettings m_GridSettings;
    bool m_Mode_SwitchFocusOnPlayStop;
    LaunchPlatforms m_Mode_CurrentLaunchPlatform;
    bool m_Debug_DrawPhysicsDebugShapes;
    std::vector<std::string> m_Lua_RecentScripts;
    std::vector<std::string> m_File_RecentScenes;
    std::vector<std::string> m_Document_RecentDocuments;

#if MYFW_USING_IMGUI
    ImGuiStylePrefs* m_pImGuiStylePrefs;
#endif

public:
    EditorPrefs();
    ~EditorPrefs();

    void Init();
    void LoadWindowSizePrefs();
    void LoadPrefs();
    void LoadLastSceneLoaded();
    cJSON* SaveStart();
    void SaveFinish(cJSON* jPrefs);

    cJSON* GetEditorPrefsJSONString() { return m_jEditorPrefs; }

    int GetWindowX() { return m_WindowX; }
    int GetWindowY() { return m_WindowY; }
    int GetWindowWidth() { return m_WindowWidth; }
    int GetWindowHeight() { return m_WindowHeight; }
    bool IsWindowMaximized() { return m_IsWindowMaximized; }

    // Preference Setters
    void SetWindowProperties(int x, int y, int w, int h, bool maximized)
    {
        m_WindowX = x;
        m_WindowY = y;
        m_WindowWidth = w;
        m_WindowHeight = h;
        m_IsWindowMaximized = false;
    }

    GridSettings* GetGridSettings() { return &m_GridSettings; }

    bool Get_View_ShowEditorIcons() { return m_View_ShowEditorIcons; }
    bool Get_View_EditorCamDeferred() { return m_View_EditorCamDeferred; }
    bool Get_View_SelectedObjects_ShowWireframe() { return m_View_SelectedObjects_ShowWireframe; }
    bool Get_View_SelectedObjects_ShowEffect() { return m_View_SelectedObjects_ShowEffect; }
    GLViewTypes Get_Aspect_GameAspectRatio() { return m_Aspect_CurrentGameWindowAspectRatio; }
    bool Get_Grid_Visible() { return m_GridSettings.visible; }
    bool Get_Grid_SnapEnabled() { return m_GridSettings.snapEnabled; }
    bool Get_Mode_SwitchFocusOnPlayStop() { return m_Mode_SwitchFocusOnPlayStop; }
    LaunchPlatforms Get_Mode_LaunchPlatform() { return m_Mode_CurrentLaunchPlatform; }
    bool Get_Debug_DrawPhysicsDebugShapes() { return m_Debug_DrawPhysicsDebugShapes; }
    uint32 Get_Lua_NumRecentScripts() { return (uint32)m_Lua_RecentScripts.size(); }
    std::string Get_Lua_RecentScript(int index) { return m_Lua_RecentScripts[index]; }
    uint32 Get_File_NumRecentScenes() { return (uint32)m_File_RecentScenes.size(); }
    std::string Get_File_RecentScene(int index) { return m_File_RecentScenes[index]; }
    uint32 Get_Document_NumRecentDocuments() { return (uint32)m_Document_RecentDocuments.size(); }
    std::string Get_Document_RecentDocument(int index) { return m_Document_RecentDocuments[index]; }    

    void Toggle_View_ShowEditorIcons() { m_View_ShowEditorIcons = !m_View_ShowEditorIcons; }
    void Toggle_View_EditorCamDeferred() { m_View_EditorCamDeferred = !m_View_EditorCamDeferred; }
    void Toggle_View_SelectedObjects_ShowWireframe() { m_View_SelectedObjects_ShowWireframe = !m_View_SelectedObjects_ShowWireframe; }
    void Toggle_View_SelectedObjects_ShowEffect() { m_View_SelectedObjects_ShowEffect = !m_View_SelectedObjects_ShowEffect; }
    void Set_Aspect_GameAspectRatio(GLViewTypes newaspect) { m_Aspect_CurrentGameWindowAspectRatio = newaspect; }
    void Toggle_Grid_Visible();
    void Toggle_Grid_SnapEnabled();
    void Toggle_Mode_SwitchFocusOnPlayStop() { m_Mode_SwitchFocusOnPlayStop = !m_Mode_SwitchFocusOnPlayStop; }
    void Set_Mode_LaunchPlatform(LaunchPlatforms platform) { m_Mode_CurrentLaunchPlatform = platform; }
    void Toggle_Debug_DrawPhysicsDebugShapes() { m_Debug_DrawPhysicsDebugShapes = !m_Debug_DrawPhysicsDebugShapes; }

    void AddRecentLuaScript(const char* relativePath);
    void AddRecentScene(const char* relativePath);
    void AddRecentDocument(const char* relativePath);

    void FillGridSettingsWindow();

#if MYFW_USING_IMGUI
    ImGuiStylePrefs* GetImGuiStylePrefs() { return m_pImGuiStylePrefs; }
#endif
};

#endif //__EditorPrefs_H__
