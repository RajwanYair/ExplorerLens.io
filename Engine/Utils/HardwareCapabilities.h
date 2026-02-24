//==============================================================================
// ExplorerLens Engine - Comprehensive Hardware Detection
// Copyright (c) 2026 - ExplorerLens Project
//
// Detects and reports all CPU and GPU capabilities for optimal code path
// selection
//==============================================================================

#pragma once

#include "../Core/EngineAPI.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// CPU Capabilities
//==============================================================================

struct CPUCapabilities {
  // Vendor identification
  std::string vendor;      // "GenuineIntel", "AuthenticAMD", "CentaurHauls"
  std::string brandString; // Full CPU model name

  // Core information
  uint32_t physicalCores;
  uint32_t logicalCores;
  uint32_t cacheLineSize;
  uint32_t l1CacheKB;
  uint32_t l2CacheKB;
  uint32_t l3CacheKB;

  // CPU family/model/stepping
  uint32_t family;
  uint32_t model;
  uint32_t stepping;

  // Base instruction set features
  bool hasMMX;
  bool hasSSE;
  bool hasSSE2;
  bool hasSSE3;
  bool hasSSSE3;
  bool hasSSE41;
  bool hasSSE42;

  // Advanced instruction sets
  bool hasAVX;
  bool hasAVX2;
  bool hasAVX512F;    // AVX-512 Foundation
  bool hasAVX512DQ;   // Doubleword and Quadword
  bool hasAVX512IFMA; // Integer Fused Multiply-Add
  bool hasAVX512PF;   // Prefetch
  bool hasAVX512ER;   // Exponential and Reciprocal
  bool hasAVX512CD;   // Conflict Detection
  bool hasAVX512BW;   // Byte and Word
  bool hasAVX512VL;   // Vector Length Extensions
  bool hasAVX512VBMI; // Vector Bit Manipulation
  bool hasAVX512VBMI2;
  bool hasAVX512VNNI; // Vector Neural Network Instructions
  bool hasAVX512BITALG;
  bool hasAVX512VPOPCNTDQ;

  // Cryptography extensions
  bool hasAESNI; // AES New Instructions
  bool hasSHA;   // SHA extensions

  // Other features
  bool hasFMA;         // Fused Multiply-Add
  bool hasFMA4;        // AMD FMA4
  bool hasF16C;        // Half-precision float conversion
  bool hasBMI1;        // Bit Manipulation Instruction Set 1
  bool hasBMI2;        // Bit Manipulation Instruction Set 2
  bool hasADX;         // Multi-Precision Add-Carry
  bool hasPOPCNT;      // Population Count
  bool hasLZCNT;       // Leading Zero Count
  bool hasPREFETCHWT1; // Prefetch Vector Data Into Caches
  bool hasRDRAND;      // Hardware Random Number Generator
  bool hasRDSEED;      // Seed for PRNG
  bool hasCLFLUSH;     // Cache Line Flush
  bool hasCLFLUSHOPT;  // Optimized Cache Line Flush
  bool hasCLWB;        // Cache Line Write Back

  // Virtualization
  bool hasVMX; // Intel VT-x
  bool hasSVM; // AMD-V

  // Get best available SIMD instruction set string
  std::string GetBestSIMD() const;

  // Get human-readable summary
  std::string GetSummary() const;
};

//==============================================================================
// GPU Information
//==============================================================================

enum class GPUVendor {
  Unknown,
  Intel,
  NVIDIA,
  AMD,
  Microsoft // Software renderer
};

struct GPUInfo {
  std::string name;
  std::string driverVersion;
  GPUVendor vendor;
  uint64_t dedicatedMemoryMB;
  uint64_t sharedMemoryMB;
  uint32_t vendorID;
  uint32_t deviceID;

  // DirectX capabilities
  uint32_t featureLevel; // D3D_FEATURE_LEVEL (e.g., 0xc000 = 12.0)
  bool supportsD3D11;
  bool supportsD3D12;

  // Compute capabilities
  bool supportsCompute;
  uint32_t computeUnits;

  // Get vendor from ID
  static GPUVendor GetVendorFromID(uint32_t vendorID);

  // Get human-readable summary
  std::string GetSummary() const;
};

//==============================================================================
// Hardware Capabilities Manager
//==============================================================================

class ENGINE_API HardwareCapabilities {
public:
  static HardwareCapabilities &Get();

  // CPU information
  const CPUCapabilities &GetCPU() const { return m_cpu; }

  // GPU information
  const std::vector<GPUInfo> &GetGPUs() const { return m_gpus; }
  const GPUInfo *GetPrimaryGPU() const;
  const GPUInfo *GetPreferredGPU() const; // Best for rendering

  // System memory
  uint64_t GetTotalMemoryMB() const { return m_totalMemoryMB; }
  uint64_t GetAvailableMemoryMB() const;

  // Get comprehensive hardware report
  std::string GetFullReport() const;

  // Get concise summary for UI
  std::string GetSummary() const;

  // Check if specific feature is available
  bool HasFeature(const char *feature) const;

  // Refresh GPU information (call if GPU configuration changes)
  void RefreshGPUs();

private:
  HardwareCapabilities();
  ~HardwareCapabilities() = default;

  // Detection methods
  void DetectCPU();
  void DetectGPUs();
  void DetectMemory();

  // CPUID helper
  void CPUID(int function, int subfunction, int *regs);
  uint64_t XGETBV(uint32_t xcr);

  CPUCapabilities m_cpu;
#pragma warning(push)
#pragma warning(disable : 4251) // STL members in DLL-exported class
  std::vector<GPUInfo> m_gpus;
#pragma warning(pop)
  uint64_t m_totalMemoryMB;
};

} // namespace Engine
} // namespace ExplorerLens
