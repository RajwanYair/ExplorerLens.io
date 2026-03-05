// FaceDetectionThumbnail.h — Face Detection for Smart Portrait Cropping
// Copyright (c) 2026 ExplorerLens Project
//
// Face detection for smart portrait cropping. Implements simplified Viola-Jones
// cascade classifier for locating faces and centering thumbnail crops on subjects.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <array>

namespace ExplorerLens {
namespace Engine {

struct DetectedFaceRect {
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    float confidence = 0.0f;
};

struct FaceCropRegion {
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    bool containsFace = false;
};

struct FaceDetectionConfig {
    float minConfidence = 0.5f;
    uint32_t minFaceSize = 20;
    uint32_t maxFaceSize = 0;
    float scaleFactor = 1.2f;
    uint32_t stepSize = 2;
    float cropPaddingRatio = 0.3f;
};

class FaceDetectionThumbnail {
public:
    static FaceDetectionThumbnail& Instance() {
        static FaceDetectionThumbnail instance;
        return instance;
    }

    inline std::vector<DetectedFaceRect> DetectFaces(const uint8_t* grayPixels, uint32_t width, uint32_t height,
        const FaceDetectionConfig& config = {}) const {
        std::vector<DetectedFaceRect> faces;
        if (!grayPixels || width < config.minFaceSize || height < config.minFaceSize) return faces;

        std::vector<int64_t> integralImage = ComputeIntegralImage(grayPixels, width, height);

        uint32_t maxFaceSize = config.maxFaceSize > 0 ? config.maxFaceSize : (std::min)(width, height);

        for (uint32_t faceSize = config.minFaceSize; faceSize <= maxFaceSize;
            faceSize = static_cast<uint32_t>(faceSize * config.scaleFactor)) {

            for (uint32_t y = 0; y + faceSize < height; y += config.stepSize) {
                for (uint32_t x = 0; x + faceSize < width; x += config.stepSize) {
                    float score = EvaluateCascade(integralImage, width, height, x, y, faceSize);
                    if (score >= config.minConfidence) {
                        DetectedFaceRect face;
                        face.x = static_cast<int32_t>(x);
                        face.y = static_cast<int32_t>(y);
                        face.width = faceSize;
                        face.height = faceSize;
                        face.confidence = score;
                        faces.push_back(face);
                    }
                }
            }
        }

        return NonMaxSuppression(faces, 0.3f);
    }

    inline FaceCropRegion ComputeSmartCrop(const std::vector<DetectedFaceRect>& faces,
        uint32_t imageWidth, uint32_t imageHeight,
        uint32_t targetWidth, uint32_t targetHeight,
        float paddingRatio = 0.3f) const {
        FaceCropRegion crop;
        crop.width = targetWidth;
        crop.height = targetHeight;

        if (faces.empty()) {
            crop.x = static_cast<int32_t>((imageWidth - targetWidth) / 2);
            crop.y = static_cast<int32_t>((imageHeight - targetHeight) / 2);
            crop.containsFace = false;
            return crop;
        }

        float bestConf = 0.0f;
        DetectedFaceRect primaryFace = faces[0];
        for (const auto& face : faces) {
            if (face.confidence > bestConf) {
                bestConf = face.confidence;
                primaryFace = face;
            }
        }

        float faceCenterX = primaryFace.x + primaryFace.width * 0.5f;
        float faceCenterY = primaryFace.y + primaryFace.height * 0.5f;

        faceCenterY -= primaryFace.height * 0.15f;

        crop.x = static_cast<int32_t>(faceCenterX - targetWidth * 0.5f);
        crop.y = static_cast<int32_t>(faceCenterY - targetHeight * 0.5f);

        crop.x = (std::max)(0, (std::min)(crop.x, static_cast<int32_t>(imageWidth - targetWidth)));
        crop.y = (std::max)(0, (std::min)(crop.y, static_cast<int32_t>(imageHeight - targetHeight)));
        crop.containsFace = true;
        return crop;
    }

    inline float ComputeSkinLikelihood(uint8_t r, uint8_t g, uint8_t b) const {
        float rf = r / 255.0f, gf = g / 255.0f, bf = b / 255.0f;
        bool rule1 = rf > 0.36f && gf > 0.21f && bf > 0.17f;
        bool rule2 = (rf - gf) > 0.03f;
        bool rule3 = rf > gf && rf > bf;
        float maxC = (std::max)({ rf, gf, bf });
        float minC = (std::min)({ rf, gf, bf });
        bool rule4 = (maxC - minC) > 0.05f;
        return (rule1 && rule2 && rule3 && rule4) ? 0.8f : 0.1f;
    }

private:
    FaceDetectionThumbnail() = default;

