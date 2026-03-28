#pragma once
//==============================================================================
// EBookDecoder
// E-Book format decoder (EPUB, MOBI, FB2, AZW3) — cover extraction
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class DecoderEBookFormat : uint8_t {
 EPUB = 0,
 MOBI,
 FB2,
 AZW3,
 CBZ, // Comic Book ZIP (duplicate for ebook gallery)
 DJVU,
 LIT,
 Unknown
};

struct EBookMetadata {
 std::wstring title;
 std::wstring author;
 std::wstring publisher;
 std::wstring language;
 std::wstring isbn;
 uint32_t pageCount = 0;
 DecoderEBookFormat format = DecoderEBookFormat::Unknown;
 bool hasCoverImage = false;
 uint64_t fileSizeBytes = 0;
};

struct EBookCoverResult {
 bool success = false;
 uint32_t width = 0;
 uint32_t height = 0;
 std::vector<uint8_t> coverData; // RGBA pixels
 std::wstring coverMimeType;
 std::wstring error;
};

//------------------------------------------------------------------------------
class EBookDecoder {
public:
 EBookDecoder();
 ~EBookDecoder() = default;

 // Format detection
 static bool IsEBookFile(const uint8_t* data, size_t size);
 static DecoderEBookFormat DetectFormat(const uint8_t* data, size_t size);
 static uint32_t GetExtensionCount();
 static std::vector<std::wstring> GetExtensions();

 // Metadata
 EBookMetadata ReadMetadata(const uint8_t* data, size_t size) const;

 // Cover extraction
 EBookCoverResult ExtractCover(const uint8_t* data, size_t size) const;

 // EPUB helpers
 static bool IsEPUB(const uint8_t* data, size_t size);
 static bool IsMOBI(const uint8_t* data, size_t size);

 // Static helpers
 static const wchar_t* GetFormatName(DecoderEBookFormat format);
 static const wchar_t* GetFormatExtension(DecoderEBookFormat format);
 static const wchar_t* GetFormatMimeType(DecoderEBookFormat format);

private:
 bool ExtractEPUBCover(const uint8_t* data, size_t size,
 std::vector<uint8_t>& coverOut) const;
 bool ParseOPFMetadata(const std::string& opfContent, EBookMetadata& meta) const;
};

}} // namespace ExplorerLens::Engine

