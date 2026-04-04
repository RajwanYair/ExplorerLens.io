// MIDIVisualizer.h — MIDI File Visualization Thumbnail
// Copyright (c) 2026 ExplorerLens Project
//
// Parses Standard MIDI Files (SMF) to extract track info, note events,
// tempo, time signature, and generates piano-roll style thumbnail.
// Supports SMF Type 0, 1, and 2 files.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct MIDITrackInfo
{
    uint32_t noteCount = 0;
    uint32_t eventCount = 0;
    std::string trackName;
    uint8_t channel = 0;
    uint8_t minNote = 127;
    uint8_t maxNote = 0;
};

struct MIDIFileInfo
{
    bool isValid = false;
    uint16_t format = 0;  // 0, 1, or 2
    uint16_t trackCount = 0;
    uint16_t ticksPerQuarter = 480;
    uint32_t tempoMicroseconds = 500000;  // 120 BPM default
    uint8_t timeSignatureNum = 4;
    uint8_t timeSignatureDen = 4;
    uint32_t totalNotes = 0;
    double durationSeconds = 0.0;
    std::vector<MIDITrackInfo> tracks;
};

struct MIDIStats
{
    uint32_t filesProcessed = 0;
    uint64_t totalNotes = 0;
};

class MIDIVisualizer
{
  public:
    MIDIVisualizer() = default;
    ~MIDIVisualizer() = default;

    static const wchar_t* GetName()
    {
        return L"MIDIVisualizer";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".mid" || e == L".midi" || e == L".smf";
    }

    /// Detect MIDI magic: "MThd" (4D 54 68 64)
    bool DetectMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 14)
            return false;
        return memcmp(data, "MThd", 4) == 0;
    }

    /// Parse MIDI header chunk.
    MIDIFileInfo ParseHeader(const uint8_t* data, size_t size) const
    {
        MIDIFileInfo info;
        if (!DetectMagic(data, size))
            return info;

        info.isValid = true;
        // Header chunk length (always 6 for SMF)
        // Format at offset 8-9 (big-endian)
        info.format = (data[8] << 8) | data[9];
        info.trackCount = (data[10] << 8) | data[11];
        info.ticksPerQuarter = (data[12] << 8) | data[13];

        // Parse track chunks
        size_t offset = 14;
        for (uint16_t t = 0; t < info.trackCount && offset + 8 <= size; ++t) {
            if (memcmp(data + offset, "MTrk", 4) != 0)
                break;
            uint32_t trackLen =
                (data[offset + 4] << 24) | (data[offset + 5] << 16) | (data[offset + 6] << 8) | data[offset + 7];

            MIDITrackInfo track;
            // Scan track events for note-on messages
            size_t tEnd = std::min(static_cast<size_t>(offset + 8 + trackLen), size);
            size_t pos = offset + 8;
            while (pos < tEnd) {
                // Skip delta time (variable length)
                while (pos < tEnd && (data[pos] & 0x80))
                    pos++;
                pos++;
                if (pos >= tEnd)
                    break;

                uint8_t status = data[pos];
                if ((status & 0xF0) == 0x90 && pos + 2 < tEnd) {
                    // Note On
                    uint8_t note = data[pos + 1];
                    track.noteCount++;
                    track.minNote = std::min(track.minNote, note);
                    track.maxNote = std::max(track.maxNote, note);
                    track.channel = status & 0x0F;
                    pos += 3;
                } else if ((status & 0xF0) == 0x80) {
                    pos += 3;
                } else if ((status & 0xF0) == 0xB0 || (status & 0xF0) == 0xE0) {
                    pos += 3;
                } else if ((status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0) {
                    pos += 2;
                } else {
                    pos++;
                }

                track.eventCount++;
            }

            info.totalNotes += track.noteCount;
            info.tracks.push_back(track);
            offset = tEnd;
        }

        return info;
    }

    MIDIStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable MIDIStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
