// AdaptiveDecoderRouter.h — Intelligent Decoder Selection Based on Content
// Copyright (c) 2026 ExplorerLens Project
//
// Uses file signature analysis, historical performance data, and system state
// to route thumbnail requests to the optimal decoder. Replaces static
// extension-based routing with content-aware selection.

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Decoder selection strategy
enum class RoutingStrategy : uint8_t {
  ExtensionBased = 0,     ///< Classic: match file extension to decoder
  SignatureBased = 1,     ///< Inspect file magic bytes/header
  HybridFast = 2,         ///< Extension first, signature as fallback
  PerformanceOptimal = 3, ///< Route to fastest decoder for format
  QualityOptimal = 4,     ///< Route to highest-quality decoder
  COUNT
};

/// File signature pattern for content detection
struct FileSignature {
  std::array<uint8_t, 16> magic = {}; ///< Magic bytes
  uint8_t magicLength = 0;
  uint32_t offset = 0; ///< Offset from file start
  const wchar_t *formatName = nullptr;
  const wchar_t *decoderName = nullptr;
};

/// Decoder performance record for adaptive routing
struct DecoderPerformance {
  std::wstring decoderName;
  double avgDecodeTimeMs = 0.0;
  double p95DecodeTimeMs = 0.0;
  double successRate = 1.0; ///< 0-1
  uint64_t totalCalls = 0;
  uint64_t failureCount = 0;
  double avgQualityScore = 0.0; ///< 0-1 SSIM average
  bool gpuCapable = false;
};

/// Routing decision result
struct RoutingDecision {
  std::wstring selectedDecoder;
  std::wstring reason;
  RoutingStrategy strategyUsed = RoutingStrategy::ExtensionBased;
  double confidence = 1.0;
  bool isFallback = false;
  double estimatedTimeMs = 0.0;
};

/// Adaptive Decoder Router
class AdaptiveDecoderRouter {
public:
  static const wchar_t *StrategyName(RoutingStrategy s) {
    switch (s) {
    case RoutingStrategy::ExtensionBased:
      return L"Extension-Based";
    case RoutingStrategy::SignatureBased:
      return L"Signature-Based";
    case RoutingStrategy::HybridFast:
      return L"Hybrid Fast";
    case RoutingStrategy::PerformanceOptimal:
      return L"Performance Optimal";
    case RoutingStrategy::QualityOptimal:
      return L"Quality Optimal";
    default:
      return L"Unknown";
    }
  }

  static constexpr size_t StrategyCount() {
    return static_cast<size_t>(RoutingStrategy::COUNT);
  }

  /// Built-in file signatures for common formats
  static const std::vector<FileSignature> &GetBuiltinSignatures() {
    static const std::vector<FileSignature> sigs = {
        // PNG: 89 50 4E 47 0D 0A 1A 0A
        {{0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A},
         8,
         0,
         L"PNG",
         L"ImageDecoder"},
        // JPEG: FF D8 FF
        {{0xFF, 0xD8, 0xFF}, 3, 0, L"JPEG", L"ImageDecoder"},
        // GIF87a/89a: 47 49 46 38
        {{0x47, 0x49, 0x46, 0x38}, 4, 0, L"GIF", L"ImageDecoder"},
        // BMP: 42 4D
        {{0x42, 0x4D}, 2, 0, L"BMP", L"ImageDecoder"},
        // WebP: 52 49 46 46 xx xx xx xx 57 45 42 50
        {{0x52, 0x49, 0x46, 0x46}, 4, 0, L"WebP", L"WebPDecoder"},
        // TIFF LE: 49 49 2A 00
        {{0x49, 0x49, 0x2A, 0x00}, 4, 0, L"TIFF-LE", L"ImageDecoder"},
        // TIFF BE: 4D 4D 00 2A
        {{0x4D, 0x4D, 0x00, 0x2A}, 4, 0, L"TIFF-BE", L"ImageDecoder"},
        // PSD: 38 42 50 53
        {{0x38, 0x42, 0x50, 0x53}, 4, 0, L"PSD", L"PSDDecoder"},
        // ZIP/CBZ: 50 4B 03 04
        {{0x50, 0x4B, 0x03, 0x04}, 4, 0, L"ZIP", L"ArchiveDecoder"},
        // RAR: 52 61 72 21 1A 07
        {{0x52, 0x61, 0x72, 0x21, 0x1A, 0x07}, 6, 0, L"RAR", L"ArchiveDecoder"},
        // 7z: 37 7A BC AF 27 1C
        {{0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C}, 6, 0, L"7z", L"ArchiveDecoder"},
        // PDF: 25 50 44 46
        {{0x25, 0x50, 0x44, 0x46}, 4, 0, L"PDF", L"PDFDecoder"},
        // EXR: 76 2F 31 01
        {{0x76, 0x2F, 0x31, 0x01}, 4, 0, L"OpenEXR", L"EXRDecoder"},
        // DDS: 44 44 53 20
        {{0x44, 0x44, 0x53, 0x20}, 4, 0, L"DDS", L"DDSDecoder"},
        // QOI: 71 6F 69 66
        {{0x71, 0x6F, 0x69, 0x66}, 4, 0, L"QOI", L"QOIDecoder"},
    };
    return sigs;
  }

  /// Match file header bytes against known signatures
  static const FileSignature *MatchSignature(const uint8_t *header,
                                             size_t headerLen) {
    if (!header || headerLen < 2)
      return nullptr;
    for (const auto &sig : GetBuiltinSignatures()) {
      if (headerLen >= sig.offset + sig.magicLength) {
        bool match = true;
        for (uint8_t i = 0; i < sig.magicLength; ++i) {
          if (header[sig.offset + i] != sig.magic[i]) {
            match = false;
            break;
          }
        }
        if (match)
          return &sig;
      }
    }
    return nullptr;
  }
};

} // namespace Engine
} // namespace ExplorerLens
