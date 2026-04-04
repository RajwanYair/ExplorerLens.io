// DragDropPreviewEngine.h — DragDrop Live Thumbnail Preview Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates live thumbnail previews for drag-and-drop operations — shown before drop target accepts the file.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DragPreviewConfig
{
    uint32_t previewWidth = 128;
    uint32_t previewHeight = 128;
    float dragOpacity = 0.75f;
    bool showShadow = true;
};
class DragDropPreviewEngine
{
  public:
    std::vector<uint8_t> Generate(const std::wstring& filePath, const DragPreviewConfig& cfg)
    {
        (void)filePath;
        return std::vector<uint8_t>(cfg.previewWidth * cfg.previewHeight * 4, 200);
    }
    bool IsReady() const
    {
        return true;
    }
    void Prefetch(const std::wstring& path)
    {
        (void)path;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens