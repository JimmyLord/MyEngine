//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Camera2D_H__
#define __Camera2D_H__

class Camera2D
{
public:

public: //protected:
    // the size in pixels of the device.
    float m_DeviceWidth;
    float m_DeviceHeight;

    // The size of the guaranteed "safe area" of the game.
    float m_GameWidth;
    float m_GameHeight;

    // size/offset of the "safe area" in pixels, likely useless outside of internal calculations.
    float m_ScreenWidth;
    float m_ScreenHeight;
    float m_ScreenOffsetX;
    float m_ScreenOffsetY;

    // the coordinates of the edges of device space relative to the "safe area" game size
    float m_OrthoLeft;
    float m_OrthoRight;
    float m_OrthoBottom;
    float m_OrthoTop;

    MyMatrix m_matView;
    MyMatrix m_matProj;

    MyMatrix m_matViewProj;

public:
    Camera2D();
    ~Camera2D();

    void UpdateMatrices();

    void Setup(float devicewidth, float deviceheight, float gamewidth, float gameheight, float nearZ, float farZ);
    void SetupDirect(float left, float right, float bottom, float top, float nearZ, float farZ);
    void SetPosZoom(Vector3 position, float zoom);
};

#endif //__Camera2D_H__
