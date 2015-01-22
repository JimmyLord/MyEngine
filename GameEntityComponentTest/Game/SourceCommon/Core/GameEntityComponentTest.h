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
    Layer_Editor    = 0x0001,
    Layer_EditorFG  = 0x0002,
    Layer_MainScene = 0x0004,
    Layer_HUD       = 0x0008,
};

class GameEntityComponentTest : public GameCore
{
public:
    ComponentSystemManager* m_pComponentSystemManager;

    BulletWorld* m_pBulletWorld;
    LuaGameState* m_pLuaGameState;

    bool m_EditorMode;
#if MYFW_USING_WX
    EditorState m_EditorState;
#endif //MYFW_USING_WX

    double m_TimeSinceLastPhysicsStep;

    ShaderGroup* m_pShader_TintColor;
    ShaderGroup* m_pShader_TestNormals;
    ShaderGroup* m_pShader_Texture;
    ShaderGroup* m_pShader_MousePicker;

    float m_GameWidth;
    float m_GameHeight;

    MyFileObject* m_pOBJTestFiles[4];
    TextureDefinition* m_pTextures[4];
    MyFileObject* m_pScriptFiles[4];

public:
    GameEntityComponentTest();
    virtual ~GameEntityComponentTest();

    virtual void OneTimeInit();
    virtual void Tick(double TimePassed);
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);
    virtual void OnDrawFrame();

    virtual void OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual void OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
    virtual void OnKeyDown(int keycode, int unicodechar);
    virtual void OnKeyUp(int keycode, int unicodechar);

    void HandleEditorInput(int keydown, int keycode, int action, int id, float x, float y, float pressure);

    void SaveScene(const char* fullpath);
    void UnloadScene(unsigned int sceneid = UINT_MAX);
    void LoadScene(const char* fullpath, unsigned int sceneid);

    GameObject* GetObjectAtPixel(unsigned int x, unsigned int y);
    void GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end);

#if MYFW_USING_WX
    static void StaticOnObjectListTreeSelectionChanged(void* pObjectPtr) { ((GameEntityComponentTest*)pObjectPtr)->OnObjectListTreeSelectionChanged(); }
    void OnObjectListTreeSelectionChanged();
#endif //MYFW_USING_WX
};

#endif //__GameEntityComponentTest_H__
