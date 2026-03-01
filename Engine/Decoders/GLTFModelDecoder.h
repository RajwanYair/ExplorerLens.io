#pragma once
// glTF/GLB 3D Model Decoder
// Asset parse, scene bounding box, camera placement, D3D11 rasterization.
// Fallback: bounding-box wireframe or PBR base-color texture for complex scenes.

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <optional>
#include <array>

namespace ExplorerLens::Decoders {

// ─── Asset variant ────────────────────────────────────────────────────────────

enum class GLTFVariant : uint32_t {
 GLTF = 0, // JSON + external resources
 GLB = 1, // Binary (self-contained)
};

// ─── Scene bounding box ───────────────────────────────────────────────────────

struct AABB3D {
 float minX { 0.0f }, minY { 0.0f }, minZ { 0.0f };
 float maxX { 1.0f }, maxY { 1.0f }, maxZ { 1.0f };

 float Width() const { return maxX - minX; }
 float Height() const { return maxY - minY; }
 float Depth() const { return maxZ - minZ; }

 float MaxExtent() const {
 float w = Width(), h = Height(), d = Depth();
 return w > h ? (w > d ? w : d) : (h > d ? h : d);
 }

 std::array<float, 3> Center() const {
 return { (minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f };
 }
};

// ─── Mesh complexity ─────────────────────────────────────────────────────────

struct MeshComplexity {
 uint32_t meshCount { 0 };
 uint32_t primitiveCount { 0 };
 uint64_t totalVertices { 0 };
 uint64_t totalTriangles { 0 };
 uint32_t materialCount { 0 };
 bool hasNormals { false };
 bool hasTexCoords { false };
 bool hasPBRMaterial { false };

 static constexpr uint64_t kComplexityThreshold = 100000; // >100K triangles = complex

 bool IsComplex() const { return totalTriangles > kComplexityThreshold; }
};

// ─── Camera placement ────────────────────────────────────────────────────────

struct Camera3D {
 std::array<float, 3> position { 1.5f, 1.5f, 1.5f };
 std::array<float, 3> target { 0.0f, 0.0f, 0.0f };
 std::array<float, 3> up { 0.0f, 1.0f, 0.0f };
 float fovYDegrees { 45.0f };
 float nearPlane { 0.01f };
 float farPlane { 1000.0f };

 static Camera3D DefaultForScene(const AABB3D& bounds) {
 Camera3D cam;
 float e = bounds.MaxExtent() * 1.5f;
 auto c = bounds.Center();
 cam.position = { c[0] + e, c[1] + e, c[2] + e };
 cam.target = c;
 return cam;
 }
};

// ─── Render path ─────────────────────────────────────────────────────────────

enum class GLTFRenderPath : uint32_t {
 FullPBR = 0, // D3D11 PBR shader rasterization
 WireframeFallback = 1, // wireframe when complexity > threshold
 TextureFlatmap = 2, // blit base colour texture directly
 ShapeOnly = 3, // solid geometry, no texture
};

inline std::string ToString(GLTFRenderPath p) {
 switch (p) {
 case GLTFRenderPath::FullPBR: return "FullPBR";
 case GLTFRenderPath::WireframeFallback: return "WireframeFallback";
 case GLTFRenderPath::TextureFlatmap: return "TextureFlatmap";
 case GLTFRenderPath::ShapeOnly: return "ShapeOnly";
 default: return "Unknown";
 }
}

// ─── Decode result ────────────────────────────────────────────────────────────

struct GLTFDecodeResult {
 bool success { false };
 uint32_t widthPx { 0 };
 uint32_t heightPx { 0 };
 GLTFRenderPath renderPath { GLTFRenderPath::FullPBR };
 MeshComplexity complexity;
 AABB3D sceneBounds;
 double decodeMs { 0.0 };
 std::string errorMsg;

 bool WasFallback() const {
 return renderPath == GLTFRenderPath::WireframeFallback ||
 renderPath == GLTFRenderPath::TextureFlatmap;
 }
};

// ─── Decoder ─────────────────────────────────────────────────────────────────

struct GLTFModelDecoder {
 static std::vector<std::string> SupportedExtensions() {
 return { ".gltf", ".glb" };
 }

 static GLTFVariant DetectVariant(const std::string& ext) {
 return (ext == ".glb") ? GLTFVariant::GLB : GLTFVariant::GLTF;
 }

