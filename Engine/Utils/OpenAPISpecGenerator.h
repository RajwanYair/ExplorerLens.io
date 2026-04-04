// OpenAPISpecGenerator.h — OpenAPI 3.1 Specification Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates OpenAPI 3.1 YAML/JSON specifications from registered REST routes,
// enabling automatic SDK generation and interactive API documentation.
//
#pragma once
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class OpenAPISchemaType {
    String,
    Integer,
    Boolean,
    Array,
    Object
};
enum class OpenAPIOutputFormat {
    YAML,
    JSON
};

struct OpenAPIParameter
{
    std::string name;
    std::string location;  // "query", "path", "header"
    bool required = false;
    OpenAPISchemaType type = OpenAPISchemaType::String;
    std::string description;
};

struct OpenAPIEndpoint
{
    std::string method;  // GET, POST…
    std::string path;
    std::string summary;
    std::vector<OpenAPIParameter> parameters;
    std::string requestBodySchema;
    std::string responseSchema;
    std::vector<std::string> tags;
};

struct OpenAPISpec
{
    std::string title = "ExplorerLens API";
    std::string version = "1.0.0";
    std::vector<OpenAPIEndpoint> endpoints;
};

class OpenAPISpecGenerator
{
  public:
    explicit OpenAPISpecGenerator() = default;

    void AddEndpoint(OpenAPIEndpoint ep)
    {
        m_spec.endpoints.push_back(std::move(ep));
    }

    void SetTitle(const std::string& title)
    {
        m_spec.title = title;
    }
    void SetVersion(const std::string& version)
    {
        m_spec.version = version;
    }

    std::string Generate(OpenAPIOutputFormat fmt = OpenAPIOutputFormat::YAML) const
    {
        return fmt == OpenAPIOutputFormat::YAML ? GenerateYAML() : GenerateJSON();
    }

    const OpenAPISpec& Spec() const noexcept
    {
        return m_spec;
    }
    size_t EndpointCount() const noexcept
    {
        return m_spec.endpoints.size();
    }

  private:
    std::string GenerateYAML() const
    {
        std::ostringstream oss;
        oss << "openapi: \"3.1.0\"\ninfo:\n  title: \"" << m_spec.title << "\"\n  version: \"" << m_spec.version
            << "\"\npaths:\n";
        for (const auto& ep : m_spec.endpoints) {
            oss << "  " << ep.path << ":\n    " << ep.method << ":\n"
                << "      summary: \"" << ep.summary << "\"\n";
        }
        return oss.str();
    }
    std::string GenerateJSON() const
    {
        std::ostringstream oss;
        oss << "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"" << m_spec.title << "\",\"version\":\"" << m_spec.version
            << "\"},\"paths\":{";
        bool first = true;
        for (const auto& ep : m_spec.endpoints) {
            if (!first)
                oss << ",";
            oss << "\"" << ep.path << "\":{\"" << ep.method << "\":{\"summary\":\"" << ep.summary << "\"}}";
            first = false;
        }
        oss << "}}";
        return oss.str();
    }

    OpenAPISpec m_spec;
};

}  // namespace Engine
}  // namespace ExplorerLens
