// ShaderPermutationManager.h — Shader Variant Management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages shader permutations for different formats, GPU vendors, and
// quality levels to select optimal shader variants at runtime.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ShaderPermFeature : uint32_t {
    None = 0,
    HalfFloat = 1 << 0,
    PackedMath = 1 << 1,
    WaveOps = 1 << 2,
    SubgroupOps = 1 << 3,
    Int64 = 1 << 4,
    DoubleFloat = 1 << 5,
    RayQuery = 1 << 6
};

inline ShaderPermFeature operator|(ShaderPermFeature a, ShaderPermFeature b)
{
    return static_cast<ShaderPermFeature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline ShaderPermFeature operator&(ShaderPermFeature a, ShaderPermFeature b)
{
    return static_cast<ShaderPermFeature>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

struct ShaderPermutation
{
    uint32_t permutationId = 0;
    std::string shaderName;
    ShaderPermFeature requiredFeatures = ShaderPermFeature::None;
    std::string entryPoint;
    bool isCompiled = false;
    uint32_t binarySize = 0;
};

struct PermutationKey
{
    std::string shaderBase;
    ShaderPermFeature features = ShaderPermFeature::None;
    bool operator==(const PermutationKey& other) const
    {
        return shaderBase == other.shaderBase && features == other.features;
    }
};

class ShaderPermutationManager
{
  public:
    ShaderPermutationManager() = default;

    uint32_t RegisterPermutation(const ShaderPermutation& perm)
    {
        uint32_t id = m_nextId++;
        ShaderPermutation registered = perm;
        registered.permutationId = id;
        m_permutations.push_back(registered);
        return id;
    }

    const ShaderPermutation* FindBestPermutation(const std::string& shaderName,
                                                 ShaderPermFeature availableFeatures) const
    {
        const ShaderPermutation* best = nullptr;
        uint32_t bestScore = 0;
        for (const auto& perm : m_permutations) {
            if (perm.shaderName != shaderName)
                continue;
            if ((perm.requiredFeatures & availableFeatures) != perm.requiredFeatures)
                continue;
            uint32_t score = CountBits(static_cast<uint32_t>(perm.requiredFeatures));
            if (score >= bestScore) {
                bestScore = score;
                best = &perm;
            }
        }
        return best;
    }

    size_t GetPermutationCount() const
    {
        return m_permutations.size();
    }
    size_t GetCompiledCount() const
    {
        size_t count = 0;
        for (const auto& p : m_permutations)
            if (p.isCompiled)
                count++;
        return count;
    }

    void MarkCompiled(uint32_t permutationId, uint32_t binarySize)
    {
        for (auto& p : m_permutations) {
            if (p.permutationId == permutationId) {
                p.isCompiled = true;
                p.binarySize = binarySize;
                break;
            }
        }
    }

  private:
    static uint32_t CountBits(uint32_t v)
    {
        uint32_t c = 0;
        while (v) {
            c += v & 1;
            v >>= 1;
        }
        return c;
    }

    std::vector<ShaderPermutation> m_permutations;
    uint32_t m_nextId = 1;
};

}  // namespace Engine
}  // namespace ExplorerLens
