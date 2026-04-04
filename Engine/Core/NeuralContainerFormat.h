// NeuralContainerFormat.h — Neural Container Format
// Copyright (c) 2026 ExplorerLens Project
//
// Binary container format (.ncf) for neural-compressed thumbnail streams with metadata.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct NCFMetadata
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t codecVersion = 2;
    std::string model;
    float quality = 0.0f;
};

struct NCFContainer
{
    NCFMetadata metadata;
    std::vector<uint8_t> payload;
};

class NeuralContainerFormat
{
  public:
    static std::vector<uint8_t> Serialize(const NCFContainer& container)
    {
        // 16-byte header + payload
        std::vector<uint8_t> out;
        out.push_back(static_cast<uint8_t>(container.metadata.width >> 8));
        out.push_back(static_cast<uint8_t>(container.metadata.width & 0xFF));
        out.push_back(static_cast<uint8_t>(container.metadata.height >> 8));
        out.push_back(static_cast<uint8_t>(container.metadata.height & 0xFF));
        out.insert(out.end(), container.payload.begin(), container.payload.end());
        return out;
    }
    static bool Deserialize(const std::vector<uint8_t>& data, NCFContainer& out)
    {
        if (data.size() < 4)
            return false;
        out.metadata.width = (static_cast<uint32_t>(data[0]) << 8) | data[1];
        out.metadata.height = (static_cast<uint32_t>(data[2]) << 8) | data[3];
        out.payload.assign(data.begin() + 4, data.end());
        return true;
    }
    static std::string Extension()
    {
        return ".ncf";
    }
    static bool IsNCFData(const std::vector<uint8_t>& data)
    {
        return data.size() >= 4;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
