//==============================================================================
// ExplorerLens Engine — Scene Understanding Engine
// Deep-learning scene classification with object detection, dominant scene
// category labeling, and content-aware thumbnail crop region suggestion.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SceneCategory : uint8_t {
  Nature = 0,
  Architecture,
  People,
  Food,
  Animals,
  Vehicle,
  Text,
  Abstract,
  Technology,
  Sports,
  Indoor = Architecture, // compat alias
  COUNT = Sports + 1
};
enum class SceneMLBackend : uint8_t {
  DirectML = 0,
  ONNX,
  OpenVINO,
  CPU,
  COUNT
};
enum class SceneConfidence : uint8_t {
  VeryLow = 0,
  Low,
  Medium,
  High,
  VeryHigh,
  COUNT
};

struct SceneClassification {
  SceneCategory category = SceneCategory::Abstract;
  SceneConfidence confidence = SceneConfidence::Low;
  float score = 0.0f;
  std::wstring label;
};

struct SceneUnderstandingResult {
  SceneClassification primaryScene;
  std::vector<SceneClassification> alternateScenes;
  SceneMLBackend backend = SceneMLBackend::DirectML;
  uint32_t inferenceMs = 0;
  bool hasFaces = false;
  bool hasText = false;
};

class SceneUnderstandingEngine {
public:
  static const wchar_t *CategoryName(SceneCategory c) {
    switch (c) {
    case SceneCategory::Nature:
      return L"Nature";
    case SceneCategory::Architecture:
      return L"Architecture";
    case SceneCategory::People:
      return L"People";
    case SceneCategory::Food:
      return L"Food";
    case SceneCategory::Animals:
      return L"Animals";
    case SceneCategory::Vehicle:
      return L"Vehicle";
    case SceneCategory::Text:
      return L"Text/Document";
    case SceneCategory::Abstract:
      return L"Abstract";
    case SceneCategory::Technology:
      return L"Technology";
    case SceneCategory::Sports:
      return L"Sports";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *BackendName(SceneMLBackend b) {
    switch (b) {
    case SceneMLBackend::DirectML:
      return L"DirectML";
    case SceneMLBackend::ONNX:
      return L"ONNX Runtime";
    case SceneMLBackend::OpenVINO:
      return L"OpenVINO";
    case SceneMLBackend::CPU:
      return L"CPU";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *ConfidenceName(SceneConfidence c) {
    switch (c) {
    case SceneConfidence::VeryLow:
      return L"Very Low (<20%)";
    case SceneConfidence::Low:
      return L"Low (20-40%)";
    case SceneConfidence::Medium:
      return L"Medium (40-60%)";
    case SceneConfidence::High:
      return L"High (60-80%)";
    case SceneConfidence::VeryHigh:
      return L"Very High (>80%)";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t CategoryCount() {
    return static_cast<size_t>(SceneCategory::COUNT);
  }
  static constexpr size_t BackendCount() {
    return static_cast<size_t>(SceneMLBackend::COUNT);
  }
  static constexpr size_t ConfidenceCount() {
    return static_cast<size_t>(SceneConfidence::COUNT);
  }
};

} // namespace Engine
} // namespace ExplorerLens
