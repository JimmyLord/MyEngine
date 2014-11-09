#include "GameCommonHeader.h"
#include "BasicTimer.h"

//#define SCREEN_WIDTH    1024 //1280
//#define SCREEN_HEIGHT   576 //720

#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include "Direct3DBase.h"

#include "WP8Main.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Phone::UI::Input;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace concurrency;

WP8App^ g_pWP8App;

WP8App::WP8App()
: m_windowClosed(false)
, m_windowVisible(true)
{
    g_pWP8App = this;
    
    //m_Orientation = WP8Orientation_Portrait;
    m_Orientation = WP8Orientation_Landscape90;
    m_OrientationLocked = false;

    m_ExitOnBackButton = true;
}

void WP8App::Initialize(CoreApplicationView^ applicationView)
{
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WP8App::OnActivated);

    CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &WP8App::OnSuspending);

    CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &WP8App::OnResuming);

    HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs^>(this, &WP8App::OnBackPressed);

    m_renderer = ref new CubeRenderer();

    WSAData wsaData;
    int code = WSAStartup(MAKEWORD(1, 1), &wsaData);
    if( code != 0 )
    {
        printf("shite. %d\n",code);
    }
}

void WP8App::SetWindow(CoreWindow^ window)
{
    window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WP8App::OnVisibilityChanged);

    window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &WP8App::OnWindowClosed);

    window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WP8App::OnKeyDown);
    window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WP8App::OnKeyUp);

    window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WP8App::OnPointerPressed);
    window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WP8App::OnPointerMoved);
    window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WP8App::OnPointerReleased);

    m_renderer->Initialize(CoreWindow::GetForCurrentThread());
}

void WP8App::Load(Platform::String^ entryPoint)
{
}

void WP8App::Run()
{
    BasicTimer^ timer = ref new BasicTimer();

    while( !m_windowClosed )
    {
        if( m_windowVisible )
        {
            timer->Update();
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

            //if( m_CoreWindow->Visible == false && g_pGameCore->m_HasFocus == true )
            //    g_pGameCore->OnFocusLost();
            //if( m_CoreWindow->Visible == true && g_pGameCore->m_HasFocus == false )
            //    g_pGameCore->OnFocusGained();

            g_pGameCore->Tick( timer->Delta );
            g_pGameCore->OnDrawFrame();

            m_renderer->Present(); // This call is synchronized to the display frame rate.
        }
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }
}

void WP8App::Uninitialize()
{
}

void WP8App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    m_windowVisible = args->Visible;
}

void WP8App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    m_windowClosed = true;

    g_pGameCore->OnPrepareToDie();
}

void WP8App::RotateInputCoords(float x, float y, float* newx, float* newy)
{
    // convert input for top-left in given orientation:
    if( m_Orientation == WP8Orientation_Landscape90 )
    {
        *newx = y; // 0y to 800y becomes 0x to 480x
        *newy = m_renderer->GetWindowWidth()-x; // 480x to 0x becomes 0y to 800y
    }
    else if( m_Orientation == WP8Orientation_Landscape270 )
    {
        *newx = m_renderer->GetWindowHeight()-y;
        *newy = x;
    }
}

void WP8App::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    int bp = 1;
}

void WP8App::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    int bp = 1;
}

void WP8App::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
    if( g_pGameCore )
    {
        // TODO: id seems to increment with each press... will need to handle it somehow.
        int id = args->CurrentPoint->PointerId;
        id = 0;
        float x, y;
        x = args->CurrentPoint->Position.X; y = args->CurrentPoint->Position.Y;
        float ww = m_renderer->GetWindowWidth();
        float wh = m_renderer->GetWindowHeight();
        RotateInputCoords( args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y, &x, &y );

        float rtw = m_renderer->GetRenderTargetWidth();
        float rth = m_renderer->GetRenderTargetHeight();
        x = x/ww * rtw;
        y = y/wh * rth;

        //LOGInfo( LOGTag, "OnPointerPressed %f, %f\n", x, y );
        g_pGameCore->OnTouch( GCBA_Down, 0, GCBA_Down, 0, id, x, y, 0, 0 );
    }
}

