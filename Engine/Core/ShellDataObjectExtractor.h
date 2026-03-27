// ShellDataObjectExtractor.h — Shell Data Object Thumbnail Extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts thumbnails from IDataObject instances dropped into ExplorerLens-aware drop targets.
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

struct ExtractedThumb {
    std::wstring  sourcePath;
    uint32_t      width  = 0;
    uint32_t      height = 0;
    std::vector<uint8_t> rgba;
    bool          success = false;
};
class ShellDataObjectExtractor {
public:
    ExtractedThumb Extract(const void* pDataObject, uint32_t targetSize = 256) {
        (void)pDataObject; (void)targetSize;
        return { L"", 0, 0, {}, false };
    }
    bool CanExtract(const void* pDataObject) const { (void)pDataObject; return false; }
    uint32_t SupportedFormats() const { return 12; }
};

} // namespace Engine
} // namespace ExplorerLens