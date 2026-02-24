// Smart Memory Management Utilities for ExplorerLens
// Provides RAII wrappers and memory tracking

#pragma once

#include <windows.h>
#include <memory>
#include <d3d11.h>
#include <mutex>
#include <unordered_map>
#include <atomic>

namespace ExplorerLens {

// COM smart pointer helpers
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

// RAII wrapper for HBITMAP
class BitmapHandle {
private:
    HBITMAP m_bitmap;

public:
    BitmapHandle() : m_bitmap(nullptr) {}
    explicit BitmapHandle(HBITMAP bitmap) : m_bitmap(bitmap) {}
    
    ~BitmapHandle() {
        Reset();
    }

    // Move semantics
    BitmapHandle(BitmapHandle&& other) noexcept : m_bitmap(other.m_bitmap) {
        other.m_bitmap = nullptr;
    }

    BitmapHandle& operator=(BitmapHandle&& other) noexcept {
        if (this != &other) {
            Reset();
            m_bitmap = other.m_bitmap;
            other.m_bitmap = nullptr;
        }
        return *this;
    }

    // Delete copy
    BitmapHandle(const BitmapHandle&) = delete;
    BitmapHandle& operator=(const BitmapHandle&) = delete;

    void Reset(HBITMAP newBitmap = nullptr) {
        if (m_bitmap) {
            DeleteObject(m_bitmap);
        }
        m_bitmap = newBitmap;
    }

    HBITMAP Get() const { return m_bitmap; }
    HBITMAP* GetAddressOf() { Reset(); return &m_bitmap; }
    HBITMAP Release() {
        HBITMAP result = m_bitmap;
        m_bitmap = nullptr;
        return result;
    }

    explicit operator bool() const { return m_bitmap != nullptr; }
};

// RAII wrapper for malloc/free
template<typename T>
class MallocPtr {
private:
    T* m_ptr;

public:
    MallocPtr() : m_ptr(nullptr) {}
    explicit MallocPtr(size_t count) : m_ptr(static_cast<T*>(malloc(count * sizeof(T)))) {}
    
    ~MallocPtr() {
        Reset();
    }

    // Move semantics
    MallocPtr(MallocPtr&& other) noexcept : m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }

    MallocPtr& operator=(MallocPtr&& other) noexcept {
        if (this != &other) {
            Reset();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    // Delete copy
    MallocPtr(const MallocPtr&) = delete;
    MallocPtr& operator=(const MallocPtr&) = delete;

    void Reset(T* newPtr = nullptr) {
        if (m_ptr) {
            free(m_ptr);
        }
        m_ptr = newPtr;
    }

    T* Get() const { return m_ptr; }
    T** GetAddressOf() { Reset(); return &m_ptr; }
    T* Release() {
        T* result = m_ptr;
        m_ptr = nullptr;
        return result;
    }

    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }
    T& operator[](size_t index) const { return m_ptr[index]; }
    
    explicit operator bool() const { return m_ptr != nullptr; }
};

// RAII wrapper for CoTaskMemAlloc/CoTaskMemFree
template<typename T>
class CoTaskMemPtr {
private:
    T* m_ptr;

public:
    CoTaskMemPtr() : m_ptr(nullptr) {}
    explicit CoTaskMemPtr(size_t count) 
        : m_ptr(static_cast<T*>(CoTaskMemAlloc(count * sizeof(T)))) {}
    
    ~CoTaskMemPtr() {
        Reset();
    }

    // Move semantics
    CoTaskMemPtr(CoTaskMemPtr&& other) noexcept : m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }

    CoTaskMemPtr& operator=(CoTaskMemPtr&& other) noexcept {
        if (this != &other) {
            Reset();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    // Delete copy
    CoTaskMemPtr(const CoTaskMemPtr&) = delete;
    CoTaskMemPtr& operator=(const CoTaskMemPtr&) = delete;

    void Reset(T* newPtr = nullptr) {
        if (m_ptr) {
            CoTaskMemFree(m_ptr);
        }
        m_ptr = newPtr;
    }

    T* Get() const { return m_ptr; }
    T** GetAddressOf() { Reset(); return &m_ptr; }
    T* Release() {
        T* result = m_ptr;
        m_ptr = nullptr;
        return result;
    }

    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }
    
    explicit operator bool() const { return m_ptr != nullptr; }
};

// Memory usage tracker
class MemoryTracker {
private:
    struct AllocationInfo {
        size_t size;
        std::string category;
        
        AllocationInfo() : size(0) {}
        AllocationInfo(size_t s, const std::string& c) : size(s), category(c) {}
    };

