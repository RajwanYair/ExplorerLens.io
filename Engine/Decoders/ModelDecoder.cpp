//==============================================================================
// Model Decoder - 3D Model Thumbnail Provider Implementation
// Sprint 12: Basic 3D model format support
//==============================================================================

#include "ModelDecoder.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace DarkThumbs {
namespace Engine {

    // Static extension array
    wchar_t* ModelDecoder::s_extensions[5] = {
        const_cast<wchar_t*>(L".obj"),
        const_cast<wchar_t*>(L".stl"),
        const_cast<wchar_t*>(L".gltf"),
        const_cast<wchar_t*>(L".glb"),
        nullptr
    };

    ModelDecoder::ModelDecoder()
        : m_device(nullptr)
        , m_context(nullptr)
        , m_d3dInitialized(false)
    {
    }

    ModelDecoder::~ModelDecoder()
    {
        CleanupD3D();
    }

    bool ModelDecoder::CanDecode(const wchar_t* filePath)
    {
        if (!filePath) return false;

        const wchar_t* ext = wcsrchr(filePath, L'.');
        if (!ext) return false;

        return (_wcsicmp(ext, L".obj") == 0 ||
                _wcsicmp(ext, L".stl") == 0 ||
                _wcsicmp(ext, L".gltf") == 0 ||
                _wcsicmp(ext, L".glb") == 0);
    }

    const wchar_t** ModelDecoder::GetSupportedExtensions() const
    {
        return const_cast<const wchar_t**>(s_extensions);
    }

    DecoderInfo ModelDecoder::GetInfo() const
    {
        DecoderInfo info;
        info.name = L"ModelDecoder";
        info.version = L"1.0.0";
        info.description = L"3D Model Thumbnail Provider (OBJ, STL, GLTF)";
        info.supportedExtensions = const_cast<const wchar_t**>(s_extensions);
        info.extensionCount = 4;
        info.supportsGPU = true;
        info.isArchiveDecoder = false;
        return info;
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
            }
            else if (_wcsicmp(ext, L".stl") == 0) {
                loaded = LoadSTL(request.filePath, mesh);
            }
            else if (_wcsicmp(ext, L".gltf") == 0 || _wcsicmp(ext, L".glb") == 0) {
                loaded = LoadGLTF(request.filePath, mesh);
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
        if (!file.is_open()) return false;

        std::vector<DirectX::XMFLOAT3> positions;
        std::vector<DirectX::XMFLOAT3> normals;
        std::vector<DirectX::XMFLOAT2> texcoords;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "v") {
                // Vertex position
                DirectX::XMFLOAT3 pos;
                iss >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            }
            else if (type == "vn") {
                // Vertex normal
                DirectX::XMFLOAT3 normal;
                iss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            }
            else if (type == "vt") {
                // Texture coordinate
                DirectX::XMFLOAT2 tc;
                iss >> tc.x >> tc.y;
                texcoords.push_back(tc);
            }
            else if (type == "f") {
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
        if (!data || fileSize < 84) return false;

        // Check if binary STL (has 80-byte header + 4-byte triangle count)
        // ASCII STL starts with "solid"
        if (fileSize >= 5 && memcmp(data.get(), "solid", 5) == 0) {
            // Might be ASCII STL
            return LoadSTLAscii(reinterpret_cast<const char*>(data.get()), mesh);
        }
        else {
            // Binary STL
            return LoadSTLBinary(data.get(), fileSize, mesh);
        }
    }

    bool ModelDecoder::LoadSTLBinary(const uint8_t* data, size_t size, MeshData& mesh)
    {
        if (size < 84) return false;

        // Binary STL format:
        // 80 bytes: header
        // 4 bytes: number of triangles (uint32_t)
        // For each triangle (50 bytes):
        //   12 bytes: normal vector (3 floats)
        //   12 bytes: vertex 1 (3 floats)
        //   12 bytes: vertex 2 (3 floats)
        //   12 bytes: vertex 3 (3 floats)
        //   2 bytes: attribute byte count

        const uint8_t* ptr = data + 80;
        uint32_t triangleCount = *reinterpret_cast<const uint32_t*>(ptr);
        ptr += 4;

        if (size < 84 + static_cast<size_t>(triangleCount) * 50) return false;

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

            ptr += 2; // Skip attribute bytes
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
                lineStream >> word; // "normal"
                lineStream >> currentNormal.x >> currentNormal.y >> currentNormal.z;
                currentTriangle.clear();
            }
            else if (word == "vertex") {
                Vertex v;
                lineStream >> v.position.x >> v.position.y >> v.position.z;
                v.normal = currentNormal;
                v.texcoord = DirectX::XMFLOAT2(0, 0);
                currentTriangle.push_back(v);
            }
            else if (word == "endfacet") {
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
        return false;
    }

