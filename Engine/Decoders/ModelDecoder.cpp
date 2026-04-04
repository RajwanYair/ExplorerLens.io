//==============================================================================
// Model Decoder - 3D Model Thumbnail Provider Implementation
// Basic 3D model format support
// Enhanced model decoder — PLY, DAE, flat-shading lighting
//==============================================================================

#include "ModelDecoder.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

namespace ExplorerLens {
namespace Engine {

// Static extension array — 8 formats supported
wchar_t* ModelDecoder::s_extensions[9] = {
    const_cast<wchar_t*>(L".obj"), const_cast<wchar_t*>(L".stl"), const_cast<wchar_t*>(L".gltf"),
    const_cast<wchar_t*>(L".glb"), const_cast<wchar_t*>(L".ply"), const_cast<wchar_t*>(L".dae"),
    const_cast<wchar_t*>(L".3ds"), const_cast<wchar_t*>(L".fbx"), nullptr};

ModelDecoder::ModelDecoder() : m_device(nullptr), m_context(nullptr), m_d3dInitialized(false) {}

ModelDecoder::~ModelDecoder()
{
    CleanupD3D();
}

bool ModelDecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath)
        return false;

    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext)
        return false;

    return (_wcsicmp(ext, L".obj") == 0 || _wcsicmp(ext, L".stl") == 0 || _wcsicmp(ext, L".gltf") == 0
            || _wcsicmp(ext, L".glb") == 0 || _wcsicmp(ext, L".ply") == 0 || _wcsicmp(ext, L".dae") == 0
            || _wcsicmp(ext, L".3ds") == 0 || _wcsicmp(ext, L".fbx") == 0);
}

const wchar_t** ModelDecoder::GetSupportedExtensions() const
{
    return const_cast<const wchar_t**>(s_extensions);
}

DecoderInfo ModelDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name = L"ModelDecoder";
    info.version = L"2.0.0";
    info.supportedExtensions = const_cast<const wchar_t**>(s_extensions);
    info.extensionCount = 8;
    info.supportsGPU = true;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t* ModelDecoder::GetName() const
{
    return L"ModelDecoder";
}

uint32_t ModelDecoder::GetExtensionCount() const
{
    return 8;
}

bool ModelDecoder::SupportsGPU() const
{
    return true;
}

bool ModelDecoder::IsArchiveDecoder() const
{
    return false;
}

HRESULT ModelDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    // Initialize result
    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    result.status = E_FAIL;
    result.usedGPU = false;

    // Validate input
    if (!request.filePath) {
        result.status = E_INVALIDARG;
        return E_INVALIDARG;
    }

    // Check file exists
    DWORD attrs = GetFileAttributesW(request.filePath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        result.status = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        return result.status;
    }

    // Load mesh data
    MeshData mesh;
    bool loaded = false;

    const wchar_t* ext = wcsrchr(request.filePath, L'.');
    if (ext) {
        if (_wcsicmp(ext, L".obj") == 0) {
            loaded = LoadOBJ(request.filePath, mesh);
        } else if (_wcsicmp(ext, L".stl") == 0) {
            loaded = LoadSTL(request.filePath, mesh);
        } else if (_wcsicmp(ext, L".gltf") == 0 || _wcsicmp(ext, L".glb") == 0) {
            loaded = LoadGLTF(request.filePath, mesh);
        } else if (_wcsicmp(ext, L".ply") == 0) {
            loaded = LoadPLY(request.filePath, mesh);
        } else if (_wcsicmp(ext, L".dae") == 0) {
            loaded = LoadDAE(request.filePath, mesh);
        }
    }

    if (!loaded || mesh.vertices.empty()) {
        result.status = E_FAIL;
        return E_FAIL;
    }

    // Render preview
    result.hBitmap = RenderMeshPreview(mesh, request.width, request.height);
    if (result.hBitmap) {
        result.status = S_OK;
        result.width = request.width;
        result.height = request.height;
        result.usedGPU = m_d3dInitialized;
        return S_OK;
    }

    result.status = E_FAIL;
    return E_FAIL;
}

