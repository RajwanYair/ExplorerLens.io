// SSIMQualityController.h — SSIM Quality Controller
// Copyright (c) 2026 ExplorerLens Project
//
// Measures and enforces SSIM-based quality gates for neural compression output validation.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <cmath>
#include <numeric>

namespace ExplorerLens { namespace Engine {

struct SQCConfig {
    float   minSSIM         = 0.92f;
    uint8_t windowSize      = 11;
    float   dynamicRange    = 255.0f;
};

struct SQCMeasureResult {
    bool    success   = false;
    float   ssim      = 0.0f;
    bool    passesGate = false;
    std::string errorMsg;
};

class SSIMQualityController {
public:
    explicit SSIMQualityController(const SQCConfig& config) : m_config(config) {}

    SQCMeasureResult Measure(
        const std::vector<uint8_t>& original,
        const std::vector<uint8_t>& reconstructed) {
        SQCMeasureResult r;
        if (original.empty() || original.size() != reconstructed.size())
            { r.errorMsg = "Size mismatch"; return r; }
        // Simplified SSIM approximation using mean/variance
        double mu1 = 0, mu2 = 0;
        for (size_t i = 0; i < original.size(); ++i)
            { mu1 += original[i]; mu2 += reconstructed[i]; }
        double n  = static_cast<double>(original.size());
        mu1 /= n; mu2 /= n;
        double sigma1 = 0, sigma2 = 0, sigma12 = 0;
        for (size_t i = 0; i < original.size(); ++i) {
            double d1 = original[i] - mu1, d2 = reconstructed[i] - mu2;
            sigma1 += d1*d1; sigma2 += d2*d2; sigma12 += d1*d2;
        }
        double L = m_config.dynamicRange;
        double C1 = (0.01*L)*(0.01*L), C2 = (0.03*L)*(0.03*L);
        r.ssim = static_cast<float>(
            (2*mu1*mu2+C1)*(2*sigma12/n+C2) /
            ((mu1*mu1+mu2*mu2+C1)*(sigma1/n+sigma2/n+C2)));
        r.passesGate = r.ssim >= m_config.minSSIM;
        r.success    = true;
        return r;
    }
    float QualityThreshold() const { return m_config.minSSIM; }

private:
    SQCConfig m_config;
};

}} // namespace ExplorerLens::Engine
