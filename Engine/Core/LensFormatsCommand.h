//==============================================================================
// ExplorerLens Engine — `lens formats` CLI command (Sprint S231)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §6.3 — Phase 1: JSON format catalogue for tooling
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// <summary>
/// Supported-format catalogue emitter for the `lens formats` CLI command.
/// Emits machine-readable JSON describing every registered decoder family,
/// its MIME types, and whether it is enabled in the current build.
/// </summary>
class LensFormatsCommand
{
  public:
    struct FormatEntry
    {
        std::string familyId;        // e.g. "jpeg", "heif", "raw"
        std::string displayName;     // e.g. "JPEG / JFIF"
        std::vector<std::string> extensions; // {".jpg", ".jpeg"}
        std::vector<std::string> mimeTypes;  // {"image/jpeg"}
        std::string decoder;         // "libjpeg-turbo", "LibRaw", etc.
        bool        enabled = true;
        bool        gpuAccelerated = false;
    };

    LensFormatsCommand() = default;

    /// <summary>Append a format family to the catalogue.</summary>
    void Register(FormatEntry entry) { m_entries.push_back(std::move(entry)); }

    std::size_t Count() const noexcept { return m_entries.size(); }

    /// <summary>Render catalogue as JSON (UTF-8).</summary>
    std::string RenderJson() const
    {
        std::string out;
        out.reserve(256 + m_entries.size() * 128);
        out += "{\"schema\":\"lens.formats.v1\",\"formats\":[";
        for (std::size_t i = 0; i < m_entries.size(); ++i) {
            if (i > 0) out += ',';
            AppendEntry(out, m_entries[i]);
        }
        out += "]}";
        return out;
    }

    const std::vector<FormatEntry>& Entries() const noexcept { return m_entries; }

  private:
    static void AppendEntry(std::string& out, const FormatEntry& e)
    {
        out += "{\"id\":\"";   AppendEscaped(out, e.familyId);
        out += "\",\"name\":\""; AppendEscaped(out, e.displayName);
        out += "\",\"decoder\":\""; AppendEscaped(out, e.decoder);
        out += "\",\"enabled\":"; out += (e.enabled ? "true" : "false");
        out += ",\"gpu\":";       out += (e.gpuAccelerated ? "true" : "false");
        out += ",\"ext\":[";
        for (std::size_t i = 0; i < e.extensions.size(); ++i) {
            if (i > 0) out += ',';
            out += '"';
            AppendEscaped(out, e.extensions[i]);
            out += '"';
        }
        out += "],\"mime\":[";
        for (std::size_t i = 0; i < e.mimeTypes.size(); ++i) {
            if (i > 0) out += ',';
            out += '"';
            AppendEscaped(out, e.mimeTypes[i]);
            out += '"';
        }
        out += "]}";
    }

    static void AppendEscaped(std::string& out, const std::string& s)
    {
        for (char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[8];
                        (void)std::snprintf(buf, sizeof(buf), "\\u%04X", c);
                        out += buf;
                    } else {
                        out += c;
                    }
                    break;
            }
        }
    }

    std::vector<FormatEntry> m_entries;
};

} // namespace Engine
} // namespace ExplorerLens
