// MainWindow.xaml.cpp — WinUI 3 Main Window Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Handles page navigation, window title updates, registration status
// detection, and version chip initialization.
//
#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "Pages/DashboardPage.xaml.h"
#include "Pages/FormatsPage.xaml.h"
#include "Pages/GpuPage.xaml.h"
#include "Pages/CachePage.xaml.h"
#include "Pages/PluginsPage.xaml.h"
#include "Pages/DiagnosticsPage.xaml.h"
#include "Pages/SettingsPage.xaml.h"
#include "Pages/AboutPage.xaml.h"

namespace winrt {
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
}

namespace winrt::LENSManager::implementation {

// Fixed CLSID — never change this
static constexpr wchar_t LENS_CLSID[] =
    L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

MainWindow::MainWindow() {
    InitializeComponent();

    // Set window title
    Title(L"ExplorerLens Manager");

    // Read version from registry or resource
    VersionChip().Text(L"v15.7.0");

    // Check registration status
    HKEY hKey = nullptr;
    std::wstring regPath = L"SOFTWARE\\Classes\\CLSID\\";
    regPath += LENS_CLSID;
    LSTATUS status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath.c_str(),
                                   0, KEY_READ, &hKey);
    bool registered = (status == ERROR_SUCCESS);
    if (hKey) RegCloseKey(hKey);
    UpdateStatusIndicator(registered);
}

void MainWindow::NavView_Loaded(IInspectable const&, RoutedEventArgs const&) {
    // Navigate to Dashboard on startup
    NavigateTo(L"DashboardPage");
    NavView().SelectedItem(NavView().MenuItems().GetAt(0));
}

void MainWindow::NavView_SelectionChanged(
    NavigationView const&,
    NavigationViewSelectionChangedEventArgs const& args)
{
    if (args.IsSettingsSelected()) {
        NavigateTo(L"SettingsPage");
        return;
    }
    auto item = args.SelectedItem().try_as<NavigationViewItem>();
    if (item) {
        auto tag = unbox_value_or<hstring>(item.Tag(), L"");
        NavigateTo(tag);
    }
}

void MainWindow::NavigateTo(winrt::hstring const& tag) {
    if (tag == m_currentPage) return;
    m_currentPage = tag;

    struct PageMap {
        const wchar_t* tag;
        const wchar_t* title;
        Windows::UI::Xaml::Interop::TypeName type;
    };

    // clang-format off
    if      (tag == L"DashboardPage")   { PageTitle().Text(L"Dashboard");   ContentFrame().Navigate(xaml_typename<Pages::DashboardPage>());   }
    else if (tag == L"FormatsPage")     { PageTitle().Text(L"File Formats"); ContentFrame().Navigate(xaml_typename<Pages::FormatsPage>());     }
    else if (tag == L"GpuPage")         { PageTitle().Text(L"GPU Pipeline"); ContentFrame().Navigate(xaml_typename<Pages::GpuPage>());         }
    else if (tag == L"CachePage")       { PageTitle().Text(L"Cache");        ContentFrame().Navigate(xaml_typename<Pages::CachePage>());       }
    else if (tag == L"PluginsPage")     { PageTitle().Text(L"Plugins");      ContentFrame().Navigate(xaml_typename<Pages::PluginsPage>());     }
    else if (tag == L"DiagnosticsPage") { PageTitle().Text(L"Diagnostics");  ContentFrame().Navigate(xaml_typename<Pages::DiagnosticsPage>()); }
    else if (tag == L"SettingsPage")    { PageTitle().Text(L"Settings");     ContentFrame().Navigate(xaml_typename<Pages::SettingsPage>());    }
    else if (tag == L"AboutPage")       { PageTitle().Text(L"About");        ContentFrame().Navigate(xaml_typename<Pages::AboutPage>());       }
    // clang-format on
}

void MainWindow::UpdateStatusIndicator(bool registered) {
    // Green dot = registered, Red dot = not registered
    Windows::UI::Color color =
        registered
            ? Windows::UI::ColorHelper::FromArgb(0xFF, 0x3F, 0xB9, 0x50) // Green
            : Windows::UI::ColorHelper::FromArgb(0xFF, 0xF8, 0x51, 0x49); // Red
    StatusDot().Fill(Media::SolidColorBrush(color));
}

} // namespace winrt::LENSManager::implementation
