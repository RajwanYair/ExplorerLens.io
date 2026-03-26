// SecureStringPool.h — Locked Memory String Storage with CryptProtectMemory
// Copyright (c) 2026 ExplorerLens Project
//
// Stores sensitive strings (license keys, activation tokens, credentials) in
// VirtualLock'd pages protected by CryptProtectMemory. Automatically zeroes
// memory on destruction to prevent secrets lingering in the process heap.
//
#pragma once
#include <windows.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>

#pragma comment(lib, "crypt32.lib")

namespace ExplorerLens { namespace Engine {

static constexpr DWORD CRYPT_PROTECT_MEMORY_SAME_PROCESS = 0;

// A single protected string slot
struct SecureSlot {
    uint8_t* page    = nullptr; // VirtualAlloc'd, VirtualLock'd page
    size_t   pageBytes = 0;    // Always page-aligned
    size_t   dataLen  = 0;     // Actual string byte length (without null)
    bool     encrypted = false; // Whether CryptProtectMemory is applied
};

class SecureStringPool {
public:
    ~SecureStringPool() { Clear(); }

    // Store a wide string under a key; encrypts in place
    bool Store(const std::wstring& keyName, const std::wstring& secret) {
        std::lock_guard<std::mutex> lk(m_mtx);
        size_t byteLen = (secret.size() + 1) * sizeof(wchar_t);
        size_t pageSize = AlignToPage(byteLen);

        SecureSlot slot{};
        slot.page      = static_cast<uint8_t*>(
            VirtualAlloc(nullptr, pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (!slot.page) return false;

        slot.pageBytes = pageSize;
        slot.dataLen   = byteLen;

        // Copy plaintext into locked page
        memcpy(slot.page, secret.c_str(), byteLen);

        // Lock physical pages into RAM (prevent swap)
        VirtualLock(slot.page, pageSize);

        // Encrypt with CryptProtectMemory (CRYPTPROTECTMEMORY_SAME_PROCESS scope)
        // Block size for CryptProtectMemory must be multiple of CRYPTPROTECTMEMORY_BLOCK_SIZE (16)
        DWORD blockSize = 16;
        size_t encLen = (byteLen + blockSize - 1) & ~(blockSize - 1);
        if (encLen <= pageSize) {
            if (CryptProtectMemory(slot.page, static_cast<DWORD>(encLen),
                    CRYPT_PROTECT_MEMORY_SAME_PROCESS))
                slot.encrypted = true;
        }

        // Remove old slot if exists
        auto it = m_slots.find(keyName);
        if (it != m_slots.end()) FreeSlot(it->second);
        m_slots[keyName] = slot;
        return true;
    }

    // Retrieve a secret (decrypts temporarily; re-encrypts after copy)
    bool Retrieve(const std::wstring& keyName, std::wstring& outSecret) {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_slots.find(keyName);
        if (it == m_slots.end()) return false;
        SecureSlot& slot = it->second;

        // Temporarily decrypt
        if (slot.encrypted) {
            DWORD blockSize = 16;
            size_t encLen = (slot.dataLen + blockSize - 1) & ~(blockSize - 1);
            CryptUnprotectMemory(slot.page, static_cast<DWORD>(encLen),
                    CRYPT_PROTECT_MEMORY_SAME_PROCESS);
        }

        outSecret = reinterpret_cast<const wchar_t*>(slot.page);

        // Re-encrypt immediately
        if (slot.encrypted) {
            DWORD blockSize = 16;
            size_t encLen = (slot.dataLen + blockSize - 1) & ~(blockSize - 1);
            CryptProtectMemory(slot.page, static_cast<DWORD>(encLen),
                    CRYPT_PROTECT_MEMORY_SAME_PROCESS);
        }
        return true;
    }

    bool Contains(const std::wstring& keyName) const {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_slots.find(keyName) != m_slots.end();
    }

    void Remove(const std::wstring& keyName) {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_slots.find(keyName);
        if (it != m_slots.end()) {
            FreeSlot(it->second);
            m_slots.erase(it);
        }
    }

    void Clear() {
        std::lock_guard<std::mutex> lk(m_mtx);
        for (auto& [k, s] : m_slots) FreeSlot(s);
        m_slots.clear();
    }

    size_t Count() const {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_slots.size();
    }

private:
    static size_t AlignToPage(size_t bytes) {
        SYSTEM_INFO si = {}; GetSystemInfo(&si);
        size_t ps = si.dwPageSize ? si.dwPageSize : 4096;
        return (bytes + ps - 1) & ~(ps - 1);
    }

    static void FreeSlot(SecureSlot& slot) {
        if (!slot.page) return;
        // Zero-fill before freeing (SecureZeroMemory prevents optimizer removal)
        SecureZeroMemory(slot.page, slot.pageBytes);
        VirtualUnlock(slot.page, slot.pageBytes);
        VirtualFree(slot.page, 0, MEM_RELEASE);
        slot.page = nullptr;
    }

    mutable std::mutex m_mtx;
    std::unordered_map<std::wstring, SecureSlot> m_slots;
};

}} // namespace ExplorerLens::Engine
