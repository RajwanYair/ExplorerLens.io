#pragma once
// CAD Format Plugin Scaffold
// Plugin contract for DWG/DXF support via isolated plugin adapter.
// Provides fallback badge thumbnail when no CAD plugin is loaded.

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace ExplorerLens::Decoders {

// ─── CAD extension types ──────────────────────────────────────────────────────

enum class CADFormat : uint32_t {
 DWG = 0, // AutoCAD Drawing
 DXF = 1, // Drawing Exchange Format
 DWF = 2, // Design Web Format
 DWFx = 3, // Design Web Format XPS
 DGN = 4, // MicroStation Design
 STEP = 5, // ISO STEP (.stp / .step)
 IGES = 6, // IGES exchange (.igs / .iges)
 Unknown = 99,
};

inline std::string ToString(CADFormat f) {
 switch (f) {
 case CADFormat::DWG: return "DWG";
 case CADFormat::DXF: return "DXF";
 case CADFormat::DWF: return "DWF";
 case CADFormat::DWFx: return "DWFx";
 case CADFormat::DGN: return "DGN";
 case CADFormat::STEP: return "STEP";
 case CADFormat::IGES: return "IGES";
 default: return "Unknown";
 }
}

static inline CADFormat DetectCADFormat(const std::string& ext) {
 if (ext == ".dwg") return CADFormat::DWG;
 if (ext == ".dxf") return CADFormat::DXF;
 if (ext == ".dwf") return CADFormat::DWF;
 if (ext == ".dwfx") return CADFormat::DWFx;
 if (ext == ".dgn") return CADFormat::DGN;
 if (ext == ".stp" || ext == ".step") return CADFormat::STEP;
 if (ext == ".igs" || ext == ".iges") return CADFormat::IGES;
 return CADFormat::Unknown;
}

// ─── CAD bounding box ────────────────────────────────────────────────────────

struct CADBoundingBox {
 double minX { 0.0 }, minY { 0.0 };
 double maxX { 1.0 }, maxY { 1.0 };

 double Width() const { return maxX - minX; }
 double Height() const { return maxY - minY; }
 bool IsValid() const { return Width() > 0 && Height() > 0; }

 double AspectRatio() const { return IsValid() ? Width() / Height() : 1.0; }
};

// ─── Layer enumeration ────────────────────────────────────────────────────────

struct CADLayer {
 std::string name;
 bool isVisible { true };
 uint32_t colour { 0xFFFFFF };
 uint32_t entityCount { 0 };
};

// ─── ICADDecoder plugin interface ─────────────────────────────────────────────

struct CADDecodeCapabilities {
 bool canExtractBoundingBox { false };
 bool canRasterize { false };
 bool canEnumerateLayers { false };
 bool supportsArx { false }; // .arx object support
};

struct CADDecodeRequest {
 const uint8_t* data { nullptr };
 size_t size { 0 };
 CADFormat format { CADFormat::Unknown };
 uint32_t targetW { 256 };
 uint32_t targetH { 256 };
 bool renderAll { true }; // false = visible layers only
};

struct CADDecodeResult {
 bool success { false };
 uint32_t widthPx { 0 };
 uint32_t heightPx { 0 };
 CADBoundingBox bounds;
 std::vector<CADLayer> layers;
 double decodeMs { 0.0 };
 std::string errorMsg;
};

// ─── Plugin adapter interface ─────────────────────────────────────────────────

class ICADDecoderPlugin {
public:
 virtual ~ICADDecoderPlugin() = default;
 virtual CADDecodeCapabilities GetCapabilities() const = 0;
 virtual bool CanDecode(CADFormat format) const = 0;
 virtual CADDecodeResult Decode(const CADDecodeRequest& req) = 0;
};

// ─── Fallback badge thumbnail ─────────────────────────────────────────────────

struct CADBadgeThumbnail {
 uint32_t widthPx { 256 };
 uint32_t heightPx { 256 };
 uint8_t r { 48 }, g { 120 }, b { 200 }; // blue = CAD brand colour
 std::string label; // e.g., "DWG" / "DXF"

 static CADBadgeThumbnail ForFormat(CADFormat f) {
 CADBadgeThumbnail b;
 b.label = ToString(f);
 return b;
 }
};

// ─── CAD format plugin scaffold ───────────────────────────────────────────────

struct CADFormatPlugin {
 static std::vector<std::string> SupportedExtensions() {
 return { ".dwg", ".dxf", ".dwf", ".dwfx", ".dgn", ".stp", ".step", ".igs", ".iges" };
 }

 static bool IsPluginBacked() { return true; }

 static CADBadgeThumbnail GetFallbackThumbnail(const std::string& ext) {
 return CADBadgeThumbnail::ForFormat(DetectCADFormat(ext));
 }
};

} // namespace ExplorerLens::Decoders

