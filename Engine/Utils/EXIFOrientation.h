//==============================================================================
// ExplorerLens Engine - EXIF Orientation Utilities
// Copyright (c) 2026 - ExplorerLens Project
// Version: 6.0.0
//
// Shared utility for applying EXIF orientation transformations to bitmaps.
// Supports all 8 EXIF orientation cases (1-8).
//==============================================================================

#pragma once

#include <windows.h>

namespace ExplorerLens {
namespace Engine {
namespace Utils {

/// EXIF Orientation Tag Values
/// Based on EXIF 2.3 specification
enum class EXIFOrientation : int {
 Normal = 1, // No transformation
 FlipHorizontal = 2, // Flip horizontally (mirror)
 Rotate180 = 3, // Rotate 180 degrees
 FlipVertical = 4, // Flip vertically
 Transpose = 5, // Flip horizontal + rotate 90 CW
 Rotate90CW = 6, // Rotate 90 degrees clockwise
 Transverse = 7, // Flip horizontal + rotate 270 CW
 Rotate270CW = 8 // Rotate 270 degrees clockwise (90 CCW)
};

/// Apply EXIF orientation transformation to an HBITMAP
///
/// @param hBitmap - Source bitmap handle (GDI HBITMAP)
/// @param orientation - EXIF orientation value (1-8), or 0 for no transformation
/// @return Transformed bitmap handle, or NULL on failure.
/// If orientation is 1 (Normal) or 0, returns the original hBitmap.
/// Otherwise returns a new bitmap - caller must delete the original if needed.
///
/// @note The function creates a new bitmap for transformations (cases 2-8).
/// The original bitmap is NOT deleted - caller must manage bitmap lifetimes.
/// For case 1 (Normal) and invalid values, returns the original bitmap unchanged.
///
/// @example
/// HBITMAP hOriginal = LoadBitmapFromFile(path);
/// int exifOrientation = ReadEXIFOrientation(path);
/// HBITMAP hCorrected = ApplyEXIFOrientation(hOriginal, exifOrientation);
/// if (hCorrected && hCorrected != hOriginal) {
/// DeleteObject(hOriginal); // Delete original, use corrected
/// }
HBITMAP ApplyEXIFOrientation(HBITMAP hBitmap, int orientation);

} // namespace Utils
} // namespace Engine
} // namespace ExplorerLens
