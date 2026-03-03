/******************************************************************************
 * audio_thumbnail.cpp
 * audio_thumbnail.cpp — Audio Thumbnail Generation Implementation
 * Extracts album art from audio files and generates waveform visualizations
 ******************************************************************************/

#include "StdAfx.h"
#include "audio_thumbnail.h"
#include "GdiplusRAII.h"
#include <algorithm>
#include <atlbase.h>
#include <gdiplus.h>
#include <propkey.h>
#include <propsys.h>
#include <propvarutil.h>
#include <shlobj.h>
#include <vector>

#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "gdiplus.lib")

namespace ExplorerLens {

bool AudioThumbnail::IsPropertySystemAvailable() {
  // Windows Property System available on Vista+
  return true;
}

HBITMAP AudioThumbnail::ExtractAlbumArt(const std::wstring &audioPath) {
  // Try Windows Property System first (fastest, works for most formats)
  HBITMAP hBitmap = ExtractAlbumArtPropertySystem(audioPath);
  if (hBitmap)
    return hBitmap;

  // Fall back to manual extraction for specific formats
  return ExtractAlbumArtManual(audioPath);
}

HBITMAP
AudioThumbnail::ExtractAlbumArtPropertySystem(const std::wstring &audioPath) {
  // Initialize COM if needed
  static bool comInitialized = false;
  if (!comInitialized) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
      comInitialized = true;
    }
  }

  // Get property store for audio file
  CComPtr<IPropertyStore> propStore;
  HRESULT hr = SHGetPropertyStoreFromParsingName(
      audioPath.c_str(), NULL, GPS_DEFAULT, IID_PPV_ARGS(&propStore));

  if (FAILED(hr) || !propStore) {
    return NULL;
  }

  // Try to get thumbnail first (PKEY_ThumbnailStream)
  PROPVARIANT propVar;
  PropVariantInit(&propVar);

  hr = propStore->GetValue(PKEY_ThumbnailStream, &propVar);
  if (SUCCEEDED(hr) && propVar.vt == VT_STREAM && propVar.pStream) {
    // Ensure GDI+ is initialized
    GdiplusRAII &gdiplus = GdiplusRAII::GetInstance();
    if (!gdiplus.IsInitialized()) {
      PropVariantClear(&propVar);
      return NULL;
    }

    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromStream(propVar.pStream);
    if (bitmap && bitmap->GetLastStatus() == Gdiplus::Ok) {
      HBITMAP hBitmap = NULL;
      bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);
      delete bitmap;

      PropVariantClear(&propVar);
      return hBitmap;
    }

    if (bitmap)
      delete bitmap;
  }

  PropVariantClear(&propVar);
  return NULL;
}

HBITMAP AudioThumbnail::ExtractAlbumArtManual(const std::wstring &audioPath) {
  // Detect format by extension
  std::wstring ext = audioPath.substr(audioPath.find_last_of(L"."));
  std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

  if (ext == L".mp3") {
    return ExtractAlbumArtMP3(audioPath);
  } else if (ext == L".flac") {
    return ExtractAlbumArtFLAC(audioPath);
  } else if (ext == L".m4a" || ext == L".mp4") {
    return ExtractAlbumArtM4A(audioPath);
  } else if (ext == L".ogg" || ext == L".opus") {
    return ExtractAlbumArtOgg(audioPath);
  }

  return NULL;
}

HBITMAP AudioThumbnail::ExtractAlbumArtMP3(const std::wstring &audioPath) {
  // FUTURE ENHANCEMENT: Implement ID3v2 tag parsing
  // For now, rely on Property System (works for most MP3 files)
  // Full implementation would parse:
  // - ID3v2.3/2.4 APIC frame (Attached Picture)
  // - Extract JPEG/PNG image data
  // - Create HBITMAP from image data
  return NULL;
}

HBITMAP AudioThumbnail::ExtractAlbumArtFLAC(const std::wstring &audioPath) {
  // FUTURE ENHANCEMENT: Implement FLAC metadata block parsing
  // Parse METADATA_BLOCK_PICTURE
  return NULL;
}

HBITMAP AudioThumbnail::ExtractAlbumArtM4A(const std::wstring &audioPath) {
  // FUTURE ENHANCEMENT: Implement MP4 atom parsing
  // Look for 'covr' atom in metadata
  return NULL;
}

HBITMAP AudioThumbnail::ExtractAlbumArtOgg(const std::wstring &audioPath) {
  // FUTURE ENHANCEMENT: Implement Ogg Vorbis comment parsing
  // Extract METADATA_BLOCK_PICTURE from comments
  return NULL;
}

