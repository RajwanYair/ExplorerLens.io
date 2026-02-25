// IntegrationTestSuite.h — Pre-Built Integration Test Suite
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 382
// Copyright (c) 2026 ExplorerLens Project
//
// Pre-defined integration test cases covering all 25 decoder families
// with the data/corpus test archive collection.

#pragma once

#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Test case definition for integration tests
struct IntegrationTestCase {
    const char* name = nullptr;           ///< Test name
    const char* inputFile = nullptr;      ///< Relative path in data/corpus
    const char* expectedDecoder = nullptr; ///< Expected decoder class
    const char* formatFamily = nullptr;   ///< Format category
    uint32_t expectedMinWidth = 1;
    uint32_t expectedMinHeight = 1;
    uint32_t expectedMaxTimeMs = 5000;    ///< Max decode time
    bool requiresGPU = false;
    bool requiresLibrary = false;         ///< Needs external lib (skip if missing)
    const char* requiredFeatureFlag = nullptr; ///< e.g., "HAS_MUPDF"
};

/// Pre-built test suite
class IntegrationTestSuite {
public:
    static IntegrationTestSuite& Instance() {
        static IntegrationTestSuite inst;
        return inst;
    }

    /// Total test case count
    static constexpr uint32_t TEST_CASE_COUNT = 30;

    /// Get test case by index
    static const IntegrationTestCase& GetTestCase(uint32_t index) {
        static const IntegrationTestCase cases[] = {
            // Archives
            { "ZIP_Basic",           "corpus/archives/sample.zip",     "ArchiveDecoder", "Archives",  64, 64, 2000, false, false, nullptr },
            { "7Z_LZMA2",           "corpus/archives/sample.7z",      "ArchiveDecoder", "Archives",  64, 64, 3000, false, false, nullptr },
            { "RAR5_Standard",      "corpus/archives/sample.rar",     "ArchiveDecoder", "Archives",  64, 64, 2000, false, false, nullptr },
            { "TAR_GZ_Unix",        "corpus/archives/sample.tar.gz",  "ArchiveDecoder", "Archives",  64, 64, 2000, false, false, nullptr },

            // Comics
            { "CBZ_ComicBook",      "corpus/comics/sample.cbz",       "ArchiveDecoder", "Comics",    128, 128, 3000, false, false, nullptr },
            { "CBR_ComicBook",      "corpus/comics/sample.cbr",       "ArchiveDecoder", "Comics",    128, 128, 3000, false, false, nullptr },

            // eBooks
            { "EPUB_Standard",      "corpus/ebooks/sample.epub",      "EBookDecoder",   "eBooks",    128, 128, 5000, false, false, nullptr },

            // Modern Images
            { "WebP_Lossy",         "corpus/images/sample.webp",      "WebPDecoder",    "Images",    256, 256, 1000, false, false, nullptr },
            { "AVIF_AV1",           "corpus/images/sample.avif",      "AVIFDecoder",    "Images",    256, 256, 2000, false, false, nullptr },
            { "JXL_Lossless",       "corpus/images/sample.jxl",       "JXLDecoder",     "Images",    256, 256, 1500, false, false, nullptr },
            { "HEIF_HEVC",          "corpus/images/sample.heif",      "HEIFDecoder",    "Images",    256, 256, 2000, false, false, nullptr },
            { "PSD_Layers",         "corpus/images/sample.psd",       "PSDDecoder",     "Images",    256, 256, 3000, false, false, nullptr },
            { "SVG_Vector",         "corpus/images/sample.svg",       "SVGDecoder",     "Images",    256, 256, 1000, false, false, nullptr },
            { "QOI_Fast",           "corpus/images/sample.qoi",       "QOIDecoder",     "Images",    256, 256, 500,  false, false, nullptr },

            // RAW Photos
            { "CR2_CanonRAW",       "corpus/raw/sample.cr2",          "LibRawDecoder",  "RAW Photos", 256, 256, 3000, false, true, nullptr },
            { "NEF_NikonRAW",       "corpus/raw/sample.nef",          "LibRawDecoder",  "RAW Photos", 256, 256, 3000, false, true, nullptr },
            { "DNG_AdobeRAW",       "corpus/raw/sample.dng",          "LibRawDecoder",  "RAW Photos", 256, 256, 3000, false, true, nullptr },

            // Documents
            { "PDF_SinglePage",     "corpus/docs/sample.pdf",         "MuPDFDecoder",   "Documents", 256, 256, 5000, false, true, "HAS_MUPDF" },

            // Fonts
            { "TTF_TrueType",       "corpus/fonts/sample.ttf",        "FontDecoder",    "Fonts",     128, 128, 1000, false, false, nullptr },
            { "OTF_OpenType",       "corpus/fonts/sample.otf",        "FontDecoder",    "Fonts",     128, 128, 1000, false, false, nullptr },

            // 3D Models
            { "STL_Binary",         "corpus/3d/sample.stl",           "CADDecoder",     "3D Models", 256, 256, 5000, true,  false, nullptr },
            { "OBJ_Wavefront",      "corpus/3d/sample.obj",           "CADDecoder",     "3D Models", 256, 256, 5000, true,  false, nullptr },

            // HDR/Specialized
            { "EXR_HDR",            "corpus/hdr/sample.exr",          "HDRDecoder",     "Specialized", 256, 256, 2000, false, false, nullptr },
            { "HDR_Radiance",       "corpus/hdr/sample.hdr",          "HDRDecoder",     "Specialized", 256, 256, 2000, false, false, nullptr },
            { "DDS_BCn",            "corpus/gpu/sample.dds",          "DDSDecoder",     "Specialized", 256, 256, 1000, false, false, nullptr },

            // Error handling
            { "Corrupt_ZIP",        "corpus/error/corrupt.zip",       "ArchiveDecoder", "Error",     0, 0, 2000, false, false, nullptr },
            { "Truncated_WebP",     "corpus/error/truncated.webp",    "WebPDecoder",    "Error",     0, 0, 1000, false, false, nullptr },
            { "ZeroByte_File",      "corpus/error/zero.bin",          "None",           "Error",     0, 0, 500,  false, false, nullptr },
            { "PasswordProtected",  "corpus/error/password.zip",      "ArchiveDecoder", "Error",     0, 0, 1000, false, false, nullptr },
            { "LargeArchive_1GB",   "corpus/stress/large.7z",         "ArchiveDecoder", "Stress",    64, 64, 10000, false, false, nullptr },
        };

        static const IntegrationTestCase empty{};
        return index < TEST_CASE_COUNT ? cases[index] : empty;
    }

    /// Count tests by format family
    uint32_t CountByFamily(const char* family) const {
        uint32_t count = 0;
        for (uint32_t i = 0; i < TEST_CASE_COUNT; ++i) {
            const auto& tc = GetTestCase(i);
            if (tc.formatFamily && family &&
                strcmp(tc.formatFamily, family) == 0) {
                count++;
            }
        }
        return count;
    }

    /// Check if test requires a feature flag that may not be available
    static bool RequiresFeatureFlag(uint32_t index) {
        const auto& tc = GetTestCase(index);
        return tc.requiredFeatureFlag != nullptr;
    }

private:
    IntegrationTestSuite() = default;
};

} // namespace Engine
} // namespace ExplorerLens
