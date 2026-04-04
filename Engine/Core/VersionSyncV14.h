//==============================================================================
// ExplorerLens Engine — Version Sync & v14.0 Architecture Overview
// Version bootstrap and architectural baseline declaration for v14.0 block.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// v14.0 version components
struct V14Version
{
    uint32_t major = 14;
    uint32_t minor = 0;
    uint32_t patch = 0;
    std::wstring label = L"v14.0.0";
    std::wstring codename = L"Apex";
};

/// Architecture domain identifiers for v14.0
enum class V14Domain : uint8_t {
    GPUPipelineV3 = 0,
    FormatIntelligence,
    DeveloperExperience,
    SecurityExcellence,
    UXExcellence,
    AIMLExpansion,
    EnterpriseCloudV2,
    PlatformHardening,
    PerformanceSummit,
    ReleaseGate,
    COUNT
};

/// v14.0 feature flag status
enum class FeatureStatus : uint8_t {
    Planned,
    InProgress,
    Implemented,
    Deprecated,
    COUNT
};

/// Domain descriptor
struct V14DomainDescriptor
{
    V14Domain domain;
    FeatureStatus status = FeatureStatus::Planned;
    uint32_t milestoneStart = 0;
    uint32_t milestoneEnd = 0;
    std::wstring title;
};

/// Version sync engine for v14.0
class VersionSyncV14
{
  public:
    static const wchar_t* DomainName(V14Domain d)
    {
        switch (d) {
            case V14Domain::GPUPipelineV3:
                return L"GPU Pipeline V3";
            case V14Domain::FormatIntelligence:
                return L"Format Intelligence";
            case V14Domain::DeveloperExperience:
                return L"Developer Experience";
            case V14Domain::SecurityExcellence:
                return L"Security Excellence";
            case V14Domain::UXExcellence:
                return L"UX Excellence";
            case V14Domain::AIMLExpansion:
                return L"AI/ML Expansion";
            case V14Domain::EnterpriseCloudV2:
                return L"Enterprise & Cloud V2";
            case V14Domain::PlatformHardening:
                return L"Platform Hardening";
            case V14Domain::PerformanceSummit:
                return L"Performance Summit";
            case V14Domain::ReleaseGate:
                return L"Release Gate";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* FeatureStatusName(FeatureStatus s)
    {
        switch (s) {
            case FeatureStatus::Planned:
                return L"Planned";
            case FeatureStatus::InProgress:
                return L"In Progress";
            case FeatureStatus::Implemented:
                return L"Implemented";
            case FeatureStatus::Deprecated:
                return L"Deprecated";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t DomainCount()
    {
        return static_cast<size_t>(V14Domain::COUNT);
    }
    static constexpr size_t FeatureStatusCount()
    {
        return static_cast<size_t>(FeatureStatus::COUNT);
    }

    static V14Version GetVersion()
    {
        return V14Version{};
    }

    static std::vector<V14DomainDescriptor> GetDomainPlan()
    {
        return {
            {V14Domain::GPUPipelineV3, FeatureStatus::Planned, 299, 304, L"GPU Pipeline V3"},
            {V14Domain::FormatIntelligence, FeatureStatus::Planned, 305, 309, L"Format Intelligence"},
            {V14Domain::DeveloperExperience, FeatureStatus::Planned, 310, 314, L"Developer Experience"},
            {V14Domain::SecurityExcellence, FeatureStatus::Planned, 315, 319, L"Security Excellence"},
            {V14Domain::UXExcellence, FeatureStatus::Planned, 320, 324, L"UX Excellence"},
            {V14Domain::AIMLExpansion, FeatureStatus::Planned, 325, 329, L"AI/ML Expansion"},
            {V14Domain::EnterpriseCloudV2, FeatureStatus::Planned, 330, 334, L"Enterprise & Cloud V2"},
            {V14Domain::PlatformHardening, FeatureStatus::Planned, 335, 339, L"Platform Hardening"},
            {V14Domain::PerformanceSummit, FeatureStatus::Planned, 340, 344, L"Performance Summit"},
            {V14Domain::ReleaseGate, FeatureStatus::Planned, 345, 348, L"v14.0 Release"},
        };
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
