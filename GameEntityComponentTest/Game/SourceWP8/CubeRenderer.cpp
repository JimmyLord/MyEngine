#include "GameCommonHeader.h"
#include "CubeRenderer.h"
#include "WP8Main.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

CubeRenderer::CubeRenderer()
{
}

void CubeRenderer::CreateDeviceResources()
{
    Direct3DBase::CreateDeviceResources();
    return;
}

void CubeRenderer::CreateWindowSizeDependentResources()
{
    Direct3DBase::CreateWindowSizeDependentResources();

    DXWrapper_InitDeviceAndContext( m_d3dDevice, m_d3dContext, m_renderTargetView, m_depthStencilView );

    if( g_pGameCore == 0 )
    {
        DXWrapper_InitShadersAndBuffers();

        g_pGameCore = MyNew GameEntityComponentTest;
        g_pGameCore->OnSurfaceCreated();
        if( g_pWP8App->GetOrientation() == WP8Orientation_Portrait )
            g_pGameCore->OnSurfaceChanged( 0, 0, (unsigned int)m_renderTargetSize.Width, (unsigned int)m_renderTargetSize.Height );
        else
            g_pGameCore->OnSurfaceChanged( 0, 0, (unsigned int)m_renderTargetSize.Height, (unsigned int)m_renderTargetSize.Width );
        g_pGameCore->OneTimeInit();
    }
    else
    {
        if( g_pWP8App->GetOrientation() == WP8Orientation_Portrait )
            g_pGameCore->OnSurfaceChanged( 0, 0, (unsigned int)m_windowBounds.Width, (unsigned int)m_windowBounds.Height );
        else
            g_pGameCore->OnSurfaceChanged( 0, 0, (unsigned int)m_renderTargetSize.Height, (unsigned int)m_renderTargetSize.Width );
    }
}

void CubeRenderer::Render()
{
}
