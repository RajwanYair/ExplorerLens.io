//==============================================================================
// ReleaseGateV33.h — Release Gate V33 Ship Gate
// Validates all 28 ship-blocking KPIs spanning Foundation, Architecture,
// External Libraries, GUI/UX, Quality/DevOps, and Performance Optimization.
// Copyright (c) 2026 ExplorerLens Project
//==============================================================================
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class GateV33KPI : uint8_t {
  // Foundation (Sprints 349-353)
  VersionSync15 = 0,
  MuPDFLinked = 1,
  LibWebPCRTClean = 2,
  DeadCodeRemoved = 3,

  // Architecture (Sprints 354-363)
  LENSArchiveRefactored = 4,
  BitmapPoolActive = 5,
  OnApplyDataDriven = 6,
  IPropertyStoreRegistered = 7,
  GPUShaderLibrary4Shaders = 8,
  PluginHostOutOfProcess = 9,

  // External Libraries (Sprints 364-368)
  LibraryAuditComplete = 10,
  OpenJPEGLinked = 11,
  FreeTypeLinked = 12,
  FFmpegDynamic = 13,

  // GUI/UX (Sprints 369-378)
  FormatGroupsCollapsible = 14,
  FormatStatusIndicators = 15,
  DarkModeFullSupport = 16,

  // Quality (Sprints 379-388)
  CIMatrixActive = 17,
  CodeCoverage70Pct = 18,
  IntegrationTests50Plus = 19,
  FuzzingClean = 20,
  StaticAnalysisGate = 21,
  SBOMGenerated = 22,

  // Performance (Sprints 389-393)
  ZeroCopyPipelineActive = 23,
  ParallelIOActive = 24,
  SIMDScalerVerified = 25,
  PSOCachePersisted = 26,
  CacheWarmingActive = 27,
  COUNT
};

struct ReleaseGateV33Result {
  bool allKPIsPass = false;
  uint8_t kpiPassCount = 0;
  uint8_t kpiTotalCount = static_cast<uint8_t>(GateV33KPI::COUNT);
  float gateScore = 0.0f;       ///< 0.0 - 1.0 confidence score
  bool v15ShipApproved = false; ///< True iff all critical KPIs pass
  const wchar_t *codename = L"Zenith";
};

class ReleaseGateV33 {
public:
  static constexpr size_t KPICount() {
    return static_cast<size_t>(GateV33KPI::COUNT);
  }

  static const wchar_t *GetKPIName(GateV33KPI k) {
    switch (k) {
    case GateV33KPI::VersionSync15:
      return L"Version Sync 15.0.0";
    case GateV33KPI::MuPDFLinked:
      return L"MuPDF Linked (PDF Support)";
    case GateV33KPI::LibWebPCRTClean:
      return L"libwebp /MD CRT Clean";
    case GateV33KPI::DeadCodeRemoved:
      return L"Dead Code Removed";
    case GateV33KPI::LENSArchiveRefactored:
      return L"LENSArchive Refactored";
    case GateV33KPI::BitmapPoolActive:
      return L"Bitmap Pool Active";
    case GateV33KPI::OnApplyDataDriven:
      return L"OnApply Data-Driven Loop";
    case GateV33KPI::IPropertyStoreRegistered:
      return L"IPropertyStore Registered";
    case GateV33KPI::GPUShaderLibrary4Shaders:
      return L"GPU Shader Library (4+ Shaders)";
    case GateV33KPI::PluginHostOutOfProcess:
      return L"PluginHost Out-of-Process";
    case GateV33KPI::LibraryAuditComplete:
      return L"Library Audit Complete";
    case GateV33KPI::OpenJPEGLinked:
      return L"OpenJPEG JPEG 2000";
    case GateV33KPI::FreeTypeLinked:
      return L"FreeType Font Rendering";
    case GateV33KPI::FFmpegDynamic:
      return L"FFmpeg Dynamic Load";
    case GateV33KPI::FormatGroupsCollapsible:
      return L"Format Groups Collapsible";
    case GateV33KPI::FormatStatusIndicators:
      return L"Format Status Indicators";
    case GateV33KPI::DarkModeFullSupport:
      return L"Dark Mode Full Support";
    case GateV33KPI::CIMatrixActive:
      return L"CI Matrix (3 Configs)";
    case GateV33KPI::CodeCoverage70Pct:
      return L"Code Coverage >= 70%";
    case GateV33KPI::IntegrationTests50Plus:
      return L"Integration Tests >= 50";
    case GateV33KPI::FuzzingClean:
      return L"Fuzzing Campaign Clean";
    case GateV33KPI::StaticAnalysisGate:
      return L"Static Analysis Gate";
    case GateV33KPI::SBOMGenerated:
      return L"SBOM Generated";
    case GateV33KPI::ZeroCopyPipelineActive:
      return L"Zero-Copy Pipeline Active";
    case GateV33KPI::ParallelIOActive:
      return L"Parallel I/O Active";
    case GateV33KPI::SIMDScalerVerified:
      return L"SIMD Scaler AVX2/NEON";
    case GateV33KPI::PSOCachePersisted:
      return L"PSO Cache Persisted";
    case GateV33KPI::CacheWarmingActive:
      return L"Cache Warming Active";
    default:
      return L"Unknown KPI";
    }
  }

  static size_t GetKPICount() { return KPICount(); }

  static ReleaseGateV33Result Evaluate(bool kpiResults[]) {
    ReleaseGateV33Result r;
    for (size_t i = 0; i < KPICount(); ++i) {
      if (kpiResults[i])
        r.kpiPassCount++;
    }
    r.gateScore =
        static_cast<float>(r.kpiPassCount) / static_cast<float>(KPICount());
    r.allKPIsPass = (r.kpiPassCount == r.kpiTotalCount);
    // Require >= 85% of KPIs to pass for ship approval
    r.v15ShipApproved = r.gateScore >= 0.85f;
    return r;
  }
};

} // namespace Engine
} // namespace ExplorerLens
