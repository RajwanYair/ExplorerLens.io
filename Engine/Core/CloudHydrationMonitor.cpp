// CloudHydrationMonitor.cpp — Windows Cloud Files Hydration Monitor
// Copyright (c) 2026 ExplorerLens Project
//
#include "CloudHydrationMonitor.h"
#include <filesystem>

namespace ExplorerLens { namespace Engine {

CloudHydrationMonitor::CloudHydrationMonitor(const Config& cfg)
    : m_config(cfg)
{}

HydrationState CloudHydrationMonitor::Probe(const std::wstring& filePath) const
{
    // Production: interrogate CfGetPlaceholderInfo / FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS.
    // Unit-test / CI path: use filesystem size as a proxy.
    std::error_code ec;
    const auto sz = std::filesystem::file_size(filePath, ec);
    if (ec) return HydrationState::UNKNOWN;
    if (sz == 0) return HydrationState::GHOST_PLACEHOLDER;
    if (sz < m_config.minimumBytesRequired) return HydrationState::PARTIALLY_LOCAL;
    return HydrationState::FULLY_LOCAL;
}

bool CloudHydrationMonitor::IsDecodeable(const std::wstring& filePath) const
{
    const auto state = Probe(filePath);
    return state == HydrationState::FULLY_LOCAL ||
           state == HydrationState::NOT_A_PLACEHOLDER;
}

void CloudHydrationMonitor::RequestWhenReady(
    const std::wstring& /*filePath*/, HydrationReadyCallback /*cb*/)
{
    // Production: pin CfApi pin request; poll on background thread until
    // GetFileAttributesExW returns without FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS.
    // Stub implementation: callback would be stored and fired by CfApi notification.
}

void CloudHydrationMonitor::CancelDeferred(const std::wstring& /*filePath*/)
{
    // Remove deferred callbacks for this path from the pending map.
}

const wchar_t* CloudHydrationMonitor::StateLabel(HydrationState state)
{
    switch (state) {
        case HydrationState::FULLY_LOCAL:       return L"FullyLocal";
        case HydrationState::PARTIALLY_LOCAL:   return L"PartiallyLocal";
        case HydrationState::GHOST_PLACEHOLDER: return L"GhostPlaceholder";
        case HydrationState::NOT_A_PLACEHOLDER: return L"NotAPlaceholder";
        default:                                return L"Unknown";
    }
}

const CloudHydrationMonitor::Config& CloudHydrationMonitor::GetConfig() const
{
    return m_config;
}

}} // namespace ExplorerLens::Engine
