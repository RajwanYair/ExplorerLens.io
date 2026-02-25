#pragma once
// FaceDetectionOrientation.h — Face Detection Orientation Engine
// EXIF orientation correction + face detection for auto-rotation of portrait
// thumbnails, ensuring faces are always upright and centered.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// EXIF orientation tag values (1-8)
enum class EXIFOrientation : uint8_t {
  Normal = 1,
  FlipH = 2,
  Rotate180 = 3,
  FlipV = 4,
  Transpose = 5,
  Rotate270 = 6,
  Transverse = 7,
  Rotate90 = 8,
  COUNT = 9 // 9 values (0 unused + 1-8)
};

/// Face detection backend
enum class FaceDetBackend : uint8_t {
  None = 0,
  HaarCascade,   // Classical Haar cascade (fast, low accuracy)
  HOGDescriptor, // HOG + SVM (balanced)
  DirectML_ONNX, // Neural network via DirectML
  COUNT
};

struct FaceRect {
  float x = 0.0f; // normalized 0-1
  float y = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  float confidence = 0.0f;
};

struct OrientationResult {
  EXIFOrientation exifTag = EXIFOrientation::Normal;
  int rotationDegrees = 0;
  bool flipHorizontal = false;
  bool flipVertical = false;
  uint32_t facesDetected = 0;
  bool autoRotated = false;
};

class FaceDetectionOrientation {
public:
  static constexpr size_t BackendCount() {
    return static_cast<size_t>(FaceDetBackend::COUNT);
  }

  static const wchar_t *BackendName(FaceDetBackend b) {
    switch (b) {
    case FaceDetBackend::None:
      return L"None";
    case FaceDetBackend::HaarCascade:
      return L"Haar Cascade";
    case FaceDetBackend::HOGDescriptor:
      return L"HOG Descriptor";
    case FaceDetBackend::DirectML_ONNX:
      return L"DirectML ONNX";
    default:
      return L"Unknown";
    }
  }

  /// Convert EXIF orientation to rotation degrees
  static int OrientationToDegrees(EXIFOrientation o) {
    switch (o) {
    case EXIFOrientation::Normal:
    case EXIFOrientation::FlipH:
      return 0;
    case EXIFOrientation::Rotate180:
    case EXIFOrientation::FlipV:
      return 180;
    case EXIFOrientation::Transpose:
    case EXIFOrientation::Rotate270:
      return 270;
    case EXIFOrientation::Transverse:
    case EXIFOrientation::Rotate90:
      return 90;
    default:
      return 0;
    }
  }

  /// Check if orientation requires width/height swap
  static bool RequiresDimensionSwap(EXIFOrientation o) {
    return o == EXIFOrientation::Transpose || o == EXIFOrientation::Rotate270 ||
           o == EXIFOrientation::Transverse || o == EXIFOrientation::Rotate90;
  }
};

} // namespace Engine
} // namespace ExplorerLens
