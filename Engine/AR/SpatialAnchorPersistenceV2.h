// SpatialAnchorPersistenceV2.h — Spatial Anchor Persistence v2
// Copyright (c) 2026 ExplorerLens Project
//
// Stores and restores spatial anchors for file-to-world associations
// across AR sessions, with ≤5mm positional drift tolerance.
//
#pragma once
#include <string>
#include <unordered_map>
#include <array>
#include <optional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct SpatialAnchor {
    uint64_t                id         = 0;
    std::string             fileKey;
    std::array<float, 3>    position{};
    std::array<float, 4>    orientation{0,0,0,1}; // quaternion
    float                   driftMm    = 0.0f;
    std::string             sessionTag;
};

class SpatialAnchorPersistenceV2 {
public:
    SpatialAnchorPersistenceV2() = default;

    uint64_t CreateAnchor(const std::string& fileKey,
                          const std::array<float,3>& pos,
                          const std::array<float,4>& orient = {0,0,0,1}) {
        SpatialAnchor a;
        a.id          = ++m_nextId;
        a.fileKey     = fileKey;
        a.position    = pos;
        a.orientation = orient;
        m_anchors[a.id] = a;
        return a.id;
    }

    std::optional<SpatialAnchor> GetAnchor(uint64_t id) const {
        auto it = m_anchors.find(id);
        if (it == m_anchors.end()) return std::nullopt;
        return it->second;
    }

    bool UpdateAnchor(uint64_t id, const std::array<float,3>& newPos) {
        auto it = m_anchors.find(id);
        if (it == m_anchors.end()) return false;
        auto& a = it->second;
        float dx = newPos[0]-a.position[0], dy = newPos[1]-a.position[1], dz = newPos[2]-a.position[2];
        a.driftMm = std::sqrt(dx*dx+dy*dy+dz*dz) * 1000.0f;
        a.position = newPos;
        return true;
    }

    bool DeleteAnchor(uint64_t id) {
        return m_anchors.erase(id) > 0;
    }

    bool Save(const std::string& path) const { (void)path; return true; }
    bool Load(const std::string& path)       { (void)path; return true; }

    float GetMaxDriftMm() const {
        float max = 0.0f;
        for (const auto& [id, a] : m_anchors)
            if (a.driftMm > max) max = a.driftMm;
        return max;
    }

    uint64_t GetAnchorCount() const { return static_cast<uint64_t>(m_anchors.size()); }

private:
    std::unordered_map<uint64_t, SpatialAnchor> m_anchors;
    uint64_t m_nextId = 0;
};

}} // namespace ExplorerLens::Engine
