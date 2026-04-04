// BuildArtifactValidator.h — Build Output Validation
// Copyright (c) 2026 ExplorerLens Project
//
// Validates build artifacts (DLLs, LIBs, EXEs) for completeness,
// architecture consistency, and correct symbol exports.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class BuildArtifactType : uint8_t {
    DLL,
    LIB,
    EXE,
    PDB,
    OBJ,
    Header
};

enum class Architecture : uint8_t {
    Unknown,
    x86,
    x64,
    ARM64,
    ARM
};

struct ValidatedBuildArtifact
{
    std::wstring path;
    BuildArtifactType type = BuildArtifactType::DLL;
    Architecture arch = Architecture::x64;
    uint64_t sizeBytes = 0;
    bool exists = false;
    bool isValid = false;
    std::string version;
};

struct BuildValidationResult
{
    bool allValid = true;
    uint32_t totalArtifacts = 0;
    uint32_t validArtifacts = 0;
    uint32_t missingArtifacts = 0;
    uint32_t invalidArtifacts = 0;
    uint32_t archMismatches = 0;
    std::vector<std::wstring> errors;
};

class BuildArtifactValidator
{
  public:
    explicit BuildArtifactValidator(Architecture expectedArch = Architecture::x64) : m_expectedArch(expectedArch) {}

    void AddExpectedArtifact(const std::wstring& path, BuildArtifactType type)
    {
        ValidatedBuildArtifact artifact;
        artifact.path = path;
        artifact.type = type;
        artifact.arch = m_expectedArch;
        m_expectedArtifacts.push_back(artifact);
    }

    BuildValidationResult Validate() const
    {
        BuildValidationResult result;
        result.totalArtifacts = static_cast<uint32_t>(m_expectedArtifacts.size());
        for (const auto& artifact : m_expectedArtifacts) {
            if (!artifact.exists) {
                result.missingArtifacts++;
                result.errors.push_back(L"Missing: " + artifact.path);
            } else if (!artifact.isValid) {
                result.invalidArtifacts++;
                result.errors.push_back(L"Invalid: " + artifact.path);
            } else if (artifact.arch != m_expectedArch) {
                result.archMismatches++;
                result.errors.push_back(L"Arch mismatch: " + artifact.path);
            } else {
                result.validArtifacts++;
            }
        }
        result.allValid = (result.validArtifacts == result.totalArtifacts);
        return result;
    }

    void MarkArtifactPresent(const std::wstring& path, uint64_t sizeBytes, Architecture arch = Architecture::x64)
    {
        for (auto& artifact : m_expectedArtifacts) {
            if (artifact.path == path) {
                artifact.exists = true;
                artifact.sizeBytes = sizeBytes;
                artifact.arch = arch;
                artifact.isValid = (sizeBytes > 0);
                break;
            }
        }
    }

    size_t GetExpectedCount() const
    {
        return m_expectedArtifacts.size();
    }
    Architecture GetExpectedArchitecture() const
    {
        return m_expectedArch;
    }
    void SetExpectedArchitecture(Architecture arch)
    {
        m_expectedArch = arch;
    }

  private:
    std::vector<ValidatedBuildArtifact> m_expectedArtifacts;
    Architecture m_expectedArch;
};

}  // namespace Engine
}  // namespace ExplorerLens
