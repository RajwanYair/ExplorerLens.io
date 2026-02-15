//==============================================================================
// DarkThumbs Engine - Decoder Priority System
// Copyright (c) 2026 - DarkThumbs Project
// Task B10: Smart decoder selection and fallback chains
//==============================================================================

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace DarkThumbs {
namespace Engine {

    /// <summary>
    /// Decoder priority levels
    /// </summary>
    enum class DecoderPriority {
        Critical = 0,   // Essential decoders (JPEG, PNG, etc.)
        High = 1,       // Modern formats (WebP, AVIF, JXL)
        Normal = 2,     // Standard formats (BMP, TIFF, ICO)
        Low = 3,        // Specialty formats (PSD, DDS, HDR)
        Fallback = 4    // Last resort decoders
    };

    /// <summary>
    /// Decoder registration with priority
    /// </summary>
    struct DecoderRegistration {
        std::wstring name;
        std::vector<std::wstring> extensions;
        DecoderPriority priority;
        bool isAvailable;
        double successRate;
        
        // Sort by priority, then success rate
        bool operator<(const DecoderRegistration& other) const {
            if (priority != other.priority) {
                return priority < other.priority;
            }
            return successRate > other.successRate;
        }
    };

    /// <summary>
    /// Manages decoder selection and fallback chains
    /// </summary>
    class DecoderPriorityManager {
    public:
        static DecoderPriorityManager& GetInstance() {
            static DecoderPriorityManager instance;
            return instance;
        }

        /// <summary>
        /// Register decoder with priority
        /// </summary>
        void RegisterDecoder(const wchar_t* name, 
                           const std::vector<std::wstring>& extensions,
                           DecoderPriority priority = DecoderPriority::Normal) {
            DecoderRegistration reg;
            reg.name = name;
            reg.extensions = extensions;
            reg.priority = priority;
            reg.isAvailable = true;
            reg.successRate = 1.0;
            
            m_decoders.push_back(reg);
            
            // Update extension map
            for (const auto& ext : extensions) {
                m_extensionMap[ext].push_back(name);
            }
            
            SortDecoders();
        }

        /// <summary>
        /// Get ordered list of decoders for extension
        /// </summary>
        std::vector<std::wstring> GetDecodersForExtension(const wchar_t* ext) const {
            auto it = m_extensionMap.find(ext);
            if (it == m_extensionMap.end()) {
                return {};
            }

            // Return decoders sorted by priority
            std::vector<std::wstring> result = it->second;
            std::sort(result.begin(), result.end(), [this](const std::wstring& a, const std::wstring& b) {
                const DecoderRegistration* regA = FindDecoder(a.c_str());
                const DecoderRegistration* regB = FindDecoder(b.c_str());
                if (!regA || !regB) return false;
                return *regA < *regB;
            });
            
            return result;
        }

        /// <summary>
        /// Get primary decoder for extension
        /// </summary>
        std::wstring GetPrimaryDecoder(const wchar_t* ext) const {
            auto decoders = GetDecodersForExtension(ext);
            return decoders.empty() ? L"" : decoders[0];
        }

        /// <summary>
        /// Get fallback decoder for extension
        /// </summary>
        std::wstring GetFallbackDecoder(const wchar_t* ext, const wchar_t* exclude = nullptr) const {
            auto decoders = GetDecodersForExtension(ext);
            for (const auto& decoder : decoders) {
                if (!exclude || decoder != exclude) {
                    const DecoderRegistration* reg = FindDecoder(decoder.c_str());
                    if (reg && reg->isAvailable) {
                        return decoder;
                    }
                }
            }
            return L"";
        }

        /// <summary>
        /// Update decoder availability
        /// </summary>
        void SetDecoderAvailable(const wchar_t* name, bool available) {
            for (auto& decoder : m_decoders) {
                if (decoder.name == name) {
                    decoder.isAvailable = available;
                    break;
                }
            }
        }

        /// <summary>
        /// Update decoder success rate
        /// </summary>
        void UpdateSuccessRate(const wchar_t* name, double rate) {
            for (auto& decoder : m_decoders) {
                if (decoder.name == name) {
                    decoder.successRate = rate;
                    SortDecoders();
                    break;
                }
            }
        }

    private:
        DecoderPriorityManager() = default;
        ~DecoderPriorityManager() = default;
        DecoderPriorityManager(const DecoderPriorityManager&) = delete;
        DecoderPriorityManager& operator=(const DecoderPriorityManager&) = delete;

        void SortDecoders() {
            std::sort(m_decoders.begin(), m_decoders.end());
        }

        const DecoderRegistration* FindDecoder(const wchar_t* name) const {
            for (const auto& decoder : m_decoders) {
                if (decoder.name == name) {
                    return &decoder;
                }
            }
            return nullptr;
        }

        std::vector<DecoderRegistration> m_decoders;
        std::unordered_map<std::wstring, std::vector<std::wstring>> m_extensionMap;
    };

} // namespace Engine
} // namespace DarkThumbs
