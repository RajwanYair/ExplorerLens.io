// ThumbnailPackFile.cpp — Compact Multi-Thumbnail Bundle (.tlpk)
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailPackFile.h"
#include <cstring>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

void ThumbnailPackFile::AddEntry(uint64_t pathHash, const std::vector<uint8_t>& blob)
{
    // Replace existing entry if hash already present
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].pathHash == pathHash) {
            m_blobs[i] = blob;
            m_entries[i].origSize = static_cast<uint32_t>(blob.size());
            return;
        }
    }
    PackEntry e;
    e.pathHash = pathHash;
    e.origSize = static_cast<uint32_t>(blob.size());
    m_entries.push_back(e);
    m_blobs.push_back(blob);
}

std::vector<uint8_t> ThumbnailPackFile::Pack() const
{
    // Layout: [magic:4][version:1][entry_count:4]
    // foreach entry: [pathHash:8][offset:4][compSize:4][origSize:4]
    // [payload: concatenation of all blobs (uncompressed in stub)]
    std::vector<uint8_t> buf;

    auto appendU8 = [&](uint8_t v)  { buf.push_back(v); };
    auto appendU32 = [&](uint32_t v) {
        for (int i = 0; i < 4; ++i) buf.push_back((v >> (i * 8)) & 0xFF);
    };
    auto appendU64 = [&](uint64_t v) {
        for (int i = 0; i < 8; ++i) buf.push_back((v >> (i * 8)) & 0xFF);
    };

    appendU32(TLPK_MAGIC);
    appendU8(1); // format version
    appendU32(static_cast<uint32_t>(m_entries.size()));

    // Compute offsets
    uint32_t payloadBase = static_cast<uint32_t>(
        4 + 1 + 4 + m_entries.size() * (8 + 4 + 4 + 4));
    uint32_t offset = payloadBase;
    for (size_t i = 0; i < m_entries.size(); ++i) {
        appendU64(m_entries[i].pathHash);
        appendU32(offset);
        const uint32_t sz = static_cast<uint32_t>(m_blobs[i].size());
        appendU32(sz);  // compSize (no compression in stub)
        appendU32(sz);  // origSize
        offset += sz;
    }
    for (const auto& blob : m_blobs)
        buf.insert(buf.end(), blob.begin(), blob.end());

    return buf;
}

PackFileError ThumbnailPackFile::Load(const std::vector<uint8_t>& data)
{
    if (data.size() < 9) {
        m_lastError = PackFileError::CORRUPT_HEADER;
        return m_lastError;
    }

    auto readU32 = [&](size_t pos) -> uint32_t {
        uint32_t v = 0;
        for (int i = 0; i < 4; ++i) v |= static_cast<uint32_t>(data[pos + i]) << (i * 8);
        return v;
    };
    auto readU64 = [&](size_t pos) -> uint64_t {
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(data[pos + i]) << (i * 8);
        return v;
    };

    if (readU32(0) != TLPK_MAGIC) {
        m_lastError = PackFileError::INVALID_MAGIC;
        return m_lastError;
    }

    const uint32_t count = readU32(5);
    m_entries.resize(count);
    m_blobs.resize(count);
    m_packedData = data;

    size_t pos = 9;
    for (uint32_t i = 0; i < count && pos + 20 <= data.size(); ++i) {
        m_entries[i].pathHash  = readU64(pos);     pos += 8;
        m_entries[i].offset    = readU32(pos);      pos += 4;
        m_entries[i].compSize  = readU32(pos);      pos += 4;
        m_entries[i].origSize  = readU32(pos);      pos += 4;
    }

    m_lastError = PackFileError::OK;
    return m_lastError;
}

std::vector<uint8_t> ThumbnailPackFile::Extract(uint64_t pathHash) const
{
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].pathHash == pathHash) {
            const uint32_t off = m_entries[i].offset;
            const uint32_t sz  = m_entries[i].compSize;
            if (off + sz > m_packedData.size()) return {};
            return std::vector<uint8_t>(
                m_packedData.begin() + off,
                m_packedData.begin() + off + sz);
        }
    }
    return {};
}

} // namespace Engine
} // namespace ExplorerLens
