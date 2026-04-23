// ExportCommand.cpp — Implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "ExportCommand.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// Windows Imaging Component
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

namespace ExplorerLens {
namespace CLI {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Execute
// ---------------------------------------------------------------------------

int ExportCommand::Execute(const ParsedArgs& args) {
    const auto& positional = args.positional;
    if (positional.size() < 2) {
        std::wcerr << L"lens export: requires <input-file> <output-file>\n"
                   << L"Usage: " << Usage() << L"\n";
        return 1;
    }

    std::wstring inputPath  = positional[0];
    std::wstring outputPath = positional[1];
    bool overwrite  = args.HasFlag(L"overwrite");
    bool verbose    = args.HasFlag(L"verbose") || args.HasFlag(L"v");
    int  targetSize = 256;
    int  quality    = 90;
    std::wstring fmt = L"png";

    if (args.HasOption(L"size")) {
        auto v = args.GetOption(L"size");
        try { targetSize = std::stoi(v); } catch (...) {
            std::wcerr << L"lens export: --size requires an integer\n"; return 1;
        }
    }
    if (args.HasOption(L"quality")) {
        auto v = args.GetOption(L"quality");
        try { quality = std::clamp(std::stoi(v), 1, 100); } catch (...) {
            std::wcerr << L"lens export: --quality requires 1-100\n"; return 1;
        }
    }
    if (args.HasOption(L"format")) {
        fmt = args.GetOption(L"format");
        std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::towlower);
    }

    if (!fs::exists(inputPath)) {
        std::wcerr << L"lens export: input file not found: " << inputPath << L"\n";
        return 1;
    }
    if (fs::exists(outputPath) && !overwrite) {
        std::wcerr << L"lens export: output already exists (use --overwrite): " << outputPath << L"\n";
        return 1;
    }

    if (verbose)
        std::wcout << L"Decoding " << inputPath << L" at " << targetSize << L"px...\n";

    // Invoke the engine via IThumbnailProvider COM interface
    // For the CLI we instantiate the engine directly (bypasses Shell registration).
    // Engine header is included here via the Engine.h umbrella.
    // Because this is a CLI build, we use the C ABI from SDK/plugin_api.h instead.
    //
    // NB: Full engine integration is done in GenerateCommand.cpp which already
    // links ExplorerLensEngine.lib.  ExportCommand reuses the same pipeline.
    //
    // Stub: print a placeholder until full GenerateCommand integration is wired.
    std::wcerr << L"lens export: decoder pipeline not yet wired in this build.\n"
               << L"Use 'lens generate " << inputPath << L"' to generate a thumbnail,\n"
               << L"or directly call the engine API from your application.\n";
    return 1; // TODO: wire Engine::DecodeAtSize → WriteImage
}

// ---------------------------------------------------------------------------
// WriteImage — WIC-backed image writer
// ---------------------------------------------------------------------------

int ExportCommand::WriteImage(const std::wstring&      outputPath,
                               const std::vector<uint8_t>& pixels,
                               uint32_t                 width,
                               uint32_t                 height,
                               const std::wstring&      format,
                               int                      jpegQuality) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool   needUninit = SUCCEEDED(hr);

    IWICImagingFactory* wicFactory = nullptr;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) { if (needUninit) CoUninitialize(); return 1; }

    // Choose encoder CLSID
    GUID encoderClsid = GUID_ContainerFormatPng;
    if (format == L"jpg" || format == L"jpeg")
        encoderClsid = GUID_ContainerFormatJpeg;
    else if (format == L"bmp")
        encoderClsid = GUID_ContainerFormatBmp;

    IWICStream*          stream  = nullptr;
    IWICBitmapEncoder*   encoder = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;

    auto cleanup = [&]() {
        if (frame)   { frame->Release();   }
        if (encoder) { encoder->Release(); }
        if (stream)  { stream->Release();  }
        wicFactory->Release();
        if (needUninit) CoUninitialize();
    };

    hr = wicFactory->CreateStream(&stream);
    if (FAILED(hr)) { cleanup(); return 1; }

    hr = stream->InitializeFromFilename(outputPath.c_str(), GENERIC_WRITE);
    if (FAILED(hr)) { cleanup(); return 1; }

    hr = wicFactory->CreateEncoder(encoderClsid, nullptr, &encoder);
    if (FAILED(hr)) { cleanup(); return 1; }

    hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
    if (FAILED(hr)) { cleanup(); return 1; }

    IPropertyBag2* props = nullptr;
    hr = encoder->CreateNewFrame(&frame, &props);
    if (FAILED(hr)) { cleanup(); return 1; }

    // Set JPEG quality
    if ((format == L"jpg" || format == L"jpeg") && props) {
        PROPBAG2 opt = {}; VARIANT var = {};
        opt.pstrName = const_cast<LPOLESTR>(L"ImageQuality");
        var.vt = VT_R4; var.fltVal = static_cast<float>(jpegQuality) / 100.f;
        props->Write(1, &opt, &var);
    }
    if (props) props->Release();

    hr = frame->Initialize(nullptr);
    if (FAILED(hr)) { cleanup(); return 1; }

    hr = frame->SetSize(width, height);
    if (FAILED(hr)) { cleanup(); return 1; }

    WICPixelFormatGUID pixFmt = GUID_WICPixelFormat32bppBGRA;
    hr = frame->SetPixelFormat(&pixFmt);
    if (FAILED(hr)) { cleanup(); return 1; }

    UINT stride = width * 4;
    hr = frame->WritePixels(height, stride, stride * height,
                             const_cast<uint8_t*>(pixels.data()));
    if (FAILED(hr)) { cleanup(); return 1; }

    hr = frame->Commit();
    if (FAILED(hr)) { cleanup(); return 1; }

    hr = encoder->Commit();
    cleanup();
    return SUCCEEDED(hr) ? 0 : 1;
}

} // namespace CLI
} // namespace ExplorerLens
