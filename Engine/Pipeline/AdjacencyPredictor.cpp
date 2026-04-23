// AdjacencyPredictor.cpp — Sibling Directory Adjacency Predictor
// Copyright (c) 2026 ExplorerLens Project
//
#include "Pipeline/AdjacencyPredictor.h"
#include <algorithm>

#if defined(_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#     define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>
#endif

namespace ExplorerLens { namespace Engine {

AdjacencyPredictor::AdjacencyPredictor(
    const AdjacencyPredictorConfig& config) noexcept
    : m_config(config)
{}

void AdjacencyPredictor::RecordNavigation(const std::wstring& dirPath) noexcept
{
    // Avoid duplicate consecutive entries.
    if (!m_mru.empty() && m_mru.front() == dirPath) return;

    m_mru.insert(m_mru.begin(), dirPath);
    if (m_mru.size() > static_cast<size_t>(m_config.mruHistoryDepth)) {
        m_mru.resize(m_config.mruHistoryDepth);
    }
}

// Returns the parent directory of `path` (everything before last '\\').
static std::wstring ParentOf(const std::wstring& path) noexcept
{
    const size_t sep = path.rfind(L'\\');
    if (sep == std::wstring::npos || sep == 0) return path;
    return path.substr(0, sep);
}

std::vector<std::wstring> AdjacencyPredictor::GetSiblings(
    const std::wstring& dirPath) noexcept
{
    std::vector<std::wstring> result;
#if defined(_WIN32)
    const std::wstring parent = ParentOf(dirPath);
    std::wstring pattern = parent + L"\\*";
    WIN32_FIND_DATAW fd{};
    HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return result;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        if (fd.cFileName[0] == L'.') continue;  // Skip . and ..
        std::wstring candidate = parent + L"\\" + fd.cFileName;
        if (candidate != dirPath) result.push_back(std::move(candidate));
    } while (FindNextFileW(h, &fd));
    FindClose(h);
#endif
    return result;
}

std::vector<AdjacencyPrediction> AdjacencyPredictor::Predict(
    const std::wstring& currentDir, uint32_t maxResults) const noexcept
{
    std::vector<AdjacencyPrediction> predictions;
    if (maxResults == 0) return predictions;

    // MRU-based predictions: directories recently visited that share the same parent.
    const std::wstring parent = ParentOf(currentDir);
    for (const auto& mruDir : m_mru) {
        if (mruDir == currentDir) continue;
        if (ParentOf(mruDir) != parent) continue;  // Only siblings for now

        AdjacencyPrediction pred{};
        pred.directoryPath = mruDir;
        pred.confidence    = 0.7f;
        pred.isSibling     = true;
        predictions.push_back(std::move(pred));
        if (predictions.size() >= maxResults) return predictions;
    }

    // Fill remaining slots with adjacent siblings not in MRU.
    auto siblings = GetSiblings(currentDir);
    for (auto& sib : siblings) {
        if (predictions.size() >= maxResults) break;
        bool alreadyIn = false;
        for (const auto& p : predictions)
            if (p.directoryPath == sib) { alreadyIn = true; break; }
        if (alreadyIn) continue;

        AdjacencyPrediction pred{};
        pred.directoryPath = sib;
        pred.confidence    = 0.35f;
        pred.isSibling     = true;
        predictions.push_back(std::move(pred));
    }

    // Filter by minimum confidence.
    predictions.erase(
        std::remove_if(predictions.begin(), predictions.end(),
            [this](const AdjacencyPrediction& p) {
                return p.confidence < m_config.minConfidence;
            }),
        predictions.end());

    return predictions;
}

void AdjacencyPredictor::Reset() noexcept { m_mru.clear(); }

}} // namespace ExplorerLens::Engine
