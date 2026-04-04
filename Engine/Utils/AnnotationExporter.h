// AnnotationExporter.h — Annotation Export Engine (JSON/XML/CSV/EXIF Sidecar)
// Copyright (c) 2026 ExplorerLens Project
//
// Serialises annotation data to multiple formats: JSON metadata sidecar,
// XML Dublin Core, CSV batch export, and EXIF-compatible XMP sidecar files.
//
#pragma once
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AnnotationExportFormat {
    JSON,
    XML,
    CSV,
    XMP
};

struct ExportableAnnotation
{
    std::wstring filePath;
    int starRating = 0;
    uint32_t colorLabel = 0;
    std::wstring comment;
    std::vector<std::wstring> tags;
    std::wstring author;
    std::string isoTimestamp;  // RFC 3339
};

struct AnnotationExportResult
{
    bool success = false;
    std::string content;  // serialised output
    std::string errorMsg;
    int itemCount = 0;
};

class AnnotationExporter
{
  public:
    explicit AnnotationExporter() = default;

    AnnotationExportResult Export(const std::vector<ExportableAnnotation>& items, AnnotationExportFormat fmt) const
    {
        switch (fmt) {
            case AnnotationExportFormat::JSON:
                return ExportJSON(items);
            case AnnotationExportFormat::XML:
                return ExportXML(items);
            case AnnotationExportFormat::CSV:
                return ExportCSV(items);
            case AnnotationExportFormat::XMP:
                return ExportXMP(items);
        }
        return {false, {}, "Unknown format", 0};
    }

    std::string FormatName(AnnotationExportFormat fmt) const noexcept
    {
        switch (fmt) {
            case AnnotationExportFormat::JSON:
                return "JSON";
            case AnnotationExportFormat::XML:
                return "XML";
            case AnnotationExportFormat::CSV:
                return "CSV";
            case AnnotationExportFormat::XMP:
                return "XMP";
        }
        return "Unknown";
    }

  private:
    static std::string WToU8(const std::wstring& w)
    {
        std::string s;
        for (wchar_t c : w)
            s += (c < 128) ? static_cast<char>(c) : '?';
        return s;
    }

    AnnotationExportResult ExportJSON(const std::vector<ExportableAnnotation>& items) const
    {
        std::ostringstream oss;
        oss << "[\n";
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& a = items[i];
            oss << "  {\"file\":\"" << WToU8(a.filePath) << "\""
                << ",\"stars\":" << a.starRating << ",\"author\":\"" << WToU8(a.author) << "\""
                << ",\"comment\":\"" << WToU8(a.comment) << "\"}" << (i + 1 < items.size() ? "," : "") << "\n";
        }
        oss << "]";
        return {true, oss.str(), {}, static_cast<int>(items.size())};
    }

    AnnotationExportResult ExportXML(const std::vector<ExportableAnnotation>& items) const
    {
        std::ostringstream oss;
        oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<annotations>\n";
        for (const auto& a : items) {
            oss << "  <annotation file=\"" << WToU8(a.filePath) << "\""
                << " stars=\"" << a.starRating << "\"/>\n";
        }
        oss << "</annotations>";
        return {true, oss.str(), {}, static_cast<int>(items.size())};
    }

    AnnotationExportResult ExportCSV(const std::vector<ExportableAnnotation>& items) const
    {
        std::ostringstream oss;
        oss << "file,stars,colorLabel,author,comment\n";
        for (const auto& a : items) {
            oss << "\"" << WToU8(a.filePath) << "\"," << a.starRating << "," << a.colorLabel << ","
                << "\"" << WToU8(a.author) << "\","
                << "\"" << WToU8(a.comment) << "\"\n";
        }
        return {true, oss.str(), {}, static_cast<int>(items.size())};
    }

    AnnotationExportResult ExportXMP(const std::vector<ExportableAnnotation>& items) const
    {
        std::ostringstream oss;
        oss << "<?xpacket begin='' id='W5M0MpCehiHzreSzNTczkc9d'?>\n"
            << "<x:xmpmeta xmlns:x='adobe:ns:meta/'>\n"
            << " <rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>\n";
        for (const auto& a : items) {
            oss << "  <rdf:Description rdf:about='" << WToU8(a.filePath) << "'"
                << " xmp:Rating='" << a.starRating << "'/>\n";
        }
        oss << " </rdf:RDF>\n</x:xmpmeta>\n<?xpacket end='w'?>";
        return {true, oss.str(), {}, static_cast<int>(items.size())};
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
