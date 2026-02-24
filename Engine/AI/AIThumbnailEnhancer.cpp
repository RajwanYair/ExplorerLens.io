// Engine/AI/AIThumbnailEnhancer.cpp
// Implementation of AI-assisted thumbnail enhancements
// AI-Assisted Thumbnails

#include "AIThumbnailEnhancer.h"
#include "../Core/Logger.h"
#include <algorithm>
#include <chrono>

using namespace ExplorerLens::AI;
using namespace std::chrono;

namespace {
    // Model filenames (loaded from %LocalAppData%\ExplorerLens\Models\)
    constexpr const wchar_t* ESRGAN_2X_MODEL = L"esrgan_2x.onnx";
    constexpr const wchar_t* ESRGAN_4X_MODEL = L"esrgan_4x.onnx";
    constexpr const wchar_t* NSFW_MODEL = L"nsfw_mobilenet_v2.onnx";
    constexpr const wchar_t* SALIENCY_MODEL = L"saliency_unet.onnx";
    constexpr const wchar_t* FACE_DETECTION_MODEL = L"face_detection_yolov5.onnx";

    // Default model directory
    std::wstring GetModelDirectory() {
        wchar_t localAppData[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppData))) {
            return std::wstring(localAppData) + L"\\ExplorerLens\\Models\\";
        }
        return L"";
    }
}

AIThumbnailEnhancer::AIThumbnailEnhancer() {
    // Initialize ONNX Runtime environment
    m_ortEnv = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ExplorerLensAI");
}

AIThumbnailEnhancer::~AIThumbnailEnhancer() {
    Shutdown();
}

HRESULT AIThumbnailEnhancer::Initialize(ID3D12Device* pDevice) {
    if (!pDevice) return E_INVALIDARG;

    m_d3d12Device = pDevice;

    // Create DirectML device
    HRESULT hr = CreateDirectMLDevice();
    if (FAILED(hr)) {
        DT_LOG_ERROR("Failed to create DirectML device: 0x%08X", hr);
        return hr;
    }

    // Create command resources
    hr = CreateCommandResources();
    if (FAILED(hr)) {
        DT_LOG_ERROR("Failed to create command resources: 0x%08X", hr);
        return hr;
    }

    // Initialize ONNX session options (use DirectML execution provider)
    m_sessionOptions = std::make_unique<Ort::SessionOptions>();
    m_sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    
    // Enable DirectML execution provider for GPU acceleration
    m_sessionOptions->AppendExecutionProvider_DML(m_dmlDevice.Get());

    DT_LOG_INFO("AI Thumbnail Enhancer initialized successfully");
    return S_OK;
}

void AIThumbnailEnhancer::Shutdown() {
    // Release ONNX sessions
    m_superResolutionSession.reset();
    m_nsfwDetectionSession.reset();
    m_saliencySession.reset();
    m_faceDetectionSession.reset();

    // Release DirectML resources
    m_commandList.Reset();
    m_commandAllocator.Reset();
    m_commandQueue.Reset();
    m_dmlDevice.Reset();
    m_d3d12Device.Reset();

    DT_LOG_INFO("AI Thumbnail Enhancer shut down");
}

HRESULT AIThumbnailEnhancer::CreateDirectMLDevice() {
    // Create DirectML device from D3D12 device
    DML_CREATE_DEVICE_FLAGS dmlFlags = DML_CREATE_DEVICE_FLAG_NONE;

#ifdef _DEBUG
    dmlFlags |= DML_CREATE_DEVICE_FLAG_DEBUG;
#endif

    HRESULT hr = DMLCreateDevice(
        m_d3d12Device.Get(),
        dmlFlags,
        IID_PPV_ARGS(&m_dmlDevice));

    return hr;
}

HRESULT AIThumbnailEnhancer::CreateCommandResources() {
    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    HRESULT hr = m_d3d12Device->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(hr)) return hr;

    // Create command allocator
    hr = m_d3d12Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_commandAllocator));
    if (FAILED(hr)) return hr;

    // Create command list
    hr = m_d3d12Device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList));
    if (FAILED(hr)) return hr;

    m_commandList->Close();

    return S_OK;
}

