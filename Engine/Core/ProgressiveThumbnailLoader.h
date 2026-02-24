//==============================================================================
// ExplorerLens Engine — Progressive Thumbnail Loader
// Async progressive decode with low-res placeholder, incremental quality
// upgrade, and priority-based preload for smooth Explorer browsing.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ProgressiveLoadStage : uint8_t {
  Placeholder = 0,
  LowRes,
  MedRes,
  FullRes,
  Enhanced,
  COUNT
};
enum class ProgressiveLoadStrategy : uint8_t {
  Immediate = 0,
  Progressive,
  Lazy,
  Background,
  BlurToSharp = Progressive, // compat alias
  COUNT = Background + 1
};
enum class ThumbnailPlaceholder : uint8_t {
  FileIcon = 0,
  BlurHash,
  DominantColor,
  Skeleton,
  None,
  ColorSwatch = DominantColor, // compat alias
  COUNT = None + 1
};

struct ProgressiveLoadConfig {
  ProgressiveLoadStrategy strategy = ProgressiveLoadStrategy::Progressive;
  ThumbnailPlaceholder placeholder = ThumbnailPlaceholder::BlurHash;
  uint32_t lowResPx = 64;
  uint32_t medResPx = 128;
  uint32_t fullResPx = 256;
  uint32_t dedupeWindowMs = 50;
};

struct ProgressiveLoadStatus {
  ProgressiveLoadStage stage = ProgressiveLoadStage::Placeholder;
  float progressPct = 0.0f;
  bool complete = false;
  uint32_t elapsedMs = 0;
};

class ProgressiveThumbnailLoader {
public:
  static const wchar_t *StageName(ProgressiveLoadStage s) {
    switch (s) {
    case ProgressiveLoadStage::Placeholder:
      return L"Placeholder";
    case ProgressiveLoadStage::LowRes:
      return L"Low Res (64px)";
    case ProgressiveLoadStage::MedRes:
      return L"Med Res (128px)";
    case ProgressiveLoadStage::FullRes:
      return L"Full Res (256px)";
    case ProgressiveLoadStage::Enhanced:
      return L"Enhanced (AI)";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *StrategyName(ProgressiveLoadStrategy s) {
    switch (s) {
    case ProgressiveLoadStrategy::Immediate:
      return L"Immediate";
    case ProgressiveLoadStrategy::Progressive:
      return L"Progressive";
    case ProgressiveLoadStrategy::Lazy:
      return L"Lazy";
    case ProgressiveLoadStrategy::Background:
      return L"Background";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *PlaceholderName(ThumbnailPlaceholder p) {
    switch (p) {
    case ThumbnailPlaceholder::FileIcon:
      return L"File Icon";
    case ThumbnailPlaceholder::BlurHash:
      return L"BlurHash";
    case ThumbnailPlaceholder::DominantColor:
      return L"Dominant Color";
    case ThumbnailPlaceholder::Skeleton:
      return L"Skeleton";
    case ThumbnailPlaceholder::None:
      return L"None";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t StageCount() {
    return static_cast<size_t>(ProgressiveLoadStage::COUNT);
  }
  static constexpr size_t StrategyCount() {
    return static_cast<size_t>(ProgressiveLoadStrategy::COUNT);
  }
  static constexpr size_t PlaceholderCount() {
    return static_cast<size_t>(ThumbnailPlaceholder::COUNT);
  }
  static ProgressiveLoadConfig DefaultConfig() {
    return ProgressiveLoadConfig{};
  }
};

} // namespace Engine
} // namespace ExplorerLens
