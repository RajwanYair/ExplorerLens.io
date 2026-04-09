// GPUDecodeFormatRouter.cpp — Format-to-GPU-Path Routing Table
// Copyright (c) 2026 ExplorerLens Project
//
#include "GPU/GPUDecodeFormatRouter.h"
#include <algorithm>
#include <cwchar>

namespace ExplorerLens { namespace Engine {

namespace {
    GPUDecodeFormatRouter g_globalRouter;
} // anonymous namespace

GPUDecodeFormatRouter& GetGlobalFormatRouter() noexcept
{
    return g_globalRouter;
}

GPUDecodeFormatRouter::GPUDecodeFormatRouter()
{
    BuildDefaultTable();
}

void GPUDecodeFormatRouter::BuildDefaultTable()
{
    // JPEG / JPEG 2000 — NVJPEG primary, QSV secondary
    for (const wchar_t* ext : { L".jpg", L".jpeg", L".jpe", L".jfif" }) {
        m_extTable[ext] = { GPUDecodePathId::NVJPEG, GPUDecodePathId::QSV_JPEG, 1.5f };
        m_mimeTable["image/jpeg"] = { GPUDecodePathId::NVJPEG, GPUDecodePathId::QSV_JPEG, 1.5f };
    }

    // PNG — DirectCompute inflate + unfilter
    for (const wchar_t* ext : { L".png" }) {
        m_extTable[ext] = { GPUDecodePathId::DirectCompute_PNG, GPUDecodePathId::CPUFallback, 2.0f };
        m_mimeTable["image/png"] = { GPUDecodePathId::DirectCompute_PNG, GPUDecodePathId::CPUFallback, 2.0f };
    }

    // AVIF — NVDEC AV1 primary, Intel QSV AV1 secondary
    for (const wchar_t* ext : { L".avif", L".avifs" }) {
        m_extTable[ext] = { GPUDecodePathId::NVDEC_AV1, GPUDecodePathId::QSV_AV1, 3.5f };
        m_mimeTable["image/avif"] = { GPUDecodePathId::NVDEC_AV1, GPUDecodePathId::QSV_AV1, 3.5f };
    }

    // HEIC / HEIF — NVDEC HEVC primary, QSV HEVC secondary
    for (const wchar_t* ext : { L".heic", L".heif", L".hif" }) {
        m_extTable[ext] = { GPUDecodePathId::NVDEC_HEVC, GPUDecodePathId::QSV_HEVC, 4.0f };
        m_mimeTable["image/heic"]  = { GPUDecodePathId::NVDEC_HEVC, GPUDecodePathId::QSV_HEVC, 4.0f };
        m_mimeTable["image/heif"]  = { GPUDecodePathId::NVDEC_HEVC, GPUDecodePathId::QSV_HEVC, 4.0f };
    }

    // RAW camera — GPU demosaic kernel
    for (const wchar_t* ext : {
        L".cr2", L".cr3", L".nef", L".nrw", L".arw", L".raf",
        L".orf", L".rw2", L".dng", L".3fr", L".fff", L".iiq"
    }) {
        m_extTable[ext] = { GPUDecodePathId::GPUDemosaic, GPUDecodePathId::CPUFallback, 9.0f };
    }
    m_mimeTable["image/x-raw"] = { GPUDecodePathId::GPUDemosaic, GPUDecodePathId::CPUFallback, 9.0f };

    // PDF — D2D + DirectWrite GPU rasterizer
    for (const wchar_t* ext : { L".pdf" }) {
        m_extTable[ext] = { GPUDecodePathId::D2D_PDF, GPUDecodePathId::CPUFallback, 8.0f };
        m_mimeTable["application/pdf"] = { GPUDecodePathId::D2D_PDF, GPUDecodePathId::CPUFallback, 8.0f };
    }
}

static std::wstring ToLower(const wchar_t* s)
{
    std::wstring out(s);
    std::transform(out.begin(), out.end(), out.begin(), ::towlower);
    return out;
}

GPUDecodeRoute GPUDecodeFormatRouter::RouteByExtension(const wchar_t* ext) const noexcept
{
    if (!ext) return { GPUDecodePathId::CPUFallback, GPUDecodePathId::CPUFallback, 0.0f };
    auto it = m_extTable.find(ToLower(ext));
    if (it != m_extTable.end()) return it->second;
    return { GPUDecodePathId::CPUFallback, GPUDecodePathId::CPUFallback, 0.0f };
}

GPUDecodeRoute GPUDecodeFormatRouter::RouteByMime(const char* mime) const noexcept
{
    if (!mime) return { GPUDecodePathId::CPUFallback, GPUDecodePathId::CPUFallback, 0.0f };
    auto it = m_mimeTable.find(mime);
    if (it != m_mimeTable.end()) return it->second;
    return { GPUDecodePathId::CPUFallback, GPUDecodePathId::CPUFallback, 0.0f };
}

void GPUDecodeFormatRouter::SetRoute(const wchar_t* ext,
                                     GPUDecodePathId primary,
                                     GPUDecodePathId secondary,
                                     float targetMs)
{
    if (!ext) return;
    m_extTable[ToLower(ext)] = { primary, secondary, targetMs };
}

}} // namespace ExplorerLens::Engine
