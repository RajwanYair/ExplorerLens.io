// ShellPropertyHandlerV2.h — Shell Property Handler V2
// Copyright (c) 2026 ExplorerLens Project
//
// Exposes ExplorerLens-decoded metadata as Windows Shell properties
// (IPropertyStore). Provides format, dimensions, color space, and AI caption.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct ShellProperty
{
    std::string key;
    std::string value;
    std::string type;  // "string","uint32","bool"
};

struct ShellHandlerPropertySet
{
    std::string filePath;
    std::unordered_map<std::string, ShellProperty> props;
};

class ShellPropertyHandlerV2
{
  public:
    ShellPropertyHandlerV2() = default;

    bool Initialize()
    {
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    ShellHandlerPropertySet BuildPropertySet(const std::string& filePath, uint32_t width, uint32_t height,
                                             const std::string& format, const std::string& colorSpace = "sRGB",
                                             const std::string& aiCaption = "") const
    {
        ShellHandlerPropertySet ps;
        ps.filePath = filePath;

        auto add = [&](const std::string& key, const std::string& val, const std::string& type) {
            ShellProperty p;
            p.key = key;
            p.value = val;
            p.type = type;
            ps.props[key] = p;
        };

        add("System.Image.HorizontalSize", std::to_string(width), "uint32");
        add("System.Image.VerticalSize", std::to_string(height), "uint32");
        add("System.Image.Dimensions", std::to_string(width) + " x " + std::to_string(height), "string");
        add("System.Kind", "picture", "string");
        add("ExplorerLens.Format", format, "string");
        add("ExplorerLens.ColorSpace", colorSpace, "string");
        if (!aiCaption.empty())
            add("ExplorerLens.AICaption", aiCaption, "string");
        return ps;
    }

    bool GetProperty(const ShellHandlerPropertySet& ps, const std::string& key, ShellProperty& out) const
    {
        auto it = ps.props.find(key);
        if (it == ps.props.end())
            return false;
        out = it->second;
        return true;
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
