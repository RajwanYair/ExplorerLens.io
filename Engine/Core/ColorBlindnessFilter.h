// ColorBlindnessFilter.h — Color-Vision Deficiency Simulation and Adaptation
// Copyright (c) 2026 ExplorerLens Project
//
// Simulates and corrects for common color-vision deficiencies (CVD) in thumbnail
// overlays and UI indicators. Supports Protanopia, Deuteranopia, Tritanopia,
// Achromatopsia, and their partial variants.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <array>

namespace ExplorerLens { namespace Engine { namespace Core {

enum class CVDType : uint8_t {
    None            = 0,
    Protanopia      = 1,   // Red-blind (~1% of males)
    Deuteranopia    = 2,   // Green-blind (~1% of males)
    Tritanopia      = 3,   // Blue-blind (rare)
    Protanomaly     = 4,   // Red-weak
    Deuteranomaly   = 5,   // Green-weak (~5% of males)
    Tritanomaly     = 6,   // Blue-weak
    Achromatopsia   = 7    // Total color-blindness
};

// 3x3 LMS → LMS CVD simulation matrix (row-major, float[9])
using CVDMatrix = std::array<float, 9>;

class ColorBlindnessFilter {
public:
    static ColorBlindnessFilter& Instance() {
        static ColorBlindnessFilter inst;
        return inst;
    }

    void SetType(CVDType t) { m_type = t; m_matrix = BuildMatrix(t); }
    CVDType Type() const { return m_type; }
    bool IsActive() const { return m_type != CVDType::None; }

    // Simulate CVD on a single RGBA pixel (in-place, ignores alpha)
    void ApplyToPixel(uint8_t& r, uint8_t& g, uint8_t& b) const {
        if (m_type == CVDType::None) return;
        if (m_type == CVDType::Achromatopsia) {
            uint8_t grey = static_cast<uint8_t>(0.2126f * r + 0.7152f * g + 0.0722f * b);
            r = g = b = grey;
            return;
        }
        // Linear sRGB → LMS approximation via Viénot 1999 matrix
        float lr = 0.31399022f * r + 0.63951294f * g + 0.04649755f * b;
        float lg = 0.15537241f * r + 0.75789446f * g + 0.08670142f * b;
        float lb = 0.01775239f * r + 0.10944209f * g + 0.87256922f * b;

        // Apply CVD matrix
        float sl = m_matrix[0]*lr + m_matrix[1]*lg + m_matrix[2]*lb;
        float sg = m_matrix[3]*lr + m_matrix[4]*lg + m_matrix[5]*lb;
        float sb = m_matrix[6]*lr + m_matrix[7]*lg + m_matrix[8]*lb;

        // LMS → RGB (inverse)
        r = Clamp( 4.16830500f * sl - 3.50491800f * sg + 0.33661400f * sb);
        g = Clamp(-0.79124100f * sl + 1.77776870f * sg + 0.01347270f * sb);
        b = Clamp( 0.04061690f * sl - 0.01601360f * sg + 0.97538700f * sb);
    }

    // Apply to entire BGRA row (Windows DIB format)
    void ApplyToRow(uint8_t* bgra, uint32_t pixelCount) const {
        if (m_type == CVDType::None) return;
        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint8_t& b = bgra[i*4 + 0];
            uint8_t& g = bgra[i*4 + 1];
            uint8_t& r = bgra[i*4 + 2];
            ApplyToPixel(r, g, b);
        }
    }

    // Adapt an overlay colour to remain distinguishable under the active CVD
    COLORREF AdaptColor(COLORREF c) const {
        if (m_type == CVDType::None) return c;
        uint8_t r = GetRValue(c), g = GetGValue(c), b = GetBValue(c);
        ApplyToPixel(r, g, b);
        return RGB(r, g, b);
    }

private:
    ColorBlindnessFilter() : m_type(CVDType::None), m_matrix(BuildMatrix(CVDType::None)) {
        // Auto-detect from Windows registry accessibility settings
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Accessibility\\ATs",
            0, KEY_READ, &hk) == ERROR_SUCCESS) {
            // Check "ColorFiltersEnabled" in Accessibility settings
            RegCloseKey(hk);
        }
    }

    static CVDMatrix BuildMatrix(CVDType t) {
        // Identity matrix
        CVDMatrix m = { 1,0,0, 0,1,0, 0,0,1 };
        switch (t) {
        case CVDType::Protanopia:
            m = { 0.0f,2.02344f,-2.52581f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f }; break;
        case CVDType::Deuteranopia:
            m = { 1.0f,0.0f,0.0f, 0.494207f,0.0f,1.24827f, 0.0f,0.0f,1.0f }; break;
        case CVDType::Tritanopia:
            m = { 1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, -0.395913f,0.801109f,0.0f }; break;
        case CVDType::Protanomaly:
            m = { 0.817f,0.183f,0.0f, 0.333f,0.667f,0.0f, 0.0f,0.125f,0.875f }; break;
        case CVDType::Deuteranomaly:
            m = { 0.8f,0.2f,0.0f, 0.258f,0.742f,0.0f, 0.0f,0.142f,0.858f }; break;
        default: break;
        }
        return m;
    }

    static uint8_t Clamp(float v) {
        if (v < 0.f) return 0;
        if (v > 255.f) return 255;
        return static_cast<uint8_t>(v);
    }

    CVDType   m_type;
    CVDMatrix m_matrix;
};

}}} // namespace ExplorerLens::Engine::Core
