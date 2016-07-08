//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineCore_H__
#define __EngineCore_H__

#if _DEBUG
#define MYFW_PROFILING_ENABLED 1
#endif

class BulletWorld;
class EngineCore;

extern EngineCore* g_pEngineCore;

enum ModifierKeys
{
    MODIFIERKEY_Control     = 0x0001,
    MODIFIERKEY_Alt         = 0x0002,
    MODIFIERKEY_Shift       = 0x0004,
    MODIFIERKEY_Space       = 0x0008,
    MODIFIERKEY_LeftMouse   = 0x0010,
    MODIFIERKEY_RightMouse  = 0x0020,
    MODIFIERKEY_MiddleMouse = 0x0040,
};

enum LayerValues
{
    Layer_MainScene             = 0x0001,
    Layer_HUD                   = 0x0002,
    Layer_Editor                = 0x2000,
    Layer_EditorUnselectable    = 0x4000,
    Layer_EditorFG              = 0x8000,
};

enum EditorInterfaceTypes
{
    EditorInterfaceType_SceneManagement,
    EditorInterfaceType_2DPointEditor,
    EditorInterfaceType_NumInterfaces,
};

struct RequestedSceneInfo
{
    MyFileObject* m_pFile; // acts as a flag whether or not scene was requested.
    unsigned int m_SceneID; // generally -1, unless scene requested for specific slot.
};

struct FrameTimingInfo
{
    float FrameTime;
    float Tick;
    float Render_Editor;
    float Render_Game;
};

extern void OnFileUpdated_CallbackFunction(MyFileObject* pFile);

class EngineCore : public GameCore
{
public:
    static const int MAX_FRAMES_TO_STORE = 60*30; // 30 seconds @ 60fps
    static const int ENGINE_SCENE_ID = 9; //9879;
    static const int MAX_SCENE_FILES_QUEUED_UP = 10;

public:
    ComponentSystemManager* m_pComponentSystemManager;
    MyStackAllocator m_SingleFrameMemoryStack;

    BulletWorld* m_pBulletWorld;

#if MYFW_USING_LUA
    LuaGameState* m_pLuaGameState;
#endif //MYFW_USING_LUA

    bool m_EditorMode;
    bool m_AllowGameToRunInEditorMode;
    bool m_Paused;
    double m_PauseTimeToAdvance; // advance clock by this much on next tick.

    Vector2 m_LastMousePos;

    float m_DebugFPS;
    int m_LuaMemoryUsedLastFrame;
    int m_LuaMemoryUsedThisFrame;
    unsigned int m_TotalMemoryAllocatedLastFrame;
    unsigned int m_SingleFrameStackSizeLastFrame;
    unsigned int m_SingleFrameStackSizeThisFrame;

    char* m_GameObjectFlagStrings[32];

#if MYFW_USING_WX
    EditorState* m_pEditorState;
    bool m_Debug_DrawMousePickerFBO;
    bool m_Debug_DrawSelectedAnimatedMesh;
    bool m_Debug_DrawSelectedMaterial;
    bool m_Debug_DrawPhysicsDebugShapes;
    bool m_Debug_ShowProfilingInfo;
    bool m_Debug_DrawGLStats;
    MyFileObject* m_pSphereMeshFile;
    MySprite* m_pDebugQuadSprite;
    MyMesh* m_pMaterialBallMesh;
    FontDefinition* m_pDebugFont;
    MyMeshText* m_pDebugTextMesh; // DEBUG_HACK_SHOWGLSTATS
#endif //MYFW_USING_WX
    bool m_FreeAllMaterialsAndTexturesWhenUnloadingScene;

    double m_TimeSinceLastPhysicsStep;

    MyFileObject* m_pShaderFile_TintColor;
    MyFileObject* m_pShaderFile_ClipSpaceTexture;
    ShaderGroup* m_pShader_TintColor;
    ShaderGroup* m_pShader_ClipSpaceTexture;
    MaterialDefinition* m_pMaterial_Box2DDebugDraw;
    MaterialDefinition* m_pMaterial_3DGrid;
    MaterialDefinition* m_pMaterial_MousePicker;
    MaterialDefinition* m_pMaterial_ClipSpaceTexture;

