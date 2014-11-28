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

class GameEntityComponentTest : public GameCore
{
public:
    ComponentSystemManager* m_pComponentSystemManager;

    ShaderGroup* m_pShader_TintColor;
    ShaderGroup* m_pShader_TestNormals;

    float m_GameWidth;
    float m_GameHeight;

    MyMesh* m_pTestOBJMesh;

public:
    GameEntityComponentTest();
    virtual ~GameEntityComponentTest();

    virtual void OneTimeInit();
    virtual void Tick(double TimePassed);
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height);
    virtual void OnDrawFrame();

    virtual void OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual void OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);
};

#endif //__GameEntityComponentTest_H__
