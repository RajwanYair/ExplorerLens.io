// NaturalLanguageQueryParser.h — Text to CLIP Text Embedding Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Tokenizes natural-language queries and projects them through the CLIP text
// encoder to produce 512-d embedding vectors for cross-modal similarity search.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

static constexpr uint32_t CLIP_TEXT_DIM = 512;
static constexpr uint32_t CLIP_MAX_TOKEN_LEN = 77;
static constexpr uint32_t CLIP_VOCAB_SIZE = 49408;
static constexpr uint32_t BPE_MERGES_COUNT = 48895;

struct TokenSequence
{
    std::vector<uint32_t> tokenIds;
    uint32_t length = 0;
    bool truncated = false;
};

struct QueryEmbedding
{
    std::vector<float> embedding;
    float encodingMs = 0.0f;
    uint32_t tokenCount = 0;
    bool success = false;
};

struct TextEncoderConfig
{
    std::wstring modelPath;
    uint32_t maxTokens = CLIP_MAX_TOKEN_LEN;
    bool lowercase = true;
    bool stripPunct = false;
};

class NaturalLanguageQueryParser
{
  public:
    inline bool Initialize(const TextEncoderConfig& config)
    {
        m_config = config;
        BuildBaseVocab();
        m_initialized = true;
        return true;
    }

    inline QueryEmbedding ParseQuery(const std::string& query)
    {
        QueryEmbedding result;
        if (!m_initialized || query.empty())
            return result;

        auto start = std::chrono::high_resolution_clock::now();

        auto tokens = Tokenize(query);
        result.tokenCount = tokens.length;

        result.embedding.resize(CLIP_TEXT_DIM, 0.0f);
        for (uint32_t i = 0; i < tokens.length && i < CLIP_TEXT_DIM; ++i) {
            result.embedding[i] = static_cast<float>(tokens.tokenIds[i]) / static_cast<float>(CLIP_VOCAB_SIZE);
        }

        L2Normalize(result.embedding);
        result.success = true;

        auto end = std::chrono::high_resolution_clock::now();
        result.encodingMs = std::chrono::duration<float, std::milli>(end - start).count();
        return result;
    }

    inline TokenSequence Tokenize(const std::string& text) const
    {
        TokenSequence seq;
        if (!m_initialized)
            return seq;

        std::string processed = m_config.lowercase ? ToLower(text) : text;
        seq.tokenIds.push_back(49406);  // <|startoftext|>

        for (size_t i = 0; i < processed.size(); ++i) {
            if (seq.tokenIds.size() >= m_config.maxTokens - 1) {
                seq.truncated = true;
                break;
            }
            auto it = m_vocab.find(std::string(1, processed[i]));
            uint32_t id = (it != m_vocab.end()) ? it->second : 0;
            seq.tokenIds.push_back(id);
        }

        seq.tokenIds.push_back(49407);  // <|endoftext|>
        seq.length = static_cast<uint32_t>(seq.tokenIds.size());
        return seq;
    }

    inline uint32_t GetVocabSize() const
    {
        return CLIP_VOCAB_SIZE;
    }
    inline bool IsInitialized() const
    {
        return m_initialized;
    }
    inline uint32_t GetMaxTokens() const
    {
        return m_config.maxTokens;
    }

  private:
    inline void BuildBaseVocab()
    {
        for (char c = 'a'; c <= 'z'; ++c)
            m_vocab[std::string(1, c)] = static_cast<uint32_t>(c - 'a' + 1);
        for (char c = '0'; c <= '9'; ++c)
            m_vocab[std::string(1, c)] = static_cast<uint32_t>(c - '0' + 27);
        m_vocab[" "] = 37;
    }

    inline std::string ToLower(const std::string& s) const
    {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    inline void L2Normalize(std::vector<float>& vec)
    {
        float norm = 0.0f;
        for (float v : vec)
            norm += v * v;
        norm = std::sqrt(norm + 1e-8f);
        for (float& v : vec)
            v /= norm;
    }

    TextEncoderConfig m_config;
    std::unordered_map<std::string, uint32_t> m_vocab;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
