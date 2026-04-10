// IFCEntityCounter.h — IFC4/IFC2x3 Entity-Type Frequency Summariser
// Copyright (c) 2026 ExplorerLens Project
//
// Scans an IFC STEP-formatted file for IFCWALL, IFCDOOR, IFCWINDOW, IFCSLAB,
// IFCCOLUMN, IFCBEAM and other architectural entity types. Produces a frequency
// table and renders a bar-chart thumbnail of the top-N entity counts.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct IFCEntityCount
{
    std::string entityType;   // e.g. "IFCWALL"
    uint32_t    count;
};

struct IFCFileSummary
{
    std::string                    ifcVersion;  // "IFC2X3", "IFC4", "IFC4X3"
    std::vector<IFCEntityCount>    topEntities; // sorted descending by count
    uint32_t                       totalEntities;
    uint32_t                       uniqueValues;  // distinct entity types found
    bool                           valid;
};

class IFCEntityCounter
{
public:
    // Returns true if the file starts with "ISO-10303-21" and contains "FILE_SCHEMA(('IFC"
    static bool IsIFC(const uint8_t* data, size_t size) noexcept;

    // Detect the IFC schema version from FILE_SCHEMA section.
    static std::string DetectVersion(const uint8_t* data, size_t size) noexcept;

    // Count occurrences of each IFCXXX entity type (uppercase).
    // Sub-samples large files to stay under 250 ms on 100 MB inputs.
    static IFCFileSummary Count(const uint8_t* data, size_t size, uint32_t topN = 10) noexcept;

    // Render a 256×256 BGRA32 horizontal bar chart of the top entity counts.
    // Bars are colour-coded by entity category (structural/architectural/MEP).
    static std::vector<uint8_t> RenderBarChart(const IFCFileSummary& summary,
                                                uint32_t width  = 256,
                                                uint32_t height = 256);
};

} // namespace Engine
} // namespace ExplorerLens
