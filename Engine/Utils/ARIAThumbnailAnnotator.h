// ARIAThumbnailAnnotator.h — ARIA Thumbnail Annotator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates WAI-ARIA compliant annotations for thumbnail UI elements.
// Provides role, aria-label, aria-describedby, and live region markup for
// screen reader compatibility.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct ARIAThumbnailAnnotation {
    std::string role           = "img";
    std::string ariaLabel;
    std::string ariaDescribedBy;
    std::string ariaLive       = "polite";
    bool        ariaHidden     = false;
    std::string tabIndex       = "0";
};

struct ARIAAnnotateRequest {
    std::string filePath;
    std::string format;
    uint32_t    width     = 0;
    uint32_t    height    = 0;
    std::string altText;
};

class ARIAThumbnailAnnotator {
public:
    ARIAThumbnailAnnotator() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    ARIAThumbnailAnnotation Annotate(const ARIAAnnotateRequest& req) const {
        ARIAThumbnailAnnotation ann;
        ann.ariaLabel = req.altText.empty()
            ? "Thumbnail for " + ExtractName(req.filePath)
            : req.altText;

        ann.ariaDescribedBy = "thumb-desc-" + ExtractName(req.filePath);

        if (!req.format.empty() && req.width > 0 && req.height > 0) {
            ann.ariaDescribedBy = req.format + " image, "
                + std::to_string(req.width) + " by "
                + std::to_string(req.height) + " pixels";
        }
        return ann;
    }

    std::string RenderHTMLAttr(const ARIAThumbnailAnnotation& ann) const {
        std::string s;
        s += "role=\"" + ann.role + "\" ";
        s += "aria-label=\"" + ann.ariaLabel + "\" ";
        if (!ann.ariaDescribedBy.empty())
            s += "aria-describedby=\"" + ann.ariaDescribedBy + "\" ";
        if (ann.ariaHidden) s += "aria-hidden=\"true\" ";
        s += "tabindex=\"" + ann.tabIndex + "\"";
        return s;
    }

    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;

    static std::string ExtractName(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        return (pos != std::string::npos) ? path.substr(pos + 1) : path;
    }
};

}} // namespace ExplorerLens::Engine
