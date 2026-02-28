//==============================================================================
// AnimatedThumbnailEngine
//==============================================================================

#include "AnimatedThumbnailEngine.h"
#include <algorithm>
#include <chrono>

namespace ExplorerLens { namespace Engine {

AnimatedThumbnailEngine::AnimatedThumbnailEngine() {}

AnimationInfo AnimatedThumbnailEngine::Probe(const std::wstring& filePath) const {
 AnimationInfo info;
 info.format = DetectFormat(filePath);

 // Probe based on format magic bytes
 FILE* f = nullptr;
 _wfopen_s(&f, filePath.c_str(), L"rb");
 if (!f) return info;

 // Read header to determine frame count
 uint8_t header[16] = {};
 fread(header, 1, sizeof(header), f);

 if (info.format == AnimatedFormat::GIF) {
 // GIF: parse logical screen descriptor
 if (header[0] == 'G' && header[1] == 'I' && header[2] == 'F') {
 info.width = header[6] | (header[7] << 8);
 info.height = header[8] | (header[9] << 8);
 info.frameCount = 1; // Would scan for frame separators
 }
 }

 fclose(f);
 return info;
}

FrameExtractionResult AnimatedThumbnailEngine::ExtractFrame(
 const std::wstring& filePath, FrameStrategy strategy) const
{
 FrameExtractionResult result;
 auto start = std::chrono::high_resolution_clock::now();

 auto info = Probe(filePath);
 if (info.frameCount == 0) {
 return result;
 }

 result.selectedFrame = SelectBestFrame(info.frameCount, strategy);
 result.width = info.width;
 result.height = info.height;
 result.success = info.frameCount > 0;

 auto end = std::chrono::high_resolution_clock::now();
 result.extractionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
 return result;
}

AnimatedFormat AnimatedThumbnailEngine::DetectFormat(const std::wstring& filePath) {
 auto ext = filePath.substr(filePath.find_last_of(L'.') + 1);
 for (auto& c : ext) c = towlower(c);

 if (ext == L"gif") return AnimatedFormat::GIF;
 if (ext == L"apng" || ext == L"png") return AnimatedFormat::APNG;
 if (ext == L"webp") return AnimatedFormat::WebPAnim;
 if (ext == L"avif" || ext == L"avifs") return AnimatedFormat::AVIFSeq;
 if (ext == L"jxl") return AnimatedFormat::JXLAnim;
 if (ext == L"flif") return AnimatedFormat::FLIF;
 return AnimatedFormat::GIF;
}

uint32_t AnimatedThumbnailEngine::SelectBestFrame(
 uint32_t totalFrames, FrameStrategy strategy)
{
 if (totalFrames == 0) return 0;
 switch (strategy) {
 case FrameStrategy::First: return 0;
 case FrameStrategy::Middle: return totalFrames / 2;
 case FrameStrategy::Keyframe: return 0; // First keyframe
 case FrameStrategy::MostDetail: return (std::min)(totalFrames / 3, totalFrames - 1);
 case FrameStrategy::Composite: return 0;
 default: return 0;
 }
}

const wchar_t* AnimatedThumbnailEngine::GetFormatName(AnimatedFormat format) {
 switch (format) {
 case AnimatedFormat::GIF: return L"GIF";
 case AnimatedFormat::APNG: return L"APNG";
 case AnimatedFormat::WebPAnim: return L"WebP Animation";
 case AnimatedFormat::AVIFSeq: return L"AVIF Sequence";
 case AnimatedFormat::JXLAnim: return L"JPEG XL Animation";
 case AnimatedFormat::FLIF: return L"FLIF";
 default: return L"Unknown";
 }
}

const wchar_t* AnimatedThumbnailEngine::GetStrategyName(FrameStrategy strategy) {
 switch (strategy) {
 case FrameStrategy::First: return L"First Frame";
 case FrameStrategy::Middle: return L"Middle Frame";
 case FrameStrategy::Keyframe: return L"Keyframe";
 case FrameStrategy::MostDetail: return L"Most Detail";
 case FrameStrategy::Composite: return L"Composite";
 default: return L"Unknown";
 }
}

}} // namespace ExplorerLens::Engine

