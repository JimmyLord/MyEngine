//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
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

enum EditorActionState
{
    EDITORACTIONSTATE_None,
    EDITORACTIONSTATE_TranslateX,
    EDITORACTIONSTATE_TranslateY,
    EDITORACTIONSTATE_TranslateZ,
    EDITORACTIONSTATE_NumStates,
};

struct EditorState
{
    unsigned int m_ModifierKeyStates;
    EditorActionState m_EditorActionState;
    Vector2 m_MouseLeftDownLocation;
    Vector2 m_MouseRightDownLocation;
    Vector2 m_MouseMiddleDownLocation;
    Vector2 m_LastMousePosition;
    Vector2 m_CurrentMousePosition;

    FBODefinition* m_pMousePickerFBO;

    GameObject* m_pSelectedGameObject;

    GameObject* m_pTransformWidgets[3];

    EditorState::EditorState()
    {
        m_ModifierKeyStates = 0;
        m_EditorActionState = EDITORACTIONSTATE_None;
        m_MouseLeftDownLocation.Set( -1, -1 );
        m_MouseRightDownLocation.Set( -1, -1 );
        m_MouseMiddleDownLocation.Set( -1, -1 );
        m_LastMousePosition.Set( -1, -1 );
        m_CurrentMousePosition.Set( -1, -1 );

        m_pMousePickerFBO = 0;

        m_pSelectedGameObject = 0;

        for( int i=0; i<3; i++ )
            m_pTransformWidgets[i] = 0;
    }

    EditorState::~EditorState()
    {
        SAFE_RELEASE( m_pMousePickerFBO );
    }
};

enum LayerValues
{
    Layer_MainScene = 0x0001,
    Layer_HUD       = 0x0002,
};

class GameEntityComponentTest : public GameCore
{
public:
    ComponentSystemManager* m_pComponentSystemManager;

    BulletWorld* m_pBulletWorld;

    bool m_EditorMode;
    EditorState m_EditorState;

    double m_TimeSinceLastPhysicsStep;

    ShaderGroup* m_pShader_TintColor;
    ShaderGroup* m_pShader_TestNormals;
    ShaderGroup* m_pShader_Texture;
    ShaderGroup* m_pShader_MousePicker;

    float m_GameWidth;
    float m_GameHeight;

    MyFileObject* m_pOBJTestFiles[4];
    TextureDefinition* m_pTextures[4];

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

    void SaveScene();
    void LoadScene();

    GameObject* GetObjectAtPixel(unsigned int x, unsigned int y);
};

#endif //__GameEntityComponentTest_H__
