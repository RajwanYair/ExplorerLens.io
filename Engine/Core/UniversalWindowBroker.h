// UniversalWindowBroker.h — Cross-Platform Window/Surface Broker
// Copyright (c) 2026 ExplorerLens Project
//
// Provides an abstract ISurfaceHandle interface with platform-specific
// implementations for creating and managing render surfaces across OS boundaries.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ExplorerLens { namespace Engine {

enum class SurfaceBackend : uint8_t {
    Win32   = 0,
    Cocoa   = 1,
    X11     = 2,
    Wayland = 3,
    Offscreen = 4
};

struct SurfaceDimensions {
    uint32_t widthPx    = 0;
    uint32_t heightPx   = 0;
    float    scaleFactor = 1.0f;

    uint32_t PhysicalWidth() const  { return static_cast<uint32_t>(widthPx * scaleFactor); }
    uint32_t PhysicalHeight() const { return static_cast<uint32_t>(heightPx * scaleFactor); }
};

struct SurfaceCreateInfo {
    uint32_t       width    = 256;
    uint32_t       height   = 256;
    SurfaceBackend backend  = SurfaceBackend::Offscreen;
    bool           visible  = false;
    std::string    title;
};

class ISurfaceHandle {
public:
    virtual ~ISurfaceHandle() = default;
    virtual void*             GetNativeHandle() const = 0;
    virtual SurfaceDimensions GetDimensions() const   = 0;
    virtual SurfaceBackend    GetBackend() const       = 0;
    virtual bool              IsValid() const          = 0;
    virtual void              Destroy()                = 0;
};

class OffscreenSurface final : public ISurfaceHandle {
public:
    OffscreenSurface(uint32_t w, uint32_t h)
        : m_dims{ w, h, 1.0f }, m_valid(w > 0 && h > 0) {}

    void*             GetNativeHandle() const override { return nullptr; }
    SurfaceDimensions GetDimensions() const override   { return m_dims; }
    SurfaceBackend    GetBackend() const override      { return SurfaceBackend::Offscreen; }
    bool              IsValid() const override         { return m_valid; }
    void              Destroy() override               { m_valid = false; }

private:
    SurfaceDimensions m_dims;
    bool              m_valid = false;
};

#ifdef _WIN32
class Win32Surface final : public ISurfaceHandle {
public:
    Win32Surface(uint32_t w, uint32_t h, const std::string& title)
        : m_dims{ w, h, 1.0f } {
        m_hwnd = CreateWindowExA(0, "STATIC", title.c_str(),
                                 WS_OVERLAPPED, 0, 0, w, h,
                                 nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        m_valid = (m_hwnd != nullptr);
    }

    ~Win32Surface() override { Destroy(); }

    void* GetNativeHandle() const override { return m_hwnd; }
    SurfaceDimensions GetDimensions() const override { return m_dims; }
    SurfaceBackend    GetBackend() const override    { return SurfaceBackend::Win32; }
    bool              IsValid() const override       { return m_valid; }

    void Destroy() override {
        if (m_hwnd) { DestroyWindow(static_cast<HWND>(m_hwnd)); m_hwnd = nullptr; }
        m_valid = false;
    }

private:
    SurfaceDimensions m_dims;
    void*             m_hwnd  = nullptr;
    bool              m_valid = false;
};
#endif

class UniversalWindowBroker {
public:
    static UniversalWindowBroker& Instance() {
        static UniversalWindowBroker s_instance;
        return s_instance;
    }

    std::unique_ptr<ISurfaceHandle> CreateSurface(const SurfaceCreateInfo& info) {
        if (info.width == 0 || info.height == 0 || info.width > MAX_DIM || info.height > MAX_DIM) {
            return nullptr;
        }

#ifdef _WIN32
        if (info.backend == SurfaceBackend::Win32) {
            return std::make_unique<Win32Surface>(info.width, info.height, info.title);
        }
#endif
        return std::make_unique<OffscreenSurface>(info.width, info.height);
    }

    SurfaceBackend GetDefaultBackend() const {
#ifdef _WIN32
        return SurfaceBackend::Win32;
#elif defined(__APPLE__)
        return SurfaceBackend::Cocoa;
#elif defined(__linux__)
        return SurfaceBackend::Wayland;
#else
        return SurfaceBackend::Offscreen;
#endif
    }

private:
    UniversalWindowBroker() = default;
    static constexpr uint32_t MAX_DIM = 16384;
};

}} // namespace ExplorerLens::Engine
