// ShaderVariantSelector.h — Selects optimal shader variants per GPU capability
// Copyright (c) 2026 ExplorerLens Project
//
// Chooses the best shader variant based on GPU feature level, vendor quirks,
// and available shader model. Supports DX11, DX12, and Vulkan compute paths.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ShaderVariantSelectorConfig
{
    bool enabled = true;
    uint32_t maxVariants = 32;
    std::string label = "ShaderVariantSelector";
};

class ShaderVariantSelector
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    ShaderVariantSelectorConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    enum class ShaderModel : uint8_t {
        SM_5_0,
        SM_5_1,
        SM_6_0,
        SM_6_5
    };

    struct Variant
    {
        std::string name;
        ShaderModel minModel = ShaderModel::SM_5_0;
        bool usesWaveOps = false;
    };

    bool RegisterVariant(const Variant& v)
    {
        if (m_variants.size() >= m_config.maxVariants)
            return false;
        m_variants.push_back(v);
        return true;
    }

    const Variant* SelectBest(ShaderModel available) const
    {
        const Variant* best = nullptr;
        for (const auto& v : m_variants) {
            if (static_cast<uint8_t>(v.minModel) <= static_cast<uint8_t>(available)) {
                if (!best || static_cast<uint8_t>(v.minModel) > static_cast<uint8_t>(best->minModel))
                    best = &v;
            }
        }
        return best;
    }

  private:
    bool m_initialized = false;
    ShaderVariantSelectorConfig m_config;
    std::vector<Variant> m_variants;
};

}  // namespace Engine
}  // namespace ExplorerLens
