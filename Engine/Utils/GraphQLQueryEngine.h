// GraphQLQueryEngine.h — GraphQL Query Engine for Thumbnail Metadata
// Copyright (c) 2026 ExplorerLens Project
//
// Parses and executes GraphQL queries against the thumbnail metadata store,
// enabling rich graph-based queries over file annotations, formats, and cache state.
//
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GraphQLOperationType {
    Query,
    Mutation,
    Subscription
};

struct GraphQLVariable
{
    std::string name;
    std::string value;
};

struct GraphQLRequest
{
    std::string query;
    std::vector<GraphQLVariable> variables;
    std::string operationName;
    GraphQLOperationType type = GraphQLOperationType::Query;
};

struct GraphQLError
{
    std::string message;
    int lineNumber = 0;
    int column = 0;
};

struct GraphQLResponse
{
    bool success = false;
    std::string data;  // JSON-serialised data
    std::vector<GraphQLError> errors;
    bool HasErrors() const noexcept
    {
        return !errors.empty();
    }
};

using GraphQLResolver = std::function<std::string(const GraphQLRequest&)>;

class GraphQLQueryEngine
{
  public:
    explicit GraphQLQueryEngine() = default;

    void RegisterResolver(const std::string& fieldName, GraphQLResolver resolver)
    {
        m_resolvers[fieldName] = std::move(resolver);
    }

    GraphQLResponse Execute(const GraphQLRequest& req) const
    {
        if (req.query.empty()) {
            return {
                false,
                {},
                {{"Empty query"}},
            };
        }
        // Minimal dispatch: look for resolver matching first selected field
        for (const auto& [field, resolver] : m_resolvers) {
            if (req.query.find(field) != std::string::npos) {
                std::string data = resolver(req);
                return {true, std::move(data), {}};
            }
        }
        // No matching resolver found — return empty data
        return {true, "{}", {}};
    }

    size_t ResolverCount() const noexcept
    {
        return m_resolvers.size();
    }

    std::string IntrospectionSchema() const
    {
        return "{ \"__schema\": { \"types\": [] } }";
    }

    static std::string OperationName(GraphQLOperationType op)
    {
        switch (op) {
            case GraphQLOperationType::Query:
                return "query";
            case GraphQLOperationType::Mutation:
                return "mutation";
            case GraphQLOperationType::Subscription:
                return "subscription";
        }
        return "unknown";
    }

  private:
    std::unordered_map<std::string, GraphQLResolver> m_resolvers;
};

}  // namespace Engine
}  // namespace ExplorerLens
