// StreamingMipChain.h — Progressive Mipmap Chain Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Progressive mipmap chain generation. Generates mip levels on-demand, streams
// from coarsest to finest for progressive thumbnail loading.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct MipLevel {
    std::vector<uint8_t> data;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t level = 0;
    uint32_t channels = 4;
};

struct MipChainInfo {
    uint32_t baseWidth = 0;
    uint32_t baseHeight = 0;
    uint32_t levelCount = 0;
    uint32_t channels = 4;
    size_t totalMemoryBytes = 0;
};

enum class MipFilter : uint8_t {
    Box,
    Triangle,
    Lanczos,
    Kaiser
};

class StreamingMipChain {
public:
    static StreamingMipChain& Instance() {
        static StreamingMipChain instance;
        return instance;
    }

    inline MipChainInfo ComputeChainInfo(uint32_t width, uint32_t height, uint32_t channels = 4) const {
        MipChainInfo info;
        info.baseWidth = width;
        info.baseHeight = height;
        info.channels = channels;
        info.levelCount = ComputeLevelCount(width, height);

        size_t total = 0;
        uint32_t w = width, h = height;
        for (uint32_t i = 0; i < info.levelCount; ++i) {
            total += static_cast<size_t>(w) * h * channels;
            w = (std::max)(1u, w / 2);
            h = (std::max)(1u, h / 2);
        }
        info.totalMemoryBytes = total;
        return info;
    }

    inline std::vector<MipLevel> GenerateFullChain(const uint8_t* baseData, uint32_t width, uint32_t height,
        uint32_t channels = 4, MipFilter filter = MipFilter::Box) const {
        std::vector<MipLevel> chain;
        uint32_t levelCount = ComputeLevelCount(width, height);

        MipLevel level0;
        level0.width = width;
        level0.height = height;
        level0.level = 0;
        level0.channels = channels;
        if (baseData) {
            level0.data.assign(baseData, baseData + static_cast<size_t>(width) * height * channels);
        }
        chain.push_back(std::move(level0));

        for (uint32_t i = 1; i < levelCount; ++i) {
            chain.push_back(GenerateNextLevel(chain.back(), filter));
        }
        return chain;
    }

    inline MipLevel GenerateNextLevel(const MipLevel& source, MipFilter filter = MipFilter::Box) const {
        MipLevel next;
        next.width = (std::max)(1u, source.width / 2);
        next.height = (std::max)(1u, source.height / 2);
        next.level = source.level + 1;
        next.channels = source.channels;
        next.data.resize(static_cast<size_t>(next.width) * next.height * next.channels);

        if (source.data.empty()) return next;

        for (uint32_t y = 0; y < next.height; ++y) {
            for (uint32_t x = 0; x < next.width; ++x) {
                for (uint32_t c = 0; c < next.channels; ++c) {
                    float value = SampleFiltered(source, x * 2, y * 2, c, filter);
                    size_t idx = (static_cast<size_t>(y) * next.width + x) * next.channels + c;
                    next.data[idx] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, value + 0.5f)));
                }
            }
        }
        return next;
    }

    inline MipLevel GetCoarsestLevel(const uint8_t* baseData, uint32_t width, uint32_t height,
        uint32_t channels, uint32_t targetSize = 4) const {
        MipLevel current;
        current.width = width;
        current.height = height;
        current.channels = channels;
        if (baseData) {
            current.data.assign(baseData, baseData + static_cast<size_t>(width) * height * channels);
        }

        while (current.width > targetSize || current.height > targetSize) {
            current = GenerateNextLevel(current);
        }
        return current;
    }

    inline uint32_t ComputeLevelCount(uint32_t width, uint32_t height) const {
        uint32_t maxDim = (std::max)(width, height);
        return maxDim > 0 ? static_cast<uint32_t>(std::floor(std::log2(static_cast<double>(maxDim)))) + 1 : 1;
    }

private:
    StreamingMipChain() = default;

    inline float SampleFiltered(const MipLevel& level, uint32_t x, uint32_t y, uint32_t c,
        MipFilter filter) const {
        switch (filter) {
        case MipFilter::Box: return SampleBox(level, x, y, c);
        case MipFilter::Triangle: return SampleTriangle(level, x, y, c);
        default: return SampleBox(level, x, y, c);
        }
    }

    inline float SampleBox(const MipLevel& level, uint32_t x, uint32_t y, uint32_t c) const {
        float sum = 0.0f;
        int count = 0;
        for (int dy = 0; dy < 2; ++dy) {
            for (int dx = 0; dx < 2; ++dx) {
                uint32_t sx = (std::min)(x + dx, level.width - 1);
                uint32_t sy = (std::min)(y + dy, level.height - 1);
                size_t idx = (static_cast<size_t>(sy) * level.width + sx) * level.channels + c;
                sum += level.data[idx];
                ++count;
            }
        }
        return sum / count;
    }

    inline float SampleTriangle(const MipLevel& level, uint32_t x, uint32_t y, uint32_t c) const {
        float sum = 0.0f;
        float weightSum = 0.0f;
        for (int dy = -1; dy <= 2; ++dy) {
            for (int dx = -1; dx <= 2; ++dx) {
                int sx = static_cast<int>(x) + dx;
                int sy = static_cast<int>(y) + dy;
                if (sx >= 0 && sx < static_cast<int>(level.width) && sy >= 0 && sy < static_cast<int>(level.height)) {
                    float dist = (std::abs(dx - 0.5f) + std::abs(dy - 0.5f));
                    float weight = (std::max)(0.0f, 2.0f - dist);
                    size_t idx = (static_cast<size_t>(sy) * level.width + sx) * level.channels + c;
                    sum += level.data[idx] * weight;
                    weightSum += weight;
                }
            }
        }
        return weightSum > 0.0f ? sum / weightSum : 0.0f;
    }
};

}
} // namespace ExplorerLens::Engine
