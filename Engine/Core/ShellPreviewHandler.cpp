#include "ShellPreviewHandler.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

ShellPreviewHandler::ShellPreviewHandler() = default;

const wchar_t* ShellPreviewHandler::GetModeName(PreviewMode mode) {
    switch (mode) {
        case PreviewMode::Thumbnail: return L"Thumbnail";
        case PreviewMode::FullImage: return L"Full Image";
        case PreviewMode::Filmstrip: return L"Filmstrip";
        case PreviewMode::Document:  return L"Document";
        case PreviewMode::HexDump:   return L"Hex Dump";
        default:                     return L"Unknown";
    }
}

const wchar_t* ShellPreviewHandler::GetStateName(PreviewState state) {
    switch (state) {
        case PreviewState::Unloaded: return L"Unloaded";
        case PreviewState::Loading:  return L"Loading";
        case PreviewState::Ready:    return L"Ready";
        case PreviewState::Error:    return L"Error";
        default:                     return L"Unknown";
    }
}

PreviewMode ShellPreviewHandler::DetectMode(const std::wstring& extension) {
    std::wstring ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    if (ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".bmp" ||
        ext == L".webp" || ext == L".avif" || ext == L".jxl" || ext == L".tiff")
        return PreviewMode::FullImage;
    if (ext == L".gif" || ext == L".apng")
        return PreviewMode::Filmstrip;
    if (ext == L".pdf" || ext == L".epub" || ext == L".doc" || ext == L".docx")
        return PreviewMode::Document;
    if (ext == L".bin" || ext == L".dat" || ext == L".exe")
        return PreviewMode::HexDump;
    return PreviewMode::Thumbnail;
}

bool ShellPreviewHandler::LoadFile(const PreviewParams& params) {
    m_state = PreviewState::Loading;
    m_params = params;
    if (params.filePath.empty()) {
        m_state = PreviewState::Error;
        return false;
    }
    m_state = PreviewState::Ready;
    return true;
}

void ShellPreviewHandler::Unload() {
    m_state = PreviewState::Unloaded;
    m_params = PreviewParams{};
}

}} // namespace ExplorerLens::Engine

