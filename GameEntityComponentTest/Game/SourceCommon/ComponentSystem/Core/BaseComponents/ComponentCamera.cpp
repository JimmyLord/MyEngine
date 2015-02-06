//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentCamera::ComponentCamera()
: ComponentBase()
{
    m_BaseType = BaseComponentType_Camera;

    m_pComponentTransform = 0;
}

ComponentCamera::~ComponentCamera()
{
}

#if MYFW_USING_WX
void ComponentCamera::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentCamera::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Camera" );
}

void ComponentCamera::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentCamera::FillPropertiesWindow(bool clear)
{
    ComponentBase::FillPropertiesWindow( clear );

    g_pPanelWatch->AddBool( "Ortho", &m_Orthographic, 0, 1, this, ComponentCamera::StaticOnValueChanged );

    g_pPanelWatch->AddFloat( "Desired width", &m_DesiredWidth, 640, 960, this, ComponentCamera::StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "Desired height", &m_DesiredHeight, 640, 960, this, ComponentCamera::StaticOnValueChanged );

    g_pPanelWatch->AddBool( "ColorBit", &m_ClearColorBuffer, 0, 1, this, ComponentCamera::StaticOnValueChanged );
    g_pPanelWatch->AddBool( "DepthBit", &m_ClearDepthBuffer, 0, 1, this, ComponentCamera::StaticOnValueChanged );

    g_pPanelWatch->AddUnsignedInt( "Layers", &m_LayersToRender, 0, 1, this, ComponentCamera::StaticOnValueChanged );
}

void ComponentCamera::OnValueChanged(int id, bool finishedchanging)
{
    ComputeProjectionMatrices();
    m_FullClearsRequired = 2;
}
#endif //MYFW_USING_WX

cJSON* ComponentCamera::ExportAsJSONObject()
{
    cJSON* component = ComponentBase::ExportAsJSONObject();

    cJSON_AddNumberToObject( component, "Ortho", m_Orthographic );

    cJSON_AddNumberToObject( component, "DWidth", m_DesiredWidth );
    cJSON_AddNumberToObject( component, "DHeight", m_DesiredHeight );

    cJSON_AddNumberToObject( component, "ColorBit", m_ClearColorBuffer );
    cJSON_AddNumberToObject( component, "DepthBit", m_ClearDepthBuffer );

    cJSON_AddNumberToObject( component, "Layers", m_LayersToRender );

    return component;
}

void ComponentCamera::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetBool( jsonobj, "Ortho", &m_Orthographic );

    cJSONExt_GetFloat( jsonobj, "DWidth", &m_DesiredWidth );
    cJSONExt_GetFloat( jsonobj, "DHeight", &m_DesiredHeight );

    cJSONExt_GetBool( jsonobj, "ColorBit", &m_ClearColorBuffer );
    cJSONExt_GetBool( jsonobj, "DepthBit", &m_ClearDepthBuffer );

    cJSONExt_GetUnsignedInt( jsonobj, "Layers", &m_LayersToRender );
}

void ComponentCamera::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;

    m_Orthographic = false;

    m_ClearColorBuffer = true;
    m_ClearDepthBuffer = true;
    
    m_DesiredWidth = 0;
    m_DesiredHeight = 0;
    
    m_LayersToRender = 0xFFFF;

    m_WindowStartX = 0;
    m_WindowStartY = 0;
    m_WindowWidth = 0;
    m_WindowHeight = 0;

#if MYFW_USING_WX
    m_FullClearsRequired = 2;
#endif
}

ComponentCamera& ComponentCamera::operator=(const ComponentCamera& other)
{
    assert( &other != this );

    ComponentBase::operator=( other );

    this->m_Orthographic = other.m_Orthographic;
    
    this->m_DesiredWidth = other.m_DesiredWidth;
    this->m_DesiredHeight = other.m_DesiredHeight;

    this->m_LayersToRender = other.m_LayersToRender;

    return *this;
}

void ComponentCamera::SetDesiredAspectRatio(float width, float height)
{
    m_DesiredWidth = width;
    m_DesiredHeight = height;

    ComputeProjectionMatrices();
}

void ComponentCamera::ComputeProjectionMatrices()
{
    if( m_WindowHeight == 0 )
        return;

    float deviceratio = (float)m_WindowWidth / (float)m_WindowHeight;
    float gameratio = m_DesiredWidth / m_DesiredHeight;

    m_Camera3D.SetupProjection( deviceratio, gameratio, 45 );
    m_Camera2D.Setup( (float)m_WindowWidth, (float)m_WindowHeight, m_DesiredWidth, m_DesiredHeight );
}

void ComponentCamera::Tick(double TimePassed)
{
    m_pComponentTransform->UpdateMatrix();
    MyMatrix matView = m_pComponentTransform->m_Transform;
    matView.Inverse();

    if( m_Orthographic )
    {
        m_Camera2D.m_matView = matView;
        m_Camera2D.UpdateMatrices();
    }
    else
    {
        m_Camera3D.m_matView = matView;
        m_Camera3D.UpdateMatrices();
    }
}

void ComponentCamera::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    m_DesiredWidth = (float)desiredaspectwidth;
    m_DesiredHeight = (float)desiredaspectheight;

    m_WindowStartX = startx;
    m_WindowStartY = starty;
    m_WindowWidth = width;
    m_WindowHeight = height;

    ComputeProjectionMatrices();

#if MYFW_USING_WX
    m_FullClearsRequired = 2;
#endif
}

void ComponentCamera::OnDrawFrame()
{
#if MYFW_USING_WX
    // if we resize the window and we're in a wx build, clear the entire backbuffer for 2 frames.
    // this is required since we're potentially GL_SCISSOR_TEST'ing an uncleared area.
    if( m_FullClearsRequired > 0 )
    {
        glDisable( GL_SCISSOR_TEST );
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        m_FullClearsRequired--;
    }
#endif

    // only draw to part of the window, but rest with scissor test and glViewPort.
    if( m_WindowStartX != 0 || m_WindowStartY != 0 )
    {
        // scissor test is really only needed for the glClear call.
        glEnable( GL_SCISSOR_TEST );
        glScissor( m_WindowStartX, m_WindowStartY, m_WindowWidth, m_WindowHeight );
    }
    else
    {
        glDisable( GL_SCISSOR_TEST );
    }

    glViewport( m_WindowStartX, m_WindowStartY, m_WindowWidth, m_WindowHeight );
    checkGlError( "glViewport" );

    glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );

    if( m_ClearColorBuffer && m_ClearDepthBuffer )
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    else if( m_ClearColorBuffer )
        glClear( GL_COLOR_BUFFER_BIT );
    else if( m_ClearDepthBuffer )
        glClear( GL_DEPTH_BUFFER_BIT );

    if( m_Orthographic )
    {   
        g_pComponentSystemManager->OnDrawFrame( this, &m_Camera2D.m_matViewProj, 0 );
    }
    else
    {
        g_pComponentSystemManager->OnDrawFrame( this, &m_Camera3D.m_matViewProj, 0 );
    }
}
