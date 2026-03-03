// MetadataTooltipRenderer.h — Rich Metadata Tooltip Text Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Builds formatted multi-line tooltip strings from key-value metadata fields.
// Used to generate rich tooltip text for shell extension thumbnail hover info.
//
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// Represents a single metadata field for tooltip display
struct TooltipField {
    std::wstring key;
    std::wstring value;
};

// Generates formatted multi-line tooltip text from metadata key-value pairs.
// Supports field truncation, max-width formatting, and duplicate detection.
class MetadataTooltipRenderer {
public:
    MetadataTooltipRenderer()
        : m_maxWidth(80) {
    }

    // Add a metadata field (key-value pair) to the tooltip
    void AddField(const std::wstring& key, const std::wstring& value) {
        if (key.empty()) return;
        m_fields.push_back({ key, value });
    }

    // Format all fields into a multi-line tooltip string
    // Format: "Key: Value\n" per field, with value truncation at max width
    std::wstring FormatTooltipText() const {
        if (m_fields.empty()) return std::wstring();

        std::wstring result;
        // Pre-calculate approximate size
        result.reserve(m_fields.size() * (m_maxWidth + 4));

        size_t longestKey = 0;
        for (const auto& f : m_fields) {
            longestKey = (std::max)(longestKey, f.key.size());
        }
        // Cap key padding to reasonable width
        const size_t keyPad = (std::min)(longestKey, static_cast<size_t>(24));

        for (size_t i = 0; i < m_fields.size(); ++i) {
            const auto& field = m_fields[i];

            std::wstring paddedKey = field.key;
            if (paddedKey.size() < keyPad) {
                paddedKey.append(keyPad - paddedKey.size(), L' ');
            }

            // Calculate available width for value
            const size_t separatorLen = 2; // ": "
            const size_t availableWidth = (m_maxWidth > paddedKey.size() + separatorLen)
                ? m_maxWidth - paddedKey.size() - separatorLen
                : 20;

            std::wstring truncatedValue = TruncateValue(field.value, availableWidth);

            result += paddedKey;
            result += L": ";
            result += truncatedValue;

            if (i + 1 < m_fields.size()) {
                result += L'\n';
            }
        }
        return result;
    }

    // Return number of fields currently stored
    size_t GetFieldCount() const {
        return m_fields.size();
    }

    // Set the maximum character width for formatted lines
    void SetMaxWidth(uint32_t chars) {
        m_maxWidth = (std::max)(chars, static_cast<uint32_t>(20));
    }

    // Truncate a value string to maxLen, appending ellipsis if needed
    static std::wstring TruncateValue(const std::wstring& value, size_t maxLen) {
        if (maxLen < 4) maxLen = 4;
        if (value.size() <= maxLen) return value;
        return value.substr(0, maxLen - 3) + L"...";
    }

    // Check if a field with the given key already exists
    bool HasField(const std::wstring& key) const {
        for (const auto& f : m_fields) {
            if (f.key == key) return true;
        }
        return false;
    }

    // Clear all fields
    void Clear() {
        m_fields.clear();
    }

    // Get the current max width setting
    uint32_t GetMaxWidth() const { return m_maxWidth; }

private:
    std::vector<TooltipField> m_fields;
    uint32_t m_maxWidth;
};

} // namespace Engine
} // namespace ExplorerLens
