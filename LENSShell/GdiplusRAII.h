#pragma once

#include <gdiplus.h>

#include <windows.h>

namespace ExplorerLens {

/// <summary>
/// RAII wrapper for GDI+ initialization and cleanup.
/// Ensures GdiplusStartup is paired with GdiplusShutdown.
/// Thread-safe singleton pattern.
/// </summary>
class GdiplusRAII
{
  public:
    // Get singleton instance
    static GdiplusRAII& GetInstance()
    {
        static GdiplusRAII instance;
        return instance;
    }

    // Delete copy/move constructors
    GdiplusRAII(const GdiplusRAII&) = delete;
    GdiplusRAII& operator=(const GdiplusRAII&) = delete;
    GdiplusRAII(GdiplusRAII&&) = delete;
    GdiplusRAII& operator=(GdiplusRAII&&) = delete;

    bool IsInitialized() const
    {
        return m_initialized;
    }

  private:
    GdiplusRAII()
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::Status status = Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
        m_initialized = (status == Gdiplus::Ok);
    }

    ~GdiplusRAII()
    {
        if (m_initialized) {
            Gdiplus::GdiplusShutdown(m_gdiplusToken);
            m_initialized = false;
        }
    }

    ULONG_PTR m_gdiplusToken = 0;
    bool m_initialized = false;
};

}  // namespace ExplorerLens