    inline std::vector<int64_t> ComputeIntegralImage(const uint8_t* gray, uint32_t w, uint32_t h) const {
        std::vector<int64_t> integral(static_cast<size_t>(w + 1) * (h + 1), 0);
        uint32_t stride = w + 1;
        for (uint32_t y = 0; y < h; ++y) {
            int64_t rowSum = 0;
            for (uint32_t x = 0; x < w; ++x) {
                rowSum += gray[y * w + x];
                integral[(y + 1) * stride + (x + 1)] = rowSum + integral[y * stride + (x + 1)];
            }
        }
        return integral;
    }

    inline int64_t RectSum(const std::vector<int64_t>& ii, uint32_t w, uint32_t x, uint32_t y,
        uint32_t rw, uint32_t rh) const {
        uint32_t stride = w + 1;
        return ii[(y + rh) * stride + (x + rw)] - ii[y * stride + (x + rw)]
            - ii[(y + rh) * stride + x] + ii[y * stride + x];
    }

    inline float EvaluateCascade(const std::vector<int64_t>& integral, uint32_t imgW, uint32_t imgH,
        uint32_t x, uint32_t y, uint32_t size) const {
        uint32_t half = size / 2;
        uint32_t quarter = size / 4;
        float area = static_cast<float>(half * size);
        if (area < 1.0f) return 0.0f;

        float leftMean = RectSum(integral, imgW, x, y, half, size) / area;
        float rightMean = RectSum(integral, imgW, x + half, y, half, size) / area;
        float diff1 = std::abs(leftMean - rightMean);

        float topMean = RectSum(integral, imgW, x, y, size, half) / area;
        float botMean = RectSum(integral, imgW, x, y + half, size, half) / area;
        float diff2 = std::abs(topMean - botMean);

        float centerArea = static_cast<float>(half * half);
        if (centerArea < 1.0f) return 0.0f;
        float outerMean = RectSum(integral, imgW, x, y, size, size) / (size * static_cast<float>(size));
        float centerMean = RectSum(integral, imgW, x + quarter, y + quarter, half, half) / centerArea;
        float diff3 = centerMean - outerMean;

        float score = 0.0f;
        if (diff1 > 10.0f) score += 0.2f;
        if (diff2 > 15.0f) score += 0.3f;
        if (diff3 > 5.0f) score += 0.3f;
        if (outerMean > 60.0f && outerMean < 200.0f) score += 0.2f;
        return score;
    }

    inline std::vector<DetectedFaceRect> NonMaxSuppression(const std::vector<DetectedFaceRect>& faces, float overlapThreshold) const {
        if (faces.empty()) return {};
        auto sorted = faces;
        std::sort(sorted.begin(), sorted.end(), [](const DetectedFaceRect& a, const DetectedFaceRect& b) {
            return a.confidence > b.confidence;
            });

        std::vector<bool> suppressed(sorted.size(), false);
        std::vector<DetectedFaceRect> result;

        for (size_t i = 0; i < sorted.size(); ++i) {
            if (suppressed[i]) continue;
            result.push_back(sorted[i]);
            for (size_t j = i + 1; j < sorted.size(); ++j) {
                if (!suppressed[j] && ComputeIoU(sorted[i], sorted[j]) > overlapThreshold) {
                    suppressed[j] = true;
                }
            }
        }
        return result;
    }

    inline float ComputeIoU(const DetectedFaceRect& a, const DetectedFaceRect& b) const {
        int32_t x1 = (std::max)(a.x, b.x);
        int32_t y1 = (std::max)(a.y, b.y);
        int32_t x2 = (std::min)(a.x + static_cast<int32_t>(a.width), b.x + static_cast<int32_t>(b.width));
        int32_t y2 = (std::min)(a.y + static_cast<int32_t>(a.height), b.y + static_cast<int32_t>(b.height));
        if (x2 <= x1 || y2 <= y1) return 0.0f;
        float intersection = static_cast<float>((x2 - x1) * (y2 - y1));
        float areaA = static_cast<float>(a.width * a.height);
        float areaB = static_cast<float>(b.width * b.height);
        return intersection / (areaA + areaB - intersection);
    }
};

}
} // namespace ExplorerLens::Engine
