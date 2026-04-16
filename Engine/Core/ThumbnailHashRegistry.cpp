// ThumbnailHashRegistry.cpp — Implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailHashRegistry.h"
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// JSON micro-parser (no external dependency — parses our MANIFEST subset)
//
// Only handles the expected_hashes array structure:
//   "expected_hashes": [
//     { "file": "images/sample.jpg", "format": "JPEG",
//       "dhash": "0123456789abcdef", "phash": "fedcba9876543210",
//       "target_size": 256 }
//   ]
// ---------------------------------------------------------------------------

namespace {

std::string ExtractString(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return {};
    pos = json.find('"', pos + needle.size());
    if (pos == std::string::npos) return {};
    ++pos;
    size_t end = json.find('"', pos);
    if (end == std::string::npos) return {};
    return json.substr(pos, end - pos);
}

int ExtractInt(const std::string& json, const std::string& key, int def = 256) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return def;
    pos = json.find_first_of("0123456789", pos + needle.size());
    if (pos == std::string::npos) return def;
    return std::stoi(json.substr(pos));
}

// Split the "expected_hashes" array content into per-object strings
std::vector<std::string> SplitObjects(const std::string& arrayContent) {
    std::vector<std::string> out;
    int depth = 0;
    size_t start = std::string::npos;
    for (size_t i = 0; i < arrayContent.size(); ++i) {
        char c = arrayContent[i];
        if (c == '{') {
            if (depth == 0) start = i;
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0 && start != std::string::npos) {
                out.push_back(arrayContent.substr(start, i - start + 1));
                start = std::string::npos;
            }
        }
    }
    return out;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// LoadFromManifest
// ---------------------------------------------------------------------------

void ThumbnailHashRegistry::LoadFromManifest(const std::string& manifestPath) {
    std::ifstream f(manifestPath);
    if (!f.is_open())
        throw std::runtime_error("ThumbnailHashRegistry: cannot open " + manifestPath);

    std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    // Find "expected_hashes" array
    const std::string key = "\"expected_hashes\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos) return; // No hashes yet — registry starts empty

    size_t arrStart = json.find('[', pos);
    if (arrStart == std::string::npos) return;
    size_t arrEnd = json.find(']', arrStart);
    if (arrEnd == std::string::npos) return;

    std::string arrayContent = json.substr(arrStart + 1, arrEnd - arrStart - 1);
    auto objects = SplitObjects(arrayContent);

    m_entries.clear();
    m_index.clear();

    for (const auto& obj : objects) {
        ExpectedHash e;
        e.filePath  = ExtractString(obj, "file");
        e.formatId  = ExtractString(obj, "format");
        std::string dhex = ExtractString(obj, "dhash");
        std::string phex = ExtractString(obj, "phash");
        e.targetSize = ExtractInt(obj, "target_size", 256);

        if (e.filePath.empty() || dhex.empty() || phex.empty()) continue;

        try {
            e.dhash = PerceptualHashUtility::FromHexString(dhex);
            e.phash = PerceptualHashUtility::FromHexString(phex);
        } catch (...) {
            continue; // Skip malformed entries
        }

        m_index[e.filePath] = m_entries.size();
        m_entries.push_back(std::move(e));
    }
}

// ---------------------------------------------------------------------------
// SaveToManifest
// ---------------------------------------------------------------------------

void ThumbnailHashRegistry::SaveToManifest(const std::string& manifestPath) const {
    // Read existing manifest
    std::ifstream fi(manifestPath);
    std::string json;
    if (fi.is_open()) {
        json.assign(std::istreambuf_iterator<char>(fi), std::istreambuf_iterator<char>());
        fi.close();
    }

    // Build JSON array for expected_hashes
    std::ostringstream arr;
    arr << "[\n";
    for (size_t i = 0; i < m_entries.size(); ++i) {
        const auto& e = m_entries[i];
        arr << "    {\n"
            << "      \"file\": \"" << e.filePath << "\",\n"
            << "      \"format\": \"" << e.formatId << "\",\n"
            << "      \"dhash\": \"" << PerceptualHashUtility::ToHexString(e.dhash) << "\",\n"
            << "      \"phash\": \"" << PerceptualHashUtility::ToHexString(e.phash) << "\",\n"
            << "      \"target_size\": " << e.targetSize << "\n"
            << "    }" << (i + 1 < m_entries.size() ? "," : "") << "\n";
    }
    arr << "  ]";

    const std::string needle = "\"expected_hashes\"";
    size_t pos = json.find(needle);

    std::string newJson;
    if (pos == std::string::npos) {
        // Insert before closing brace of root object
        size_t last = json.rfind('}');
        if (last != std::string::npos) {
            // Add comma to previous field
            size_t ins = json.rfind('"', last);
            if (ins != std::string::npos) ins = json.rfind('\n', ins);
            newJson = json.substr(0, last) +
                      ",\n  " + needle + ": " + arr.str() + "\n}";
        } else {
            newJson = json; // Can't patch — leave unchanged
        }
    } else {
        size_t arrStart = json.find('[', pos);
        size_t arrEnd   = json.find(']', arrStart);
        if (arrStart != std::string::npos && arrEnd != std::string::npos) {
            newJson = json.substr(0, arrStart) + arr.str() + json.substr(arrEnd + 1);
        } else {
            newJson = json;
        }
    }

    std::ofstream fo(manifestPath);
    if (!fo.is_open())
        throw std::runtime_error("ThumbnailHashRegistry: cannot write " + manifestPath);
    fo << newJson;
}

// ---------------------------------------------------------------------------
// Register / Lookup
// ---------------------------------------------------------------------------

void ThumbnailHashRegistry::Register(const ExpectedHash& entry) {
    auto it = m_index.find(entry.filePath);
    if (it != m_index.end()) {
        m_entries[it->second] = entry;
    } else {
        m_index[entry.filePath] = m_entries.size();
        m_entries.push_back(entry);
    }
}

std::optional<ExpectedHash> ThumbnailHashRegistry::Lookup(const std::string& filePath) const {
    auto it = m_index.find(filePath);
    if (it == m_index.end()) return std::nullopt;
    return m_entries[it->second];
}

// ---------------------------------------------------------------------------
// Check
// ---------------------------------------------------------------------------

HashCheckResult ThumbnailHashRegistry::Check(
    const std::string&       filePath,
    std::span<const uint8_t> pixels,
    uint32_t                 width,
    uint32_t                 height,
    int                      dhashThreshold,
    int                      phashThreshold) const {
    HashCheckResult r;
    r.filePath = filePath;

    auto expected = Lookup(filePath);
    if (!expected) {
        // No baseline registered — treat as a pass (first run)
        r.passed        = true;
        r.dhammingDist  = 0;
        r.phammingDist  = 0;
        return r;
    }

    PerceptualHash dh = PerceptualHashUtility::DHash(pixels, width, height);
    PerceptualHash ph = PerceptualHashUtility::PHash(pixels, width, height);

    r.dhammingDist = HammingDistance(dh, expected->dhash);
    r.phammingDist = HammingDistance(ph, expected->phash);

    bool dOk = (r.dhammingDist <= dhashThreshold);
    bool pOk = (r.phammingDist <= phashThreshold);
    r.passed = dOk && pOk;

    if (!r.passed) {
        std::ostringstream msg;
        if (!dOk) msg << "dHash dist=" << r.dhammingDist << " (threshold=" << dhashThreshold << ") ";
        if (!pOk) msg << "pHash dist=" << r.phammingDist << " (threshold=" << phashThreshold << ")";
        r.failReason = msg.str();
    }
    return r;
}

} // namespace Engine
} // namespace ExplorerLens
