// ArchiveHierarchyMapRenderer.h — Archive Directory Treemap Visualization
// Copyright (c) 2026 ExplorerLens Project
//
// Builds an in-memory tree from archive path/size entries and computes a
// squarified treemap layout for thumbnail rendering.  Pure data-structure
// logic — performs no archive I/O.
//
#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

/// Axis-aligned rectangle used by the treemap layout.
struct TreemapRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

/// A single node in the archive hierarchy.
struct HierarchyNode {
    std::wstring   name;
    uint64_t       ownSize = 0;   // size of the file (0 for dirs)
    uint64_t       totalSize = 0;   // sum of subtree
    uint32_t       depth = 0;
    bool           isDirectory = false;
    TreemapRect    rect{};
    std::vector<size_t> childIndices;
};

class ArchiveHierarchyMapRenderer {
public:
    ArchiveHierarchyMapRenderer() {
        // index 0 = synthetic root
        HierarchyNode root;
        root.name = L"<root>";
        root.isDirectory = true;
        m_nodes.push_back(root);
    }

    // ── Building the tree ────────────────────────────────────────────

    /// Add an entry using a forward-slash-separated path and its file size.
    /// Intermediate directories are created automatically.
    void AddEntry(const std::wstring& path, uint64_t fileSize) {
        std::vector<std::wstring> parts = SplitPath(path);
        if (parts.empty()) return;

        size_t parent = 0; // root
        for (size_t i = 0; i < parts.size(); ++i) {
            bool isLast = (i + 1 == parts.size());
            size_t child = FindChild(parent, parts[i]);
            if (child == SIZE_MAX) {
                child = CreateNode(parts[i], parent,
                    isLast ? false : true,
                    isLast ? fileSize : 0);
            }
            parent = child;
        }
    }

    // ── Queries ──────────────────────────────────────────────────────

    /// Total number of nodes including the synthetic root.
    uint32_t GetNodeCount() const {
        return static_cast<uint32_t>(m_nodes.size());
    }

    /// Maximum depth in the tree (root = 0).
    uint32_t GetMaxDepth() const {
        uint32_t maxD = 0;
        for (const auto& n : m_nodes)
            maxD = (std::max)(maxD, n.depth);
        return maxD;
    }

    /// Total bytes across all file entries.
    uint64_t GetTotalSize() const {
        return m_nodes.empty() ? 0 : m_nodes[0].totalSize;
    }

    /// Number of leaf (file) nodes.
    uint32_t GetFileCount() const {
        uint32_t count = 0;
        for (const auto& n : m_nodes)
            if (!n.isDirectory) ++count;
        return count;
    }

    /// Number of directory nodes (including synthetic root).
    uint32_t GetDirectoryCount() const {
        uint32_t count = 0;
        for (const auto& n : m_nodes)
            if (n.isDirectory) ++count;
        return count;
    }

    /// Read-only access to a node by index.
    const HierarchyNode& GetNode(size_t index) const {
        return m_nodes[index];
    }

    // ── Layout ───────────────────────────────────────────────────────

    /// Compute `totalSize` bottom-up, then lay out a squarified treemap into
    /// the given pixel dimensions.
    void CalculateLayout(float width, float height) {
        PropagateSize(0);
        TreemapRect bounds{ 0.0f, 0.0f, width, height };
        m_nodes[0].rect = bounds;
        LayoutChildren(0, bounds);
    }

    /// Return the flat list of all leaf rectangles (files) after layout.
    std::vector<TreemapRect> GetLeafRects() const {
        std::vector<TreemapRect> rects;
        for (const auto& n : m_nodes)
            if (!n.isDirectory)
                rects.push_back(n.rect);
        return rects;
    }

    /// Map a depth value to an ARGB colour for visualisation.
    static uint32_t ColorForDepth(uint32_t depth) {
        // Rotate through a small palette
        static const uint32_t palette[] = {
            0xFF3498DB, 0xFF2ECC71, 0xFFE67E22,
            0xFF9B59B6, 0xFFE74C3C, 0xFF1ABC9C,
            0xFFF39C12, 0xFF2980B9
        };
        constexpr size_t N = sizeof(palette) / sizeof(palette[0]);
        return palette[depth % N];
    }

private:
    std::vector<HierarchyNode> m_nodes;

    // ── Internals ────────────────────────────────────────────────────

    std::vector<std::wstring> SplitPath(const std::wstring& path) const {
        std::vector<std::wstring> parts;
        std::wstring segment;
        for (wchar_t ch : path) {
            if (ch == L'/' || ch == L'\\') {
                if (!segment.empty()) {
                    parts.push_back(segment);
                    segment.clear();
                }
            }
            else {
                segment += ch;
            }
        }
        if (!segment.empty())
            parts.push_back(segment);
        return parts;
    }

    size_t FindChild(size_t parentIdx, const std::wstring& name) const {
        for (size_t ci : m_nodes[parentIdx].childIndices)
            if (m_nodes[ci].name == name)
                return ci;
        return SIZE_MAX;
    }

    size_t CreateNode(const std::wstring& name, size_t parentIdx,
        bool isDir, uint64_t size) {
        size_t idx = m_nodes.size();
        HierarchyNode node;
        node.name = name;
        node.ownSize = size;
        node.isDirectory = isDir;
        node.depth = m_nodes[parentIdx].depth + 1;
        m_nodes.push_back(node);
        m_nodes[parentIdx].childIndices.push_back(idx);
        return idx;
    }

    uint64_t PropagateSize(size_t idx) {
        auto& node = m_nodes[idx];
        node.totalSize = node.ownSize;
        for (size_t ci : node.childIndices)
            node.totalSize += PropagateSize(ci);
        return node.totalSize;
    }

    /// Simplified strip-based treemap layout (slice-and-dice).
    void LayoutChildren(size_t idx, const TreemapRect& bounds) {
        auto& node = m_nodes[idx];
        if (node.childIndices.empty()) return;

        uint64_t total = node.totalSize;
        if (total == 0) return;

        // Sort children by totalSize descending for better packing
        std::vector<size_t> sorted = node.childIndices;
        std::sort(sorted.begin(), sorted.end(),
            [this](size_t a, size_t b) {
                return m_nodes[a].totalSize > m_nodes[b].totalSize;
            });

        bool horizontal = bounds.w >= bounds.h;
        float offset = 0.0f;
        float span = horizontal ? bounds.w : bounds.h;

        for (size_t ci : sorted) {
            float frac = static_cast<float>(
                static_cast<double>(m_nodes[ci].totalSize) /
                static_cast<double>(total));
            float size = span * frac;

            TreemapRect child{};
            if (horizontal) {
                child = { bounds.x + offset, bounds.y, size, bounds.h };
            }
            else {
                child = { bounds.x, bounds.y + offset, bounds.w, size };
            }
            m_nodes[ci].rect = child;
            offset += size;

            if (m_nodes[ci].isDirectory)
                LayoutChildren(ci, child);
        }
    }
};

} // namespace Engine
} // namespace ExplorerLens
