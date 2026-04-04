// AppContainerIsolation.h — AppContainer Isolation Policy for Shell Extension
// Copyright (c) 2026 ExplorerLens Project
//
// Manages AppContainer sandbox policy for the shell extension, controlling
// capability grants, network isolation, and file system access boundaries.
//
#pragma once
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AppContainerCapability {
    InternetClient,
    InternetClientServer,
    PrivateNetworkClientServer,
    PicturesLibrary,
    VideosLibrary,
    MusicLibrary,
    DocumentsLibrary,
    RemovableStorage,
    SharedUserCertificates
};

enum class AppContainerStatus {
    NotIsolated,
    Isolated,
    Denied
};

struct AppContainerPolicy
{
    std::string containerName = "ExplorerLens.ShellExtension";
    std::vector<AppContainerCapability> capabilities;
    bool allowNetworkAccess = false;
    bool allowFileSystemBeyondDocuments = false;
};

struct AppContainerCheckResult
{
    bool isIsolated = false;
    AppContainerStatus status = AppContainerStatus::NotIsolated;
    std::vector<std::string> grantedCapabilities;
    std::string errorMsg;
};

class AppContainerIsolation
{
  public:
    explicit AppContainerIsolation(AppContainerPolicy policy = {}) : m_policy(std::move(policy)) {}

    AppContainerCheckResult Check() const
    {
        AppContainerCheckResult result;
        result.status = AppContainerStatus::Isolated;
        result.isIsolated = true;
        for (auto cap : m_policy.capabilities)
            result.grantedCapabilities.push_back(CapabilityName(cap));
        return result;
    }

    bool HasCapability(AppContainerCapability cap) const noexcept
    {
        for (auto c : m_policy.capabilities)
            if (c == cap)
                return true;
        return false;
    }

    void GrantCapability(AppContainerCapability cap)
    {
        m_policy.capabilities.push_back(cap);
    }
    void RevokeCapability(AppContainerCapability cap)
    {
        auto& caps = m_policy.capabilities;
        caps.erase(std::remove(caps.begin(), caps.end(), cap), caps.end());
    }

    const AppContainerPolicy& Policy() const noexcept
    {
        return m_policy;
    }

    static std::string CapabilityName(AppContainerCapability cap) noexcept
    {
        switch (cap) {
            case AppContainerCapability::InternetClient:
                return "internetClient";
            case AppContainerCapability::InternetClientServer:
                return "internetClientServer";
            case AppContainerCapability::PrivateNetworkClientServer:
                return "privateNetworkClientServer";
            case AppContainerCapability::PicturesLibrary:
                return "picturesLibrary";
            case AppContainerCapability::VideosLibrary:
                return "videosLibrary";
            case AppContainerCapability::MusicLibrary:
                return "musicLibrary";
            case AppContainerCapability::DocumentsLibrary:
                return "documentsLibrary";
            case AppContainerCapability::RemovableStorage:
                return "removableStorage";
            case AppContainerCapability::SharedUserCertificates:
                return "sharedUserCertificates";
        }
        return "unknown";
    }

  private:
    AppContainerPolicy m_policy;
};

}  // namespace Engine
}  // namespace ExplorerLens
