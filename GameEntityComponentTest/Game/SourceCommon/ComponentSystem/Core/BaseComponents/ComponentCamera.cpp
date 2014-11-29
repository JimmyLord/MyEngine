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

#include "GameCommonHeader.h"

ComponentCamera::ComponentCamera()
: ComponentBase()
{
    m_BaseType = BaseComponentType_Camera;
}

ComponentCamera::ComponentCamera(GameObject* owner)
: ComponentBase( owner )
{
    m_BaseType = BaseComponentType_Camera;
}

ComponentCamera::~ComponentCamera()
{
}

#if MYFW_USING_WX
void ComponentCamera::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentCamera::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "Camera" );
}

void ComponentCamera::FillPropertiesWindow()
{
    g_pPanelWatch->ClearAllVariables();
}
#endif //MYFW_USING_WX

void ComponentCamera::Reset()
{
    ComponentBase::Reset();

    m_Orthographic = false;
    m_LayersToRender = 0xFFFF;
}

void ComponentCamera::SetDesiredAspectRatio(float width, float height)
{
    m_DesiredWidth = width;
    m_DesiredHeight = height;
}

void ComponentCamera::Tick(double TimePassed)
{
    m_Camera3D.LookAt( Vector3( 0, 0, 10 ), Vector3( 0, 1, 0 ), Vector3( 0, 0, 0 ) );
}

void ComponentCamera::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    m_DesiredWidth = desiredaspectwidth;
    m_DesiredHeight = desiredaspectheight;

    float deviceratio = (float)width / (float)height;
    float gameratio = m_DesiredWidth / m_DesiredHeight;

    m_Camera3D.SetupProjection( deviceratio, gameratio, 45 );
    m_Camera2D.Setup( width, height, m_DesiredWidth, m_DesiredHeight );
}

void ComponentCamera::OnDrawFrame()
{
    if( m_Orthographic )
    {
        m_Camera2D.UpdateMatrices();
        g_pComponentSystemManager->OnDrawFrame( this, &m_Camera2D.m_matViewProj );
    }
    else
    {
        g_pComponentSystemManager->OnDrawFrame( this, &m_Camera3D.m_matViewProj );
    }
}
