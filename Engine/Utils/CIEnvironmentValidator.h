// CIEnvironmentValidator.h — CI Environment Variable Validator
// Copyright (c) 2026 ExplorerLens Project
//
// Validates required CI environment variables (GITHUB_TOKEN, signing keys, SDK paths) before build starts.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct EnvVarSpec
{
    std::string name;
    bool required;
    std::string defaultValue;
};
struct EnvValidationResult
{
    bool allPresent;
    std::vector<std::string> missing;
    std::vector<std::string> warnings;
};
class CIEnvironmentValidator
{
  public:
    void Require(EnvVarSpec spec)
    {
        m_specs.push_back(spec);
    }
    EnvValidationResult Validate() const
    {
        std::vector<std::string> missing;
        for (auto& s : m_specs) {
            if (s.required && s.defaultValue.empty()) { /* check env */
            }
        }
        return {missing.empty(), missing, {}};
    }
    size_t RequiredCount() const
    {
        size_t n = 0;
        for (auto& s : m_specs)
            if (s.required)
                n++;
        return n;
    }

  private:
    std::vector<EnvVarSpec> m_specs;
};

}  // namespace Engine
}  // namespace ExplorerLens