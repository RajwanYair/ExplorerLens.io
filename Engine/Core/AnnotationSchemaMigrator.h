// AnnotationSchemaMigrator.h — Annotation Schema Versioning & Migration Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Manages forward and backward migration of annotation schemas between versions,
// ensuring safe upgrades and compatibility with older annotation data files.
//
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

using SchemaVersion = int;

struct SchemaMigrationStep
{
    SchemaVersion fromVersion = 0;
    SchemaVersion toVersion = 0;
    std::string description;
    // Migration function: takes JSON-like string, returns migrated string
    std::function<std::string(const std::string&)> migrate;
};

struct SchemaMigrationResult
{
    bool success = false;
    SchemaVersion finalVersion = 0;
    int stepsApplied = 0;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

class AnnotationSchemaMigrator
{
  public:
    static constexpr SchemaVersion CURRENT_SCHEMA_VERSION = 4;

    AnnotationSchemaMigrator()
    {
        // Register built-in migrations v1→v2, v2→v3, v3→v4
        RegisterStep({1, 2, "Add colorLabel field", [](const std::string& s) {
                          // Inject colorLabel:0 if absent
                          if (s.find("colorLabel") == std::string::npos) {
                              auto pos = s.rfind('}');
                              if (pos != std::string::npos)
                                  return s.substr(0, pos) + ",\"colorLabel\":0}";
                          }
                          return s;
                      }});
        RegisterStep({2, 3, "Add author field", [](const std::string& s) {
                          if (s.find("author") == std::string::npos) {
                              auto pos = s.rfind('}');
                              if (pos != std::string::npos)
                                  return s.substr(0, pos) + ",\"author\":\"\"}";
                          }
                          return s;
                      }});
        RegisterStep({3, 4, "Add schemaVersion field", [](const std::string& s) {
                          if (s.find("schemaVersion") == std::string::npos) {
                              auto pos = s.rfind('}');
                              if (pos != std::string::npos)
                                  return s.substr(0, pos) + ",\"schemaVersion\":4}";
                          }
                          return s;
                      }});
    }

    void RegisterStep(SchemaMigrationStep step)
    {
        m_steps[step.fromVersion] = std::move(step);
    }

    SchemaMigrationResult Migrate(const std::string& jsonData, SchemaVersion fromVersion,
                                  SchemaVersion toVersion = CURRENT_SCHEMA_VERSION) const
    {
        if (fromVersion >= toVersion)
            return {true, fromVersion, 0, {}};

        std::string current = jsonData;
        int stepsApplied = 0;
        SchemaVersion v = fromVersion;

        while (v < toVersion) {
            auto it = m_steps.find(v);
            if (it == m_steps.end()) {
                std::string err = "No migration step from v" + std::to_string(v);
                return {false, v, stepsApplied, std::move(err)};
            }
            if (it->second.migrate)
                current = it->second.migrate(current);
            v = it->second.toVersion;
            stepsApplied++;
        }
        return {true, v, stepsApplied, {}};
    }

    bool CanMigrate(SchemaVersion from, SchemaVersion to = CURRENT_SCHEMA_VERSION) const noexcept
    {
        SchemaVersion v = from;
        while (v < to) {
            auto it = m_steps.find(v);
            if (it == m_steps.end())
                return false;
            v = it->second.toVersion;
        }
        return true;
    }

    static SchemaVersion CurrentVersion() noexcept
    {
        return CURRENT_SCHEMA_VERSION;
    }

  private:
    std::unordered_map<SchemaVersion, SchemaMigrationStep> m_steps;
};

}  // namespace Engine
}  // namespace ExplorerLens
