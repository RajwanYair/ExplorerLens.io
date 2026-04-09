// KiCadNetlistParser.h — KiCad 8 S-Expression Netlist & Schematic Parser
// Copyright (c) 2026 ExplorerLens Project
//
// Parses KiCad 8 S-expression format files (*.kicad_sch, *.kicad_pcb,
// *.kicad_mod) to extract component references, values, footprint names,
// and PCB board outline. Renders a component-count summary thumbnail.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class KiCadFileType : uint8_t
{
    Unknown    = 0,
    Schematic, // kicad_sch
    PCBLayout, // kicad_pcb
    Footprint, // kicad_mod
    Project,   // kicad_pro
};

struct KiCadComponent
{
    std::string reference;  // e.g. "R1", "U3"
    std::string value;      // e.g. "10k", "STM32F4"
    std::string footprint;  // e.g. "Resistor_SMD:R_0402"
};

struct KiCadFileSummary
{
    KiCadFileType               fileType;
    std::vector<KiCadComponent> components;
    uint32_t                    uniqueValues;
    double                      boardWidthMM;   // 0 if not a PCB
    double                      boardHeightMM;
    bool                        valid;
};

class KiCadNetlistParser
{
public:
    // Returns true if data starts with "(kicad_sch", "(kicad_pcb", or "(kicad_mod".
    static bool IsKiCad(const uint8_t* data, size_t size) noexcept;

    // Detect file type from opening S-expression keyword.
    static KiCadFileType DetectFileType(const uint8_t* data, size_t size) noexcept;

    // Extract component list from schematic or PCB S-expression.
    // Parses up to maxComponents entries to stay under 100 ms on large boards.
    static KiCadFileSummary Parse(const uint8_t* data, size_t size,
                                   uint32_t maxComponents = 2000) noexcept;

    // Render a 256×256 BGRA32 thumbnail showing component categories as
    // a pie-chart (R/C/L/U/Q/J/Other slices in distinct colours).
    static std::vector<uint8_t> RenderPieChart(const KiCadFileSummary& summary,
                                                uint32_t width  = 256,
                                                uint32_t height = 256);
};

} // namespace Engine
} // namespace ExplorerLens
