// FormatFamilyResolver.h — Format Family Resolver
// Copyright (c) 2026 ExplorerLens Project
//
// Classifies file formats into top-level families (Image, Video, Audio, etc.)
// and builds a hierarchical format graph for routing and UI organisation.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class FormatFamily : uint8_t {
    Image      = 0,
    Video      = 1,
    Audio      = 2,
    Document   = 3,
    Archive    = 4,
    Scientific = 5
};

struct FormatNode {
    std::string              name;
    FormatFamily             family     = FormatFamily::Image;
    std::vector<std::string> extensions;
};

class FormatFamilyResolver {
public:
    FormatFamilyResolver() = default;
};

} // namespace Engine
} // namespace ExplorerLens
