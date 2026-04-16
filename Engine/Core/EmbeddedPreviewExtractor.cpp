// EmbeddedPreviewExtractor.cpp — LibRaw embedded-thumb fast path
// Copyright (c) 2026 ExplorerLens Project
//
#include "EmbeddedPreviewExtractor.h"

#include <cstring>
#include <stdexcept>

// LibRaw public API header
#if __has_include(<libraw/libraw.h>)
#  include <libraw/libraw.h>
#  define HAVE_LIBRAW 1
#else
#  define HAVE_LIBRAW 0
#endif

namespace ExplorerLens {
namespace Engine {

EmbeddedPreviewExtractor::EmbeddedPreviewExtractor() = default;
EmbeddedPreviewExtractor::~EmbeddedPreviewExtractor() = default;

void EmbeddedPreviewExtractor::SetMinimumResolution(uint32_t w, uint32_t h) noexcept
{
    m_minWidth  = w;
    m_minHeight = h;
}

#if HAVE_LIBRAW

std::optional<EmbeddedPreviewResult> EmbeddedPreviewExtractor::Extract(
    std::string_view filePath) const
{
    LibRaw raw;
    std::string pathStr(filePath);

    if (LIBRAW_SUCCESS != raw.open_file(pathStr.c_str())) return std::nullopt;

    return DoExtract(&raw);
}

std::optional<EmbeddedPreviewResult> EmbeddedPreviewExtractor::ExtractFromBuffer(
    const uint8_t* data, size_t size) const
{
    LibRaw raw;
    if (LIBRAW_SUCCESS != raw.open_buffer(data, size)) return std::nullopt;

    return DoExtract(&raw);
}

std::optional<EmbeddedPreviewResult> EmbeddedPreviewExtractor::DoExtract(
    void* handle) const
{
    LibRaw& raw = *static_cast<LibRaw*>(handle);

    // Try embedded (JPEG) thumbnail first
    if (LIBRAW_SUCCESS == raw.unpack_thumb()) {
        libraw_processed_image_t* img = raw.dcraw_make_mem_thumb(nullptr);
        if (img && img->type == LIBRAW_IMAGE_JPEG && img->data_size > 0) {
            uint32_t w = static_cast<uint32_t>(img->width);
            uint32_t h = static_cast<uint32_t>(img->height);

            if (w >= m_minWidth && h >= m_minHeight) {
                EmbeddedPreviewResult res;
                res.jpegData.assign(img->data, img->data + img->data_size);
                res.width        = w;
                res.height       = h;
                res.fromEmbedded = true;
                LibRaw::dcraw_clear_mem(img);
                return res;
            }
            LibRaw::dcraw_clear_mem(img);
        }
    }

    // Fallback: full decode → extract JPEG from output bitmap
    // (unpack + dcraw_process is expensive; only done if no embedded thumb)
    if (LIBRAW_SUCCESS != raw.unpack())     return std::nullopt;
    if (LIBRAW_SUCCESS != raw.dcraw_process()) return std::nullopt;

    int err = 0;
    libraw_processed_image_t* img = raw.dcraw_make_mem_image(&err);
    if (!img || err != LIBRAW_SUCCESS)  return std::nullopt;

    // img is RGB8; we return it wrapped as-is; caller must re-encode if needed.
    // Here we copy to a JPEG placeholder (real encode would use TurboJPEG).
    EmbeddedPreviewResult res;
    res.width        = static_cast<uint32_t>(img->width);
    res.height       = static_cast<uint32_t>(img->height);
    res.fromEmbedded = false;
    // Store raw RGB data in jpegData field (caller must check fromEmbedded
    // to determine whether this is JPEG or raw RGB8)
    res.jpegData.assign(img->data, img->data + img->data_size);
    LibRaw::dcraw_clear_mem(img);
    return res;
}

#else // !HAVE_LIBRAW — stub

std::optional<EmbeddedPreviewResult> EmbeddedPreviewExtractor::Extract(
    std::string_view) const
{
    return std::nullopt;
}

std::optional<EmbeddedPreviewResult> EmbeddedPreviewExtractor::ExtractFromBuffer(
    const uint8_t*, size_t) const
{
    return std::nullopt;
}

std::optional<EmbeddedPreviewResult> EmbeddedPreviewExtractor::DoExtract(void*) const
{
    return std::nullopt;
}

#endif // HAVE_LIBRAW

} // namespace Engine
} // namespace ExplorerLens
