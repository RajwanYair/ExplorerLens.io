// Engine/AI/AIThumbnailEnhancer.h
// DirectML/ONNX integration for AI-assisted thumbnail generation
// AI-Assisted Thumbnails

#pragma once

#include <cstdint>
#include <DirectML.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <onnxruntime_cxx_api.h>
#include <string>
#include <memory>
#include <optional>

namespace ExplorerLens::AI {

using Microsoft::WRL::ComPtr;

/// <summary>
/// AI enhancement mode for thumbnail generation
/// </summary>
enum class EnhancementMode {
 None = 0, ///< No AI enhancement
 SuperResolution2x, ///< 2x upscaling via ESRGAN
 SuperResolution4x, ///< 4x upscaling via ESRGAN
 ContentAwareCrop, ///< Smart cropping with saliency detection
 NSFWDetection, ///< NSFW content detection (returns score)
 FaceDetectionAndCenter, ///< Face detection and centering
 All = 0xFF ///< Apply all enhancements
};

/// <summary>
/// NSFW detection result
/// </summary>
struct NSFWResult {
 float score; ///< NSFW confidence score (0.0 = safe, 1.0 = NSFW)
 bool isNSFW; ///< True if score exceeds threshold (0.7)
 std::wstring category; ///< Category: "safe", "suggestive", "explicit"
};

/// <summary>
/// Face detection bounding box
/// </summary>
struct FaceBox {
 float x, y, width, height; ///< Normalized coordinates [0.0, 1.0]
 float confidence; ///< Detection confidence
};

/// <summary>
/// Content-aware crop result
/// </summary>
struct SmartCropRegion {
 uint32_t x, y, width, height; ///< Pixel coordinates
 float saliency; ///< Saliency score for this region
};

/// <summary>
/// AI Thumbnail Enhancer using DirectML and ONNX Runtime
/// Provides super-resolution, smart cropping, NSFW detection, face detection
/// </summary>
class AIThumbnailEnhancer {
public:
 AIThumbnailEnhancer();
 ~AIThumbnailEnhancer();

 // Initialization
 HRESULT Initialize(ID3D12Device* pDevice);
 void Shutdown();

 // Model management (lazy loading)
 HRESULT LoadSuperResolutionModel(const std::wstring& modelPath, int scale = 2);
 HRESULT LoadNSFWDetectionModel(const std::wstring& modelPath);
 HRESULT LoadSaliencyModel(const std::wstring& modelPath);
 HRESULT LoadFaceDetectionModel(const std::wstring& modelPath);

 // Enhancement operations
 std::optional<ComPtr<ID3D12Resource>> ApplySuperResolution(
 ID3D12Resource* pSourceTexture,
 int scale = 2);

 std::optional<SmartCropRegion> DetectSalientRegion(
 ID3D12Resource* pSourceTexture,
 uint32_t targetWidth,
 uint32_t targetHeight);

 std::optional<NSFWResult> DetectNSFW(ID3D12Resource* pSourceTexture);

 std::vector<FaceBox> DetectFaces(ID3D12Resource* pSourceTexture);

 std::optional<ComPtr<ID3D12Resource>> ApplyContentAwareCrop(
 ID3D12Resource* pSourceTexture,
 uint32_t targetWidth,
 uint32_t targetHeight);

 // Combined enhancement pipeline
 HRESULT EnhanceThumbnail(
 ID3D12Resource* pSourceTexture,
 ID3D12Resource** ppOutputTexture,
 EnhancementMode mode);

 // Configuration
 void SetNSFWThreshold(float threshold) { m_nsfwThreshold = threshold; }
 void SetMinFaceConfidence(float confidence) { m_minFaceConfidence = confidence; }
 void EnableModelCaching(bool enable) { m_enableModelCache = enable; }

 // Statistics
 struct Stats {
 uint64_t superResolutionCalls = 0;
 uint64_t nsfwDetectionCalls = 0;
 uint64_t saliencyDetectionCalls = 0;
 uint64_t faceDetectionCalls = 0;
 uint64_t totalInferenceTimeMs = 0;
 uint64_t modelLoadTimeMs = 0;
 };
 const Stats& GetStats() const { return m_stats; }
 void ResetStats() { m_stats = Stats{}; }

private:
 // DirectML infrastructure
 ComPtr<ID3D12Device> m_d3d12Device;
 ComPtr<IDMLDevice> m_dmlDevice;
 ComPtr<ID3D12CommandQueue> m_commandQueue;
 ComPtr<ID3D12CommandAllocator> m_commandAllocator;
 ComPtr<ID3D12GraphicsCommandList> m_commandList;

 // ONNX Runtime sessions (lazy-loaded)
 std::unique_ptr<Ort::Session> m_superResolutionSession;
 std::unique_ptr<Ort::Session> m_nsfwDetectionSession;
 std::unique_ptr<Ort::Session> m_saliencySession;
 std::unique_ptr<Ort::Session> m_faceDetectionSession;

 // ONNX environment
 std::unique_ptr<Ort::Env> m_ortEnv;
 std::unique_ptr<Ort::SessionOptions> m_sessionOptions;

 // Configuration
 float m_nsfwThreshold = 0.7f;
 float m_minFaceConfidence = 0.5f;
 bool m_enableModelCache = true;
 int m_superResolutionScale = 2;

 // Model paths (for lazy loading)
 std::wstring m_superResolutionModelPath;
 std::wstring m_nsfwModelPath;
 std::wstring m_saliencyModelPath;
 std::wstring m_faceDetectionModelPath;

 // Statistics
 Stats m_stats;

 // Helper methods
 HRESULT CreateDirectMLDevice();
 HRESULT CreateCommandResources();

 std::vector<float> TextureToTensor(ID3D12Resource* pTexture);
 ComPtr<ID3D12Resource> TensorToTexture(
 const std::vector<float>& tensor,
 uint32_t width,
 uint32_t height);

 HRESULT ExecuteDirectMLOperator(IDMLOperator* pOperator);

 void PreprocessImageForONNX(
 const std::vector<float>& input,
 std::vector<float>& output,
 uint32_t width,
 uint32_t height);

 SmartCropRegion FindBestCropRegion(
 const std::vector<float>& saliencyMap,
 uint32_t mapWidth,
 uint32_t mapHeight,
 uint32_t targetWidth,
 uint32_t targetHeight);

 std::wstring DetermineNSFWCategory(float score);
};

/// <summary>
/// RAII helper for AI enhancement with automatic resource cleanup
/// </summary>
class ScopedAIEnhancement {
public:
 ScopedAIEnhancement(AIThumbnailEnhancer* enhancer, EnhancementMode mode)
 : m_enhancer(enhancer), m_mode(mode) {}

 ~ScopedAIEnhancement() {
 // Cleanup any temporary resources
 }

 HRESULT Enhance(ID3D12Resource* pSource, ID3D12Resource** ppOutput) {
 return m_enhancer->EnhanceThumbnail(pSource, ppOutput, m_mode);
 }

private:
 AIThumbnailEnhancer* m_enhancer;
 EnhancementMode m_mode;
};

} // namespace ExplorerLens::AI