    std::unordered_map<void*, AllocationInfo> m_allocations;
    std::mutex m_mutex;
    std::atomic<size_t> m_totalAllocated;
    std::atomic<size_t> m_peakAllocated;
    bool m_enabled;

    MemoryTracker() : m_totalAllocated(0), m_peakAllocated(0), m_enabled(false) {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\Settings", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD tracking = 0;
            DWORD size = sizeof(DWORD);
            if (RegQueryValueExW(hKey, L"EnableMemoryTracking", nullptr, nullptr, (LPBYTE)&tracking, &size) == ERROR_SUCCESS) {
                m_enabled = (tracking != 0);
            }
            RegCloseKey(hKey);
        }
    }

public:
    static MemoryTracker& Instance() {
        static MemoryTracker instance;
        return instance;
    }

    void TrackAllocation(void* ptr, size_t size, const std::string& category = "general") {
        if (!m_enabled || !ptr) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_allocations[ptr] = AllocationInfo(size, category);
        
        size_t current = m_totalAllocated.fetch_add(size) + size;
        
        // Update peak atomically
        size_t peak = m_peakAllocated.load();
        while (current > peak && !m_peakAllocated.compare_exchange_weak(peak, current)) {
            // Loop until successful or peak is already greater
        }
    }

    void TrackDeallocation(void* ptr) {
        if (!m_enabled || !ptr) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_allocations.find(ptr);
        if (it != m_allocations.end()) {
            m_totalAllocated.fetch_sub(it->second.size);
            m_allocations.erase(it);
        }
    }

    size_t GetTotalAllocated() const {
        return m_totalAllocated.load();
    }

    size_t GetPeakAllocated() const {
        return m_peakAllocated.load();
    }

    std::string GetReport() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        
        std::ostringstream report;
        report << "Memory Usage Report\n";
        report << "===================\n\n";
        report << "Current Allocated: " << (m_totalAllocated.load() / 1024.0 / 1024.0) << " MB\n";
        report << "Peak Allocated: " << (m_peakAllocated.load() / 1024.0 / 1024.0) << " MB\n";
        report << "Active Allocations: " << m_allocations.size() << "\n\n";

        // Group by category
        std::unordered_map<std::string, size_t> categoryTotals;
        for (const auto& pair : m_allocations) {
            categoryTotals[pair.second.category] += pair.second.size;
        }

        report << "Allocations by Category:\n";
        for (const auto& pair : categoryTotals) {
            report << "  " << pair.first << ": " 
                   << (pair.second / 1024.0 / 1024.0) << " MB\n";
        }

        return report.str();
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_allocations.clear();
        m_totalAllocated = 0;
        m_peakAllocated = 0;
    }

    bool IsEnabled() const { return m_enabled; }
};

// Tracked allocation helper
template<typename T>
class TrackedPtr {
private:
    T* m_ptr;
    std::string m_category;

public:
    TrackedPtr() : m_ptr(nullptr) {}
    
    explicit TrackedPtr(size_t count, const std::string& category = "general") 
        : m_ptr(static_cast<T*>(malloc(count * sizeof(T))))
        , m_category(category)
    {
        if (m_ptr) {
            MemoryTracker::Instance().TrackAllocation(m_ptr, count * sizeof(T), category);
        }
    }
    
    ~TrackedPtr() {
        Reset();
    }

    // Move semantics
    TrackedPtr(TrackedPtr&& other) noexcept 
        : m_ptr(other.m_ptr)
        , m_category(std::move(other.m_category))
    {
        other.m_ptr = nullptr;
    }

    TrackedPtr& operator=(TrackedPtr&& other) noexcept {
        if (this != &other) {
            Reset();
            m_ptr = other.m_ptr;
            m_category = std::move(other.m_category);
            other.m_ptr = nullptr;
        }
        return *this;
    }

    // Delete copy
    TrackedPtr(const TrackedPtr&) = delete;
    TrackedPtr& operator=(const TrackedPtr&) = delete;

    void Reset() {
        if (m_ptr) {
            MemoryTracker::Instance().TrackDeallocation(m_ptr);
            free(m_ptr);
            m_ptr = nullptr;
        }
    }

    T* Get() const { return m_ptr; }
    T* Release() {
        MemoryTracker::Instance().TrackDeallocation(m_ptr);
        T* result = m_ptr;
        m_ptr = nullptr;
        return result;
    }

    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }
    T& operator[](size_t index) const { return m_ptr[index]; }
    
    explicit operator bool() const { return m_ptr != nullptr; }
};

} // namespace ExplorerLens

