///////////////////////////////////////////////////////////////////////////////
// ImageTypes.h — Image format type constants
//
// Extracted from LENSArchive.h (Sprint 355) to support modular header
// organization. These constants identify image sub-formats detected within
// archive/eBook containers for routing to the correct image decoder.
//
// Usage: #include "ImageTypes.h"
///////////////////////////////////////////////////////////////////////////////

#ifndef _IMAGETYPES_H_
#define _IMAGETYPES_H_

// ============================================================================
// Image format sub-type identifiers
// Used by CLENSArchive::DetectImageType() to classify extracted images
// ============================================================================

#define IMGTYPE_UNKNOWN     0
#define IMGTYPE_BMP         1
#define IMGTYPE_JPG         2
#define IMGTYPE_PNG         3
#define IMGTYPE_GIF         4
#define IMGTYPE_TIFF        5
#define IMGTYPE_WEBP        6
#define IMGTYPE_AVIF        7
#define IMGTYPE_JXL         8
#define IMGTYPE_HEIF        9
#define IMGTYPE_SVG         10
#define IMGTYPE_ICO         11
#define IMGTYPE_TGA         12
#define IMGTYPE_PSD         13
#define IMGTYPE_EXR         14
#define IMGTYPE_HDR         15
#define IMGTYPE_DDS         16
#define IMGTYPE_PNM         17     // PBM/PGM/PPM/PAM
#define IMGTYPE_QOI         18
#define IMGTYPE_XPM         19
#define IMGTYPE_XCF         20     // GIMP native
#define IMGTYPE_RAW         21     // Camera RAW (CR2/NEF/ARW/etc.)
#define IMGTYPE_JPEG2000    22     // JP2/J2K
#define IMGTYPE_VTF         23     // Valve Texture Format
#define IMGTYPE_KTX         24     // Khronos Texture
#define IMGTYPE_DPX         25     // Digital Picture Exchange
#define IMGTYPE_SGI         26     // Silicon Graphics Image
#define IMGTYPE_PCX         27     // ZSoft PC Paintbrush

// ============================================================================
// Image type detection helpers
// ============================================================================

/// Maximum number of image types defined
#define IMGTYPE_COUNT       28

/// Check if an image type is a standard web format
#define IMGTYPE_IS_WEB(t)   ((t) == IMGTYPE_JPG || (t) == IMGTYPE_PNG || \
                             (t) == IMGTYPE_GIF || (t) == IMGTYPE_WEBP || \
                             (t) == IMGTYPE_AVIF || (t) == IMGTYPE_JXL || \
                             (t) == IMGTYPE_SVG)

/// Check if an image type requires a specialized decoder
#define IMGTYPE_IS_SPECIAL(t) ((t) == IMGTYPE_PSD || (t) == IMGTYPE_EXR || \
                               (t) == IMGTYPE_HDR || (t) == IMGTYPE_DDS || \
                               (t) == IMGTYPE_RAW || (t) == IMGTYPE_VTF || \
                               (t) == IMGTYPE_KTX || (t) == IMGTYPE_DPX)

/// Check if an image type supports HDR/float pixel data
#define IMGTYPE_IS_HDR(t)   ((t) == IMGTYPE_EXR || (t) == IMGTYPE_HDR || \
                             (t) == IMGTYPE_DPX)

#endif // _IMAGETYPES_H_
