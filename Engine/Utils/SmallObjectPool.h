//==============================================================================
// ExplorerLens Engine - Small Object Memory Pool 
// Copyright (c) 2026 - ExplorerLens Project
// Task A25: Fast allocation for small objects
//==============================================================================

#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

 /// <summary>
 /// Fast memory pool for small allocations (e.g., decoder request objects)
 /// Reduces heap fragmentation and allocation overhead
 /// </summary>
 template<typename T, size_t PoolSize = 256>
 class SmallObjectPool {
 public:
 SmallObjectPool() {
 m_freeList.reserve(PoolSize);
 for (size_t i = 0; i < PoolSize; ++i) {
 m_pool.push_back(std::make_unique<T>());
 m_freeList.push_back(m_pool.back().get());
 }
 }

 ~SmallObjectPool() = default;

 T* Allocate() {
 std::lock_guard<std::mutex> lock(m_mutex);
 
 if (m_freeList.empty()) {
 // Pool exhausted, allocate new object (falls back to heap)
 m_overflow.push_back(std::make_unique<T>());
 m_overflowCount++;
 return m_overflow.back().get();
 }
 
 T* obj = m_freeList.back();
 m_freeList.pop_back();
 m_allocCount++;
 return obj;
 }

 void Deallocate(T* obj) {
 if (!obj) return;
 
 std::lock_guard<std::mutex> lock(m_mutex);
 
 // Check if object is from pool or overflow
 bool isFromPool = false;
 for (auto& poolObj : m_pool) {
 if (poolObj.get() == obj) {
 isFromPool = true;
 break;
 }
 }
 
 if (isFromPool) {
 m_freeList.push_back(obj);
 m_deallocCount++;
 }
 // Overflow objects cleaned up by unique_ptr
 }

 size_t GetAllocCount() const { return m_allocCount.load(); }
 size_t GetDeallocCount() const { return m_deallocCount.load(); }
 size_t GetOverflowCount() const { return m_overflowCount.load(); }
 size_t GetPoolSize() const { return PoolSize; }
 size_t GetFreeCount() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 return m_freeList.size();
 }

 private:
 std::vector<std::unique_ptr<T>> m_pool;
 std::vector<std::unique_ptr<T>> m_overflow;
 std::vector<T*> m_freeList;
 mutable std::mutex m_mutex;
 std::atomic<size_t> m_allocCount{0};
 std::atomic<size_t> m_deallocCount{0};
 std::atomic<size_t> m_overflowCount{0};
 };

 /// <summary>
 /// RAII wrapper for pool-allocated objects
 /// </summary>
 template<typename T>
 class PoolPtr {
 public:
 explicit PoolPtr(SmallObjectPool<T>* pool = nullptr)
 : m_pool(pool), m_ptr(pool ? pool->Allocate() : nullptr) {}

 ~PoolPtr() {
 if (m_ptr && m_pool) {
 m_pool->Deallocate(m_ptr);
 }
 }

 PoolPtr(PoolPtr&& other) noexcept
 : m_pool(other.m_pool), m_ptr(other.m_ptr) {
 other.m_ptr = nullptr;
 }

 PoolPtr& operator=(PoolPtr&& other) noexcept {
 if (this != &other) {
 if (m_ptr && m_pool) {
 m_pool->Deallocate(m_ptr);
 }
 m_pool = other.m_pool;
 m_ptr = other.m_ptr;
 other.m_ptr = nullptr;
 }
 return *this;
 }

 PoolPtr(const PoolPtr&) = delete;
 PoolPtr& operator=(const PoolPtr&) = delete;

 T* get() const { return m_ptr; }
 T* operator->() const { return m_ptr; }
 T& operator*() const { return *m_ptr; }
 explicit operator bool() const { return m_ptr != nullptr; }

 private:
 SmallObjectPool<T>* m_pool;
 T* m_ptr;
 };

} // namespace Engine
} // namespace ExplorerLens