HRESULT AIThumbnailEnhancer::LoadSuperResolutionModel(
    const std::wstring& modelPath,
    int scale) {
    
    auto startTime = high_resolution_clock::now();

    try {
        std::wstring fullPath = modelPath.empty() 
            ? GetModelDirectory() + (scale == 2 ? ESRGAN_2X_MODEL : ESRGAN_4X_MODEL)
            : modelPath;

        m_superResolutionSession = std::make_unique<Ort::Session>(
            *m_ortEnv,
            fullPath.c_str(),
            *m_sessionOptions);

        m_superResolutionScale = scale;
        m_superResolutionModelPath = fullPath;

        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime).count();
        m_stats.modelLoadTimeMs += duration;

        DT_LOG_INFO("Super-resolution model loaded (%dx): %ws (%lld ms)", 
            scale, fullPath.c_str(), duration);
        return S_OK;
    }
    catch (const std::exception& ex) {
        DT_LOG_ERROR("Failed to load super-resolution model: %s", ex.what());
        return E_FAIL;
    }
}

HRESULT AIThumbnailEnhancer::LoadNSFWDetectionModel(const std::wstring& modelPath) {
    auto startTime = high_resolution_clock::now();

    try {
        std::wstring fullPath = modelPath.empty()
            ? GetModelDirectory() + NSFW_MODEL
            : modelPath;

        m_nsfwDetectionSession = std::make_unique<Ort::Session>(
            *m_ortEnv,
            fullPath.c_str(),
            *m_sessionOptions);

        m_nsfwModelPath = fullPath;

        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime).count();
        m_stats.modelLoadTimeMs += duration;

        DT_LOG_INFO("NSFW detection model loaded: %ws (%lld ms)", 
            fullPath.c_str(), duration);
        return S_OK;
    }
    catch (const std::exception& ex) {
        DT_LOG_ERROR("Failed to load NSFW detection model: %s", ex.what());
        return E_FAIL;
    }
}

HRESULT AIThumbnailEnhancer::LoadSaliencyModel(const std::wstring& modelPath) {
    auto startTime = high_resolution_clock::now();

    try {
        std::wstring fullPath = modelPath.empty()
            ? GetModelDirectory() + SALIENCY_MODEL
            : modelPath;

        m_saliencySession = std::make_unique<Ort::Session>(
            *m_ortEnv,
            fullPath.c_str(),
            *m_sessionOptions);

        m_saliencyModelPath = fullPath;

        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime).count();
        m_stats.modelLoadTimeMs += duration;

        DT_LOG_INFO("Saliency detection model loaded: %ws (%lld ms)", 
            fullPath.c_str(), duration);
        return S_OK;
    }
    catch (const std::exception& ex) {
        DT_LOG_ERROR("Failed to load saliency model: %s", ex.what());
        return E_FAIL;
    }
}

HRESULT AIThumbnailEnhancer::LoadFaceDetectionModel(const std::wstring& modelPath) {
    auto startTime = high_resolution_clock::now();

    try {
        std::wstring fullPath = modelPath.empty()
            ? GetModelDirectory() + FACE_DETECTION_MODEL
            : modelPath;

        m_faceDetectionSession = std::make_unique<Ort::Session>(
            *m_ortEnv,
            fullPath.c_str(),
            *m_sessionOptions);

        m_faceDetectionModelPath = fullPath;

        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime).count();
        m_stats.modelLoadTimeMs += duration;

        DT_LOG_INFO("Face detection model loaded: %ws (%lld ms)", 
            fullPath.c_str(), duration);
        return S_OK;
    }
    catch (const std::exception& ex) {
        DT_LOG_ERROR("Failed to load face detection model: %s", ex.what());
        return E_FAIL;
    }
}