    HBITMAP ModelDecoder::RenderMeshPreview(const MeshData& mesh, uint32_t width, uint32_t height)
    {
        // For now, create a simple placeholder bitmap with wireframe-style rendering
        // Production version would use D3D11 to render actual 3D preview

        HDC hdc = GetDC(NULL);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // Fill background
        RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        HBRUSH bgBrush = CreateSolidBrush(RGB(45, 45, 48)); // Dark gray background
        FillRect(memDC, &rect, bgBrush);
        DeleteObject(bgBrush);

        // Draw simple wireframe representation
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(100, 180, 255)); // Light blue
        HPEN oldPen = (HPEN)SelectObject(memDC, pen);

        // Calculate projection (very basic orthographic)
        float scaleX = width / (mesh.boundsMax.x - mesh.boundsMin.x);
        float scaleY = height / (mesh.boundsMax.y - mesh.boundsMin.y);
        float scale = min(scaleX, scaleY) * 0.8f; // 80% of available space

        float centerX = (mesh.boundsMin.x + mesh.boundsMax.x) / 2.0f;
        float centerY = (mesh.boundsMin.y + mesh.boundsMax.y) / 2.0f;

        // Draw triangles
        for (size_t i = 0; i + 2 < mesh.vertices.size(); i += 3) {
            for (int j = 0; j < 3; ++j) {
                const auto& v1 = mesh.vertices[i + j];
                const auto& v2 = mesh.vertices[i + ((j + 1) % 3)];

                int x1 = static_cast<int>((v1.position.x - centerX) * scale + width / 2);
                int y1 = static_cast<int>(height / 2 - (v1.position.y - centerY) * scale);
                int x2 = static_cast<int>((v2.position.x - centerX) * scale + width / 2);
                int y2 = static_cast<int>(height / 2 - (v2.position.y - centerY) * scale);

                MoveToEx(memDC, x1, y1, NULL);
                LineTo(memDC, x2, y2);
            }
        }

        SelectObject(memDC, oldPen);
        DeleteObject(pen);

        SelectObject(memDC, hOldBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, hdc);

        return hBitmap;
    }

    void ModelDecoder::CalculateBounds(MeshData& mesh)
    {
        if (mesh.vertices.empty()) return;

        mesh.boundsMin = mesh.vertices[0].position;
        mesh.boundsMax = mesh.vertices[0].position;

        for (const auto& v : mesh.vertices) {
            mesh.boundsMin.x = min(mesh.boundsMin.x, v.position.x);
            mesh.boundsMin.y = min(mesh.boundsMin.y, v.position.y);
            mesh.boundsMin.z = min(mesh.boundsMin.z, v.position.z);

            mesh.boundsMax.x = max(mesh.boundsMax.x, v.position.x);
            mesh.boundsMax.y = max(mesh.boundsMax.y, v.position.y);
            mesh.boundsMax.z = max(mesh.boundsMax.z, v.position.z);
        }
    }

    DirectX::XMMATRIX ModelDecoder::CalculateViewMatrix(const MeshData& mesh)
    {
        DirectX::XMFLOAT3 center(
            (mesh.boundsMin.x + mesh.boundsMax.x) / 2.0f,
            (mesh.boundsMin.y + mesh.boundsMax.y) / 2.0f,
            (mesh.boundsMin.z + mesh.boundsMax.z) / 2.0f
        );

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
        if (m_d3dInitialized) return true;

        // Initialize D3D11 device for GPU rendering
        D3D_FEATURE_LEVEL featureLevel;
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &m_device,
            &featureLevel,
            &m_context
        );

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

} // namespace Engine
} // namespace DarkThumbs
