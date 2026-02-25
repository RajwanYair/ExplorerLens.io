#pragma once
// VisualSimilarityIndex.h — Visual Similarity Index
// Perceptual hashing (pHash/dHash/aHash) and embedding-based similarity search
// for duplicate detection and visual clustering in large thumbnail caches.
#include <cmath>
#include <cstdint>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

/// Perceptual hash algorithm
enum class VisualHashAlgo : uint8_t {
  AverageHash = 0, // aHash — fastest, least accurate
  DifferenceHash,  // dHash — good balance
  PerceptualHash,  // pHash — DCT-based, most accurate
  ColorMomentHash, // Color distribution hash
  WaveletHash,     // Wavelet-based
  COUNT
};

/// Similarity classification
enum class SimilarityClass : uint8_t {
  Identical = 0, // Hamming distance 0
  NearDuplicate, // distance 1-5
  Similar,       // distance 6-12
  Related,       // distance 13-20
  Different,     // distance > 20
  COUNT
};

struct PerceptualHashResult {
  uint64_t hash = 0;
  VisualHashAlgo algorithm = VisualHashAlgo::DifferenceHash;
  double computeTimeMs = 0.0;
};

class VisualSimilarityIndex {
public:
  static constexpr size_t AlgorithmCount() {
    return static_cast<size_t>(VisualHashAlgo::COUNT);
  }
  static constexpr size_t ClassCount() {
    return static_cast<size_t>(SimilarityClass::COUNT);
  }

  static const wchar_t *AlgorithmName(VisualHashAlgo a) {
    switch (a) {
    case VisualHashAlgo::AverageHash:
      return L"Average Hash";
    case VisualHashAlgo::DifferenceHash:
      return L"Difference Hash";
    case VisualHashAlgo::PerceptualHash:
      return L"Perceptual Hash";
    case VisualHashAlgo::ColorMomentHash:
      return L"Color Moment Hash";
    case VisualHashAlgo::WaveletHash:
      return L"Wavelet Hash";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *ClassName(SimilarityClass c) {
    switch (c) {
    case SimilarityClass::Identical:
      return L"Identical";
    case SimilarityClass::NearDuplicate:
      return L"Near-Duplicate";
    case SimilarityClass::Similar:
      return L"Similar";
    case SimilarityClass::Related:
      return L"Related";
    case SimilarityClass::Different:
      return L"Different";
    default:
      return L"Unknown";
    }
  }

  /// Compute Hamming distance between two 64-bit hashes
  static uint32_t HammingDistance(uint64_t a, uint64_t b) {
    uint64_t diff = a ^ b;
    uint32_t count = 0;
    while (diff) {
      count += diff & 1;
      diff >>= 1;
    }
    return count;
  }

  /// Classify similarity based on Hamming distance
  static SimilarityClass Classify(uint32_t distance) {
    if (distance == 0)
      return SimilarityClass::Identical;
    if (distance <= 5)
      return SimilarityClass::NearDuplicate;
    if (distance <= 12)
      return SimilarityClass::Similar;
    if (distance <= 20)
      return SimilarityClass::Related;
    return SimilarityClass::Different;
  }

  /// Simple difference hash for 8x8 grayscale grid
  static uint64_t ComputeDHash(const uint8_t *gray8x8) {
    uint64_t hash = 0;
    for (int row = 0; row < 8; ++row) {
      for (int col = 0; col < 7; ++col) {
        if (gray8x8[row * 8 + col] < gray8x8[row * 8 + col + 1])
          hash |= (1ULL << (row * 7 + col));
      }
    }
    return hash;
  }
};

} // namespace Engine
} // namespace ExplorerLens
