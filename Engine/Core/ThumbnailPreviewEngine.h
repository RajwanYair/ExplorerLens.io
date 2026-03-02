// ThumbnailPreviewEngine.h — Real-Time Thumbnail Preview Renderer (Sprint 586)
// Copyright (c) 2026 ExplorerLens Project
//
// Provides an interactive thumbnail preview with zoom, pan, checkerboard
// transparency background, pixel-grid overlay, and coordinate transforms.
// Renders via GDI (StretchDIBits with HALFTONE stretch mode) into a
// double-buffered memory DC, then BitBlt to the target. Designed for the
// LENSManager preview pane. Thread-safe via SRWLOCK.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Current view state for the preview engine.
struct ThumbnailPreviewState {
    float    zoomLevel      = 1.0f;
    float    panX           = 0.0f;
    float    panY           = 0.0f;
    uint32_t viewportWidth  = 0;
    uint32_t viewportHeight = 0;
    bool     showGrid       = false;
    bool     showInfo       = false;
};

/// Interactive thumbnail preview renderer with zoom and pan.
class ThumbnailPreviewEngine {
public:
    ThumbnailPreviewEngine() {
        InitializeSRWLock(&m_lock);
    }

    ~ThumbnailPreviewEngine() {
        AcquireSRWLockExclusive(&m_lock);
        m_imageData.clear();
        ReleaseSRWLockExclusive(&m_lock);
    }

    ThumbnailPreviewEngine(const ThumbnailPreviewEngine&) = delete;
    ThumbnailPreviewEngine& operator=(const ThumbnailPreviewEngine&) = delete;

    // ── Image loading ─────────────────────────────────────────────────────
    /// Load source image from RGBA pixel data (32 bpp, top-down).
    inline bool LoadImage(const uint8_t* rgbaData, uint32_t width, uint32_t height) {
        if (!rgbaData || width == 0 || height == 0) return false;

        size_t byteCount = static_cast<size_t>(width) * height * 4;

        AcquireSRWLockExclusive(&m_lock);
        m_imageWidth  = width;
        m_imageHeight = height;
        m_imageData.assign(rgbaData, rgbaData + byteCount);

        // Convert RGBA → BGRA for GDI (StretchDIBits expects BGRA in BI_RGB with 32bpp)
        for (size_t i = 0; i < byteCount; i += 4) {
            uint8_t tmp       = m_imageData[i];      // R
            m_imageData[i]     = m_imageData[i + 2];  // B
            m_imageData[i + 2] = tmp;                  // R
        }
        ReleaseSRWLockExclusive(&m_lock);
        return true;
    }

    // ── Rendering ─────────────────────────────────────────────────────────
    /// Render the current view to a device context.
    /// Uses double-buffering: draw to memory DC, then BitBlt to hdc.
    inline bool RenderToHDC(HDC hdc, const RECT& destRect) {
        if (!hdc) return false;

        int destW = destRect.right  - destRect.left;
        int destH = destRect.bottom - destRect.top;
        if (destW <= 0 || destH <= 0) return false;

        // Double-buffer: create memory DC
        HDC memDC = CreateCompatibleDC(hdc);
        if (!memDC) return false;

        HBITMAP memBmp = CreateCompatibleBitmap(hdc, destW, destH);
        if (!memBmp) { DeleteDC(memDC); return false; }
        HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

        // Fill background
        HBRUSH bgBrush = CreateSolidBrush(m_bgColor);
        RECT fullRect = { 0, 0, destW, destH };
        FillRect(memDC, &fullRect, bgBrush);
        DeleteObject(bgBrush);

        AcquireSRWLockShared(&m_lock);
        bool hasImage = !m_imageData.empty();
        uint32_t imgW = m_imageWidth;
        uint32_t imgH = m_imageHeight;

        if (hasImage) {
            // Draw checkerboard background for transparency
            if (m_useCheckerboard) {
                DrawCheckerboard(memDC, destW, destH);
            }

            // Compute image rect in viewport coordinates
            float zoom = m_state.zoomLevel;
            float renderW = static_cast<float>(imgW) * zoom;
            float renderH = static_cast<float>(imgH) * zoom;

            float offsetX = (static_cast<float>(destW) - renderW) * 0.5f + m_state.panX * zoom;
            float offsetY = (static_cast<float>(destH) - renderH) * 0.5f + m_state.panY * zoom;

            int dx = static_cast<int>(offsetX);
            int dy = static_cast<int>(offsetY);
            int dw = static_cast<int>(renderW);
            int dh = static_cast<int>(renderH);

            // Set HALFTONE stretch mode for quality downscaling
            SetStretchBltMode(memDC, HALFTONE);
            SetBrushOrgEx(memDC, 0, 0, nullptr);

            // Prepare BITMAPINFO for StretchDIBits (top-down BGRA)
            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth       = static_cast<LONG>(imgW);
            bmi.bmiHeader.biHeight      = -static_cast<LONG>(imgH);  // top-down
            bmi.bmiHeader.biPlanes      = 1;
            bmi.bmiHeader.biBitCount    = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            StretchDIBits(memDC,
                dx, dy, dw, dh,        // dest
                0, 0, static_cast<int>(imgW), static_cast<int>(imgH),  // src
                m_imageData.data(), &bmi,
                DIB_RGB_COLORS, SRCCOPY);

            // Draw pixel grid at high zoom
            if (m_state.showGrid && zoom >= 4.0f) {
                DrawPixelGrid(memDC, dx, dy, imgW, imgH, zoom);
            }

            // Info overlay
            if (m_state.showInfo) {
                DrawInfoOverlay(memDC, destW, destH, imgW, imgH);
            }
        }
        ReleaseSRWLockShared(&m_lock);

        // BitBlt to target
        BitBlt(hdc, destRect.left, destRect.top, destW, destH,
               memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);
        return true;
    }

