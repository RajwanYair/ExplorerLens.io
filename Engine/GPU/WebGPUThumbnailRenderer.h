#pragma once
// WebGPUThumbnailRenderer.h — WebGPU Thumbnail Renderer
// Cross-API GPU rendering backend using WebGPU (Dawn) for
// future-proof GPU abstraction beyond D3D11/D3D12/Vulkan.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// WebGPU backend implementation
enum class WebGPUBackend : uint8_t {
  Dawn_D3D12 = 0, // Dawn → D3D12 (Windows primary)
  Dawn_Vulkan,    // Dawn → Vulkan (cross-platform)
  Dawn_D3D11,     // Dawn → D3D11 (legacy fallback)
  wgpu_native,    // wgpu-native (Rust-based)
  Emscripten,     // Browser WebGPU (future)
  COUNT
};

/// Texture usage flags
enum class WGPUTextureUsage : uint8_t {
  CopySrc = 0,
  CopyDst,
  TextureBinding,
  RenderAttachment,
  StorageBinding,
  COUNT
};

struct WebGPUDeviceInfo {
  WebGPUBackend backend = WebGPUBackend::Dawn_D3D12;
  const wchar_t *adapterName = nullptr;
  uint32_t vendorId = 0;
  uint32_t deviceId = 0;
  uint64_t vramBytes = 0;
  bool computeShaders = false;
  bool timestampQuery = false;
  uint32_t maxTextureDim = 8192;
};

struct WebGPURendererConfig {
  bool enabled = false; // Experimental
  WebGPUBackend preferredBackend = WebGPUBackend::Dawn_D3D12;
  uint32_t maxTextureSize = 4096;
  bool asyncCompute = true;
  bool depthBuffer = false;
  uint32_t sampleCount = 1;
  bool validation = false; // Debug layer
};

class WebGPUThumbnailRenderer {
public:
  static constexpr size_t BackendCount() {
    return static_cast<size_t>(WebGPUBackend::COUNT);
  }
  static constexpr size_t TextureUsageCount() {
    return static_cast<size_t>(WGPUTextureUsage::COUNT);
  }

  static const wchar_t *BackendName(WebGPUBackend b) {
    switch (b) {
    case WebGPUBackend::Dawn_D3D12:
      return L"Dawn (D3D12)";
    case WebGPUBackend::Dawn_Vulkan:
      return L"Dawn (Vulkan)";
    case WebGPUBackend::Dawn_D3D11:
      return L"Dawn (D3D11)";
    case WebGPUBackend::wgpu_native:
      return L"wgpu-native";
    case WebGPUBackend::Emscripten:
      return L"Emscripten WebGPU";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *TextureUsageName(WGPUTextureUsage u) {
    switch (u) {
    case WGPUTextureUsage::CopySrc:
      return L"Copy Source";
    case WGPUTextureUsage::CopyDst:
      return L"Copy Destination";
    case WGPUTextureUsage::TextureBinding:
      return L"Texture Binding";
    case WGPUTextureUsage::RenderAttachment:
      return L"Render Attachment";
    case WGPUTextureUsage::StorageBinding:
      return L"Storage Binding";
    default:
      return L"Unknown";
    }
  }

  /// Check if backend supports async compute
  static bool SupportsAsyncCompute(WebGPUBackend backend) {
    return backend == WebGPUBackend::Dawn_D3D12 ||
           backend == WebGPUBackend::Dawn_Vulkan ||
           backend == WebGPUBackend::wgpu_native;
  }
};

} // namespace Engine
} // namespace ExplorerLens