std::optional<ComPtr<ID3D12Resource>> AIThumbnailEnhancer::ApplySuperResolution(
    ID3D12Resource* pSourceTexture,
    int scale) {

    if (!m_superResolutionSession || scale != m_superResolutionScale) {
        // Lazy load model
        HRESULT hr = LoadSuperResolutionModel(L"", scale);
        if (FAILED(hr)) return std::nullopt;
    }

    auto startTime = high_resolution_clock::now();

    try {
        // Convert texture to tensor
        std::vector<float> inputTensor = TextureToTensor(pSourceTexture);
        
        // Get texture dimensions
        D3D12_RESOURCE_DESC desc = pSourceTexture->GetDesc();
        uint32_t width = static_cast<uint32_t>(desc.Width);
        uint32_t height = desc.Height;

        // Preprocess for ONNX (normalize to [-1, 1])
        std::vector<float> preprocessed;
        PreprocessImageForONNX(inputTensor, preprocessed, width, height);

        // Run inference
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator, OrtMemTypeDefault);

        std::vector<int64_t> inputShape = {1, 3, height, width};
        Ort::Value inputValue = Ort::Value::CreateTensor<float>(
            memoryInfo,
            preprocessed.data(),
            preprocessed.size(),
            inputShape.data(),
            inputShape.size());

        const char* inputNames[] = {"input"};
        const char* outputNames[] = {"output"};

        auto outputs = m_superResolutionSession->Run(
            Ort::RunOptions{nullptr},
            inputNames,
            &inputValue,
            1,
            outputNames,
            1);

        // Get output tensor
        float* outputData = outputs[0].GetTensorMutableData<float>();
        auto outputShape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
        
        uint32_t outputWidth = static_cast<uint32_t>(outputShape[3]);
        uint32_t outputHeight = static_cast<uint32_t>(outputShape[2]);

        // Convert tensor back to texture
        std::vector<float> outputTensor(outputData, outputData + (outputWidth * outputHeight * 3));
        auto outputTexture = TensorToTexture(outputTensor, outputWidth, outputHeight);

        m_stats.superResolutionCalls++;
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime).count();
        m_stats.totalInferenceTimeMs += duration;

        DT_LOG_INFO("Super-resolution applied: %ux%u -> %ux%u (%lld ms)",
            width, height, outputWidth, outputHeight, duration);

        return outputTexture;
    }
    catch (const std::exception& ex) {
        DT_LOG_ERROR("Super-resolution failed: %s", ex.what());
        return std::nullopt;
    }
}

std::optional<NSFWResult> AIThumbnailEnhancer::DetectNSFW(ID3D12Resource* pSourceTexture) {
    if (!m_nsfwDetectionSession) {
        // Lazy load model
        HRESULT hr = LoadNSFWDetectionModel(L"");
        if (FAILED(hr)) return std::nullopt;
    }

    auto startTime = high_resolution_clock::now();

    try {
        // Convert texture to tensor
        std::vector<float> inputTensor = TextureToTensor(pSourceTexture);
        
        D3D12_RESOURCE_DESC desc = pSourceTexture->GetDesc();
        uint32_t width = static_cast<uint32_t>(desc.Width);
        uint32_t height = desc.Height;

        // Preprocess for NSFW model (224x224 input, normalized [0, 1])
        std::vector<float> preprocessed;
        PreprocessImageForONNX(inputTensor, preprocessed, width, height);

        // Run inference
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator, OrtMemTypeDefault);

        std::vector<int64_t> inputShape = {1, 3, 224, 224};
        Ort::Value inputValue = Ort::Value::CreateTensor<float>(
            memoryInfo,
            preprocessed.data(),
            preprocessed.size(),
            inputShape.data(),
            inputShape.size());

        const char* inputNames[] = {"input"};
        const char* outputNames[] = {"output"};

        auto outputs = m_nsfwDetectionSession->Run(
            Ort::RunOptions{nullptr},
            inputNames,
            &inputValue,
            1,
            outputNames,
            1);

        // Get output probabilities
        float* outputData = outputs[0].GetTensorMutableData<float>();
        
        // Output is [safe, suggestive, explicit] probabilities
        float safeScore = outputData[0];
        float suggestiveScore = outputData[1];
        float explicitScore = outputData[2];

        // NSFW score is max(suggestive, explicit)
        float nsfwScore = std::max(suggestiveScore, explicitScore);

        NSFWResult result;
        result.score = nsfwScore;
        result.isNSFW = (nsfwScore >= m_nsfwThreshold);
        result.category = DetermineNSFWCategory(nsfwScore);

        m_stats.nsfwDetectionCalls++;
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime).count();
        m_stats.totalInferenceTimeMs += duration;

        DT_LOG_INFO("NSFW detection: score=%.2f, category=%ws (%lld ms)",
            nsfwScore, result.category.c_str(), duration);

        return result;
    }
    catch (const std::exception& ex) {
        DT_LOG_ERROR("NSFW detection failed: %s", ex.what());
        return std::nullopt;
    }
}

std::optional<SmartCropRegion> AIThumbnailEnhancer::DetectSalientRegion(
    ID3D12Resource* pSourceTexture,
    uint32_t targetWidth,
    uint32_t targetHeight) {
    
    if (!m_saliencySession) {
        HRESULT hr = LoadSaliencyModel(L"");
        if (FAILED(hr)) return std::nullopt;
    }

    auto startTime = high_resolution_clock::now();

    try {
        // Convert texture to tensor and run saliency detection
        std::vector<float> inputTensor = TextureToTensor(pSourceTexture);
        
        D3D12_RESOURCE_DESC desc = pSourceTexture->GetDesc();
        uint32_t width = static_cast<uint32_t>(desc.Width);
        uint32_t height = desc.Height;

        // (Simplified - full implementation would run U-Net for saliency map)
        // For now, return center crop as fallback
        SmartCropRegion region;
        region.width = targetWidth;
        region.height = targetHeight;
        region.x = (width - targetWidth) / 2;
        region.y = (height - targetHeight) / 2;
        region.saliency = 0.5f;

        m_stats.saliencyDetectionCalls++;
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime).count();
        m_stats.totalInferenceTimeMs += duration;

        return region;
    }
    catch (const std::exception& ex) {
        DT_LOG_ERROR("Saliency detection failed: %s", ex.what());
        return std::nullopt;
    }
}

