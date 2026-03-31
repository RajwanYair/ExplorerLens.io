// FormatGroupManager.h — Supported format enumeration and grouping
// Copyright (c) 2026 ExplorerLens Project
//
// Manages the registry of all supported file formats, their enabled state,
// and logical grouping (images, archives, documents, etc.).
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct FormatGroup {
    std::string name;
    std::vector<std::string> extensions;
    bool enabled = true;
};

class FormatGroupManager {
public:
    FormatGroupManager() {
        m_groups = {
            {"Images",    {".jpg",".jpeg",".png",".bmp",".gif",".tiff",".webp",".avif",".jxl",".heic"}, true},
            {"RAW",       {".cr2",".nef",".arw",".dng",".orf",".rw2"}, true},
            {"Archives",  {".zip",".rar",".7z",".tar",".gz",".bz2",".xz",".zst"}, true},
            {"Documents", {".pdf",".epub",".cbz",".cbr"}, true},
            {"Video",     {".mp4",".mkv",".avi",".mov",".wmv"}, true},
            {"CAD",       {".dwg",".dxf",".step",".iges"}, true},
            {"3D",        {".gltf",".glb",".obj",".fbx",".stl"}, true},
            {"Scientific",{".fits",".dicom",".nifti"}, true},
            {"Vector",    {".svg",".eps",".ai"}, true},
            {"Font",      {".ttf",".otf",".woff",".woff2"}, true},
        };
    }

    int GetTotalFormats() const {
        int total = 0;
        for (auto& g : m_groups)
            total += static_cast<int>(g.extensions.size());
        return total;
    }

    int GetEnabledFormats() const {
        int count = 0;
        for (auto& g : m_groups)
            if (g.enabled) count += static_cast<int>(g.extensions.size());
        return count;
    }

    std::vector<FormatGroup> GetAllGroups() const { return m_groups; }

private:
    std::vector<FormatGroup> m_groups;
};

} // namespace Engine
} // namespace ExplorerLens
