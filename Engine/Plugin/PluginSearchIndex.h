// PluginSearchIndex.h — Local Search Index of Installed Plugins
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains an in-memory inverted index of installed plugins for instant-search
// by display name, author, supported format extensions, or description keywords.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "PluginPackageManifest.h"

namespace ExplorerLens {
namespace Engine {

// ---- Search Query -----------------------------------------------------------

struct PluginSearchQuery {
    std::string  text;              // Full-text search across name/desc/author
    std::string  extensionFilter;   // ".xyz" — only plugins supporting this ext
    bool         enabledOnly = false;
    uint32_t     maxResults  = 20;
};

// ---- Search Result ----------------------------------------------------------

struct PluginSearchHit {
    std::string  pluginId;
    std::string  displayName;
    std::string  version;
    float        score = 0.0f;      // Relevance score [0.0 - 1.0]
    std::vector<std::string> matchedTokens;
};

// ---- PluginSearchIndex ------------------------------------------------------

class PluginSearchIndex {
public:
    PluginSearchIndex();
    ~PluginSearchIndex();

    // Rebuild the full index from the installed plugin list.
    void Rebuild(const std::vector<PluginPackageManifest>& plugins);

    // Incrementally add or update a single plugin entry.
    void Upsert(const PluginPackageManifest& manifest);

    // Remove a plugin from the index.
    void Remove(const std::string& pluginId);

    // Search the index.
    std::vector<PluginSearchHit> Search(const PluginSearchQuery& query) const;

    // Look up all plugins that handle a given file extension.
    std::vector<std::string> FindByExtension(const std::string& ext) const;

    // Return total indexed plugin count.
    size_t IndexedCount() const;

    void Clear();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // Tokenise a string into lowercase words for the inverted index.
    static std::vector<std::string> Tokenize(const std::string& text);
};

} // namespace Engine
} // namespace ExplorerLens
