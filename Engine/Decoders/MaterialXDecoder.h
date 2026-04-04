// MaterialXDecoder.h — MaterialX Shader Material Preview
// Copyright (c) 2026 ExplorerLens Project
//
// MaterialX shader material preview decoder. Parses MTLX XML, extracts material
// properties, and generates color swatch thumbnails.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct MaterialColor
{
    float r = 0.5f, g = 0.5f, b = 0.5f, a = 1.0f;

    inline uint32_t ToRGBA8() const
    {
        auto toByte = [](float v) -> uint8_t {
            return static_cast<uint8_t>((std::max)(0.0f, (std::min)(1.0f, v)) * 255.0f + 0.5f);
        };
        return (static_cast<uint32_t>(toByte(r))) | (static_cast<uint32_t>(toByte(g)) << 8)
               | (static_cast<uint32_t>(toByte(b)) << 16) | (static_cast<uint32_t>(toByte(a)) << 24);
    }
};

struct MaterialProperty
{
    std::string name;
    std::string type;
    MaterialColor color;
    float scalarValue = 0.0f;
    std::string texturePath;
};

struct MaterialXInfo
{
    std::string materialName;
    std::string shaderModel;
    std::vector<MaterialProperty> properties;
    MaterialColor baseColor;
    float roughness = 0.5f;
    float metallic = 0.0f;
    float specular = 0.5f;
    bool isValid = false;
};

class MaterialXDecoder
{
  public:
    static MaterialXDecoder& Instance()
    {
        static MaterialXDecoder instance;
        return instance;
    }

    inline bool IsMaterialXFile(const uint8_t* data, size_t size) const
    {
        if (!data || size < 20)
            return false;
        std::string header(reinterpret_cast<const char*>(data), (std::min)(size, static_cast<size_t>(512)));
        return header.find("<?xml") != std::string::npos && header.find("materialx") != std::string::npos;
    }

    inline MaterialXInfo ParseMaterial(const uint8_t* data, size_t size) const
    {
        MaterialXInfo info;
        if (!data || size < 20)
            return info;

        std::string content(reinterpret_cast<const char*>(data), size);
        info.isValid = IsMaterialXFile(data, size);
        if (!info.isValid)
            return info;

        info.materialName = ExtractAttribute(content, "name=");
        info.shaderModel = ExtractAttribute(content, "shadingmodel=");
        if (info.shaderModel.empty())
            info.shaderModel = "standard_surface";

        auto baseColorStr = ExtractAttribute(content, "base_color=");
        if (!baseColorStr.empty()) {
            info.baseColor = ParseColorString(baseColorStr);
        }

        auto roughStr = ExtractAttribute(content, "roughness=");
        if (!roughStr.empty())
            info.roughness = ParseFloat(roughStr, 0.5f);

        auto metalStr = ExtractAttribute(content, "metallic=");
        if (!metalStr.empty())
            info.metallic = ParseFloat(metalStr, 0.0f);

        return info;
    }

