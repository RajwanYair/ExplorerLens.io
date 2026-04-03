/******************************************************************************
 * video_thumbnail.h
 * Video Frame Extraction for ExplorerLens
 * Extracts thumbnails from video files using DirectShow
 ******************************************************************************/

#pragma once

#include <windows.h>

#include <string>

namespace ExplorerLens {

class VideoThumbnail
{
  public:
    // Extract frame from video file at specified position
    // position: 0.0 (start) to 1.0 (end), default 0.1 (10% into video)
    static HBITMAP ExtractFrame(const std::wstring& videoPath, double position = 0.1);

    // Check if DirectShow is available
    static bool IsDirectShowAvailable();

    // Get video duration in seconds
    static double GetVideoDuration(const std::wstring& videoPath);

  private:
    // Initialize COM for DirectShow
    static bool InitializeCOM();

    // Create DirectShow graph for video file
    static bool CreateGraphForVideo(const std::wstring& videoPath, void** ppGraph, void** ppMediaControl,
                                    void** ppMediaSeeking);
};

}  // namespace ExplorerLens
