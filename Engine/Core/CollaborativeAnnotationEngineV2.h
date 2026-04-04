// CollaborativeAnnotationEngineV2.h — Collaborative Annotation Engine v2
// Copyright (c) 2026 ExplorerLens Project
//
// CRDT-based collaborative annotation with conflict-free merge for multi-user scenarios.
//
#pragma once
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CAEv2Operation
{
    std::string operationId;
    std::string authorId;
    std::string payload;
    uint64_t logicalClock = 0;
};

struct CAEv2MergeResult
{
    bool success = false;
    uint32_t opsApplied = 0;
    uint32_t conflicts = 0;
};

class CollaborativeAnnotationEngineV2
{
  public:
    void ApplyOp(const CAEv2Operation& op)
    {
        m_ops[op.operationId] = op;
        m_vectorClock[op.authorId] = std::max(m_vectorClock[op.authorId], op.logicalClock + 1);
    }
    CAEv2MergeResult MergeFrom(const std::vector<CAEv2Operation>& ops)
    {
        CAEv2MergeResult r;
        for (const auto& op : ops) {
            if (m_ops.find(op.operationId) == m_ops.end())
                ApplyOp(op);
            else {
                // Conflict: same opId different payload
                if (m_ops[op.operationId].payload != op.payload)
                    ++r.conflicts;
            }
            ++r.opsApplied;
        }
        r.success = true;
        return r;
    }
    uint32_t OpCount() const
    {
        return static_cast<uint32_t>(m_ops.size());
    }
    uint64_t CurrentClock(const std::string& authorId) const
    {
        auto it = m_vectorClock.find(authorId);
        return it != m_vectorClock.end() ? it->second : 0ull;
    }

  private:
    std::unordered_map<std::string, CAEv2Operation> m_ops;
    std::unordered_map<std::string, uint64_t> m_vectorClock;
};

}  // namespace Engine
}  // namespace ExplorerLens
