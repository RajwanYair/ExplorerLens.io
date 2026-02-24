//==============================================================================
// AutoDocGenerator
//==============================================================================

#include "AutoDocGenerator.h"
#include <sstream>
#include <chrono>

namespace ExplorerLens { namespace Engine {

AutoDocGenerator::AutoDocGenerator() {}

void AutoDocGenerator::RegisterDecoder(const DecoderDocEntry& entry) {
    m_decoders.push_back(entry);
}

DocGenerationResult AutoDocGenerator::GenerateAll(
    const std::wstring& outputDir, DocFormat format)
{
    DocGenerationResult result;
    auto start = std::chrono::high_resolution_clock::now();

    for (uint8_t i = 0; i < static_cast<uint8_t>(DocSection::SectionCount); i++) {
        auto section = static_cast<DocSection>(i);
        auto content = GenerateSection(section, format);
        if (!content.empty()) {
            result.sectionsGenerated++;
            result.totalBytes += content.size() * sizeof(wchar_t);
            result.files.push_back(outputDir + L"\\" +
                GetSectionName(section) + GetFormatExtension(format));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.generationTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    result.success = result.sectionsGenerated > 0;
    return result;
}

std::wstring AutoDocGenerator::GenerateSection(DocSection section, DocFormat format) const {
    if (format == DocFormat::Markdown) {
        return FormatMarkdown(section);
    }
    return L"";
}

std::wstring AutoDocGenerator::FormatMarkdown(DocSection section) const {
    std::wstringstream ss;
    ss << L"# " << GetSectionName(section) << L"\n\n";

    switch (section) {
        case DocSection::Decoders:
            ss << GenerateDecoderTable();
            break;
        case DocSection::Overview:
            ss << L"ExplorerLens is a Windows Shell Extension providing GPU-accelerated\n";
            ss << L"thumbnails for " << GetTotalExtensions() << L"+ file formats.\n";
            break;
        case DocSection::Testing:
            ss << L"Total tests: " << GetTotalTests() << L"\n";
            break;
        default:
            ss << L"Section content to be generated.\n";
            break;
    }
    return ss.str();
}

std::wstring AutoDocGenerator::GenerateDecoderTable() const {
    std::wstringstream ss;
    ss << L"| Decoder | Extensions | Tests | GPU |\n";
    ss << L"|---------|-----------|-------|-----|\n";
    for (const auto& d : m_decoders) {
        ss << L"| " << d.name << L" | ";
        for (size_t i = 0; i < d.extensions.size(); i++) {
            if (i > 0) ss << L", ";
            ss << d.extensions[i];
        }
        ss << L" | " << d.testCount;
        ss << L" | " << (d.gpuAccelerated ? L"✅" : L"❌");
        ss << L" |\n";
    }
    return ss.str();
}

std::wstring AutoDocGenerator::GenerateExtensionList() const {
    std::wstringstream ss;
    for (const auto& d : m_decoders) {
        for (const auto& ext : d.extensions) {
            ss << ext << L"\n";
        }
    }
    return ss.str();
}

uint32_t AutoDocGenerator::GetTotalExtensions() const {
    uint32_t total = 0;
    for (const auto& d : m_decoders) {
        total += static_cast<uint32_t>(d.extensions.size());
    }
    return total;
}

uint32_t AutoDocGenerator::GetTotalTests() const {
    uint32_t total = 0;
    for (const auto& d : m_decoders) {
        total += d.testCount;
    }
    return total;
}

const wchar_t* AutoDocGenerator::GetSectionName(DocSection section) {
    switch (section) {
        case DocSection::Overview:       return L"Overview";
        case DocSection::Installation:   return L"Installation";
        case DocSection::Configuration:  return L"Configuration";
        case DocSection::Decoders:       return L"Decoders";
        case DocSection::GPUPipeline:    return L"GPU Pipeline";
        case DocSection::PluginAPI:      return L"Plugin API";
        case DocSection::Testing:        return L"Testing";
        case DocSection::Performance:    return L"Performance";
        case DocSection::Changelog:      return L"Changelog";
        default: return L"Unknown";
    }
}

const wchar_t* AutoDocGenerator::GetFormatName(DocFormat format) {
    switch (format) {
        case DocFormat::Markdown:          return L"Markdown";
        case DocFormat::HTML:              return L"HTML";
        case DocFormat::ReStructuredText:  return L"reStructuredText";
        case DocFormat::AsciiDoc:          return L"AsciiDoc";
        default: return L"Unknown";
    }
}

const wchar_t* AutoDocGenerator::GetFormatExtension(DocFormat format) {
    switch (format) {
        case DocFormat::Markdown:          return L".md";
        case DocFormat::HTML:              return L".html";
        case DocFormat::ReStructuredText:  return L".rst";
        case DocFormat::AsciiDoc:          return L".adoc";
        default: return L".txt";
    }
}

}} // namespace ExplorerLens::Engine