void WP8App::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
    if( g_pGameCore )
    {
        int id = args->CurrentPoint->PointerId;
        id = 0;
        float x, y;
        x = args->CurrentPoint->Position.X; y = args->CurrentPoint->Position.Y;
        float ww = m_renderer->GetWindowWidth();
        float wh = m_renderer->GetWindowHeight();
        RotateInputCoords( args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y, &x, &y );

        float rtw = m_renderer->GetRenderTargetWidth();
        float rth = m_renderer->GetRenderTargetHeight();
        x = x/ww * rtw;
        y = y/wh * rth;

        g_pGameCore->OnTouch( GCBA_Held, 0, GCBA_Held, 0, id, x, y, 0, 0 );
    }
}

void WP8App::OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
{
    if( g_pGameCore )
    {
        int id = args->CurrentPoint->PointerId;
        id = 0;
        float x, y;
        x = args->CurrentPoint->Position.X; y = args->CurrentPoint->Position.Y;
        float ww = m_renderer->GetWindowWidth();
        float wh = m_renderer->GetWindowHeight();
        RotateInputCoords( args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y, &x, &y );

        float rtw = m_renderer->GetRenderTargetWidth();
        float rth = m_renderer->GetRenderTargetHeight();
        x = x/ww * rtw;
        y = y/wh * rth;

        g_pGameCore->OnTouch( GCBA_Up, 0, GCBA_Up, 0, id, x, y, 0, 0 );
    }
}

void WP8App::OnBackPressed(_In_ Platform::Object^ sender, _In_ BackPressedEventArgs^ args)
{
    if( m_ExitOnBackButton )
    {
    }
    else
    {
        g_pGameCore->OnButtons( GCBA_Down, GCBI_Back );
        args->Handled = true;
    }
}

void WP8App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    CoreWindow::GetForCurrentThread()->Activate();

    DataTransferManager^ dataTransferManager = DataTransferManager::GetForCurrentView();
    dataTransferManager->DataRequested += ref new TypedEventHandler<DataTransferManager^, DataRequestedEventArgs^>(
        this, &WP8App::DataRequestedEventHandler);
}

void WP8App::DataRequestedEventHandler(DataTransferManager^ sender, DataRequestedEventArgs^ e)
{
    wchar_t widetext[1000];

    //MultiByteToWideChar( CP_ACP, 0, g_TextToShare_Subject, strlen(g_TextToShare_Subject), widetext, 1000 );
    //widetext[strlen(g_TextToShare_Subject)] = 0;
    //e->Request->Data->Properties->Title = ref new Platform::String( widetext ); //"Share Text Title";

    //e->Request->Data->Properties->Description = ""; //"Share Text Description";

    MultiByteToWideChar( CP_ACP, 0, g_TextToShare_Body, strlen(g_TextToShare_Body), widetext, 1000 );
    widetext[strlen(g_TextToShare_Body)] = 0;
    e->Request->Data->SetText( ref new Platform::String( widetext ) ); //"Main text" );

    //e->Request->Data->SetUri( ref new Uri("http://www.flatheadgames.com") );
}

void WP8App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
    // Save app state asynchronously after requesting a deferral. Holding a deferral
    // indicates that the application is busy performing suspending operations. Be
    // aware that a deferral may not be held indefinitely. After about five seconds,
    // the app will be forced to exit.
    SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
    m_renderer->ReleaseResourcesForSuspending();

    create_task([this, deferral]()
    {
        // Insert your code here.
        g_pGameCore->OnFocusLost();

        deferral->Complete();
    });
}

void WP8App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
    // Restore any data or state that was unloaded on suspend. By default, data
    // and state are persisted when resuming from suspend. Note that this event
    // does not occur if the app was previously terminated.
    m_renderer->CreateWindowSizeDependentResources();

    g_pGameCore->OnFocusGained();
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new WP8App();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto direct3DApplicationSource = ref new Direct3DApplicationSource();
    CoreApplication::Run(direct3DApplicationSource);
    return 0;
}
