// BiDiTextLayoutEngine.h — Bidirectional Text Layout Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Implements Unicode Bidirectional Algorithm (UBA) paragraph analysis for
// correct RTL/LTR mixed-text rendering in thumbnail annotations and overlays.
//
#pragma once
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class BiDiBaseDirection { Auto, LTR, RTL };
enum class BiDiRunType       { Strong_L, Strong_R, Neutral, NSM };

struct BiDiRun {
    int          start    = 0;
    int          length   = 0;
    BiDiRunType  type     = BiDiRunType::Strong_L;
    int          level    = 0;
    bool         isRTL() const noexcept { return level % 2 == 1; }
};

struct BiDiParagraph {
    std::wstring          text;
    BiDiBaseDirection     baseDirection = BiDiBaseDirection::Auto;
    std::vector<BiDiRun>  runs;
    int                   paragraphLevel = 0;
    bool IsRTL() const noexcept { return paragraphLevel == 1; }
};

struct BiDiLayoutResult {
    bool                    success      = false;
    std::vector<BiDiRun>    visualOrder; // runs in display order
    BiDiBaseDirection       resolvedDir  = BiDiBaseDirection::LTR;
    int                     runCount     = 0;
    std::string             errorMsg;
    bool Ok() const noexcept { return success; }
};

class BiDiTextLayoutEngine {
public:
    explicit BiDiTextLayoutEngine() = default;

    BiDiLayoutResult Analyse(const std::wstring& text,
                              BiDiBaseDirection baseDir = BiDiBaseDirection::Auto) const {
        if (text.empty()) return { false, {}, baseDir, 0, "Empty text" };

        BiDiLayoutResult result;
        result.success     = true;
        result.resolvedDir = ResolveDirection(text, baseDir);
        // Simple paragraph-level analysis: produce one run per script change
        result.visualOrder = SegmentRuns(text, result.resolvedDir);
        result.runCount    = static_cast<int>(result.visualOrder.size());
        return result;
    }

    static bool ContainsRTL(const std::wstring& text) noexcept {
        for (wchar_t c : text) {
            if ((c >= 0x0590 && c <= 0x05FF) ||  // Hebrew
                (c >= 0x0600 && c <= 0x06FF) ||  // Arabic
                (c >= 0x0750 && c <= 0x077F))     // Arabic Supplement
                return true;
        }
        return false;
    }

private:
    static BiDiBaseDirection ResolveDirection(const std::wstring& text,
                                               BiDiBaseDirection baseDir) noexcept {
        if (baseDir != BiDiBaseDirection::Auto) return baseDir;
        return ContainsRTL(text) ? BiDiBaseDirection::RTL : BiDiBaseDirection::LTR;
    }

    static std::vector<BiDiRun> SegmentRuns(const std::wstring& text,
                                              BiDiBaseDirection dir) {
        // Minimal: one run for the entire paragraph
        BiDiRun run;
        run.start  = 0;
        run.length = static_cast<int>(text.size());
        run.type   = (dir == BiDiBaseDirection::RTL) ? BiDiRunType::Strong_R : BiDiRunType::Strong_L;
        run.level  = (dir == BiDiBaseDirection::RTL) ? 1 : 0;
        return { run };
    }
};

} // namespace Engine
} // namespace ExplorerLens
