// VideoCodecRouter.h — Intelligent Video Codec Routing Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Routes video thumbnail requests to the best available decoder backend:
// Media Foundation (fast path for MP4/MOV), FFmpeg (broad codec support),
// or GPU decode (NVDEC/QuickSync/AMF for hardware acceleration).

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Video decoder backend selection
enum class VideoBackend : uint8_t {
 Auto = 0, ///< Automatic selection based on format + hardware
 MediaFoundation = 1, ///< Windows Media Foundation (default for MP4)
 FFmpeg = 2, ///< FFmpeg libavcodec (broad format support)
 GPUDecode = 3, ///< Hardware decode (NVDEC/QuickSync/AMF)
 DirectShow = 4 ///< Legacy DirectShow (last resort)
};

/// Route decision result
struct VideoRouteDecision {
 VideoBackend primary = VideoBackend::MediaFoundation;
 VideoBackend fallback = VideoBackend::FFmpeg;
 const char* reason = "";
 bool gpuAccelerable = false;
 float expectedSpeedMs = 50.0f;
};

/// Video codec router — selects optimal decode path
class VideoCodecRouter {
public:
 static VideoCodecRouter& Instance() {
 static VideoCodecRouter instance;
 return instance;
 }

 /// Route a video file to the best decoder backend
 VideoRouteDecision Route(const wchar_t* filePath) const {
 VideoRouteDecision decision;
 if (!filePath) return decision;

 auto ext = GetLowerExtension(filePath);

 // Media Foundation fast path (well-supported codecs)
 if (ext == L".mp4" || ext == L".m4v" || ext == L".mov" ||
 ext == L".wmv" || ext == L".avi") {
 decision.primary = VideoBackend::MediaFoundation;
 decision.fallback = VideoBackend::FFmpeg;
 decision.reason = "Native MF support";
 decision.gpuAccelerable = true;
 decision.expectedSpeedMs = 20.0f;
 }
 // FFmpeg preferred (MF poor/no support)
 else if (ext == L".mkv" || ext == L".webm" || ext == L".flv" ||
 ext == L".ogv" || ext == L".ogg" || ext == L".rmvb" ||
 ext == L".rm" || ext == L".ts" || ext == L".mts") {
 decision.primary = VideoBackend::FFmpeg;
 decision.fallback = VideoBackend::MediaFoundation;
 decision.reason = "Container not natively supported by MF";
 decision.gpuAccelerable = (ext == L".mkv" || ext == L".webm");
 decision.expectedSpeedMs = 40.0f;
 }
 // Fallback for unknown
 else {
 decision.primary = VideoBackend::MediaFoundation;
 decision.fallback = VideoBackend::FFmpeg;
 decision.reason = "Unknown format — trying MF first";
 decision.expectedSpeedMs = 60.0f;
 }

 // Override with GPU decode if available and beneficial
 if (decision.gpuAccelerable && m_gpuDecodeAvailable) {
 decision.primary = VideoBackend::GPUDecode;
 decision.fallback = VideoBackend::MediaFoundation;
 decision.expectedSpeedMs *= 0.5f; // ~2x faster
 }

 return decision;
 }

 /// Get list of extensions handled by each backend
 std::vector<std::wstring> GetExtensionsForBackend(VideoBackend backend) const {
 std::vector<std::wstring> exts;
 switch (backend) {
 case VideoBackend::MediaFoundation:
 exts = {L".mp4", L".m4v", L".mov", L".wmv", L".avi", L".3gp"};
 break;
 case VideoBackend::FFmpeg:
 exts = {L".mkv", L".webm", L".flv", L".ogv", L".ogg",
 L".rmvb", L".rm", L".ts", L".mts"};
 break;
 case VideoBackend::GPUDecode:
 exts = {L".mp4", L".mkv", L".webm", L".mov"};
 break;
 default:
 break;
 }
 return exts;
 }

 /// Backend name lookup
 static const char* BackendName(VideoBackend b) {
 switch (b) {
 case VideoBackend::Auto: return "Auto";
 case VideoBackend::MediaFoundation: return "MediaFoundation";
 case VideoBackend::FFmpeg: return "FFmpeg";
 case VideoBackend::GPUDecode: return "GPUDecode";
 case VideoBackend::DirectShow: return "DirectShow";
 default: return "Unknown";
 }
 }

 static constexpr uint32_t GetBackendCount() { return 5; }

 /// Set GPU decode availability
 void SetGPUDecodeAvailable(bool available) { m_gpuDecodeAvailable = available; }
 bool IsGPUDecodeAvailable() const { return m_gpuDecodeAvailable; }

private:
 VideoCodecRouter() {
 // Probe for GPU decode capability at initialization
 m_gpuDecodeAvailable = ProbeGPUDecode();
 }

 bool ProbeGPUDecode() {
 // Check for NVDEC, QuickSync, or AMF presence
 // Simplified — real implementation queries vendor APIs
 return false;
 }

 static std::wstring GetLowerExtension(const wchar_t* path) {
 std::wstring p(path);
 auto dot = p.rfind(L'.');
 if (dot == std::wstring::npos) return L"";
 std::wstring ext = p.substr(dot);
 for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
 return ext;
 }

 bool m_gpuDecodeAvailable = false;
};

} // namespace Engine
} // namespace ExplorerLens