bool ModelDecoder::LoadOBJ(const wchar_t* filePath, MeshData& mesh)
{
    // Simple OBJ parser - reads vertices and faces
    std::ifstream file(filePath);
    if (!file.is_open())
        return false;

    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<DirectX::XMFLOAT2> texcoords;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            // Vertex position
            DirectX::XMFLOAT3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        } else if (type == "vn") {
            // Vertex normal
            DirectX::XMFLOAT3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (type == "vt") {
            // Texture coordinate
            DirectX::XMFLOAT2 tc;
            iss >> tc.x >> tc.y;
            texcoords.push_back(tc);
        } else if (type == "f") {
            // Face (only handle triangles for simplicity)
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;

            // Parse vertex indices (format: v/vt/vn or v//vn or v/vt or v)
            auto parseVertex = [&](const std::string& vertStr) -> Vertex {
                Vertex vert = {};
                size_t pos1 = vertStr.find('/');
                size_t pos2 = vertStr.find('/', pos1 + 1);

                int vIdx = std::stoi(vertStr.substr(0, pos1 != std::string::npos ? pos1 : vertStr.size())) - 1;
                if (vIdx >= 0 && vIdx < static_cast<int>(positions.size())) {
                    vert.position = positions[vIdx];
                }

                if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1 + 1) {
                    int vtIdx = std::stoi(vertStr.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
                    if (vtIdx >= 0 && vtIdx < static_cast<int>(texcoords.size())) {
                        vert.texcoord = texcoords[vtIdx];
                    }
                }

                if (pos2 != std::string::npos && pos2 + 1 < vertStr.size()) {
                    int vnIdx = std::stoi(vertStr.substr(pos2 + 1)) - 1;
                    if (vnIdx >= 0 && vnIdx < static_cast<int>(normals.size())) {
                        vert.normal = normals[vnIdx];
                    }
                }

                return vert;
            };

            mesh.vertices.push_back(parseVertex(v1));
            mesh.vertices.push_back(parseVertex(v2));
            mesh.vertices.push_back(parseVertex(v3));
        }
    }

    CalculateBounds(mesh);
    return !mesh.vertices.empty();
}

bool ModelDecoder::LoadSTL(const wchar_t* filePath, MeshData& mesh)
{
    // Read file data
    size_t fileSize = 0;
    auto data = ReadFileData(filePath, fileSize);
    if (!data || fileSize < 84)
        return false;

    // Check if binary STL (has 80-byte header + 4-byte triangle count)
    // ASCII STL starts with "solid"
    if (fileSize >= 5 && memcmp(data.get(), "solid", 5) == 0) {
        // Might be ASCII STL
        return LoadSTLAscii(reinterpret_cast<const char*>(data.get()), mesh);
    } else {
        // Binary STL
        return LoadSTLBinary(data.get(), fileSize, mesh);
    }
}

bool ModelDecoder::LoadSTLBinary(const uint8_t* data, size_t size, MeshData& mesh)
{
    if (size < 84)
        return false;

    // Binary STL format:
    // 80 bytes: header
    // 4 bytes: number of triangles (uint32_t)
    // For each triangle (50 bytes):
    // 12 bytes: normal vector (3 floats)
    // 12 bytes: vertex 1 (3 floats)
    // 12 bytes: vertex 2 (3 floats)
    // 12 bytes: vertex 3 (3 floats)
    // 2 bytes: attribute byte count

    const uint8_t* ptr = data + 80;
    uint32_t triangleCount = *reinterpret_cast<const uint32_t*>(ptr);
    ptr += 4;

    if (size < 84 + static_cast<size_t>(triangleCount) * 50)
        return false;

    for (uint32_t i = 0; i < triangleCount; ++i) {
        DirectX::XMFLOAT3 normal;
        memcpy(&normal, ptr, 12);
        ptr += 12;

        for (int j = 0; j < 3; ++j) {
            Vertex v;
            memcpy(&v.position, ptr, 12);
            ptr += 12;
            v.normal = normal;
            v.texcoord = DirectX::XMFLOAT2(0, 0);
            mesh.vertices.push_back(v);
        }

        ptr += 2;  // Skip attribute bytes
    }

    CalculateBounds(mesh);
    return true;
}

