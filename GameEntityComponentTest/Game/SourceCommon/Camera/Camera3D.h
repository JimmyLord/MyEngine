//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
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

#ifndef __Camera3D_H__
#define __Camera3D_H__

class Camera3D
{
public:
    float m_WindowAspectRatio;
    float m_DesiredGameAspectRatio;
    float m_DesiredHalfVerticalFoV;
    float m_DesiredHalfHorizontalFoV;

    Vector3 m_Eye;
    Vector3 m_Up;
    Vector3 m_At;
    float m_Zoom;

public: //protected:
    MyMatrix m_matView;
    MyMatrix m_matProj;

    MyMatrix m_matViewProj;

public:
    Camera3D();
    ~Camera3D();

    void UpdateMatrices();

    void SetupProjection(float windowaspectratio, float desiredgameaspectratio, float halfyfov);
    void LookAt(Vector3& eye, Vector3& up, Vector3& at);
};

#endif //__Camera3D_H__
