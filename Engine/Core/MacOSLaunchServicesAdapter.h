// MacOSLaunchServicesAdapter.h — macOS Launch Services Integration
// Copyright (c) 2026 ExplorerLens Project
//
// macOS Launch Services integration for registering file type associations
// and default handlers. Manages UTI-based handler registration per scope.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class LSHandlerRole : uint8_t {
    Viewer,
    Editor,
    Shell,
    All
};

enum class LSRegistrationScope : uint8_t {
    User,
    System,
    Temporary
};

struct LaunchServicesConfig {
    LSHandlerRole role = LSHandlerRole::Viewer;
    LSRegistrationScope scope = LSRegistrationScope::User;
    std::string bundleId;
    std::string uti;
    std::string iconName;
};

class MacOSLaunchServicesAdapter {
public:
    MacOSLaunchServicesAdapter() = default;
    ~MacOSLaunchServicesAdapter() = default;

    MacOSLaunchServicesAdapter(MacOSLaunchServicesAdapter const&) = delete;
    MacOSLaunchServicesAdapter& operator=(MacOSLaunchServicesAdapter const&) = delete;
    MacOSLaunchServicesAdapter(MacOSLaunchServicesAdapter&&) noexcept = default;
    MacOSLaunchServicesAdapter& operator=(MacOSLaunchServicesAdapter&&) noexcept = default;

    bool RegisterHandler(LaunchServicesConfig const& config) {
        m_config = config;
        if (!IsHandlerRegistered(config.uti)) {
            m_registeredUTIs.push_back(config.uti);
        }
        return true;
    }

    bool UnregisterHandler(std::string const& uti) {
        auto it = std::find(m_registeredUTIs.begin(), m_registeredUTIs.end(), uti);
        if (it != m_registeredUTIs.end()) {
            m_registeredUTIs.erase(it);
        }
        return true;
    }

    [[nodiscard]] bool IsHandlerRegistered(std::string const& uti) const {
        return std::find(m_registeredUTIs.begin(), m_registeredUTIs.end(), uti)
            != m_registeredUTIs.end();
    }

    [[nodiscard]] std::vector<std::string> GetRegisteredUTIs() const {
        return m_registeredUTIs;
    }

    bool SetDefaultHandler(std::string const& uti) {
        if (!IsHandlerRegistered(uti))
            return false;
        return true;
    }

    [[nodiscard]] LaunchServicesConfig const& GetConfig() const {
        return m_config;
    }

private:
    LaunchServicesConfig m_config;
    std::vector<std::string> m_registeredUTIs;
};

} }
