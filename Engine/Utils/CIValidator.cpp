//==============================================================================
// CIValidator
//==============================================================================

#include "CIValidator.h"
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

CIValidator::CIValidator() { m_platforms.push_back(CIPlatform::Windows_x64); }

void CIValidator::AddPlatform(CIPlatform platform) {
  if (std::find(m_platforms.begin(), m_platforms.end(), platform) ==
      m_platforms.end()) {
    m_platforms.push_back(platform);
  }
}

CIPipelineResult CIValidator::ValidatePipeline() const {
  CIPipelineResult result;
  result.stagesRun = static_cast<uint32_t>(m_stageResults.size());
  result.stagesPassed = 0;
  for (const auto &[stage, passed] : m_stageResults) {
    if (passed)
      result.stagesPassed++;
  }
  result.passed =
      (result.stagesPassed == result.stagesRun) && result.stagesRun > 0;
  return result;
}

CIPipelineResult CIValidator::ValidatePipeline(CIPlatform platform) const {
  CIPipelineResult result = ValidatePipeline();
  result.platform = platform;
  return result;
}

bool CIValidator::ValidateArtifact(const std::wstring &path,
                                   ArtifactType /*type*/) const {
  return !path.empty();
}

bool CIValidator::ValidateBuildOutput(const CIBuildResult &build) const {
  return build.compiled && build.errors == 0 && build.warnings == 0;
}

bool CIValidator::RunStage(CIStage stage) {
  m_currentStage = stage;
  m_stageResults[stage] = true;
  return true;
}

const wchar_t *CIValidator::GetPlatformName(CIPlatform platform) {
  switch (platform) {
  case CIPlatform::Windows_x64:
    return L"Windows x64";
  case CIPlatform::Windows_ARM64:
    return L"Windows ARM64";
  case CIPlatform::Linux_x64:
    return L"Linux x64";
  case CIPlatform::macOS_ARM64:
    return L"macOS ARM64";
  case CIPlatform::GitHubActions:
    return L"GitHub Actions";
  case CIPlatform::AzureDevOps:
    return L"Azure DevOps";
  case CIPlatform::Jenkins:
    return L"Jenkins";
  case CIPlatform::TeamCity:
    return L"TeamCity";
  case CIPlatform::CircleCI:
    return L"CircleCI";
  default:
    return L"Unknown";
  }
}

const wchar_t *CIValidator::GetStageName(CIStage stage) {
  switch (stage) {
  case CIStage::Checkout:
    return L"Checkout";
  case CIStage::Configure:
    return L"Configure";
  case CIStage::Build:
    return L"Build";
  case CIStage::Test:
    return L"Test";
  case CIStage::Package:
    return L"Package";
  case CIStage::Sign:
    return L"Sign";
  case CIStage::Publish:
    return L"Publish";
  default:
    return L"Unknown";
  }
}

const wchar_t *CIValidator::GetArtifactTypeName(ArtifactType type) {
  switch (type) {
  case ArtifactType::DLL:
    return L"DLL";
  case ArtifactType::EXE:
    return L"EXE";
  case ArtifactType::LIB:
    return L"LIB";
  case ArtifactType::MSI:
    return L"MSI";
  case ArtifactType::MSIX:
    return L"MSIX";
  case ArtifactType::PDB:
    return L"PDB";
  case ArtifactType::NuGet:
    return L"NuGet";
  case ArtifactType::ZIP:
    return L"ZIP";
  default:
    return L"Unknown";
  }
}

uint32_t CIValidator::GetStageCount() {
  return static_cast<uint32_t>(CIStage::StageCount);
}

} // namespace Engine
} // namespace ExplorerLens
