// DeviceSyncManifest.cpp — Cross-Device Thumbnail Cache Manifest Implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "DeviceSyncManifest.h"
#include <algorithm>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

DeviceSyncManifest::DeviceSyncManifest(std::string deviceId)
    : m_deviceId(std::move(deviceId))
{}

bool DeviceSyncManifest::Upsert(const SyncManifestEntry& entry)
{
    for (auto& e : m_entries) {
        if (e.pathHash == entry.pathHash) {
            e = entry;
            return false; // updated existing
        }
    }
    m_entries.push_back(entry);
    return true; // new entry
}

bool DeviceSyncManifest::Remove(uint64_t pathHash)
{
    auto it = std::find_if(m_entries.begin(), m_entries.end(),
        [pathHash](const SyncManifestEntry& e){ return e.pathHash == pathHash; });
    if (it == m_entries.end()) return false;
    m_entries.erase(it);
    return true;
}

std::optional<SyncManifestEntry> DeviceSyncManifest::Find(uint64_t pathHash) const
{
    for (const auto& e : m_entries) {
        if (e.pathHash == pathHash) return e;
    }
    return std::nullopt;
}

std::vector<uint8_t> DeviceSyncManifest::Serialize() const
{
    // Simple little-endian binary layout:
    // [version:1] [deviceId_len:2] [deviceId:n] [entry_count:4]
    // foreach entry: [pathHash:8][etag_len:2][etag:n][w:4][h:4][sz:4][ts:8]
    std::vector<uint8_t> buf;
    buf.push_back(static_cast<uint8_t>(ManifestVersion::V1));

    auto appendU16 = [&](uint16_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
    };
    auto appendU32 = [&](uint32_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
    };
    auto appendU64 = [&](uint64_t v) {
        for (int i = 0; i < 8; ++i)
            buf.push_back((v >> (i * 8)) & 0xFF);
    };
    auto appendStr = [&](const std::string& s) {
        appendU16(static_cast<uint16_t>(s.size()));
        buf.insert(buf.end(), s.begin(), s.end());
    };

    appendStr(m_deviceId);
    appendU32(static_cast<uint32_t>(m_entries.size()));
    for (const auto& e : m_entries) {
        appendU64(e.pathHash);
        appendStr(e.etag);
        appendU32(e.width);
        appendU32(e.height);
        appendU32(e.sizeBytes);
        appendU64(e.generatedAt);
    }
    return buf;
}

bool DeviceSyncManifest::Deserialize(const std::vector<uint8_t>& data)
{
    if (data.empty()) return false;
    size_t pos = 0;

    auto readU8 = [&]() -> uint8_t {
        return (pos < data.size()) ? data[pos++] : 0u;
    };
    auto readU16 = [&]() -> uint16_t {
        uint16_t v = 0;
        for (int i = 0; i < 2; ++i) v |= static_cast<uint16_t>(readU8()) << (i * 8);
        return v;
    };
    auto readU32 = [&]() -> uint32_t {
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i) v |= static_cast<uint32_t>(readU8()) << (i * 8);
        return v;
    };
    auto readU64 = [&]() -> uint64_t {
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(readU8()) << (i * 8);
        return v;
    };
    auto readStr = [&]() -> std::string {
        uint16_t len = readU16();
        if (pos + len > data.size()) return {};
        std::string s(reinterpret_cast<const char*>(data.data() + pos), len);
        pos += len;
        return s;
    };

    const uint8_t ver = readU8();
    if (ver != static_cast<uint8_t>(ManifestVersion::V1)) return false;

    m_deviceId = readStr();
    const uint32_t count = readU32();
    m_entries.clear();
    m_entries.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        SyncManifestEntry e;
        e.pathHash    = readU64();
        e.etag        = readStr();
        e.width       = readU32();
        e.height      = readU32();
        e.sizeBytes   = readU32();
        e.generatedAt = readU64();
        m_entries.push_back(e);
    }
    return pos <= data.size();
}

} // namespace Engine
} // namespace ExplorerLens
