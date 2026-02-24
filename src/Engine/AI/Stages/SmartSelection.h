#pragma once

#include <vector>
#include <cstdint>

namespace ExplorerLens::Engine::AI::Stages {

    struct PageScore {
        uint32_t pageIndex;
        float score; // 0.0 - 1.0 (How likely is this a cover/significant frame?)
        bool hasTitleText;
        bool hasFace;
    };

    class ISmartSelectionStage {
    public:
        virtual ~ISmartSelectionStage() = default;

        // Analyze a set of candidate images (e.g. first 5 pages of a PDF, or frames of a video)
        // Returns the index of the "best" representative image.
        virtual uint32_t SelectBestFrame(const std::vector<const std::vector<uint8_t>*>& candidates, uint32_t width, uint32_t height) = 0;
        
        // Detailed analysis
        virtual std::vector<PageScore> ScoreFrames(const std::vector<const std::vector<uint8_t>*>& candidates, uint32_t width, uint32_t height) = 0;
    };

}