bool ModelDecoder::LoadSTLAscii(const char* text, MeshData& mesh)
{
    // Simple ASCII STL parser
    std::istringstream iss(text);
    std::string line, word;

    DirectX::XMFLOAT3 currentNormal = {};
    std::vector<Vertex> currentTriangle;

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        lineStream >> word;

        if (word == "facet") {
            lineStream >> word;  // "normal"
            lineStream >> currentNormal.x >> currentNormal.y >> currentNormal.z;
            currentTriangle.clear();
        } else if (word == "vertex") {
            Vertex v;
            lineStream >> v.position.x >> v.position.y >> v.position.z;
            v.normal = currentNormal;
            v.texcoord = DirectX::XMFLOAT2(0, 0);
            currentTriangle.push_back(v);
        } else if (word == "endfacet") {
            if (currentTriangle.size() == 3) {
                mesh.vertices.insert(mesh.vertices.end(), currentTriangle.begin(), currentTriangle.end());
            }
        }
    }

    CalculateBounds(mesh);
    return !mesh.vertices.empty();
}

bool ModelDecoder::LoadGLTF(const wchar_t* filePath, MeshData& mesh)
{
    // GLTF/GLB loading requires a full parser (use tinygltf or cgltf in production)
    // For now, return placeholder indicating format is recognized but not implemented
    (void)filePath;  // Suppress unused parameter warning
    (void)mesh;      // Suppress unused parameter warning
    return false;
}

//==========================================================================
// PLY Parser — Stanford Polygon Format (ASCII and binary little-endian)
// Added PLY support for point clouds and triangle meshes
//==========================================================================
bool ModelDecoder::LoadPLY(const wchar_t* filePath, MeshData& mesh)
{
    size_t fileSize = 0;
    auto data = ReadFileData(filePath, fileSize);
    if (!data || fileSize < 10)
        return false;

    // Parse PLY header
    std::string header(reinterpret_cast<const char*>(data.get()), std::min(fileSize, static_cast<size_t>(8192)));

    // Verify magic
    if (header.substr(0, 3) != "ply")
        return false;

    // Determine format
    bool isBinary = false;
    bool isLittleEndian = true;
    size_t headerEnd = 0;

    std::istringstream headerStream(header);
    std::string line;
    uint32_t vertexCount = 0;
    uint32_t faceCount = 0;
    bool inVertexElement = false;
    std::vector<std::string> vertexProps;

    while (std::getline(headerStream, line)) {
        // Trim carriage return
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line == "end_header") {
            headerEnd = static_cast<size_t>(headerStream.tellg());
            break;
        }

        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;

        if (token == "format") {
            std::string fmt;
            lineStream >> fmt;
            if (fmt == "binary_little_endian") {
                isBinary = true;
                isLittleEndian = true;
            } else if (fmt == "binary_big_endian") {
                isBinary = true;
                isLittleEndian = false;
            }
            // else ascii
        } else if (token == "element") {
            std::string elemType;
            uint32_t count = 0;
            lineStream >> elemType >> count;
            if (elemType == "vertex") {
                vertexCount = count;
                inVertexElement = true;
            } else {
                if (elemType == "face")
                    faceCount = count;
                inVertexElement = false;
            }
        } else if (token == "property" && inVertexElement) {
            std::string propType, propName;
            lineStream >> propType >> propName;
            vertexProps.push_back(propName);
        }
    }

    if (headerEnd == 0 || vertexCount == 0)
        return false;

    // Cap vertex count for thumbnail safety
    if (vertexCount > 2000000)
        vertexCount = 2000000;

    if (isBinary) {
        return LoadPLYBinary(data.get(), headerEnd, vertexProps, vertexCount, faceCount, isLittleEndian, mesh);
    } else {
        return LoadPLYAscii(reinterpret_cast<const char*>(data.get()), headerEnd, vertexProps, vertexCount, faceCount,
                            mesh);
    }
}

