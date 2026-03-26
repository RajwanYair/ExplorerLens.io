// NavigationViewModel.h — NavigationView ViewModel for Manager.WinUI
// Copyright (c) 2026 ExplorerLens Project
//
// Implements the MVVM ViewModel backing the NavigationView in the WinUI 3
// Manager app. Manages page registration, breadcrumb trail, back-stack,
// and deep-link navigation from command-line or toast activation.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>

namespace ExplorerLens { namespace Engine { namespace WinUI {

// Navigation item descriptor
struct NavItem {
    std::wstring key;          // Unique page identifier e.g. L"Settings"
    std::wstring label;        // Display label (localized)
    std::wstring iconGlyph;    // Segoe MDL2 glyph e.g. L"\uE713"
    bool         isHeader    = false; // Section header (not navigable)
    bool         isSeparator = false;
    bool         isFooter    = false; // Placed in footer pane
};

// Navigation entry in the back-stack
struct NavHistoryEntry {
    std::wstring pageKey;
    std::wstring parameter;     // Serialized JSON or empty
};

// Callback type: void(pageKey, parameter)
using NavCallback = std::function<void(const std::wstring&, const std::wstring&)>;

class NavigationViewModel {
public:
    NavigationViewModel() { RegisterDefaultItems(); }

    // Register a page for navigation
    void RegisterPage(const NavItem& item) {
        m_items.push_back(item);
    }

    // Navigate to a page; pushes to back-stack
    bool NavigateTo(const std::wstring& key, const std::wstring& param = L"") {
        if (!FindItem(key)) return false;
        m_backStack.push_back({ m_currentPage, m_currentParam });
        m_currentPage  = key;
        m_currentParam = param;
        if (m_navCallback) m_navCallback(key, param);
        return true;
    }

    // Navigate back — returns false if back-stack is empty
    bool GoBack() {
        if (m_backStack.empty()) return false;
        auto prev = m_backStack.back();
        m_backStack.pop_back();
        m_currentPage  = prev.pageKey;
        m_currentParam = prev.parameter;
        if (m_navCallback) m_navCallback(m_currentPage, m_currentParam);
        return true;
    }

    bool CanGoBack() const { return !m_backStack.empty(); }

    const std::wstring& CurrentPage()  const { return m_currentPage; }
    const std::wstring& CurrentParam() const { return m_currentParam; }

    const std::vector<NavItem>& Items() const { return m_items; }

    void SetNavCallback(NavCallback cb) { m_navCallback = std::move(cb); }

    // Breadcrumb trail: returns [root, ..., current]
    std::vector<std::wstring> BreadcrumbTrail() const {
        std::vector<std::wstring> trail;
        for (auto& e : m_backStack) trail.push_back(e.pageKey);
        trail.push_back(m_currentPage);
        return trail;
    }

    // Deep-link: parse "lens://navigate?page=Settings&param=cache" style URIs
    bool HandleDeepLink(const std::wstring& uri) {
        auto pagePos = uri.find(L"page=");
        if (pagePos == std::wstring::npos) return false;
        std::wstring remainder = uri.substr(pagePos + 5);
        std::wstring page, param;
        auto amp = remainder.find(L'&');
        if (amp != std::wstring::npos) {
            page = remainder.substr(0, amp);
            auto pPos = remainder.find(L"param=", amp);
            if (pPos != std::wstring::npos) param = remainder.substr(pPos + 6);
        } else {
            page = remainder;
        }
        return NavigateTo(page, param);
    }

private:
    std::vector<NavItem>        m_items;
    std::vector<NavHistoryEntry>m_backStack;
    std::wstring                m_currentPage  = L"Dashboard";
    std::wstring                m_currentParam;
    NavCallback                 m_navCallback;

    std::optional<NavItem> FindItem(const std::wstring& key) const {
        for (auto& i : m_items) if (i.key == key) return i;
        return std::nullopt;
    }

    void RegisterDefaultItems() {
        m_items = {
            { L"Dashboard",   L"Dashboard",      L"\uE80F" },
            { L"Formats",     L"Formats",         L"\uE8A5" },
            { L"Performance", L"Performance",     L"\uE9D9" },
            { L"Cache",       L"Cache",           L"\uE838" },
            { L"Plugins",     L"Plugins",         L"\uECAA" },
            { L"Security",    L"Security",        L"\uE72E" },
            { L"",            L"",                L"", false, true }, // separator
            { L"Settings",    L"Settings",        L"\uE713", false, false, true },
            { L"About",       L"About",           L"\uE946", false, false, true },
        };
        m_currentPage = L"Dashboard";
    }
};

}}} // namespace ExplorerLens::Engine::WinUI