    float m_GameWidth;
    float m_GameHeight;

protected:
    bool m_SceneReloadRequested;
    RequestedSceneInfo m_pSceneFilesLoading[MAX_SCENE_FILES_QUEUED_UP]; // TODO: replace this monstrosity with an ordered list.

#if MYFW_USING_WX
    EditorInterface* m_pEditorInterfaces[EditorInterfaceType_NumInterfaces];
    EditorInterfaceTypes m_CurrentEditorInterfaceType;
    EditorInterface* m_pCurrentEditorInterface;
#endif //MYFW_USING_WX

#if MYFW_PROFILING_ENABLED
    FrameTimingInfo m_FrameTimingInfo[MAX_FRAMES_TO_STORE];
    unsigned int m_FrameTimingNextEntry;
#endif

public:
    EngineCore();
    virtual ~EngineCore();

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual void InitializeManagers();
    void InitializeGameObjectFlagStrings(cJSON* jStringsArray);

    virtual ComponentTypeManager* CreateComponentTypeManager() = 0;
#if MYFW_USING_LUA
    virtual LuaGameState* CreateLuaGameState() { return MyNew LuaGameState; }
#endif //MYFW_USING_LUA

    virtual void OneTimeInit();
    virtual bool IsReadyToRender();

    virtual double Tick(double TimePassed);
    virtual void OnFocusGained();
    virtual void OnFocusLost();
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);
    virtual void OnDrawFrame(unsigned int canvasid);
    virtual void OnDrawFrameDone();
    virtual void OnFileRenamed(const char* fullpathbefore, const char* fullpathafter);

    virtual bool OnEvent(MyEvent* pEvent);

    void SetMousePosition(float x, float y);
    virtual bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
    virtual bool OnKeys(GameCoreButtonActions action, int keycode, int unicodechar);
    virtual bool OnChar(unsigned int c);

    virtual void OnModeTogglePlayStop();
    virtual void OnModePlay();
    virtual void OnModeStop();
    virtual void OnModePause();
    virtual void OnModeAdvanceTime(double time);

    virtual void RegisterGameplayButtons();
    virtual void UnregisterGameplayButtons();
#if MYFW_USING_WX
    bool HandleImGuiInput(int canvasid, int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);
    bool HandleEditorInput(int canvasid, int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);
#endif //MYFW_USING_WX

    void CreateDefaultEditorSceneObjects();
    void CreateDefaultSceneObjects();
    void ReloadScene(unsigned int sceneid);
    void ReloadSceneInternal(unsigned int sceneid);
    void RequestScene(const char* fullpath);
    RequestedSceneInfo* RequestSceneInternal(const char* fullpath);
    void SaveScene(const char* fullpath, unsigned int sceneid);
    void ExportBox2DScene(const char* fullpath, unsigned int sceneid);
    void UnloadScene(unsigned int sceneid, bool cleareditorobjects);
#if MYFW_USING_WX
    unsigned int LoadSceneFromFile(const char* fullpath);
    void Editor_QuickSaveScene(const char* fullpath);
    void Editor_QuickLoadScene(const char* fullpath);
#endif //MYFW_USING_WX
    void LoadSceneFromJSON(const char* scenename, const char* jsonstr, unsigned int sceneid);

#if MYFW_USING_WX
    void Editor_OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);

    void RenderSingleObject(GameObject* pObject);
    void GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end);

    void SetEditorInterface(EditorInterfaceTypes type);
    EditorInterface* GetEditorInterface(EditorInterfaceTypes type);
    EditorInterface* GetCurrentEditorInterface();
    EditorInterfaceTypes GetCurrentEditorInterfaceType() { return m_CurrentEditorInterfaceType; }
#endif //MYFW_USING_WX

#if MYFW_USING_WX
    static void StaticOnObjectListTreeSelectionChanged(void* pObjectPtr) { ((EngineCore*)pObjectPtr)->OnObjectListTreeSelectionChanged(); }
    void OnObjectListTreeSelectionChanged();
#endif //MYFW_USING_WX
};

#endif //__EngineCore_H__
