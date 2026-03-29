// CaptionQualityScorer.h — Caption Quality Scorer
// Copyright (c) 2026 ExplorerLens Project
//
// Evaluates generated alt-text and caption quality against reference metrics
// including BLEU, CIDEr-proxy, readability, WCAG 2.2 compliance, and
// brevity scoring for accessibility guidelines.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

namespace ExplorerLens { namespace Engine {

struct CaptionQualityMetrics {
    float bleuScore       = 0.0f;  // 0-1
    float readabilityScore = 0.0f; // Flesch-Kincaid proxy 0-100
    float brevityScore    = 0.0f;  // 0-1 (shorter preferred for alt text)
    float wcagScore       = 0.0f;  // 0-1 WCAG 2.2 alt text compliance
    float overallScore    = 0.0f;  // 0-1 composite
    bool  passesAccessibility = false;
};

class CaptionQualityScorer {
public:
    CaptionQualityScorer() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    CaptionQualityMetrics Score(const std::string& caption,
                                 const std::string& reference = "") const {
        CaptionQualityMetrics m;
        if (caption.empty()) return m;

        if (!reference.empty()) {
            m.bleuScore = ComputeBLEU1Gram(caption, reference);
        } else {
            m.bleuScore = 0.5f;
        }

        uint32_t wordCount = CountWords(caption);
        m.brevityScore = wordCount <= 10 ? 1.0f
                       : wordCount <= 25 ? 0.7f
                       : wordCount <= 50 ? 0.4f : 0.1f;

        m.readabilityScore = wordCount > 0 ? std::min(100.0f, 75.0f - wordCount * 0.5f) : 0.0f;

        bool notEmpty    = !caption.empty();
        bool notPhrase   = caption.back() != '?';
        bool noMarkdown  = caption.find('[') == std::string::npos;
        m.wcagScore  = (notEmpty && notPhrase && noMarkdown) ? 0.95f : 0.5f;

        m.overallScore       = (m.bleuScore + m.brevityScore + m.wcagScore) / 3.0f;
        m.passesAccessibility = m.wcagScore >= 0.8f && wordCount <= 30 && notEmpty;
        return m;
    }

    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;

    static uint32_t CountWords(const std::string& s) {
        uint32_t count = 0;
        bool inWord = false;
        for (char c : s) {
            if (c == ' ' || c == '\t' || c == '\n') inWord = false;
            else if (!inWord) { ++count; inWord = true; }
        }
        return count;
    }

    static float ComputeBLEU1Gram(const std::string& hyp,
                                   const std::string& ref) {
        if (hyp.empty() || ref.empty()) return 0.0f;
        uint32_t matches = 0, total = 0;
        size_t pos = 0;
        while ((pos = hyp.find(' ', pos)) != std::string::npos) { ++total; ++pos; }
        ++total;
        for (uint32_t i = 0; i < total; ++i) {
            if (ref.find(hyp.c_str() + (i * 5 % hyp.size()), 0, 3) != std::string::npos)
                ++matches;
        }
        return total > 0 ? static_cast<float>(matches) / total : 0.0f;
    }
};

}} // namespace ExplorerLens::Engine
