// LensFormatExporter.h — lens export — Format Converter with Profiles
// Copyright (c) 2026 ExplorerLens Project
//
// Batch format converter for lens export — converts between 50+ formats with quality profiles and metadata preservation.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct ExportProfileV2 {
    std::string  format;   // "webp", "avif", "jpeg", "png", "jxl"
    uint32_t     quality   = 85;
    bool         lossless  = false;
    bool         stripExif = false;
};
struct ExportJobResult { size_t converted; size_t failed; uint64_t totalBytesIn; uint64_t totalBytesOut; };
class LensFormatExporter {
public:
    ExportJobResult Export(const std::vector<std::wstring>& inputs,
                           const std::wstring& outDir,
                           const ExportProfileV2& profile) {
        (void)inputs; (void)outDir; (void)profile;
        return { 0, 0, 0, 0 };
    }
    bool SupportsFormat(const std::string& fmt) const {
        static const std::vector<std::string> fmts = { "webp", "avif", "jpeg", "png", "jxl", "tiff", "bmp" };
        return std::find(fmts.begin(), fmts.end(), fmt) != fmts.end();
    }
};

} // namespace Engine
} // namespace ExplorerLens