    inline std::vector<uint8_t> GenerateSwatchThumbnail(const MaterialXInfo& material, uint32_t width,
                                                        uint32_t height) const
    {
        std::vector<uint8_t> thumbnail(static_cast<size_t>(width) * height * 3, 0);
        if (width == 0 || height == 0)
            return thumbnail;

        float cx = width / 2.0f, cy = height / 2.0f;
        float radius = (std::min)(width, height) * 0.4f;

        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                float dx = x - cx, dy = y - cy;
                float dist = std::sqrt(dx * dx + dy * dy);
                size_t idx = (static_cast<size_t>(y) * width + x) * 3;

                if (dist <= radius) {
                    float nx = dx / radius;
                    float ny = dy / radius;
                    float nz = std::sqrt((std::max)(0.0f, 1.0f - nx * nx - ny * ny));

                    float lightX = 0.577f, lightY = -0.577f, lightZ = 0.577f;
                    float NdotL = (std::max)(0.0f, nx * lightX + ny * lightY + nz * lightZ);

                    float halfX = lightX, halfY = lightY, halfZ = lightZ + 1.0f;
                    float halfLen = std::sqrt(halfX * halfX + halfY * halfY + halfZ * halfZ);
                    if (halfLen > 0.0f) {
                        halfX /= halfLen;
                        halfY /= halfLen;
                        halfZ /= halfLen;
                    }
                    float NdotH = (std::max)(0.0f, nx * halfX + ny * halfY + nz * halfZ);

                    float specPow = (1.0f - material.roughness) * 128.0f + 2.0f;
                    float spec = std::pow(NdotH, specPow) * material.specular;

                    float fresnel = std::pow(1.0f - (std::max)(0.0f, nz), 5.0f);
                    float metalBlend = material.metallic * fresnel;

                    float ambient = 0.15f;
                    float r = material.baseColor.r * (ambient + NdotL * (1.0f - metalBlend)) + spec;
                    float g = material.baseColor.g * (ambient + NdotL * (1.0f - metalBlend)) + spec;
                    float b = material.baseColor.b * (ambient + NdotL * (1.0f - metalBlend)) + spec;

                    auto gammaCorrect = [](float v) -> uint8_t {
                        v = (std::max)(0.0f, (std::min)(1.0f, v));
                        return static_cast<uint8_t>(std::pow(v, 1.0f / 2.2f) * 255.0f + 0.5f);
                    };
                    thumbnail[idx + 0] = gammaCorrect(r);
                    thumbnail[idx + 1] = gammaCorrect(g);
                    thumbnail[idx + 2] = gammaCorrect(b);
                } else {
                    uint8_t checker = ((x / 8 + y / 8) % 2 == 0) ? 40 : 50;
                    thumbnail[idx + 0] = checker;
                    thumbnail[idx + 1] = checker;
                    thumbnail[idx + 2] = checker;
                }
            }
        }
        return thumbnail;
    }

    inline std::string FormatMaterialInfo(const MaterialXInfo& info) const
    {
        return info.materialName + " (" + info.shaderModel + ") R=" + std::to_string(info.roughness).substr(0, 4)
               + " M=" + std::to_string(info.metallic).substr(0, 4);
    }

  private:
    MaterialXDecoder() = default;

    inline std::string ExtractAttribute(const std::string& xml, const std::string& attrName) const
    {
        size_t pos = xml.find(attrName);
        if (pos == std::string::npos)
            return "";
        pos += attrName.size();
        if (pos < xml.size() && xml[pos] == '"')
            ++pos;
        size_t end = xml.find('"', pos);
        if (end == std::string::npos)
            return "";
        return xml.substr(pos, end - pos);
    }

    inline MaterialColor ParseColorString(const std::string& str) const
    {
        MaterialColor color;
        size_t idx = 0;
        float values[4] = {0.5f, 0.5f, 0.5f, 1.0f};
        int vi = 0;

        for (size_t i = 0; i <= str.size() && vi < 4; ++i) {
            if (i == str.size() || str[i] == ',' || str[i] == ' ') {
                if (i > idx) {
                    values[vi++] = ParseFloat(str.substr(idx, i - idx), 0.5f);
                }
                idx = i + 1;
            }
        }
        color.r = values[0];
        color.g = values[1];
        color.b = values[2];
        color.a = values[3];
        return color;
    }

    inline float ParseFloat(const std::string& str, float defaultVal) const
    {
        try {
            size_t pos = 0;
            for (; pos < str.size(); ++pos) {
                char c = str[pos];
                if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+')
                    break;
            }
            if (pos >= str.size())
                return defaultVal;
            float result = 0.0f;
            bool negative = false;
            if (str[pos] == '-') {
                negative = true;
                ++pos;
            } else if (str[pos] == '+') {
                ++pos;
            }
            bool hasDot = false;
            float divisor = 1.0f;
            for (; pos < str.size(); ++pos) {
                char c = str[pos];
                if (c >= '0' && c <= '9') {
                    if (hasDot) {
                        divisor *= 10.0f;
                        result += (c - '0') / divisor;
                    } else {
                        result = result * 10.0f + (c - '0');
                    }
                } else if (c == '.' && !hasDot) {
                    hasDot = true;
                } else {
                    break;
                }
            }
            return negative ? -result : result;
        } catch (...) {
            return defaultVal;
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
