//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#include "Camera2D.h"

Camera2D::Camera2D()
{
}

Camera2D::~Camera2D()
{
}

void Camera2D::UpdateMatrices()
{
    //m_matView.SetIdentity();

    m_matViewProj = m_matProj * m_matView;
}

void Camera2D::Setup(float devicewidth, float deviceheight, float gamewidth, float gameheight, float nearZ, float farZ)
{
    m_DeviceWidth = devicewidth;
    m_DeviceHeight = deviceheight;

    m_GameWidth = gamewidth;
    m_GameHeight = gameheight;

    float deviceratio = devicewidth / deviceheight;
    float gameratio = gamewidth / gameheight;

    if( deviceratio < gameratio )
    {
        m_ScreenWidth = devicewidth;
        m_ScreenHeight = devicewidth / gameratio;
    }
    else
    {
        m_ScreenWidth = deviceheight * gameratio;
        m_ScreenHeight = deviceheight;
    }

    m_ScreenOffsetX = 0;
    m_ScreenOffsetY = 0;

    if( m_ScreenWidth < devicewidth )
        m_ScreenOffsetX = (devicewidth - m_ScreenWidth)/2;
 
    if( m_ScreenHeight < deviceheight )
        m_ScreenOffsetY = (deviceheight - m_ScreenHeight)/2;

    float ortholeft   = 0 - m_ScreenOffsetX/m_ScreenWidth;
    float orthoright  = 1 + m_ScreenOffsetX/m_ScreenWidth;
    float orthobottom = 0 - m_ScreenOffsetY/m_ScreenHeight;
    float orthotop    = 1 + m_ScreenOffsetY/m_ScreenHeight;

    m_OrthoLeft   = ortholeft   * m_GameWidth;
    m_OrthoRight  = orthoright  * m_GameWidth;
    m_OrthoBottom = orthobottom * m_GameHeight;
    m_OrthoTop    = orthotop    * m_GameHeight;

    m_matProj.CreateOrtho( m_OrthoLeft, m_OrthoRight, m_OrthoBottom, m_OrthoTop, nearZ, farZ ); // 0, 0 is bottom left.

    //LOGInfo( LOGTag, "[OnSurfaceChanged]===========================================\n" );
    //LOGInfo( LOGTag, "[OnSurfaceChanged] OnSurfaceChanged\n" );
    //LOGInfo( LOGTag, "[OnSurfaceChanged] Window Size - (%0.1f, %0.1f) ratio (%f)\n", devicewidth, deviceheight, deviceratio );
    //LOGInfo( LOGTag, "[OnSurfaceChanged] Game Size   - (%0.1f, %0.1f) ratio (%f)\n", m_GameWidth, m_GameHeight, gameratio );
    //LOGInfo( LOGTag, "[OnSurfaceChanged] Screen Size - (%0.1f, %0.1f) ratio (%f)\n", m_ScreenWidth, m_ScreenHeight, m_ScreenWidth/m_ScreenHeight );
    //LOGInfo( LOGTag, "[OnSurfaceChanged] Screen Offset (%0.1f, %0.1f)\n", m_ScreenOffsetX, m_ScreenOffsetY );
    //LOGInfo( LOGTag, "[OnSurfaceChanged]===========================================\n" );
}

void Camera2D::SetupDirect(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    m_OrthoLeft   = left;
    m_OrthoRight  = right;
    m_OrthoBottom = bottom;
    m_OrthoTop    = top;

    m_matProj.CreateOrtho( m_OrthoLeft, m_OrthoRight, m_OrthoBottom, m_OrthoTop, nearZ, farZ );
}

void Camera2D::SetPosZoom(Vector3 position, float zoom)
{
}
