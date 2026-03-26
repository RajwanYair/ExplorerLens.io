// MainWindow.xaml.h — WinUI 3 Main Window Header
// Copyright (c) 2026 ExplorerLens Project
//
// Main application window hosting the NavigationView and page Frame.
// Manages navigation, window title, theme switching, and status indicator.
//
#pragma once
#include "MainWindow.g.h"

namespace winrt::LENSManager::implementation {

struct MainWindow : MainWindowT<MainWindow> {
    MainWindow();

    void NavView_Loaded(Windows::Foundation::IInspectable const& sender,
                        Microsoft::UI::Xaml::RoutedEventArgs const& args);
    void NavView_SelectionChanged(
        Microsoft::UI::Xaml::Controls::NavigationView const& sender,
        Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);

private:
    void NavigateTo(winrt::hstring const& tag);
    void UpdateStatusIndicator(bool registered);
    winrt::hstring m_currentPage{};
};

} // namespace winrt::LENSManager::implementation

namespace winrt::LENSManager::factory_implementation {
struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow> {};
} // namespace winrt::LENSManager::factory_implementation
