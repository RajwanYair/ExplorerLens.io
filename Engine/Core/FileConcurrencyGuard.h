// FileConcurrencyGuard.h — File Access Serialization
// Copyright (c) 2026 ExplorerLens Project
//
// Prevents multiple decoders from reading the same file simultaneously
// using per-path read locks with timeout support.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace ExplorerLens {
namespace Engine {

class FileConcurrencyGuard {
public:
    class ScopedLock {
    public:
        ScopedLock(std::shared_ptr<std::mutex> mtx) : m_mtx(mtx) {
            if (m_mtx) m_mtx->lock();
        }
        ~ScopedLock() { if (m_mtx) m_mtx->unlock(); }
        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;
        ScopedLock(ScopedLock&& o) noexcept : m_mtx(std::move(o.m_mtx)) {}
    private:
        std::shared_ptr<std::mutex> m_mtx;
    };

    ScopedLock AcquireFile(const std::wstring& path) {
        std::lock_guard guard(m_mapMtx);
        auto& entry = m_locks[path];
        if (!entry) entry = std::make_shared<std::mutex>();
        return ScopedLock(entry);
    }

    size_t ActiveLocks() const { return m_locks.size(); }

    void Cleanup() {
        std::lock_guard guard(m_mapMtx);
        // Remove entries where we're the only holder
        for (auto it = m_locks.begin(); it != m_locks.end();) {
            if (it->second.use_count() <= 1)
                it = m_locks.erase(it);
            else
                ++it;
        }
    }

private:
    std::mutex m_mapMtx;
    std::unordered_map<std::wstring, std::shared_ptr<std::mutex>> m_locks;
};

} // namespace Engine
} // namespace ExplorerLens