bool ModelDecoder::LoadPLYAscii(const char* data, size_t headerEnd, const std::vector<std::string>& vertexProps,
                                uint32_t vertexCount, uint32_t faceCount, MeshData& mesh)
{
    // Find property indices for x, y, z, nx, ny, nz
    int xIdx = -1, yIdx = -1, zIdx = -1;
    int nxIdx = -1, nyIdx = -1, nzIdx = -1;
    for (size_t i = 0; i < vertexProps.size(); ++i) {
        if (vertexProps[i] == "x")
            xIdx = static_cast<int>(i);
        else if (vertexProps[i] == "y")
            yIdx = static_cast<int>(i);
        else if (vertexProps[i] == "z")
            zIdx = static_cast<int>(i);
        else if (vertexProps[i] == "nx")
            nxIdx = static_cast<int>(i);
        else if (vertexProps[i] == "ny")
            nyIdx = static_cast<int>(i);
        else if (vertexProps[i] == "nz")
            nzIdx = static_cast<int>(i);
    }

    if (xIdx < 0 || yIdx < 0 || zIdx < 0)
        return false;

    std::istringstream dataStream(data + headerEnd);
    std::string line;

    // Read vertices
    std::vector<Vertex> vertices(vertexCount);
    for (uint32_t i = 0; i < vertexCount; ++i) {
        if (!std::getline(dataStream, line))
            break;
        std::istringstream lineStream(line);
        std::vector<float> values;
        float val;
        while (lineStream >> val)
            values.push_back(val);

        int propCount = static_cast<int>(vertexProps.size());
        if (static_cast<int>(values.size()) < propCount)
            continue;

        vertices[i].position = DirectX::XMFLOAT3(values[xIdx], values[yIdx], values[zIdx]);
        if (nxIdx >= 0 && nyIdx >= 0 && nzIdx >= 0) {
            vertices[i].normal = DirectX::XMFLOAT3(values[nxIdx], values[nyIdx], values[nzIdx]);
        }
        vertices[i].texcoord = DirectX::XMFLOAT2(0, 0);
    }

    // Read faces
    if (faceCount > 0) {
        for (uint32_t i = 0; i < faceCount; ++i) {
            if (!std::getline(dataStream, line))
                break;
            std::istringstream lineStream(line);
            int vertCount = 0;
            lineStream >> vertCount;
            if (vertCount >= 3) {
                std::vector<uint32_t> faceIndices(vertCount);
                for (int j = 0; j < vertCount; ++j)
                    lineStream >> faceIndices[j];
                // Fan triangulation
                for (int j = 1; j + 1 < vertCount; ++j) {
                    uint32_t i0 = faceIndices[0], i1 = faceIndices[j], i2 = faceIndices[j + 1];
                    if (i0 < vertexCount && i1 < vertexCount && i2 < vertexCount) {
                        mesh.vertices.push_back(vertices[i0]);
                        mesh.vertices.push_back(vertices[i1]);
                        mesh.vertices.push_back(vertices[i2]);
                    }
                }
            }
        }
    } else {
        // Point cloud — treat as individual points
        mesh.vertices = std::move(vertices);
    }

    ComputeFaceNormals(mesh);
    CalculateBounds(mesh);
    return !mesh.vertices.empty();
}

bool ModelDecoder::LoadPLYBinary(const uint8_t* data, size_t headerEnd, const std::vector<std::string>& vertexProps,
                                 uint32_t vertexCount, uint32_t faceCount, bool littleEndian, MeshData& mesh)
{
    // Find property indices
    int xIdx = -1, yIdx = -1, zIdx = -1;
    for (size_t i = 0; i < vertexProps.size(); ++i) {
        if (vertexProps[i] == "x")
            xIdx = static_cast<int>(i);
        else if (vertexProps[i] == "y")
            yIdx = static_cast<int>(i);
        else if (vertexProps[i] == "z")
            zIdx = static_cast<int>(i);
    }
    if (xIdx < 0 || yIdx < 0 || zIdx < 0)
        return false;

    // Simplified binary reader: assume all vertex properties are float32
    size_t vertexStride = vertexProps.size() * sizeof(float);
    const uint8_t* ptr = data + headerEnd;

    std::vector<Vertex> vertices(vertexCount);
    for (uint32_t i = 0; i < vertexCount; ++i) {
        const float* floats = reinterpret_cast<const float*>(ptr);
        // For big-endian, we'd need byte swapping; skip for now (rare)
        if (littleEndian) {
            vertices[i].position = DirectX::XMFLOAT3(floats[xIdx], floats[yIdx], floats[zIdx]);
        }
        vertices[i].texcoord = DirectX::XMFLOAT2(0, 0);
        ptr += vertexStride;
    }

    // Read faces (binary: uint8 count + N * uint32 indices)
    if (faceCount > 0) {
        for (uint32_t i = 0; i < faceCount; ++i) {
            uint8_t vertCount = *ptr++;
            if (vertCount >= 3) {
                const uint32_t* indices = reinterpret_cast<const uint32_t*>(ptr);
                for (int j = 1; j + 1 < vertCount; ++j) {
                    uint32_t i0 = indices[0], i1 = indices[j], i2 = indices[j + 1];
                    if (i0 < vertexCount && i1 < vertexCount && i2 < vertexCount) {
                        mesh.vertices.push_back(vertices[i0]);
                        mesh.vertices.push_back(vertices[i1]);
                        mesh.vertices.push_back(vertices[i2]);
                    }
                }
            }
            ptr += vertCount * sizeof(uint32_t);
        }
    } else {
        mesh.vertices = std::move(vertices);
    }

    ComputeFaceNormals(mesh);
    CalculateBounds(mesh);
    return !mesh.vertices.empty();
}

