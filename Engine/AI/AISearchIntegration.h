//==============================================================================
// DarkThumbs Engine — Sprint 328: AI Search Integration
// Visual similarity search via embedding vectors, Windows Search integration,
// semantic image retrieval, and cross-format duplicate detection via CLIP.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class AISearchMode : uint8_t { VisualSimilarity=0,SemanticText,HybridVT,Duplicate,COUNT };
enum class EmbeddingModel : uint8_t { CLIP_ViT_B32=0,CLIP_ViT_L14,DINOv2,COUNT };
enum class SearchIndexStatus : uint8_t { NotIndexed=0,Indexing,Indexed,Stale,COUNT };

struct EmbeddingVector {
    std::vector<float> values;
    EmbeddingModel     model   = EmbeddingModel::CLIP_ViT_B32;
    uint32_t           dims    = 512;
};

struct AISearchResult {
    std::wstring    filePath;
    float           similarity  = 0.0f; // 0-1
    AISearchMode    matchMode   = AISearchMode::VisualSimilarity;
    bool            isDuplicate = false;
};

class AISearchIntegration {
public:
    static const wchar_t* SearchModeName(AISearchMode m) {
        switch(m) {
            case AISearchMode::VisualSimilarity: return L"Visual Similarity";
            case AISearchMode::SemanticText:     return L"Semantic Text";
            case AISearchMode::HybridVT:         return L"Hybrid V+T";
            case AISearchMode::Duplicate:        return L"Duplicate Detection";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ModelName(EmbeddingModel m) {
        switch(m) {
            case EmbeddingModel::CLIP_ViT_B32: return L"CLIP ViT-B/32";
            case EmbeddingModel::CLIP_ViT_L14: return L"CLIP ViT-L/14";
            case EmbeddingModel::DINOv2:       return L"DINOv2";
            default: return L"Unknown";
        }
    }
    static const wchar_t* IndexStatusName(SearchIndexStatus s) {
        switch(s) {
            case SearchIndexStatus::NotIndexed: return L"Not Indexed";
            case SearchIndexStatus::Indexing:   return L"Indexing";
            case SearchIndexStatus::Indexed:    return L"Indexed";
            case SearchIndexStatus::Stale:      return L"Stale";
            default: return L"Unknown";
        }
    }
    static constexpr size_t SearchModeCount()  { return static_cast<size_t>(AISearchMode::COUNT); }
    static constexpr size_t ModelCount()       { return static_cast<size_t>(EmbeddingModel::COUNT); }
    static constexpr size_t IndexStatusCount() { return static_cast<size_t>(SearchIndexStatus::COUNT); }
};

}} // namespace DarkThumbs::Engine
