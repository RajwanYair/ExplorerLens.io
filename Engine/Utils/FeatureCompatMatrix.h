// FeatureCompatMatrix.h — Format × OS × GPU Compatibility Matrix
// Copyright (c) 2026 ExplorerLens Project
//
// Runtime-queryable compatibility matrix mapping (format, OS, GPU backend) →
// Support level.  Used by the diagnostics page, CI gate, and Store readiness
// checker to ensure all advertised features actually work on the target platform.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Support level for a (format, OS, GPU) tuple.
enum class CompatLevel : uint8_t {
    NotSupported = 0,     // Format cannot be decoded on this config
    CpuOnly = 1,          // CPU decode works; GPU path unavailable
    GpuAccelerated = 2,   // Full GPU-accelerated path available
    FullAccelerated = 3,  // GPU + hardware video decode (NVDEC/QuickSync/AMF)
};

// Identifies a specific GPU backend in the matrix.
enum class GpuBackend : uint8_t {
    CPU = 0,
    D3D11 = 1,
    D3D12 = 2,
    Vulkan = 3,
};

// A single cell in the compatibility matrix.
struct CompatCell
{
    std::string formatExtension;  // e.g. ".heic"
    std::string osVersion;        // e.g. "Win10-22H2", "Win11-23H2"
    GpuBackend backend;
    CompatLevel level;
    std::string notes;  // Optional caveat text
};

// Capabilities detected at runtime for the current machine.
struct RuntimeCapabilities
{
    bool win10Or11{false};
    bool d3d12Support{false};
    bool d3d11Support{false};
    bool vulkanSupport{false};
    bool nvdecSupport{false};
    bool quickSyncSupport{false};
    bool amfSupport{false};
    std::string gpuVendor;
    std::string gpuName;
    uint32_t driverVersionMajor{0};
    uint32_t driverVersionMinor{0};
};

// FeatureCompatMatrix — Format × OS × GPU compatibility oracle.
//
// Populated at engine initialisation from a built-in data table.
// Queryable at runtime for specific (format, OS, backend) combinations.
class FeatureCompatMatrix
{
  public:
    FeatureCompatMatrix() noexcept {}
    ~FeatureCompatMatrix() noexcept = default;

    FeatureCompatMatrix(const FeatureCompatMatrix&) = delete;
    FeatureCompatMatrix& operator=(const FeatureCompatMatrix&) = delete;

    // Detect runtime capabilities of the current machine.
    RuntimeCapabilities DetectCapabilities() const noexcept;

    // Query support level for a specific format on the current machine.
    CompatLevel QuerySupport(const std::string& formatExt, GpuBackend backend) const noexcept;

    // Get all cells where CompatLevel < minimum.
    std::vector<CompatCell> FindGaps(CompatLevel minimum = CompatLevel::CpuOnly) const noexcept;

    // Generate a Markdown compatibility matrix table for docs output.
    std::string GenerateMarkdownTable() const noexcept;

    // Validate all 200+ formats against current runtime.  Returns count of unsupported.
    uint32_t ValidateAll(RuntimeCapabilities& caps) const noexcept;

    // Singleton access.
    static FeatureCompatMatrix& Instance() noexcept
    {
        static FeatureCompatMatrix s_instance;
        return s_instance;
    }

  private:
    std::vector<CompatCell> m_cells;
    void PopulateBuiltinTable() noexcept;
};

}  // namespace Engine
}  // namespace ExplorerLens