//==========================================================================
// DAE (COLLADA) Parser — XML-based 3D exchange format
// Lightweight COLLADA parser extracting first geometry
//==========================================================================
bool ModelDecoder::LoadDAE(const wchar_t* filePath, MeshData& mesh)
{
    // Read file as text
    std::ifstream file(filePath);
    if (!file.is_open())
        return false;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Verify COLLADA
    if (content.find("<COLLADA") == std::string::npos && content.find("<collada") == std::string::npos)
        return false;

    // Find first <float_array> containing position data
    // COLLADA structure: <mesh> -> <source> -> <float_array>
    // We look for position source (usually first, or identified by "position" id)
    std::vector<float> positions;
    std::vector<uint32_t> indices;

    // Extract positions from first suitable float_array
    auto extractFloatArray = [](const std::string& xml, const std::string& startTag, std::vector<float>& out) -> bool {
        size_t pos = xml.find(startTag);
        if (pos == std::string::npos)
            return false;
        size_t dataStart = xml.find('>', pos);
        if (dataStart == std::string::npos)
            return false;
        dataStart++;
        size_t dataEnd = xml.find('<', dataStart);
        if (dataEnd == std::string::npos)
            return false;

        std::istringstream iss(xml.substr(dataStart, dataEnd - dataStart));
        float val;
        while (iss >> val)
            out.push_back(val);
        return !out.empty();
    };

    // Try to find positions — look for float_array with "position" or "mesh-positions"
    bool found = false;
    size_t searchPos = 0;
    while (!found) {
        size_t faPos = content.find("<float_array", searchPos);
        if (faPos == std::string::npos)
            break;

        // Check if this is a position array (look at parent source id)
        size_t idStart = content.rfind("<source", faPos);
        if (idStart != std::string::npos) {
            std::string sourceBlock = content.substr(idStart, faPos - idStart);
            if (sourceBlock.find("position") != std::string::npos || sourceBlock.find("Position") != std::string::npos
                || sourceBlock.find("POSITION") != std::string::npos) {
                found =
                    extractFloatArray(content, content.substr(faPos, content.find('>', faPos) - faPos + 1), positions);
                if (found)
                    break;
            }
        }

        // If no explicit position tag found, just use the first float_array
        if (!found && searchPos == 0) {
            size_t gt = content.find('>', faPos);
            if (gt != std::string::npos) {
                size_t dataEnd = content.find("</float_array>", gt);
                if (dataEnd != std::string::npos) {
                    std::istringstream iss(content.substr(gt + 1, dataEnd - gt - 1));
                    float val;
                    while (iss >> val)
                        positions.push_back(val);
                    if (!positions.empty())
                        found = true;
                }
            }
        }
        searchPos = faPos + 1;
    }

    if (positions.size() < 9)
        return false;  // Need at least 3 vertices (9 floats)

    // Extract indices from <p> element inside <triangles> or <polylist>
    size_t triPos = content.find("<triangles");
    if (triPos == std::string::npos)
        triPos = content.find("<polylist");

    if (triPos != std::string::npos) {
        size_t pPos = content.find("<p>", triPos);
        if (pPos != std::string::npos) {
            size_t pEnd = content.find("</p>", pPos);
            if (pEnd != std::string::npos) {
                std::istringstream iss(content.substr(pPos + 3, pEnd - pPos - 3));
                uint32_t val;
                while (iss >> val)
                    indices.push_back(val);
            }
        }
    }

    // Determine stride (COLLADA may interleave vertex/normal/texcoord indices)
    // Count <input> elements inside <triangles> to get stride
    uint32_t indexStride = 1;
    if (triPos != std::string::npos) {
        size_t searchEnd = content.find('>', triPos + 10);
        if (searchEnd == std::string::npos)
            searchEnd = triPos + 500;
        size_t endTag = content.find("</triangles>", triPos);
        if (endTag == std::string::npos)
            endTag = content.find("</polylist>", triPos);
        if (endTag != std::string::npos) {
            std::string triBlock = content.substr(triPos, endTag - triPos);
            size_t inputCount = 0;
            size_t sp = 0;
            while ((sp = triBlock.find("<input", sp)) != std::string::npos) {
                inputCount++;
                sp += 6;
            }
            if (inputCount > 0)
                indexStride = static_cast<uint32_t>(inputCount);
        }
    }

    uint32_t vertCount = static_cast<uint32_t>(positions.size() / 3);

    if (!indices.empty() && indexStride > 0) {
        // Use indexed triangles
        for (size_t i = 0; i + 2 * indexStride < indices.size(); i += 3 * indexStride) {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + indexStride];
            uint32_t i2 = indices[i + 2 * indexStride];

            for (uint32_t idx : {i0, i1, i2}) {
                if (idx < vertCount) {
                    Vertex v = {};
                    v.position = DirectX::XMFLOAT3(positions[idx * 3], positions[idx * 3 + 1], positions[idx * 3 + 2]);
                    mesh.vertices.push_back(v);
                }
            }
        }
    } else {
        // No indices — assume sequential triangles
        for (size_t i = 0; i + 8 < positions.size(); i += 9) {
            for (int j = 0; j < 3; ++j) {
                Vertex v = {};
                v.position =
                    DirectX::XMFLOAT3(positions[i + j * 3], positions[i + j * 3 + 1], positions[i + j * 3 + 2]);
                mesh.vertices.push_back(v);
            }
        }
    }

    ComputeFaceNormals(mesh);
    CalculateBounds(mesh);
    return !mesh.vertices.empty();
}

