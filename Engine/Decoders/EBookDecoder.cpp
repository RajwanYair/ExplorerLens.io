//==============================================================================
// EBookDecoder
//==============================================================================

#include "EBookDecoder.h"
#include <algorithm>
#include <cstring>

namespace ExplorerLens { namespace Engine {

EBookDecoder::EBookDecoder() {}

bool EBookDecoder::IsEBookFile(const uint8_t* data, size_t size) {
 if (!data || size < 4) return false;
 return IsEPUB(data, size) || IsMOBI(data, size);
}

EBookFormat EBookDecoder::DetectFormat(const uint8_t* data, size_t size) {
 if (!data || size < 4) return EBookFormat::Unknown;
 if (IsEPUB(data, size)) return EBookFormat::EPUB;
 if (IsMOBI(data, size)) return EBookFormat::MOBI;
 // FB2 is XML-based
 if (size >= 5 && memcmp(data, "<?xml", 5) == 0) {
 // Check for FictionBook root element
 std::string content(reinterpret_cast<const char*>(data),
 std::min(size, (size_t)1024));
 if (content.find("FictionBook") != std::string::npos)
 return EBookFormat::FB2;
 }
 return EBookFormat::Unknown;
}

bool EBookDecoder::IsEPUB(const uint8_t* data, size_t size) {
 // EPUB is ZIP with "mimetype" as first entry containing "application/epub+zip"
 if (size < 30) return false;
 return (data[0] == 'P' && data[1] == 'K' && data[2] == 0x03 && data[3] == 0x04);
 // Full check would verify mimetype entry
}

bool EBookDecoder::IsMOBI(const uint8_t* data, size_t size) {
 // MOBI/PalmDOC: "BOOKMOBI" at offset 60
 if (size < 68) return false;
 return (memcmp(data + 60, "BOOKMOBI", 8) == 0);
}

uint32_t EBookDecoder::GetExtensionCount() {
 return 7;
}

std::vector<std::wstring> EBookDecoder::GetExtensions() {
 return { L".epub", L".mobi", L".fb2", L".azw3", L".azw", L".lit", L".djvu" };
}

EBookMetadata EBookDecoder::ReadMetadata(const uint8_t* data, size_t size) const {
 EBookMetadata meta;
 meta.format = DetectFormat(data, size);
 meta.fileSizeBytes = size;
 if (meta.format == EBookFormat::EPUB) {
 meta.hasCoverImage = true; // Most EPUBs have covers
 }
 return meta;
}

EBookCoverResult EBookDecoder::ExtractCover(const uint8_t* data, size_t size) const {
 EBookCoverResult result;
 if (!data || size == 0) {
 result.error = L"No data provided";
 return result;
 }
 auto format = DetectFormat(data, size);
 if (format == EBookFormat::EPUB) {
 result.success = ExtractEPUBCover(data, size, result.coverData);
 }
 return result;
}

bool EBookDecoder::ExtractEPUBCover(const uint8_t* /*data*/, size_t /*size*/,
 std::vector<uint8_t>& /*coverOut*/) const {
 // Placeholder: needs ZIP extraction + OPF parsing to find cover image
 return false;
}

bool EBookDecoder::ParseOPFMetadata(const std::string& opfContent, EBookMetadata& meta) const {
 if (opfContent.empty()) return false;
 // Simple title extraction
 auto titleStart = opfContent.find("<dc:title>");
 if (titleStart != std::string::npos) {
 auto titleEnd = opfContent.find("</dc:title>", titleStart);
 if (titleEnd != std::string::npos) {
 std::string title = opfContent.substr(titleStart + 10, titleEnd - titleStart - 10);
 meta.title = std::wstring(title.begin(), title.end());
 }
 }
 return true;
}

const wchar_t* EBookDecoder::GetFormatName(EBookFormat format) {
 switch (format) {
 case EBookFormat::EPUB: return L"EPUB";
 case EBookFormat::MOBI: return L"MOBI";
 case EBookFormat::FB2: return L"FB2";
 case EBookFormat::AZW3: return L"AZW3";
 case EBookFormat::CBZ: return L"CBZ";
 case EBookFormat::DJVU: return L"DJVU";
 case EBookFormat::LIT: return L"LIT";
 default: return L"Unknown";
 }
}

const wchar_t* EBookDecoder::GetFormatExtension(EBookFormat format) {
 switch (format) {
 case EBookFormat::EPUB: return L".epub";
 case EBookFormat::MOBI: return L".mobi";
 case EBookFormat::FB2: return L".fb2";
 case EBookFormat::AZW3: return L".azw3";
 case EBookFormat::CBZ: return L".cbz";
 case EBookFormat::DJVU: return L".djvu";
 case EBookFormat::LIT: return L".lit";
 default: return L".bin";
 }
}

const wchar_t* EBookDecoder::GetFormatMimeType(EBookFormat format) {
 switch (format) {
 case EBookFormat::EPUB: return L"application/epub+zip";
 case EBookFormat::MOBI: return L"application/x-mobipocket-ebook";
 case EBookFormat::FB2: return L"application/x-fictionbook+xml";
 case EBookFormat::AZW3: return L"application/vnd.amazon.ebook";
 default: return L"application/octet-stream";
 }
}

}} // namespace ExplorerLens::Engine