 GLTFRenderPath SelectRenderPath(const MeshComplexity& c, bool hasD3D11) const {
 if (!hasD3D11) return GLTFRenderPath::ShapeOnly;
 if (c.IsComplex() && c.hasPBRMaterial) return GLTFRenderPath::TextureFlatmap;
 if (c.IsComplex()) return GLTFRenderPath::WireframeFallback;
 return GLTFRenderPath::FullPBR;
 }

 /// Parse glTF/GLB JSON to compute real mesh complexity from accessor counts.
 /// For GLB, the 12-byte header + chunk header are skipped to reach JSON.
 /// Accessors provide per-primitive index and vertex counts; "indices"
 /// accessor count / 3 = triangle count, "POSITION" accessor count = vertices.
 GLTFDecodeResult Decode(const uint8_t* data, size_t size,
 uint32_t targetW, uint32_t targetH,
 bool hasD3D11 = true) const {
 GLTFDecodeResult r;
 r.widthPx = targetW;
 r.heightPx = targetH;
 if (!data || size < 12) {
  r.errorMsg = "Input too small";
  return r;
 }

 // ── Extract JSON payload from GLB or raw glTF ───────────────
 const char* jsonPtr = nullptr;
 size_t jsonLen = 0;
 if (size >= 20 && data[0] == 'g' && data[1] == 'l' &&
     data[2] == 'T' && data[3] == 'F') {
  // GLB: header(12) + chunkLen(4) + chunkType(4) + JSON
  uint32_t c0Len = static_cast<uint32_t>(data[12]) |
   (static_cast<uint32_t>(data[13]) << 8) |
   (static_cast<uint32_t>(data[14]) << 16) |
   (static_cast<uint32_t>(data[15]) << 24);
  if (20 + static_cast<size_t>(c0Len) > size) {
   r.errorMsg = "GLB JSON chunk truncated";
   return r;
  }
  jsonPtr = reinterpret_cast<const char*>(data + 20);
  jsonLen = c0Len;
 } else {
  jsonPtr = reinterpret_cast<const char*>(data);
  jsonLen = size;
 }
 std::string json(jsonPtr, jsonLen);

 // ── Minimal JSON helpers (no library dependency) ────────────
 auto skipWS = [&](size_t p) -> size_t {
  while (p < json.size() && (json[p]==' '||json[p]=='\t'||
         json[p]=='\n'||json[p]=='\r')) ++p;
  return p;
 };
 auto parseNum = [&](size_t p, int64_t& val) -> size_t {
  p = skipWS(p);
  if (p >= json.size()) return std::string::npos;
  bool neg = (json[p] == '-');
  if (neg) ++p;
  if (p >= json.size() || json[p] < '0' || json[p] > '9')
   return std::string::npos;
  int64_t n = 0;
  while (p < json.size() && json[p] >= '0' && json[p] <= '9')
   n = n * 10 + (json[p++] - '0');
  val = neg ? -n : n;
  return p;
 };
 auto findKey = [&](const std::string& key,
                    size_t s, size_t e) -> size_t {
  std::string pat = "\"" + key + "\"";
  size_t kp = json.find(pat, s);
  if (kp == std::string::npos || kp >= e) return std::string::npos;
  size_t cp = json.find(':', kp + pat.size());
  if (cp == std::string::npos || cp >= e) return std::string::npos;
  return skipWS(cp + 1);
 };
 auto matchBracket = [&](size_t p, char op, char cl) -> size_t {
  if (p >= json.size() || json[p] != op) return std::string::npos;
  int d = 1;
  for (size_t i = p + 1; i < json.size(); ++i) {
   if (json[i] == op) ++d;
   else if (json[i] == cl) { if (--d == 0) return i; }
  }
  return std::string::npos;
 };

 // ── Parse accessors → collect "count" per accessor ──────────
 std::vector<uint64_t> accCounts;
 {
  size_t ap = findKey("accessors", 0, json.size());
  if (ap != std::string::npos && json[ap] == '[') {
   size_t ae = matchBracket(ap, '[', ']');
   if (ae != std::string::npos) {
    size_t p = ap + 1;
    while (p < ae) {
     p = skipWS(p);
     if (p >= ae) break;
     if (json[p] == '{') {
      size_t oe = matchBracket(p, '{', '}');
      if (oe == std::string::npos) break;
      int64_t cnt = 0;
      size_t cv = findKey("count", p, oe);
      if (cv != std::string::npos) parseNum(cv, cnt);
      accCounts.push_back(cnt > 0 ? static_cast<uint64_t>(cnt) : 0);
      p = oe + 1;
     } else { ++p; }
    }
   }
  }
 }

 // ── Parse meshes → primitives → indices / attributes ────────
 uint64_t totalTris = 0, totalVerts = 0;
 uint32_t meshCnt = 0, primCnt = 0;
 bool hasNorm = false, hasUV = false;
 {
  size_t mp = findKey("meshes", 0, json.size());
  if (mp != std::string::npos && json[mp] == '[') {
   size_t me = matchBracket(mp, '[', ']');
   if (me != std::string::npos) {
    size_t p = mp + 1;
    while (p < me) {
     p = skipWS(p);
     if (p >= me) break;
     if (json[p] == '{') {
      size_t oe = matchBracket(p, '{', '}');
      if (oe == std::string::npos) break;
      ++meshCnt;
      size_t pp = findKey("primitives", p, oe);
      if (pp != std::string::npos && json[pp] == '[') {
       size_t pe = matchBracket(pp, '[', ']');
       if (pe != std::string::npos) {
        size_t q = pp + 1;
        while (q < pe) {
         q = skipWS(q);
         if (q >= pe) break;
         if (json[q] == '{') {
          size_t qe = matchBracket(q, '{', '}');
          if (qe == std::string::npos) break;
          ++primCnt;
          // indices accessor → triangle count
          size_t iv = findKey("indices", q, qe);
          if (iv != std::string::npos) {
           int64_t idx = -1; parseNum(iv, idx);
           if (idx >= 0 && static_cast<size_t>(idx) < accCounts.size())
            totalTris += accCounts[static_cast<size_t>(idx)] / 3;
          }
          // attributes → POSITION, NORMAL, TEXCOORD_0
          size_t av = findKey("attributes", q, qe);
          if (av != std::string::npos && json[av] == '{') {
           size_t axe = matchBracket(av, '{', '}');
           if (axe != std::string::npos) {
            size_t pv = findKey("POSITION", av, axe);
            if (pv != std::string::npos) {
             int64_t idx = -1; parseNum(pv, idx);
             if (idx >= 0 && static_cast<size_t>(idx) < accCounts.size())
              totalVerts += accCounts[static_cast<size_t>(idx)];
            }
            if (findKey("NORMAL", av, axe) != std::string::npos)
             hasNorm = true;
            if (findKey("TEXCOORD_0", av, axe) != std::string::npos)
             hasUV = true;
           }
          }
          q = qe + 1;
         } else { ++q; }
        }
       }
      }
      p = oe + 1;
     } else { ++p; }
    }
   }
  }
 }

 // ── Count materials and detect PBR ──────────────────────────
 uint32_t matCnt = 0;
 bool hasPBR = false;
 {
  size_t mp = findKey("materials", 0, json.size());
  if (mp != std::string::npos && json[mp] == '[') {
   size_t me = matchBracket(mp, '[', ']');
   if (me != std::string::npos) {
    size_t p = mp + 1;
    while (p < me) {
     p = skipWS(p);
     if (p >= me) break;
     if (json[p] == '{') {
      size_t oe = matchBracket(p, '{', '}');
      if (oe == std::string::npos) break;
      ++matCnt;
      if (json.find("pbrMetallicRoughness", p) < oe) hasPBR = true;
      p = oe + 1;
     } else { ++p; }
    }
   }
  }
 }

 // ── Populate result ─────────────────────────────────────────
 r.complexity.meshCount = meshCnt;
 r.complexity.primitiveCount = primCnt;
 r.complexity.totalVertices = totalVerts;
 r.complexity.totalTriangles = totalTris;
 r.complexity.materialCount = matCnt;
 r.complexity.hasNormals = hasNorm;
 r.complexity.hasTexCoords = hasUV;
 r.complexity.hasPBRMaterial = hasPBR;
 r.sceneBounds = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
 r.renderPath = SelectRenderPath(r.complexity, hasD3D11);
 r.success = (meshCnt > 0 || !accCounts.empty());
 if (!r.success) r.errorMsg = "No meshes or accessors found in glTF";
 r.decodeMs = 0.0;
 return r;
 }
};

} // namespace ExplorerLens::Decoders