//==========================================================================
// Compute face normals for triangles that lack them
//==========================================================================
void ModelDecoder::ComputeFaceNormals(MeshData& mesh)
{
    // Compute per-face normals for triangles that have zero normals
    for (size_t i = 0; i + 2 < mesh.vertices.size(); i += 3) {
        auto& v0 = mesh.vertices[i];
        auto& v1 = mesh.vertices[i + 1];
        auto& v2 = mesh.vertices[i + 2];

        // Check if normals are already set
        bool hasNormal = (v0.normal.x != 0.0f || v0.normal.y != 0.0f || v0.normal.z != 0.0f);
        if (hasNormal)
            continue;

        // Compute face normal via cross product
        DirectX::XMFLOAT3 e1(v1.position.x - v0.position.x, v1.position.y - v0.position.y,
                             v1.position.z - v0.position.z);
        DirectX::XMFLOAT3 e2(v2.position.x - v0.position.x, v2.position.y - v0.position.y,
                             v2.position.z - v0.position.z);

        DirectX::XMFLOAT3 normal(e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x);

        // Normalize
        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        if (len > 1e-6f) {
            normal.x /= len;
            normal.y /= len;
            normal.z /= len;
        }

        v0.normal = v1.normal = v2.normal = normal;
    }
}

//==========================================================================
// Flat-shading lighting system
//==========================================================================
ModelDecoder::LightParams ModelDecoder::GetDefaultLighting() const
{
    LightParams light;
    // Upper-right directional light, normalized
    float invSqrt3 = 1.0f / std::sqrt(3.0f);
    light.direction = DirectX::XMFLOAT3(invSqrt3, invSqrt3, -invSqrt3);
    light.ambient = 0.25f;
    light.diffuse = 0.75f;
    light.baseColor = RGB(140, 180, 220);     // Steel blue
    light.backgroundColor = RGB(35, 35, 40);  // Dark charcoal
    return light;
}

