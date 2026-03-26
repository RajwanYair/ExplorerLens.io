// App.xaml.cpp — WinUI 3 Application Entry Point
// Copyright (c) 2026 ExplorerLens Project
//
// Main application class for LENSManager.WinUI. Handles app lifecycle,
// window creation, navigation host setup, and crash recovery.
//
#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"

namespace winrt {
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
}

namespace winrt::LENSManager::implementation {

App::App() {
    InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e) {
        if (IsDebuggerPresent()) {
            auto errorMessage = e.Message();
            __debugbreak();
        }
    });
#endif
}

void App::OnLaunched(LaunchActivatedEventArgs const&) {
    window_ = make<MainWindow>();
    window_.Activate();
}

} // namespace winrt::LENSManager::implementation
