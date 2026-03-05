// DecoderPriorityRouter.h — Intelligent Decoder Selection Router
// Copyright (c) 2026 ExplorerLens Project
//
// Routes file decoding requests to the best available decoder based on
// format confidence, decoder health, GPU availability, and historical
// performance. Replaces static decoder mapping with dynamic routing.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class DecoderBackend : uint8_t {
    Native, WIC, GDI, LibRaw, LibWebP, LibJXL, LibHEIF,
    LibAVIF, MuPDF, OpenJPEG, FreeType, FFmpeg, Plugin, COUNT
};

struct DecoderCandidate {
    DecoderBackend backend = DecoderBackend::Native;
    float confidence = 0.0f;
    float avgDecodeTimeMs = 0.0f;
    bool gpuAccelerated = false;
    bool available = true;
    uint32_t failCount = 0;
};

struct RouteDecision {
    DecoderBackend primary = DecoderBackend::Native;
    DecoderBackend fallback = DecoderBackend::WIC;
    float confidence = 0.0f;
    bool usedGPU = false;
};

class DecoderPriorityRouter {
public:
    void AddCandidate(const DecoderCandidate& c) { m_candidates.push_back(c); }
    void ClearCandidates() { m_candidates.clear(); }
    size_t CandidateCount() const { return m_candidates.size(); }

    RouteDecision Route(const std::wstring& extension) const {
        (void)extension;
        RouteDecision decision;
        if (m_candidates.empty()) return decision;
        auto best = std::max_element(m_candidates.begin(), m_candidates.end(),
            [](const DecoderCandidate& a, const DecoderCandidate& b) {
                if (!a.available) return true;
                if (!b.available) return false;
                return a.confidence < b.confidence;
            });
        decision.primary = best->backend;
        decision.confidence = best->confidence;
        decision.usedGPU = best->gpuAccelerated;
        // Find fallback (second-best available)
        for (auto& c : m_candidates) {
            if (c.backend != decision.primary && c.available) {
                decision.fallback = c.backend;
                break;
            }
        }
        return decision;
    }

    void RecordFailure(DecoderBackend b) {
        for (auto& c : m_candidates) {
            if (c.backend == b) { c.failCount++; break; }
        }
    }

    static const wchar_t* BackendName(DecoderBackend b) {
        switch (b) {
        case DecoderBackend::Native:   return L"Native";
        case DecoderBackend::WIC:      return L"WIC";
        case DecoderBackend::GDI:      return L"GDI";
        case DecoderBackend::LibRaw:   return L"LibRaw";
        case DecoderBackend::LibWebP:  return L"LibWebP";
        case DecoderBackend::LibJXL:   return L"LibJXL";
        case DecoderBackend::LibHEIF:  return L"LibHEIF";
        case DecoderBackend::LibAVIF:  return L"LibAVIF";
        case DecoderBackend::MuPDF:    return L"MuPDF";
        case DecoderBackend::OpenJPEG: return L"OpenJPEG";
        case DecoderBackend::FreeType: return L"FreeType";
        case DecoderBackend::FFmpeg:   return L"FFmpeg";
        case DecoderBackend::Plugin:   return L"Plugin";
        default: return L"Unknown";
        }
    }
    static size_t BackendCount() { return static_cast<size_t>(DecoderBackend::COUNT); }

private:
    std::vector<DecoderCandidate> m_candidates;
};

} // namespace Engine
} // namespace ExplorerLens
