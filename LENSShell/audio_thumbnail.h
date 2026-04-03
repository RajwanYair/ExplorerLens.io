/******************************************************************************
 * audio_thumbnail.h
 * audio_thumbnail.h — Audio Thumbnail Generation
 * Extracts album art from audio files and generates waveform visualizations
 ******************************************************************************/

#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace ExplorerLens {

class AudioThumbnail
{
  public:
    // Extract album art from audio file (MP3 ID3v2, M4A iTunes, FLAC, etc.)
    // Returns HBITMAP of embedded cover art, or NULL if not found
    static HBITMAP ExtractAlbumArt(const std::wstring& audioPath);

    // Generate waveform visualization from audio file
    // Used as fallback when no album art is available
    static HBITMAP GenerateWaveform(const std::wstring& audioPath, int width = 256, int height = 256);

    // Get audio file metadata (title, artist, album, duration, bitrate)
    struct AudioMetadata
    {
        std::wstring title;
        std::wstring artist;
        std::wstring album;
        std::wstring genre;
        int duration;        // seconds
        int bitrate;         // kbps
        int sampleRate;      // Hz
        int channels;        // 1=mono, 2=stereo
        std::wstring codec;  // MP3, FLAC, Opus, etc.
    };

    static bool GetMetadata(const std::wstring& audioPath, AudioMetadata& metadata);

    // Check if Windows Property System is available for metadata extraction
    static bool IsPropertySystemAvailable();

  private:
    // Try to extract album art using Windows Property System
    static HBITMAP ExtractAlbumArtPropertySystem(const std::wstring& audioPath);

    // Try to extract album art by reading file format directly
    static HBITMAP ExtractAlbumArtManual(const std::wstring& audioPath);

    // Read MP3 ID3v2 tag for album art
    static HBITMAP ExtractAlbumArtMP3(const std::wstring& audioPath);

    // Read FLAC metadata for album art
    static HBITMAP ExtractAlbumArtFLAC(const std::wstring& audioPath);

    // Read M4A/MP4 metadata for album art
    static HBITMAP ExtractAlbumArtM4A(const std::wstring& audioPath);

    // Read Ogg Vorbis/Opus metadata for album art
    static HBITMAP ExtractAlbumArtOgg(const std::wstring& audioPath);

    // Generate simple waveform using Windows Media Foundation
    static HBITMAP GenerateWaveformWMF(const std::wstring& audioPath, int width, int height);

    // Generate waveform using direct PCM reading (WAV files)
    static HBITMAP GenerateWaveformPCM(const std::wstring& audioPath, int width, int height);

    // Draw waveform from audio samples
    static HBITMAP DrawWaveform(const std::vector<float>& samples, int width, int height,
                                COLORREF waveColor = RGB(0, 128, 255));

    // Extract audio samples for visualization
    static bool ExtractAudioSamples(const std::wstring& audioPath, std::vector<float>& samples, int maxSamples = 1024);
};

}  // namespace ExplorerLens
