/******************************************************************************
 * thumbnail_collage.h
 * Multi-Page Thumbnail Collage for DarkThumbs
 * Sprint C2: Show multiple pages in a grid (2x2, 3x3, 4x4)
 ******************************************************************************/

#pragma once

#include <windows.h>
#include <vector>

namespace DarkThumbs {

class ThumbnailCollage {
public:
    // Collage modes
    enum CollageMode {
        MODE_SINGLE = 1,   // 1x1 - single image (default)
        MODE_2X2 = 4,      // 2x2 - 4 images
        MODE_3X3 = 9,      // 3x3 - 9 images
        MODE_4X4 = 16      // 4x4 - 16 images
    };

    // Create collage from multiple bitmaps
    static HBITMAP CreateCollage(
        const std::vector<HBITMAP>& pages,
        int targetWidth,
        int targetHeight,
        CollageMode mode = MODE_2X2
    );

    // Get collage mode from registry
    static CollageMode GetCollageModeFromRegistry();

    // Set collage mode to registry
    static bool SetCollageModeToRegistry(CollageMode mode);

private:
    // Calculate grid dimensions
    static void GetGridDimensions(CollageMode mode, int& cols, int& rows);

    // Draw single bitmap into composite
    static bool DrawBitmapToComposite(
        HDC hdcDest,
        int x, int y,
        int width, int height,
        HBITMAP hSrc
    );
};

} // namespace DarkThumbs
