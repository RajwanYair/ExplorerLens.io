#pragma once
//==============================================================================
// AutoDocGenerator
// Automated documentation generation from code analysis
//==============================================================================

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DocSection : uint8_t {
    Overview = 0,
    Installation,
    Configuration,
    Decoders,
    GPUPipeline,
    PluginAPI,
    Testing,
    Performance,
    Changelog,
    SectionCount
};

enum class DocFormat : uint8_t {
    Markdown = 0,
    HTML,
    ReStructuredText,
    AsciiDoc
};

struct DecoderDocEntry
{
    std::wstring name;
    std::wstring description;
    std::vector<std::wstring> extensions;
    uint32_t testCount = 0;
    bool gpuAccelerated = false;
};

struct DocGenerationResult
{
    bool success = false;
    uint32_t sectionsGenerated = 0;
    uint64_t totalBytes = 0;
    double generationTimeMs = 0.0;
    std::vector<std::wstring> files;
    std::vector<std::wstring> warnings;
};

//------------------------------------------------------------------------------
class AutoDocGenerator
{
  public:
    AutoDocGenerator();
    ~AutoDocGenerator() = default;

    // Decoder registration
    void RegisterDecoder(const DecoderDocEntry& entry);
    uint32_t GetDecoderCount() const
    {
        return static_cast<uint32_t>(m_decoders.size());
    }

    // Generation
    DocGenerationResult GenerateAll(const std::wstring& outputDir, DocFormat format);
    std::wstring GenerateSection(DocSection section, DocFormat format) const;
    std::wstring GenerateDecoderTable() const;
    std::wstring GenerateExtensionList() const;

    // Stats
    uint32_t GetTotalExtensions() const;
    uint32_t GetTotalTests() const;

    // Static helpers
    static const wchar_t* GetSectionName(DocSection section);
    static const wchar_t* GetFormatName(DocFormat format);
    static const wchar_t* GetFormatExtension(DocFormat format);

  private:
    std::vector<DecoderDocEntry> m_decoders;
    std::wstring FormatMarkdown(DocSection section) const;
};

}  // namespace Engine
}  // namespace ExplorerLens
