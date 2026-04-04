// GraphQLSchemaIntrospector.h — GraphQL Schema Introspector
// Copyright (c) 2026 ExplorerLens Project
//
// Introspects a GraphQL schema to enumerate types, fields, and directives for tooling.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct GQLTypeInfo
{
    std::string name;
    std::string kind;  // OBJECT, SCALAR, ENUM, etc.
    std::vector<std::string> fields;
};

struct GQLIntrospectResult
{
    bool success = false;
    std::vector<GQLTypeInfo> types;
    std::string errorMsg;
};

class GraphQLSchemaIntrospector
{
  public:
    void RegisterType(const GQLTypeInfo& type)
    {
        m_types[type.name] = type;
    }

    GQLIntrospectResult Introspect() const
    {
        GQLIntrospectResult r;
        for (const auto& [name, t] : m_types)
            r.types.push_back(t);
        r.success = true;
        return r;
    }
    bool TypeExists(const std::string& name) const
    {
        return m_types.count(name) > 0;
    }
    uint32_t TypeCount() const
    {
        return static_cast<uint32_t>(m_types.size());
    }

  private:
    std::unordered_map<std::string, GQLTypeInfo> m_types;
};

}  // namespace Engine
}  // namespace ExplorerLens