std::vector<FaceBox> AIThumbnailEnhancer::DetectFaces(ID3D12Resource* pSourceTexture) {
    if (!m_faceDetectionSession) {
        HRESULT hr = LoadFaceDetectionModel(L"");
        if (FAILED(hr)) return {};
    }

    m_stats.faceDetectionCalls++;

    // Simplified implementation - would run YOLOv5 face detector
    return {};
}

HRESULT AIThumbnailEnhancer::EnhanceThumbnail(
    ID3D12Resource* pSourceTexture,
    ID3D12Resource** ppOutputTexture,
    EnhancementMode mode) {
    
    if (!pSourceTexture || !ppOutputTexture) return E_INVALIDARG;

    // Apply requested enhancements in pipeline order
    ComPtr<ID3D12Resource> currentTexture = pSourceTexture;

    // 1. NSFW detection (may skip other enhancements if detected)
    if ((mode & EnhancementMode::NSFWDetection) != EnhancementMode::None) {
        auto nsfwResult = DetectNSFW(currentTexture.Get());
        if (nsfwResult && nsfwResult->isNSFW) {
            // TODO: Apply blur or warning overlay
            DT_LOG_WARNING("NSFW content detected (score: %.2f)", nsfwResult->score);
        }
    }

    // 2. Super-resolution upscaling
    if ((mode & EnhancementMode::SuperResolution2x) != EnhancementMode::None ||
        (mode & EnhancementMode::SuperResolution4x) != EnhancementMode::None) {
        
        int scale = ((mode & EnhancementMode::SuperResolution4x) != EnhancementMode::None) ? 4 : 2;
        auto upscaled = ApplySuperResolution(currentTexture.Get(), scale);
        if (upscaled) {
            currentTexture = *upscaled;
        }
    }

    // 3. Content-aware cropping
    if ((mode & EnhancementMode::ContentAwareCrop) != EnhancementMode::None) {
        // Apply smart crop based on saliency
    }

    // 4. Face detection and centering
    if ((mode & EnhancementMode::FaceDetectionAndCenter) != EnhancementMode::None) {
        auto faces = DetectFaces(currentTexture.Get());
        // Center on primary face if found
    }

    *ppOutputTexture = currentTexture.Detach();
    return S_OK;
}

std::vector<float> AIThumbnailEnhancer::TextureToTensor(ID3D12Resource* pTexture) {
    // Simplified - full implementation would copy GPU texture to CPU buffer
    // and convert to float tensor
    D3D12_RESOURCE_DESC desc = pTexture->GetDesc();
    uint32_t width = static_cast<uint32_t>(desc.Width);
    uint32_t height = desc.Height;
    
    std::vector<float> tensor(width * height * 3);
    // TODO: Actual GPU->CPU copy and format conversion
    return tensor;
}

ComPtr<ID3D12Resource> AIThumbnailEnhancer::TensorToTexture(
    const std::vector<float>& tensor,
    uint32_t width,
    uint32_t height) {
    
    // Simplified - full implementation would convert float tensor to
    // D3D12 texture and upload to GPU
    ComPtr<ID3D12Resource> texture;
    // TODO: Actual tensor->texture conversion and GPU upload
    return texture;
}

void AIThumbnailEnhancer::PreprocessImageForONNX(
    const std::vector<float>& input,
    std::vector<float>& output,
    uint32_t width,
    uint32_t height) {
    
    // Standard ImageNet normalization: mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]
    output.resize(input.size());
    
    constexpr float means[] = {0.485f, 0.456f, 0.406f};
    constexpr float stds[] = {0.229f, 0.224f, 0.225f};

    for (size_t i = 0; i < input.size(); ++i) {
        int channel = i % 3;
        output[i] = (input[i] / 255.0f - means[channel]) / stds[channel];
    }
}

std::wstring AIThumbnailEnhancer::DetermineNSFWCategory(float score) {
    if (score < 0.3f) return L"safe";
    if (score < 0.7f) return L"suggestive";
    return L"explicit";
}