COLORREF ModelDecoder::ComputeLitColor(const DirectX::XMFLOAT3& normal, const LightParams& light) const
{
    // Lambertian diffuse: max(0, dot(normal, lightDir))
    float dot = normal.x * light.direction.x + normal.y * light.direction.y + normal.z * light.direction.z;
    if (dot < 0.0f)
        dot = -dot;  // Two-sided lighting

    float intensity = light.ambient + light.diffuse * dot;
    if (intensity > 1.0f)
        intensity = 1.0f;

    int r = static_cast<int>(GetRValue(light.baseColor) * intensity);
    int g = static_cast<int>(GetGValue(light.baseColor) * intensity);
    int b = static_cast<int>(GetBValue(light.baseColor) * intensity);

    return RGB((r > 255) ? 255 : r, (g > 255) ? 255 : g, (b > 255) ? 255 : b);
}

HBITMAP ModelDecoder::RenderMeshPreview(const MeshData& mesh, uint32_t width, uint32_t height)
{
    // Flat-shaded rendering with directional lighting
    // Projects 3D triangles to 2D using isometric-like view and fills with lit color

    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    LightParams light = GetDefaultLighting();

    // Fill background
    RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    HBRUSH bgBrush = CreateSolidBrush(light.backgroundColor);
    FillRect(memDC, &rect, bgBrush);
    DeleteObject(bgBrush);

    // Calculate isometric-style projection (30-degree rotation around Y, 20-degree tilt)
    float cosA = std::cos(0.52f);  // ~30 degrees
    float sinA = std::sin(0.52f);
    float cosB = std::cos(0.35f);  // ~20 degrees tilt
    float sinB = std::sin(0.35f);

    // Calculate projection scale
    float rangeX = mesh.boundsMax.x - mesh.boundsMin.x;
    float rangeY = mesh.boundsMax.y - mesh.boundsMin.y;
    float rangeZ = mesh.boundsMax.z - mesh.boundsMin.z;
    float maxRange = (std::max)({rangeX, rangeY, rangeZ, 0.001f});
    float scale = (std::min)(width, height) * 0.7f / maxRange;

    float centerX = (mesh.boundsMin.x + mesh.boundsMax.x) / 2.0f;
    float centerY = (mesh.boundsMin.y + mesh.boundsMax.y) / 2.0f;
    float centerZ = (mesh.boundsMin.z + mesh.boundsMax.z) / 2.0f;

    // Project 3D point to 2D using isometric transform
    auto project = [&](const DirectX::XMFLOAT3& p) -> POINT {
        float x = p.x - centerX;
        float y = p.y - centerY;
        float z = p.z - centerZ;
        // Rotate around Y axis
        float rx = x * cosA + z * sinA;
        float ry = y;
        float rz = -x * sinA + z * cosA;
        // Tilt around X axis
        float px = rx;
        float py = ry * cosB - rz * sinB;
        POINT pt;
        pt.x = static_cast<LONG>(px * scale + width / 2.0f);
        pt.y = static_cast<LONG>(height / 2.0f - py * scale);
        return pt;
    };

    // Depth sort triangles (painter's algorithm — back to front)
    struct SortedTri
    {
        POINT pts[3];
        float depth;
        COLORREF color;
    };
    std::vector<SortedTri> sortedTriangles;

    for (size_t i = 0; i + 2 < mesh.vertices.size(); i += 3) {
        const auto& v0 = mesh.vertices[i];
        const auto& v1 = mesh.vertices[i + 1];
        const auto& v2 = mesh.vertices[i + 2];

        SortedTri tri;
        tri.pts[0] = project(v0.position);
        tri.pts[1] = project(v1.position);
        tri.pts[2] = project(v2.position);

        // Average depth for sorting (using rotated Z)
        float z0 = -(v0.position.x - centerX) * sinA + (v0.position.z - centerZ) * cosA;
        float z1 = -(v1.position.x - centerX) * sinA + (v1.position.z - centerZ) * cosA;
        float z2 = -(v2.position.x - centerX) * sinA + (v2.position.z - centerZ) * cosA;
        tri.depth = (z0 + z1 + z2) / 3.0f;

        // Compute lit color from face normal
        tri.color = ComputeLitColor(v0.normal, light);
        sortedTriangles.push_back(tri);
    }

    // Sort back-to-front
    std::sort(sortedTriangles.begin(), sortedTriangles.end(),
              [](const SortedTri& a, const SortedTri& b) { return a.depth < b.depth; });

    // Draw filled triangles with outline
    HPEN edgePen = CreatePen(PS_SOLID, 1, RGB(60, 60, 65));
    HPEN oldPen = (HPEN)SelectObject(memDC, edgePen);

    for (const auto& tri : sortedTriangles) {
        HBRUSH fillBrush = CreateSolidBrush(tri.color);
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, fillBrush);
        Polygon(memDC, tri.pts, 3);
        SelectObject(memDC, oldBrush);
        DeleteObject(fillBrush);
    }

    // If it's a point cloud (too few triangles), draw points
    if (sortedTriangles.empty() && !mesh.vertices.empty()) {
        HPEN pointPen = CreatePen(PS_SOLID, 2, RGB(140, 180, 220));
        SelectObject(memDC, pointPen);
        for (const auto& v : mesh.vertices) {
            POINT pt = project(v.position);
            MoveToEx(memDC, pt.x, pt.y, NULL);
            LineTo(memDC, pt.x + 1, pt.y + 1);
        }
        SelectObject(memDC, edgePen);
        DeleteObject(pointPen);
    }

    SelectObject(memDC, oldPen);
    DeleteObject(edgePen);

    SelectObject(memDC, hOldBitmap);
    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);

    return hBitmap;
}

