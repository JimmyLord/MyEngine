//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "Camera3D.h"

Camera3D::Camera3D()
{
    //m_WindowAspectRatio = 1;

    //m_DesiredGameAspectRatio = 1;
    //m_DesiredVerticalFoV = 45;
    //m_DesiredHorizontalFoV = 45;

    //m_Eye.Set( 0, 0, 10 );
    //m_Up.Set( 0, 1, 0 );
    //m_At.Set( 0, 0, 0 );
    //m_Zoom = 1;

    m_matView.SetIdentity();
    m_matProj.SetIdentity();
    m_matViewProj.SetIdentity();
}

Camera3D::~Camera3D()
{
}

void Camera3D::UpdateMatrices()
{
    m_matViewProj = m_matProj * m_matView;
}

void Camera3D::SetupProjection(float windowaspectratio, float desiredgameaspectratio, float vertfov, float nearZ, float farZ)
{
    //m_WindowAspectRatio = windowaspectratio;
    //m_DesiredGameAspectRatio = desiredgameaspectratio;
    //m_DesiredVerticalFoV = vertfov;

    // Calculate the desired horizontal FoV based on the vertical and aspect ratio.
        // aspect_ratio = tan( HorizontalFoV/2 ) / tan( VerticalFoV/2 ) 
    float vertunits = tanf( vertfov / 360.0f * PI );
    float horunits = vertunits * desiredgameaspectratio;
    float horfov = atanf( horunits ) * 360.0f / PI;
    //m_DesiredHorizontalFoV = horfov;

    // if the window is taller than we asked for, lock in the horizontal FoV, otherwise lock to the Vertical
    if( windowaspectratio < desiredgameaspectratio )
        m_matProj.CreatePerspectiveHFoV( horfov, windowaspectratio, nearZ, farZ );
    else
        m_matProj.CreatePerspectiveVFoV( vertfov, windowaspectratio, nearZ, farZ );

    UpdateMatrices();
}

void Camera3D::LookAt(Vector3& eye, Vector3& up, Vector3& at)
{
    m_matView.CreateLookAtView( eye, up, at );

    UpdateMatrices();
}