HBITMAP AudioThumbnail::GenerateWaveform(const std::wstring &audioPath,
                                         int width, int height) {
  // Try WAV file first (simple PCM reading)
  std::wstring ext = audioPath.substr(audioPath.find_last_of(L"."));
  std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

  if (ext == L".wav") {
    HBITMAP hBitmap = GenerateWaveformPCM(audioPath, width, height);
    if (hBitmap)
      return hBitmap;
  }

  // Fall back to Windows Media Foundation for other formats
  return GenerateWaveformWMF(audioPath, width, height);
}

HBITMAP AudioThumbnail::GenerateWaveformPCM(const std::wstring &audioPath,
                                            int width, int height) {
  // Extract audio samples
  std::vector<float> samples;
  if (!ExtractAudioSamples(audioPath, samples, width * 2)) {
    return NULL;
  }

  // Draw waveform
  return DrawWaveform(samples, width, height, RGB(0, 150, 255));
}

HBITMAP AudioThumbnail::GenerateWaveformWMF(const std::wstring &audioPath,
                                            int width, int height) {
  // FUTURE ENHANCEMENT: Implement Windows Media Foundation audio decoding
  // For now, create a simple placeholder waveform
  std::vector<float> samples(width, 0.0f);
  return DrawWaveform(samples, width, height, RGB(100, 100, 100));
}

bool AudioThumbnail::ExtractAudioSamples(const std::wstring &audioPath,
                                         std::vector<float> &samples,
                                         int maxSamples) {
  // Simple WAV file reader
  HANDLE hFile = CreateFileW(audioPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                             NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }

  // Read WAV header
  struct WAVHeader {
    char riff[4]; // "RIFF"
    DWORD fileSize;
    char wave[4]; // "WAVE"
    char fmt[4];  // "fmt "
    DWORD fmtSize;
    WORD audioFormat;
    WORD numChannels;
    DWORD sampleRate;
    DWORD byteRate;
    WORD blockAlign;
    WORD bitsPerSample;
  };

  WAVHeader header;
  DWORD bytesRead;
  if (!ReadFile(hFile, &header, sizeof(WAVHeader), &bytesRead, NULL) ||
      bytesRead != sizeof(WAVHeader)) {
    CloseHandle(hFile);
    return false;
  }

  // Verify it's a WAV file
  if (memcmp(header.riff, "RIFF", 4) != 0 ||
      memcmp(header.wave, "WAVE", 4) != 0) {
    CloseHandle(hFile);
    return false;
  }

  // Find data chunk
  char chunkID[4];
  DWORD chunkSize;
  while (ReadFile(hFile, chunkID, 4, &bytesRead, NULL) && bytesRead == 4) {
    ReadFile(hFile, &chunkSize, 4, &bytesRead, NULL);

    if (memcmp(chunkID, "data", 4) == 0) {
      // Found data chunk - read samples
      int samplesPerPixel =
          max(1, (int)(chunkSize / header.blockAlign / maxSamples));
      int samplesRead = 0;

      samples.clear();
      samples.reserve(maxSamples);

      std::vector<BYTE> buffer(header.blockAlign * samplesPerPixel);

      while (samplesRead < maxSamples &&
             ReadFile(hFile, buffer.data(), (DWORD)buffer.size(), &bytesRead,
                      NULL) &&
             bytesRead > 0) {
        // Convert to float and take max amplitude
        float maxAmplitude = 0.0f;

        for (DWORD i = 0; i < bytesRead; i += header.blockAlign) {
          if (header.bitsPerSample == 16) {
            short *sample = (short *)(buffer.data() + i);
            float amplitude = abs(*sample) / 32768.0f;
            maxAmplitude = max(maxAmplitude, amplitude);
          } else if (header.bitsPerSample == 8) {
            BYTE sample = buffer[i];
            float amplitude = abs((int)sample - 128) / 128.0f;
            maxAmplitude = max(maxAmplitude, amplitude);
          }
        }

        samples.push_back(maxAmplitude);
        samplesRead++;
      }

      CloseHandle(hFile);
      return !samples.empty();
    } else {
      // Skip this chunk
      SetFilePointer(hFile, chunkSize, NULL, FILE_CURRENT);
    }
  }

  CloseHandle(hFile);
  return false;
}

