// FormatCategoryManager.h — Format Category Classification Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Groups supported file formats into logical categories for UI filtering,
// batch operations, and decoder routing.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class FormatCategoryGroup : uint8_t {
    Archives,
    Images,
    Video,
    Audio,
    Documents,
    Scientific,
    COUNT = 6
};

class FormatCategoryManager {
public:
    static size_t CategoryCount() noexcept {
        return static_cast<size_t>(FormatCategoryGroup::COUNT);
    }
    static const wchar_t *CategoryName(FormatCategoryGroup g) noexcept {
        switch (g) {
        case FormatCategoryGroup::Archives:   return L"Archives";
        case FormatCategoryGroup::Images:     return L"Images";
        case FormatCategoryGroup::Video:      return L"Video";
        case FormatCategoryGroup::Audio:      return L"Audio";
        case FormatCategoryGroup::Documents:  return L"Documents";
        case FormatCategoryGroup::Scientific: return L"Scientific";
        default: return L"Unknown";
        }
    }
};

} // namespace Engine
} // namespace ExplorerLens
