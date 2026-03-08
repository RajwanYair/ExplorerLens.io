// ContainerPreviewDecoder.h — Archive container content preview
// Copyright (c) 2026 ExplorerLens Project
//
// Generates preview thumbnails for archive containers by extracting and
// compositing the top-N image entries into a collage view.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ContainerPreviewDecoderConfig {
    bool enabled = true;
    uint32_t maxPreviewItems = 4;
    uint32_t collageSize = 256;
    std::string label = "ContainerPreviewDecoder";
};

class ContainerPreviewDecoder {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ContainerPreviewDecoderConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct EntryInfo {
        std::string name;
        uint64_t size = 0;
        bool isImage = false;
    };

    std::vector<EntryInfo> SelectPreviewEntries(const std::vector<EntryInfo>& entries) const {
        std::vector<EntryInfo> selected;
        for (const auto& e : entries) {
            if (e.isImage) {
                selected.push_back(e);
                if (selected.size() >= m_config.maxPreviewItems) break;
            }
        }
        return selected;
    }

    struct CollageLayout {
        uint32_t cols = 2, rows = 2;
        uint32_t cellWidth = 128, cellHeight = 128;
    };

    CollageLayout CalculateLayout(uint32_t itemCount) const {
        CollageLayout layout;
        if (itemCount <= 1) { layout.cols = 1; layout.rows = 1; }
        else if (itemCount <= 2) { layout.cols = 2; layout.rows = 1; }
        else { layout.cols = 2; layout.rows = 2; }
        layout.cellWidth = m_config.collageSize / layout.cols;
        layout.cellHeight = m_config.collageSize / layout.rows;
        return layout;
    }

private:
    bool m_initialized = false;
    ContainerPreviewDecoderConfig m_config;
};

}
} // namespace ExplorerLens::Engine
