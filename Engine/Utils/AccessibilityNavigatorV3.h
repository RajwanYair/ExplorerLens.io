// AccessibilityNavigatorV3.h — Accessibility Navigator v3 (UIA + AT-SPI)
// Copyright (c) 2026 ExplorerLens Project
//
// Exposes the thumbnail grid and annotation panels as an accessible UI Automation
// provider tree (Windows) and AT-SPI2 object tree (Linux/WSL), enabling screen reader
// and navigation assistant access to all ExplorerLens controls.
//
#pragma once
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class ANV3ControlType {
    Button, CheckBox, ComboBox, DataGrid, DataItem, Edit,
    Group, List, ListItem, Menu, MenuItem, Pane, Text, TreeItem, Window
};

enum class AccessibilityPlatform { UIAutomation, ATSPI2, NSAccessibility };

struct ANV3Element {
    std::string      automationId;
    std::string      name;
    std::string      helpText;
    ANV3ControlType   controlType = ANV3ControlType::DataItem;
    bool             isEnabled   = true;
    bool             isFocusable = true;
    bool             isKeyboardFocusable = true;
    std::string      accessKey;
    std::string      shortcutKey;
};

struct AccessibilityHit {
    bool          found = false;
    ANV3Element   element;
    std::string   path; // e.g. "ThumbnailGrid > Item[3]"
};

using AccessibilityInvokeCallback = std::function<void(const ANV3Element&)>;

class AccessibilityNavigatorV3 {
public:
    explicit AccessibilityNavigatorV3(AccessibilityPlatform platform = AccessibilityPlatform::UIAutomation)
        : m_platform(platform) {}

    void RegisterElement(ANV3Element element) {
        m_elements[element.automationId] = std::move(element);
    }

    AccessibilityHit FindById(const std::string& automationId) const {
        auto it = m_elements.find(automationId);
        if (it == m_elements.end()) return { false, {}, {} };
        return { true, it->second, automationId };
    }

    void SetInvokeCallback(AccessibilityInvokeCallback cb) { m_invokeCallback = std::move(cb); }

    bool Invoke(const std::string& automationId) {
        auto hit = FindById(automationId);
        if (!hit.found || !hit.element.isEnabled) return false;
        if (m_invokeCallback) m_invokeCallback(hit.element);
        return true;
    }

    size_t ElementCount() const noexcept { return m_elements.size(); }
    AccessibilityPlatform Platform() const noexcept { return m_platform; }

    static std::string ControlTypeName(ANV3ControlType t) noexcept {
        switch (t) {
        case ANV3ControlType::Button:   return "Button";
        case ANV3ControlType::List:     return "List";
        case ANV3ControlType::ListItem: return "ListItem";
        case ANV3ControlType::DataGrid: return "DataGrid";
        case ANV3ControlType::DataItem: return "DataItem";
        case ANV3ControlType::Text:     return "Text";
        case ANV3ControlType::Edit:     return "Edit";
        default:                        return "Control";
        }
    }

private:
    AccessibilityPlatform m_platform;
    std::unordered_map<std::string, ANV3Element> m_elements;
    AccessibilityInvokeCallback m_invokeCallback;
};

} // namespace Engine
} // namespace ExplorerLens
