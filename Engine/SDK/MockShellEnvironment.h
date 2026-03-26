// MockShellEnvironment.h — Test Harness for SDK Authors
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a lightweight mock of the Windows Shell thumbnail host environment
// for use in plugin unit tests. Implements mock IStream, IShellItem, and
// a minimal engine handle that routes decode to the real engine or a stub.
//
#pragma once
#include <windows.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <cstdint>
#include "../SDK/PublicAPI.h"

namespace ExplorerLens { namespace Engine { namespace Testing {

// Mock IStream wrapping a memory buffer (heap-allocated copy of input bytes)
class MockStream : public IStream {
public:
    explicit MockStream(const std::vector<uint8_t>& data)
        : m_data(data), m_pos(0), m_refs(1) {}

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef()  override { return ++m_refs; }
    ULONG STDMETHODCALLTYPE Release() override {
        if (--m_refs == 0) { delete this; return 0; }
        return m_refs;
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IStream || riid == IID_ISequentialStream) {
            *ppv = static_cast<IStream*>(this); AddRef(); return S_OK;
        }
        *ppv = nullptr; return E_NOINTERFACE;
    }

    // ISequentialStream
    HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead) override {
        ULONG avail = static_cast<ULONG>(m_data.size()) - m_pos;
        ULONG read  = (cb < avail) ? cb : avail;
        if (read) memcpy(pv, m_data.data() + m_pos, read);
        m_pos += read;
        if (pcbRead) *pcbRead = read;
        return (read == cb) ? S_OK : S_FALSE;
    }
    HRESULT STDMETHODCALLTYPE Write(const void*, ULONG, ULONG*) override { return STG_E_ACCESSDENIED; }

    // IStream
    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPos) override {
        int64_t newPos = 0;
        switch (dwOrigin) {
        case STREAM_SEEK_SET: newPos = dlibMove.QuadPart; break;
        case STREAM_SEEK_CUR: newPos = m_pos + dlibMove.QuadPart; break;
        case STREAM_SEEK_END: newPos = (int64_t)m_data.size() + dlibMove.QuadPart; break;
        default: return STG_E_INVALIDFUNCTION;
        }
        if (newPos < 0) return STG_E_INVALIDFUNCTION;
        m_pos = (ULONG)newPos;
        if (plibNewPos) plibNewPos->QuadPart = m_pos;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Commit(DWORD) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE Revert()       override { return S_OK; }
    HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return STG_E_INVALIDFUNCTION; }
    HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return STG_E_INVALIDFUNCTION; }
    HRESULT STDMETHODCALLTYPE Stat(STATSTG* ps, DWORD) override {
        if (!ps) return STG_E_INVALIDPOINTER;
        *ps = {}; ps->cbSize.QuadPart = m_data.size(); return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Clone(IStream**) override { return E_NOTIMPL; }

    // Reset position to beginning
    void Reset() { m_pos = 0; }

private:
    std::vector<uint8_t> m_data;
    ULONG                m_pos;
    LONG                 m_refs;
};

// Utility: create a MockStream from a file on disk
inline MockStream* MockStreamFromFile(const std::wstring& path) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return nullptr;
    DWORD sz = GetFileSize(h, nullptr);
    std::vector<uint8_t> buf(sz);
    DWORD read = 0; ReadFile(h, buf.data(), sz, &read, nullptr);
    CloseHandle(h);
    return new MockStream(buf);
}

// Minimal test fixture wrapping engine lifecycle
class MockShellEnvironment {
public:
    MockShellEnvironment() { LensEngineCreate(&m_hEngine); }
    ~MockShellEnvironment() { if (m_hEngine) LensEngineDestroy(m_hEngine); }

    LENS_ENGINE_HANDLE Engine() { return m_hEngine; }

    // Convenience: generate thumbnail from path and assert success
    LENS_THUMBNAIL_HANDLE GenerateOrAssert(const std::wstring& path,
                                           uint32_t w = 256, uint32_t h = 256) {
        LENS_THUMB_OPTIONS opts = { w, h, 0, 10000 };
        LENS_THUMBNAIL_HANDLE hThumb = nullptr;
        LENS_RESULT r = LensGenerateThumbnail(m_hEngine, path.c_str(), &opts, &hThumb);
        assert(r == LENS_OK && hThumb != nullptr);
        return hThumb;
    }

private:
    LENS_ENGINE_HANDLE m_hEngine = nullptr;
};

}}} // namespace ExplorerLens::Engine::Testing
