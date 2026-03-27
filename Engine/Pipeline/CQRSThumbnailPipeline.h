// CQRSThumbnailPipeline.h — CQRS Command/Query Separation for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Separates thumbnail write model (commands) from read model (queries) enabling independent scaling and caching.
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

struct ThumbGenCommand  { std::wstring path; uint32_t width; uint32_t height; };
struct ThumbQueryResult { bool found; std::vector<uint8_t> rgba; uint32_t width; uint32_t height; };
class CQRSThumbnailPipeline {
public:
    uint64_t Dispatch(ThumbGenCommand cmd)  {
        (void)cmd; return ++m_cmdSeq;
    }
    ThumbQueryResult Query(const std::wstring& path) const {
        (void)path; return { false, {}, 0, 0 };
    }
    uint64_t CommandsDispatched() const { return m_cmdSeq; }
private:
    std::atomic<uint64_t> m_cmdSeq{0};
    mutable std::mutex    m_mu;
};

} // namespace Engine
} // namespace ExplorerLens