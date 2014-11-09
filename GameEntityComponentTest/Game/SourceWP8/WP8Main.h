#pragma once

//#include "pch.h"
#include "CubeRenderer.h"

enum WP8Orientations
{
    WP8Orientation_Portrait,
    WP8Orientation_Landscape90,
    WP8Orientation_Landscape270,
};

ref class WP8App;

extern WP8App^ g_pWP8App;

ref class WP8App sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
private:
    CubeRenderer^ m_renderer;
    bool m_windowClosed;
    bool m_windowVisible;
    WP8Orientations m_Orientation;
    bool m_OrientationLocked;

    bool m_ExitOnBackButton;

public:
    WP8App();

    // IFrameworkView Methods.
    virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
    virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
    virtual void Load(Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

    int GetOrientation() { return m_Orientation; }

    void SetExitOnBackButton(bool exit) { m_ExitOnBackButton = exit; }

protected:
    void RotateInputCoords(float x, float y, float* newx, float* newy);

    // Event Handlers.
    void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
    void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
    void OnResuming(Platform::Object^ sender, Platform::Object^ args);
    void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
    void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
    void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
    void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
    void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

    void OnBackPressed(_In_ Platform::Object^ sender, _In_ Windows::Phone::UI::Input::BackPressedEventArgs^ args);

    void DataRequestedEventHandler(Windows::ApplicationModel::DataTransfer::DataTransferManager^ sender, Windows::ApplicationModel::DataTransfer::DataRequestedEventArgs^ e);
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