void ModelDecoder::CalculateBounds(MeshData& mesh)
{
    if (mesh.vertices.empty())
        return;

    mesh.boundsMin = mesh.vertices[0].position;
    mesh.boundsMax = mesh.vertices[0].position;

    for (const auto& v : mesh.vertices) {
        mesh.boundsMin.x = (mesh.boundsMin.x < v.position.x) ? mesh.boundsMin.x : v.position.x;
        mesh.boundsMin.y = (mesh.boundsMin.y < v.position.y) ? mesh.boundsMin.y : v.position.y;
        mesh.boundsMin.z = (mesh.boundsMin.z < v.position.z) ? mesh.boundsMin.z : v.position.z;

        mesh.boundsMax.x = (mesh.boundsMax.x > v.position.x) ? mesh.boundsMax.x : v.position.x;
        mesh.boundsMax.y = (mesh.boundsMax.y > v.position.y) ? mesh.boundsMax.y : v.position.y;
        mesh.boundsMax.z = (mesh.boundsMax.z > v.position.z) ? mesh.boundsMax.z : v.position.z;
    }
}

DirectX::XMMATRIX ModelDecoder::CalculateViewMatrix(const MeshData& mesh)
{
    DirectX::XMFLOAT3 center((mesh.boundsMin.x + mesh.boundsMax.x) / 2.0f, (mesh.boundsMin.y + mesh.boundsMax.y) / 2.0f,
                             (mesh.boundsMin.z + mesh.boundsMax.z) / 2.0f);

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(center.x, center.y + 5.0f, center.z + 10.0f, 1.0f);
    DirectX::XMVECTOR at = DirectX::XMVectorSet(center.x, center.y, center.z, 1.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    return DirectX::XMMatrixLookAtLH(eye, at, up);
}

std::unique_ptr<uint8_t[]> ModelDecoder::ReadFileData(const wchar_t* filePath, size_t& outSize)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        outSize = 0;
        return nullptr;
    }

    outSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    auto data = std::make_unique<uint8_t[]>(outSize);
    file.read(reinterpret_cast<char*>(data.get()), outSize);

    return data;
}

bool ModelDecoder::InitializeD3D()
{
    if (m_d3dInitialized)
        return true;

    // Initialize D3D11 device for GPU rendering
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
                                   &m_device, &featureLevel, &m_context);

    if (SUCCEEDED(hr)) {
        m_d3dInitialized = true;
    }

    return m_d3dInitialized;
}

void ModelDecoder::CleanupD3D()
{
    if (m_context) {
        m_context->Release();
        m_context = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }
    m_d3dInitialized = false;
}

}  // namespace Engine
}  // namespace ExplorerLens
