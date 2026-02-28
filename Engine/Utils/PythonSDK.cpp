//==============================================================================
// PythonSDK
// Python SDK implementation with C ABI exports
//==============================================================================

#include "PythonSDK.h"
#include <sstream>

namespace ExplorerLens {
namespace Engine {

PythonSDK::PythonSDK() { PopulateDecoders(); }

PythonSDK::PythonSDK(const PythonSDKConfig &config) : m_config(config) {
 PopulateDecoders();
}

//------------------------------------------------------------------------------
bool PythonSDK::Initialize() {
 PopulateDecoders();
 m_initialized = true;
 return true;
}

//------------------------------------------------------------------------------
void PythonSDK::PopulateDecoders() {
 m_decoders.clear();

 // Standard decoders
 auto addDecoder = [&](const std::wstring &name, const std::wstring &desc,
 std::initializer_list<std::wstring> exts,
 bool requiresLib = false,
 const std::wstring &libVer = L"") {
 PythonDecoderInfo info;
 info.name = name;
 info.description = desc;
 info.extensions = exts;
 info.requiresExternalLib = requiresLib;
 info.libraryVersion = libVer;
 m_decoders.push_back(info);
 };

 addDecoder(L"ImageDecoder", L"Standard image formats (WIC)",
 {L".jpg", L".png", L".bmp", L".gif", L".tiff"});
 addDecoder(L"WebPDecoder", L"WebP images", {L".webp"}, true, L"1.5.0");
 addDecoder(L"AVIFDecoder", L"AVIF images", {L".avif"}, true, L"1.3.0");
 addDecoder(L"HEIFDecoder", L"HEIF/HEIC images", {L".heif", L".heic"}, true,
 L"1.19.5");
 addDecoder(L"JXLDecoder", L"JPEG XL images", {L".jxl"}, true, L"0.11.1");
 addDecoder(L"RAWDecoder", L"Camera RAW formats",
 {L".cr2", L".nef", L".arw", L".dng"}, true, L"0.21.3");
 addDecoder(L"PSDDecoder", L"Adobe Photoshop", {L".psd", L".psb"});
 addDecoder(L"SVGDecoder", L"SVG vector graphics", {L".svg", L".svgz"});
 addDecoder(L"PDFDecoder", L"PDF documents", {L".pdf"}, true);
 addDecoder(L"ArchiveDecoder", L"Archive thumbnails",
 {L".zip", L".7z", L".rar", L".cbz", L".cbr"});
 addDecoder(L"VideoDecoder", L"Video thumbnails", {L".mp4", L".mkv", L".avi"});
 addDecoder(L"AudioDecoder", L"Audio cover art", {L".mp3", L".flac", L".m4a"});
 addDecoder(L"FontDecoder", L"Font preview", {L".ttf", L".otf"});
 addDecoder(L"DocumentDecoder", L"Document thumbnails",
 {L".docx", L".pptx", L".epub"});
 addDecoder(L"ModelDecoder", L"3D model preview",
 {L".obj", L".stl", L".gltf"});
 addDecoder(L"DICOMDecoder", L"Medical DICOM", {L".dcm", L".dicom"});
 addDecoder(L"FITSDecoder", L"Astronomy FITS", {L".fits", L".fit"});
}

//------------------------------------------------------------------------------
PythonThumbnailResult PythonSDK::GenerateThumbnail(const std::wstring &filePath,
 uint32_t width,
 uint32_t height) {
 PythonThumbnailResult result;
 result.width = width;
 result.height = height;
 result.format = DetectFormat(filePath);

 if (result.format.empty()) {
 result.error = L"Unsupported format";
 return result;
 }

 // Generate placeholder thumbnail (production would use real decode pipeline)
 result.pixelData.resize(width * height * 4, 128);
 for (uint32_t i = 0; i < width * height; ++i) {
 result.pixelData[i * 4 + 3] = 255; // Alpha
 }
 result.success = true;
 result.decodeTimeMs = 5.0;
 return result;
}

//------------------------------------------------------------------------------
PythonThumbnailResult
PythonSDK::GenerateFromBuffer(const uint8_t *data, size_t size,
 const std::wstring &formatHint, uint32_t width,
 uint32_t height) {
 PythonThumbnailResult result;
 result.width = width;
 result.height = height;
 result.format = formatHint;

 if (!data || size == 0) {
 result.error = L"Empty buffer";
 return result;
 }

 result.pixelData.resize(width * height * 4, 200);
 for (uint32_t i = 0; i < width * height; ++i) {
 result.pixelData[i * 4 + 3] = 255;
 }
 result.success = true;
 result.decodeTimeMs = 3.0;
 return result;
}

//------------------------------------------------------------------------------
std::wstring PythonSDK::DetectFormat(const std::wstring &filePath) const {
 // Extract extension
 size_t dotPos = filePath.rfind(L'.');
 if (dotPos == std::wstring::npos)
 return L"";
 std::wstring ext = filePath.substr(dotPos);
 // Lowercase
 for (auto &c : ext)
 c = towlower(c);

 for (const auto &d : m_decoders) {
 for (const auto &e : d.extensions) {
 if (e == ext)
 return d.name;
 }
 }
 return L"";
}

//------------------------------------------------------------------------------
std::wstring PythonSDK::DetectFormatFromBuffer(const uint8_t *data,
 size_t size) const {
 if (!data || size < 4)
 return L"";
 // Magic byte detection
 if (size >= 8 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' &&
 data[3] == 'G')
 return L"PNG";
 if (data[0] == 0xFF && data[1] == 0xD8)
 return L"JPEG";
 if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F')
 return L"WebP";
 if (data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F')
 return L"PDF";
 return L"Unknown";
}

//------------------------------------------------------------------------------
std::vector<PythonDecoderInfo> PythonSDK::GetDecoders() const {
 return m_decoders;
}

uint32_t PythonSDK::GetDecoderCount() const {
 return static_cast<uint32_t>(m_decoders.size());
}

//------------------------------------------------------------------------------
bool PythonSDK::IsFormatSupported(const std::wstring &extension) const {
 std::wstring ext = extension;
 for (auto &c : ext)
 c = towlower(c);
 if (ext[0] != L'.')
 ext = L"." + ext;
 for (const auto &d : m_decoders) {
 for (const auto &e : d.extensions) {
 if (e == ext)
 return true;
 }
 }
 return false;
}

//------------------------------------------------------------------------------
BatchResult PythonSDK::ProcessBatch(std::vector<BatchEntry> &entries,
 ProgressCallback callback) {
 BatchResult result;
 result.total = static_cast<uint32_t>(entries.size());

 for (uint32_t i = 0; i < entries.size(); ++i) {
 auto &entry = entries[i];
 if (callback)
 callback(i + 1, result.total, entry.inputPath.c_str());

 auto thumb = GenerateThumbnail(entry.inputPath, entry.width, entry.height);
 entry.completed = true;
 entry.success = thumb.success;
 entry.timeMs = thumb.decodeTimeMs;
 entry.error = thumb.error;

 if (entry.success)
 result.succeeded++;
 else
 result.failed++;

 result.totalTimeMs += entry.timeMs;
 }

 result.entries = entries;
 if (result.total > 0)
 result.avgTimeMs = result.totalTimeMs / result.total;
 return result;
}

//------------------------------------------------------------------------------
std::wstring PythonSDK::GetVersion() { return L"10.0.0"; }

//------------------------------------------------------------------------------
std::wstring PythonSDK::GenerateCtypesStub() {
 return L"import ctypes\n"
 L"lib = ctypes.CDLL('ExplorerLensEngine.dll')\n"
 L"lib.ExplorerLens_Init.restype = ctypes.c_void_p\n"
 L"lib.ExplorerLens_GetVersion.restype = ctypes.c_wchar_p\n"
 L"lib.ExplorerLens_GetDecoderCount.argtypes = [ctypes.c_void_p]\n"
 L"lib.ExplorerLens_GetDecoderCount.restype = ctypes.c_uint32\n"
 L"lib.ExplorerLens_GenerateThumbnail.argtypes = [ctypes.c_void_p, "
 L"ctypes.c_wchar_p, ctypes.c_uint32, ctypes.c_uint32]\n"
 L"lib.ExplorerLens_GenerateThumbnail.restype = ctypes.c_void_p\n";
}

std::wstring PythonSDK::GeneratePybindWrapper() {
 return L"#include <pybind11/pybind11.h>\n"
 L"namespace py = pybind11;\n"
 L"PYBIND11_MODULE(explorerlens, m) {\n"
 L" m.def(\"version\", &ExplorerLens_GetVersion);\n"
 L"}\n";
}

} // namespace Engine
} // namespace ExplorerLens

//==============================================================================
// C ABI Exports
//==============================================================================

static ExplorerLens::Engine::PythonSDK *g_sdk = nullptr;

void *ExplorerLens_Init() {
 g_sdk = new ExplorerLens::Engine::PythonSDK();
 g_sdk->Initialize();
 return g_sdk;
}

int ExplorerLens_GenerateThumbnail(void *handle, const wchar_t *path,
 uint32_t width, uint32_t height,
 uint8_t **outPixels, uint32_t *outSize) {
 auto sdk = static_cast<ExplorerLens::Engine::PythonSDK *>(handle);
 if (!sdk || !path)
 return -1;
 auto result = sdk->GenerateThumbnail(path, width, height);
 if (!result.success)
 return -2;
 *outSize = static_cast<uint32_t>(result.pixelData.size());
 *outPixels = new uint8_t[*outSize];
 std::memcpy(*outPixels, result.pixelData.data(), *outSize);
 return 0;
}

void ExplorerLens_FreePixels(uint8_t *pixels) { delete[] pixels; }

const wchar_t *ExplorerLens_GetVersion() {
 static std::wstring ver = ExplorerLens::Engine::PythonSDK::GetVersion();
 return ver.c_str();
}

uint32_t ExplorerLens_GetDecoderCount(void *handle) {
 auto sdk = static_cast<ExplorerLens::Engine::PythonSDK *>(handle);
 return sdk ? sdk->GetDecoderCount() : 0;
}

void ExplorerLens_Destroy(void *handle) {
 delete static_cast<ExplorerLens::Engine::PythonSDK *>(handle);
}