    // ── Zoom & pan controls ───────────────────────────────────────────────
    inline void SetZoom(float level) {
        AcquireSRWLockExclusive(&m_lock);
        m_state.zoomLevel = (std::max)(0.1f, (std::min)(level, 10.0f));
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void SetPan(float x, float y) {
        AcquireSRWLockExclusive(&m_lock);
        m_state.panX = x;
        m_state.panY = y;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Compute zoom level to fit the entire image within the given viewport.
    inline void ZoomToFit(uint32_t viewportW, uint32_t viewportH) {
        AcquireSRWLockExclusive(&m_lock);
        if (m_imageWidth == 0 || m_imageHeight == 0) {
            ReleaseSRWLockExclusive(&m_lock);
            return;
        }
        float scaleX = static_cast<float>(viewportW) / static_cast<float>(m_imageWidth);
        float scaleY = static_cast<float>(viewportH) / static_cast<float>(m_imageHeight);
        m_state.zoomLevel = (std::max)(0.1f, (std::min)((std::min)(scaleX, scaleY), 10.0f));
        m_state.panX = 0.0f;
        m_state.panY = 0.0f;
        m_state.viewportWidth  = viewportW;
        m_state.viewportHeight = viewportH;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Zoom centered on a specific image point.
    inline void ZoomToPoint(float imgX, float imgY, float newZoom) {
        AcquireSRWLockExclusive(&m_lock);
        float oldZoom = m_state.zoomLevel;
        float clampedZoom = (std::max)(0.1f, (std::min)(newZoom, 10.0f));
        float ratio = clampedZoom / oldZoom;
        m_state.panX = imgX - (imgX - m_state.panX) * ratio;
        m_state.panY = imgY - (imgY - m_state.panY) * ratio;
        m_state.zoomLevel = clampedZoom;
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Background & overlays ─────────────────────────────────────────────
    inline void SetBackgroundColor(COLORREF color) {
        m_useCheckerboard = false;
        m_bgColor = color;
    }

    inline void EnableCheckerboard(bool enable) {
        m_useCheckerboard = enable;
    }

    inline void ShowPixelGrid(bool show) {
        AcquireSRWLockExclusive(&m_lock);
        m_state.showGrid = show;
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void ShowInfoOverlay(bool show) {
        AcquireSRWLockExclusive(&m_lock);
        m_state.showInfo = show;
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ── Coordinate transforms ─────────────────────────────────────────────
    /// Transform image coordinates to screen coordinates.
    inline POINT ImageToScreen(float imgX, float imgY) const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        float zoom = m_state.zoomLevel;
        float vpW  = static_cast<float>(m_state.viewportWidth);
        float vpH  = static_cast<float>(m_state.viewportHeight);
        float renderW = static_cast<float>(m_imageWidth)  * zoom;
        float renderH = static_cast<float>(m_imageHeight) * zoom;
        float offX = (vpW - renderW) * 0.5f + m_state.panX * zoom;
        float offY = (vpH - renderH) * 0.5f + m_state.panY * zoom;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        POINT pt;
        pt.x = static_cast<LONG>(offX + imgX * zoom);
        pt.y = static_cast<LONG>(offY + imgY * zoom);
        return pt;
    }

    /// Transform screen coordinates to image coordinates.
    inline std::pair<float, float> ScreenToImage(int screenX, int screenY) const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        float zoom = m_state.zoomLevel;
        float vpW  = static_cast<float>(m_state.viewportWidth);
        float vpH  = static_cast<float>(m_state.viewportHeight);
        float renderW = static_cast<float>(m_imageWidth)  * zoom;
        float renderH = static_cast<float>(m_imageHeight) * zoom;
        float offX = (vpW - renderW) * 0.5f + m_state.panX * zoom;
        float offY = (vpH - renderH) * 0.5f + m_state.panY * zoom;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        float imgX = (static_cast<float>(screenX) - offX) / zoom;
        float imgY = (static_cast<float>(screenY) - offY) / zoom;
        return { imgX, imgY };
    }

    // ── Mouse interaction ─────────────────────────────────────────────────
    /// Mouse wheel: zoom at the cursor position.
    inline void OnMouseWheel(int delta, int x, int y) {
        auto [imgX, imgY] = ScreenToImage(x, y);
        AcquireSRWLockShared(&m_lock);
        float currentZoom = m_state.zoomLevel;
        ReleaseSRWLockShared(&m_lock);

        float factor = (delta > 0) ? 1.15f : (1.0f / 1.15f);
        float newZoom = (std::max)(0.1f, (std::min)(currentZoom * factor, 10.0f));
        ZoomToPoint(imgX, imgY, newZoom);
    }

    /// Mouse drag: pan by the given screen delta.
    inline void OnMouseDrag(int dx, int dy) {
        AcquireSRWLockExclusive(&m_lock);
        float zoom = m_state.zoomLevel;
        if (zoom > 0.0f) {
            m_state.panX += static_cast<float>(dx) / zoom;
            m_state.panY += static_cast<float>(dy) / zoom;
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Get a copy of the current preview state.
    inline ThumbnailPreviewState GetState() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        ThumbnailPreviewState s = m_state;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return s;
    }

private:
    // ── Checkerboard ──────────────────────────────────────────────────────
    /// Draw an 8x8 pixel gray/white checkerboard pattern.
    static void DrawCheckerboard(HDC hdc, int width, int height) {
        const int cellSize = 8;
        COLORREF light = RGB(255, 255, 255);
        COLORREF dark  = RGB(204, 204, 204);
        HBRUSH lightBr = CreateSolidBrush(light);
        HBRUSH darkBr  = CreateSolidBrush(dark);

        for (int y = 0; y < height; y += cellSize) {
            for (int x = 0; x < width; x += cellSize) {
                bool isDark = ((x / cellSize) + (y / cellSize)) % 2 != 0;
                RECT cell;
                cell.left   = x;
                cell.top    = y;
                cell.right  = (std::min)(x + cellSize, width);
                cell.bottom = (std::min)(y + cellSize, height);
                FillRect(hdc, &cell, isDark ? darkBr : lightBr);
            }
        }
        DeleteObject(lightBr);
        DeleteObject(darkBr);
    }

    // ── Pixel grid ────────────────────────────────────────────────────────
    /// Draw grid lines between pixels at high zoom levels.
    static void DrawPixelGrid(HDC hdc, int dx, int dy,
                              uint32_t imgW, uint32_t imgH, float zoom) {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
        HGDIOBJ oldPen = SelectObject(hdc, pen);

        // Vertical lines
        for (uint32_t x = 0; x <= imgW; ++x) {
            int sx = dx + static_cast<int>(static_cast<float>(x) * zoom);
            MoveToEx(hdc, sx, dy, nullptr);
            LineTo(hdc, sx, dy + static_cast<int>(static_cast<float>(imgH) * zoom));
        }
        // Horizontal lines
        for (uint32_t y = 0; y <= imgH; ++y) {
            int sy = dy + static_cast<int>(static_cast<float>(y) * zoom);
            MoveToEx(hdc, dx, sy, nullptr);
            LineTo(hdc, dx + static_cast<int>(static_cast<float>(imgW) * zoom), sy);
        }

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    // ── Info overlay ──────────────────────────────────────────────────────
    /// Draw format / dimensions info in the bottom-left corner.
    static void DrawInfoOverlay(HDC hdc, int vpW, int vpH,
                                uint32_t imgW, uint32_t imgH) {
        (void)vpW;
        wchar_t buf[128]{};
        _snwprintf_s(buf, _TRUNCATE, L"%ux%u  RGBA 32bpp", imgW, imgH);

        // Semi-transparent background bar
        RECT bar;
        bar.left   = 0;
        bar.top    = vpH - 24;
        bar.right  = 300;
        bar.bottom = vpH;
        HBRUSH barBr = CreateSolidBrush(RGB(40, 40, 40));
        FillRect(hdc, &bar, barBr);
        DeleteObject(barBr);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(220, 220, 220));
        HFONT font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(hdc, font);

        RECT textRect = { 8, vpH - 22, 292, vpH - 2 };
        DrawTextW(hdc, buf, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        SelectObject(hdc, oldFont);
        DeleteObject(font);
    }

    // ── State ─────────────────────────────────────────────────────────────
    mutable SRWLOCK m_lock = SRWLOCK_INIT;
    ThumbnailPreviewState m_state{};

    std::vector<uint8_t> m_imageData;
    uint32_t m_imageWidth  = 0;
    uint32_t m_imageHeight = 0;

    COLORREF m_bgColor        = RGB(48, 48, 48);
    bool     m_useCheckerboard = true;
};

} // namespace Engine
} // namespace ExplorerLens