HBITMAP AudioThumbnail::DrawWaveform(const std::vector<float> &samples,
                                     int width, int height,
                                     COLORREF waveColor) {
  if (samples.empty())
    return NULL;

  // Create bitmap
  HDC hdcScreen = GetDC(NULL);
  HDC hdc = CreateCompatibleDC(hdcScreen);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, hBitmap);

  // Fill background
  RECT rc = {0, 0, width, height};
  HBRUSH hBrushBg = CreateSolidBrush(RGB(30, 30, 30)); // Dark background
  FillRect(hdc, &rc, hBrushBg);
  DeleteObject(hBrushBg);

  // Draw waveform
  HPEN hPen = CreatePen(PS_SOLID, 2, waveColor);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

  int centerY = height / 2;
  int maxAmplitude = (height / 2) - 10;

  for (size_t i = 0; i < samples.size(); i++) {
    int x = (int)(i * width / samples.size());
    int y = centerY - (int)(samples[i] * maxAmplitude);
    int y2 = centerY + (int)(samples[i] * maxAmplitude);

    // Draw vertical line for this sample
    MoveToEx(hdc, x, y, NULL);
    LineTo(hdc, x, y2);
  }

  // Draw center line
  HPEN hPenCenter = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
  SelectObject(hdc, hPenCenter);
  MoveToEx(hdc, 0, centerY, NULL);
  LineTo(hdc, width, centerY);
  DeleteObject(hPenCenter);

  // Cleanup
  SelectObject(hdc, hOldPen);
  SelectObject(hdc, hOldBitmap);
  DeleteObject(hPen);
  DeleteDC(hdc);
  ReleaseDC(NULL, hdcScreen);

  return hBitmap;
}

bool AudioThumbnail::GetMetadata(const std::wstring &audioPath,
                                 AudioMetadata &metadata) {
  // Initialize COM if needed
  static bool comInitialized = false;
  if (!comInitialized) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
      comInitialized = true;
    }
  }

  // Get property store
  CComPtr<IPropertyStore> propStore;
  HRESULT hr = SHGetPropertyStoreFromParsingName(
      audioPath.c_str(), NULL, GPS_DEFAULT, IID_PPV_ARGS(&propStore));

  if (FAILED(hr) || !propStore) {
    return false;
  }

  // Read metadata properties
  PROPVARIANT propVar;
  PropVariantInit(&propVar);

  // Title
  if (SUCCEEDED(propStore->GetValue(PKEY_Title, &propVar)) &&
      propVar.vt == VT_LPWSTR) {
    metadata.title = propVar.pwszVal;
  }
  PropVariantClear(&propVar);

  // Artist
  if (SUCCEEDED(propStore->GetValue(PKEY_Music_Artist, &propVar))) {
    if (propVar.vt == VT_LPWSTR) {
      metadata.artist = propVar.pwszVal;
    } else if (propVar.vt == (VT_VECTOR | VT_LPWSTR) &&
               propVar.calpwstr.cElems > 0) {
      metadata.artist = propVar.calpwstr.pElems[0];
    }
  }
  PropVariantClear(&propVar);

  // Album
  if (SUCCEEDED(propStore->GetValue(PKEY_Music_AlbumTitle, &propVar)) &&
      propVar.vt == VT_LPWSTR) {
    metadata.album = propVar.pwszVal;
  }
  PropVariantClear(&propVar);

  // Genre
  if (SUCCEEDED(propStore->GetValue(PKEY_Music_Genre, &propVar))) {
    if (propVar.vt == VT_LPWSTR) {
      metadata.genre = propVar.pwszVal;
    } else if (propVar.vt == (VT_VECTOR | VT_LPWSTR) &&
               propVar.calpwstr.cElems > 0) {
      metadata.genre = propVar.calpwstr.pElems[0];
    }
  }
  PropVariantClear(&propVar);

  // Duration (in 100-nanosecond units)
  if (SUCCEEDED(propStore->GetValue(PKEY_Media_Duration, &propVar)) &&
      propVar.vt == VT_UI8) {
    metadata.duration =
        (int)(propVar.uhVal.QuadPart / 10000000); // Convert to seconds
  }
  PropVariantClear(&propVar);

  // Bitrate
  if (SUCCEEDED(propStore->GetValue(PKEY_Audio_EncodingBitrate, &propVar)) &&
      propVar.vt == VT_UI4) {
    metadata.bitrate = (int)(propVar.ulVal / 1000); // Convert to kbps
  }
  PropVariantClear(&propVar);

  // Sample Rate
  if (SUCCEEDED(propStore->GetValue(PKEY_Audio_SampleRate, &propVar)) &&
      propVar.vt == VT_UI4) {
    metadata.sampleRate = (int)propVar.ulVal;
  }
  PropVariantClear(&propVar);

  // Channels
  if (SUCCEEDED(propStore->GetValue(PKEY_Audio_ChannelCount, &propVar)) &&
      propVar.vt == VT_UI4) {
    metadata.channels = (int)propVar.ulVal;
  }
  PropVariantClear(&propVar);

  return true;
}

} // namespace ExplorerLens
