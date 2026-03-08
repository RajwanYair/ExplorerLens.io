// ContextMenuFormatter.h — Shell context menu text formatting and layout
// Copyright (c) 2026 ExplorerLens Project
//
// Formats context menu entries for shell integration — handles text
// truncation, icon placement, accelerator keys, and localized strings.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct ContextMenuFormatterConfig {
    bool enabled = true;
    uint32_t maxLabelLength = 64;
    std::string label = "ContextMenuFormatter";
};

class ContextMenuFormatter {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ContextMenuFormatterConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    std::string FormatLabel(const std::string& text) const {
        if (text.length() <= m_config.maxLabelLength) return text;
        return text.substr(0, m_config.maxLabelLength - 3) + "...";
    }

    std::string FormatWithAccelerator(const std::string& text, char key) const {
        return text + "\t" + std::string("Ctrl+") + key;
    }

private:
    bool m_initialized = false;
    ContextMenuFormatterConfig m_config;
};

}
} // namespace ExplorerLens::Engine
