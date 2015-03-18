//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameEntityComponentTest_H__
#define __GameEntityComponentTest_H__

class BulletWorld;

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
    Layer_MainScene = 0x0001,
    Layer_HUD       = 0x0002,
    Layer_Editor    = 0x4000,
    Layer_EditorFG  = 0x8000,
};

class GameEntityComponentTest : public GameCore
{
public:
    ComponentSystemManager* m_pComponentSystemManager;

    BulletWorld* m_pBulletWorld;
    LuaGameState* m_pLuaGameState;

    bool m_EditorMode;
    bool m_Paused;
    double m_PauseTimeToAdvance; // advance clock by this much on next tick.
#if MYFW_USING_WX
    EditorState* m_pEditorState;
    bool m_Debug_DrawMousePickerFBO;
    bool m_Debug_DrawSelectedAnimatedMesh;
    MySprite* m_pDebugQuadSprite;
#endif //MYFW_USING_WX

    double m_TimeSinceLastPhysicsStep;

    MyFileObject* m_pShaderFile_TintColor;
    MyFileObject* m_pShaderFile_ClipSpaceTexture;
    ShaderGroup* m_pShader_TransformGizmo;
    ShaderGroup* m_pShader_MousePicker;
    ShaderGroup* m_pShader_ClipSpaceTexture;

    float m_GameWidth;
    float m_GameHeight;

    MyFileObject* m_pSceneFileToLoad;
    bool m_SceneLoaded;

public:
    GameEntityComponentTest();
    virtual ~GameEntityComponentTest();

    virtual void OneTimeInit();
    virtual double Tick(double TimePassed);
    virtual void OnFocusGained();
    virtual void OnFocusLost();
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);
    virtual void OnDrawFrame();

    virtual void OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual void OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
    virtual void OnKey(GameCoreButtonActions action, int keycode, int unicodechar);

    void RegisterGameplayButtons();
    void UnregisterGameplayButtons();
    void HandleEditorInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);

    void SaveScene(const char* fullpath);
    void UnloadScene(unsigned int sceneid = UINT_MAX);
#if MYFW_USING_WX
    void LoadSceneFromFile(const char* fullpath, unsigned int sceneid);
#endif //MYFW_USING_WX
    void LoadScene(const char* buffer, unsigned int sceneid);

#if MYFW_USING_WX
    void RenderSingleObject(GameObject* pObject);
    GameObject* GetObjectAtPixel(unsigned int x, unsigned int y);
    void GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end);
#endif //MYFW_USING_WX

#if MYFW_USING_WX
    static void StaticOnObjectListTreeSelectionChanged(void* pObjectPtr) { ((GameEntityComponentTest*)pObjectPtr)->OnObjectListTreeSelectionChanged(); }
    void OnObjectListTreeSelectionChanged();
#endif //MYFW_USING_WX
};

#endif //__GameEntityComponentTest_H__
