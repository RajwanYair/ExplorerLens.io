#pragma once
//==============================================================================
// CIValidator — Sprint 210
// CI/CD pipeline validation, build matrix, artifact verification
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace DarkThumbs { namespace Engine {

enum class CIPlatform : uint8_t {
    Windows_x64 = 0,
    Windows_ARM64,
    Linux_x64,
    macOS_ARM64,
    PlatformCount
};

enum class CIStage : uint8_t {
    Checkout = 0,
    Configure,
    Build,
    Test,
    Package,
    Sign,
    Publish,
    StageCount
};

enum class ArtifactType : uint8_t {
    DLL = 0,
    EXE,
    LIB,
    MSI,
    MSIX,
    PDB,
    NuGet,
    ZIP
};

struct CIBuildResult {
    CIPlatform platform = CIPlatform::Windows_x64;
    bool compiled = false;
    uint32_t warnings = 0;
    uint32_t errors = 0;
    double buildTimeSec = 0.0;
    std::wstring configuration;
    std::vector<std::wstring> artifacts;
};

struct CIPipelineResult {
    bool passed = false;
    uint32_t stagesRun = 0;
    uint32_t stagesPassed = 0;
    double totalTimeSec = 0.0;
    std::vector<CIBuildResult> builds;
    std::vector<std::wstring> failures;
};

//------------------------------------------------------------------------------
class CIValidator {
public:
    CIValidator();
    ~CIValidator() = default;

    // Build matrix
    void AddPlatform(CIPlatform platform);
    std::vector<CIPlatform> GetPlatforms() const { return m_platforms; }
    uint32_t GetPlatformCount() const { return static_cast<uint32_t>(m_platforms.size()); }

    // Validation
    CIPipelineResult ValidatePipeline() const;
    bool ValidateArtifact(const std::wstring& path, ArtifactType type) const;
    bool ValidateBuildOutput(const CIBuildResult& build) const;

    // Stage execution
    bool RunStage(CIStage stage);
    CIStage GetCurrentStage() const { return m_currentStage; }

    // Static helpers
    static const wchar_t* GetPlatformName(CIPlatform platform);
    static const wchar_t* GetStageName(CIStage stage);
    static const wchar_t* GetArtifactTypeName(ArtifactType type);
    static uint32_t GetStageCount();

private:
    std::vector<CIPlatform> m_platforms;
    CIStage m_currentStage = CIStage::Checkout;
    std::map<CIStage, bool> m_stageResults;
};

}} // namespace DarkThumbs::Engine
