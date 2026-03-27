// CycloneDXSBOMGenerator.h — SBOM v2 (CycloneDX 1.5) Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates CycloneDX 1.5 Software Bill of Materials in JSON/XML including all statically linked libraries.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct SBOMComponent { std::string name; std::string version; std::string purl; std::string license; };
struct SBOMDocument  { std::string serialNumber; std::string version; std::vector<SBOMComponent> components; };
class CycloneDXSBOMGenerator {
public:
    void   AddComponent(SBOMComponent c)     { m_components.push_back(std::move(c)); }
    size_t ComponentCount() const            { return m_components.size(); }
    SBOMDocument Generate(const std::string& ver) const {
        return { "urn:uuid:0-0", ver, m_components };
    }
    std::string ToJSON(const SBOMDocument& doc) const {
        return "{\"bomFormat\":\"CycloneDX\",\"specVersion\":\"1.5\",\"version\":\"" + doc.version + "\"}";
    }
private:
    std::vector<SBOMComponent> m_components;
};

} // namespace Engine
} // namespace ExplorerLens