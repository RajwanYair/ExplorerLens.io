//==============================================================================
// ExplorerLens Engine — Smart Crop V2
// Saliency-map-driven crop with face centering, rule-of-thirds composition,
// aspect-ratio-aware padding, and golden ratio placement.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class CropStrategy : uint8_t {
    CenterCrop=0,SaliencyMap,FaceCentered,RuleOfThirds,GoldenRatio,SubjectAware,COUNT
};
enum class CropAspectRatio : uint8_t { Square=0,Landscape4_3,Portrait3_4,Wide16_9,Auto,COUNT };
enum class CropPaddingMode : uint8_t { None=0,Extend,Blur,SolidColor,COUNT };

struct SmartCropRequest {
    uint32_t        targetWidth     = 256;
    uint32_t        targetHeight    = 256;
    CropStrategy    strategy        = CropStrategy::SaliencyMap;
    CropAspectRatio aspectRatio     = CropAspectRatio::Square;
    CropPaddingMode padding         = CropPaddingMode::Blur;
};

struct SmartCropResult {
    uint32_t cropX=0,cropY=0,cropW=0,cropH=0;
    CropStrategy    usedStrategy = CropStrategy::CenterCrop;
    float           saliencyScore = 0.0f;
    bool            faceDetected  = false;
};

class SmartCropV2 {
public:
    static const wchar_t* StrategyName(CropStrategy s) {
        switch(s) {
            case CropStrategy::CenterCrop:    return L"Center Crop";
            case CropStrategy::SaliencyMap:   return L"Saliency Map";
            case CropStrategy::FaceCentered:  return L"Face Centered";
            case CropStrategy::RuleOfThirds:  return L"Rule of Thirds";
            case CropStrategy::GoldenRatio:   return L"Golden Ratio";
            case CropStrategy::SubjectAware:  return L"Subject Aware";
            default: return L"Unknown";
        }
    }
    static const wchar_t* AspectRatioName(CropAspectRatio a) {
        switch(a) {
            case CropAspectRatio::Square:       return L"1:1 Square";
            case CropAspectRatio::Landscape4_3: return L"4:3 Landscape";
            case CropAspectRatio::Portrait3_4:  return L"3:4 Portrait";
            case CropAspectRatio::Wide16_9:     return L"16:9 Wide";
            case CropAspectRatio::Auto:         return L"Auto";
            default: return L"Unknown";
        }
    }
    static const wchar_t* PaddingModeName(CropPaddingMode p) {
        switch(p) {
            case CropPaddingMode::None:       return L"None";
            case CropPaddingMode::Extend:     return L"Extend";
            case CropPaddingMode::Blur:       return L"Blur";
            case CropPaddingMode::SolidColor: return L"Solid Color";
            default: return L"Unknown";
        }
    }
    static constexpr size_t StrategyCount()    { return static_cast<size_t>(CropStrategy::COUNT); }
    static constexpr size_t AspectRatioCount() { return static_cast<size_t>(CropAspectRatio::COUNT); }
    static constexpr size_t PaddingCount()     { return static_cast<size_t>(CropPaddingMode::COUNT); }
};

}} // namespace ExplorerLens::Engine

