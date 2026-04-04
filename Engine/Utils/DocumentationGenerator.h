#pragma once
/**
 * @file DocumentationGenerator.h
 * @brief Auto-generates API documentation from registered components for SDK/plugin developers.
 * @version 15.0.0
 * @date 2026-03-02
 *
 * Provides a documentation registry where API endpoints and classes are described,
 * then generates Markdown, HTML, and C SDK header outputs suitable for plugin
 * developers. Ships with pre-registered core APIs (IThumbnailPlugin, ICADDecoderPlugin,
 * Plugin lifecycle functions).
 *
 * @note Header-only. Uses Windows API + C++20 standard library only.
 *
 * @copyright (c) 2026 ExplorerLens Contributors. All rights reserved.
 */

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// @brief Generates API documentation for ExplorerLens SDK/plugin developers.
class APIDocumentationGenerator
{
  public:
    /// @brief Describes a single API method / endpoint.
    struct APIEndpoint
    {
        std::string name;
        std::string description;
        std::string returnType;
        std::vector<std::pair<std::string, std::string>> parameters;  ///< (type, name)
        std::string example;
        std::string category;
    };

    APIDocumentationGenerator() noexcept
    {
        InitializeSRWLock(&m_lock);
        RegisterCoreAPIs();
    }

    ~APIDocumentationGenerator() = default;

    APIDocumentationGenerator(const APIDocumentationGenerator&) = delete;
    APIDocumentationGenerator& operator=(const APIDocumentationGenerator&) = delete;

    /// @brief Register a single API endpoint.
    inline void RegisterEndpoint(APIEndpoint&& endpoint)
    {
        AcquireSRWLockExclusive(&m_lock);
        std::string cat = endpoint.category;
        m_endpoints.emplace_back(std::move(endpoint));
        if (std::find(m_categories.begin(), m_categories.end(), cat) == m_categories.end()) {
            m_categories.push_back(cat);
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// @brief Register a class with all its methods.
    inline void RegisterClass(const std::string& className, const std::string& description,
                              const std::vector<APIEndpoint>& methods)
    {
        AcquireSRWLockExclusive(&m_lock);
        ClassInfo ci;
        ci.name = className;
        ci.description = description;
        ci.methods = methods;
        m_classes.emplace_back(std::move(ci));

        for (auto& m : methods) {
            m_endpoints.push_back(m);
            if (std::find(m_categories.begin(), m_categories.end(), m.category) == m_categories.end()) {
                m_categories.push_back(m.category);
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// @brief Return all unique categories.
    inline std::vector<std::string> GetCategories() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto copy = m_categories;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return copy;
    }

    /// @brief Return endpoints belonging to @p category.
    inline std::vector<APIEndpoint> GetEndpointsByCategory(const std::string& category) const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        std::vector<APIEndpoint> result;
        for (auto& ep : m_endpoints) {
            if (ep.category == category) {
                result.push_back(ep);
            }
        }
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return result;
    }

    /// @brief Total registered endpoints.
    inline uint32_t GetTotalEndpoints() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        uint32_t n = static_cast<uint32_t>(m_endpoints.size());
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return n;
    }

    // -----------------------------------------------------------------
    //  Markdown generation
    // -----------------------------------------------------------------

    /// @brief Produce a full Markdown documentation string with TOC.
    inline std::string GenerateMarkdown() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto classes = m_classes;
        auto categories = m_categories;
        auto endpoints = m_endpoints;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        std::ostringstream os;
        os << "# ExplorerLens Plugin SDK — API Reference\n\n"
           << "**Version:** 15.4.0 (Zenith-U)  \n"
           << "**Generated:** " << GetTimestamp() << "\n\n";

        // Table of Contents
        os << "## Table of Contents\n\n";
        for (auto& cat : categories) {
            os << "- [" << cat << "](#" << AnchorId(cat) << ")\n";
        }
        for (auto& cls : classes) {
            os << "- [" << cls.name << "](#" << AnchorId(cls.name) << ")\n";
        }
        os << "\n---\n\n";

        // Categories
        for (auto& cat : categories) {
            os << "## " << cat << "\n\n";
            for (auto& ep : endpoints) {
                if (ep.category != cat)
                    continue;
                os << "### `" << ep.returnType << " " << ep.name << "(";
                for (size_t i = 0; i < ep.parameters.size(); ++i) {
                    if (i > 0)
                        os << ", ";
                    os << ep.parameters[i].first << " " << ep.parameters[i].second;
                }
                os << ")`\n\n";
                os << ep.description << "\n\n";

                if (!ep.parameters.empty()) {
                    os << "| Parameter | Type | Description |\n"
                       << "|-----------|------|-------------|\n";
                    for (auto& [type, name] : ep.parameters) {
                        os << "| `" << name << "` | `" << type << "` | — |\n";
                    }
                    os << "\n";
                }

                if (!ep.example.empty()) {
                    os << "**Example:**\n```c\n" << ep.example << "\n```\n\n";
                }
            }
        }

        // Classes
        for (auto& cls : classes) {
            os << "## " << cls.name << "\n\n" << cls.description << "\n\n";
            for (auto& m : cls.methods) {
                os << "#### `" << m.returnType << " " << m.name << "(...)`\n\n" << m.description << "\n\n";
            }
        }

        return os.str();
    }

    // -----------------------------------------------------------------
    //  HTML generation
    // -----------------------------------------------------------------

    /// @brief Produce a standalone HTML documentation page.
    inline std::string GenerateHTML() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto classes = m_classes;
        auto categories = m_categories;
        auto endpoints = m_endpoints;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        std::ostringstream os;
        os << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
           << "  <meta charset=\"UTF-8\">\n"
           << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
           << "  <title>ExplorerLens SDK API Reference</title>\n"
           << "  <style>\n"
           << "    body { font-family: 'Segoe UI', Tahoma, sans-serif; margin: 0; display: flex; }\n"
           << "    nav { width: 260px; background: #1e1e2e; color: #cdd6f4; padding: 20px;\n"
           << "          position: fixed; height: 100vh; overflow-y: auto; }\n"
           << "    nav a { color: #89b4fa; text-decoration: none; display: block; padding: 4px 0; }\n"
           << "    nav a:hover { color: #f5c2e7; }\n"
           << "    main { margin-left: 300px; padding: 30px; max-width: 900px; }\n"
           << "    h1 { color: #1e1e2e; border-bottom: 3px solid #89b4fa; padding-bottom: 8px; }\n"
           << "    h2 { color: #313244; margin-top: 40px; }\n"
           << "    h3 { color: #45475a; }\n"
           << "    code { background: #f5f5f5; padding: 2px 6px; border-radius: 4px; font-size: 0.9em; }\n"
           << "    pre { background: #1e1e2e; color: #cdd6f4; padding: 16px; border-radius: 8px;\n"
           << "          overflow-x: auto; }\n"
           << "    table { border-collapse: collapse; width: 100%; margin: 12px 0; }\n"
           << "    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n"
           << "    th { background: #89b4fa; color: white; }\n"
           << "  </style>\n"
           << "</head>\n<body>\n"
           << "<nav>\n"
           << "  <h2>Navigation</h2>\n";

        for (auto& cat : categories) {
            os << "  <a href=\"#" << AnchorId(cat) << "\">" << EscapeHtml(cat) << "</a>\n";
        }
        for (auto& cls : classes) {
            os << "  <a href=\"#" << AnchorId(cls.name) << "\">" << EscapeHtml(cls.name) << "</a>\n";
        }
        os << "</nav>\n<main>\n"
           << "<h1>ExplorerLens Plugin SDK — API Reference</h1>\n"
           << "<p><strong>Version:</strong> 15.4.0 (Zenith-U) | "
           << "<strong>Generated:</strong> " << GetTimestamp() << "</p>\n";

        // Categories
        for (auto& cat : categories) {
            os << "<h2 id=\"" << AnchorId(cat) << "\">" << EscapeHtml(cat) << "</h2>\n";
            for (auto& ep : endpoints) {
                if (ep.category != cat)
                    continue;
                os << "<h3><code>" << EscapeHtml(ep.returnType) << " " << EscapeHtml(ep.name) << "(";
                for (size_t i = 0; i < ep.parameters.size(); ++i) {
                    if (i > 0)
                        os << ", ";
                    os << EscapeHtml(ep.parameters[i].first) << " " << EscapeHtml(ep.parameters[i].second);
                }
                os << ")</code></h3>\n"
                   << "<p>" << EscapeHtml(ep.description) << "</p>\n";

                if (!ep.parameters.empty()) {
                    os << "<table><tr><th>Parameter</th><th>Type</th></tr>\n";
                    for (auto& [type, name] : ep.parameters) {
                        os << "<tr><td>" << EscapeHtml(name) << "</td><td><code>" << EscapeHtml(type)
                           << "</code></td></tr>\n";
                    }
                    os << "</table>\n";
                }

                if (!ep.example.empty()) {
                    os << "<pre><code>" << EscapeHtml(ep.example) << "</code></pre>\n";
                }
            }
        }

        // Classes
        for (auto& cls : classes) {
            os << "<h2 id=\"" << AnchorId(cls.name) << "\">" << EscapeHtml(cls.name) << "</h2>\n"
               << "<p>" << EscapeHtml(cls.description) << "</p>\n";
            for (auto& m : cls.methods) {
                os << "<h3><code>" << EscapeHtml(m.returnType) << " " << EscapeHtml(m.name) << "(...)</code></h3>\n"
                   << "<p>" << EscapeHtml(m.description) << "</p>\n";
            }
        }

        os << "</main>\n</body>\n</html>\n";
        return os.str();
    }

    // -----------------------------------------------------------------
    //  SDK header generation
    // -----------------------------------------------------------------

    /// @brief Generate a C header file summarizing all plugin APIs.
    inline std::string GenerateSDKHeader() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto endpoints = m_endpoints;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        std::ostringstream os;
        os << "/**\n"
           << " * @file explorerlenssdk_generated.h\n"
           << " * @brief Auto-generated ExplorerLens Plugin SDK declarations.\n"
           << " * @version 15.0.0\n"
           << " * @date " << GetTimestamp() << "\n"
           << " *\n"
           << " * DO NOT EDIT — generated by DocumentationGenerator.\n"
           << " */\n\n"
           << "#ifndef EXPLORERLENSSDK_GENERATED_H\n"
           << "#define EXPLORERLENSSDK_GENERATED_H\n\n"
           << "#include <stdint.h>\n"
           << "#include <stdbool.h>\n\n"
           << "#ifdef __cplusplus\n"
           << "extern \"C\" {\n"
           << "#endif\n\n";

        os << "/* ---- Plugin ABI version ---- */\n"
           << "#define EXPLORERLENSSDK_ABI_VERSION 2\n\n";

        // Group by category
        std::string lastCat;
        for (auto& ep : endpoints) {
            if (ep.category != lastCat) {
                if (!lastCat.empty())
                    os << "\n";
                os << "/* ---- " << ep.category << " ---- */\n\n";
                lastCat = ep.category;
            }
            os << "/**\n * @brief " << ep.description << "\n */\n";
            os << ep.returnType << " " << ep.name << "(";
            for (size_t i = 0; i < ep.parameters.size(); ++i) {
                if (i > 0)
                    os << ", ";
                os << ep.parameters[i].first << " " << ep.parameters[i].second;
            }
            os << ");\n\n";
        }

        os << "#ifdef __cplusplus\n}\n#endif\n\n"
           << "#endif /* EXPLORERLENSSDK_GENERATED_H */\n";
        return os.str();
    }

    // -----------------------------------------------------------------
    //  File I/O
    // -----------------------------------------------------------------

    /// @brief Save documentation to file ("markdown" or "html").
    inline bool SaveDocumentation(const std::wstring& path, const std::string& format = "markdown") const
    {
        std::string content;
        if (format == "html") {
            content = GenerateHTML();
        } else {
            content = GenerateMarkdown();
        }

        HANDLE hFile =
            CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return false;

        DWORD written = 0;
        BOOL ok = WriteFile(hFile, content.data(), static_cast<DWORD>(content.size()), &written, nullptr);
        CloseHandle(hFile);
        return ok != FALSE;
    }

  private:
    struct ClassInfo
    {
        std::string name;
        std::string description;
        std::vector<APIEndpoint> methods;
    };

    SRWLOCK m_lock{};
    std::vector<APIEndpoint> m_endpoints;
    std::vector<ClassInfo> m_classes;
    std::vector<std::string> m_categories;

    /// Register the built-in ExplorerLens SDK APIs.
    inline void RegisterCoreAPIs()
    {
        // ---- IThumbnailPlugin ----
        std::string cat = "IThumbnailPlugin";
        m_categories.push_back(cat);

        m_endpoints.push_back({"Initialize",
                               "Initialize the thumbnail plugin with engine context.",
                               "bool",
                               {{"void*", "engineContext"}, {"uint32_t", "version"}},
                               "if (!Initialize(ctx, 2)) { /* handle error */ }",
                               cat});

        m_endpoints.push_back({"Decode",
                               "Decode a file into an RGBA thumbnail bitmap.",
                               "bool",
                               {{"const wchar_t*", "filePath"},
                                {"uint32_t", "maxWidth"},
                                {"uint32_t", "maxHeight"},
                                {"uint8_t*", "outputRGBA"},
                                {"uint32_t*", "outWidth"},
                                {"uint32_t*", "outHeight"}},
                               "uint8_t buf[256*256*4];\nuint32_t w, h;\nDecode(L\"test.png\", 256, 256, buf, &w, &h);",
                               cat});

        m_endpoints.push_back({"GetSupportedExtensions",
                               "Return a null-terminated list of supported extensions.",
                               "const wchar_t**",
                               {},
                               "const wchar_t** exts = GetSupportedExtensions();",
                               cat});

        m_endpoints.push_back({"GetPluginInfo",
                               "Return human-readable plugin info string.",
                               "const char*",
                               {},
                               "printf(\"%s\\n\", GetPluginInfo());",
                               cat});

        m_endpoints.push_back({"Shutdown", "Release all plugin resources.", "void", {}, "Shutdown();", cat});

        // ---- ICADDecoderPlugin ----
        cat = "ICADDecoderPlugin";
        m_categories.push_back(cat);

        m_endpoints.push_back({"LoadModel",
                               "Load a 3D/CAD model from file.",
                               "bool",
                               {{"const wchar_t*", "filePath"}, {"void**", "outModelHandle"}},
                               "void* model = NULL;\nLoadModel(L\"part.step\", &model);",
                               cat});

        m_endpoints.push_back({"RenderView",
                               "Render the model to an RGBA bitmap from a given viewpoint.",
                               "bool",
                               {{"void*", "modelHandle"},
                                {"uint32_t", "width"},
                                {"uint32_t", "height"},
                                {"float", "rotX"},
                                {"float", "rotY"},
                                {"uint8_t*", "outputRGBA"}},
                               "RenderView(model, 512, 512, 30.0f, 45.0f, rgbaBuf);",
                               cat});

        m_endpoints.push_back({"GetModelInfo",
                               "Get metadata about a loaded model (vertex count, face count, etc.).",
                               "bool",
                               {{"void*", "modelHandle"}, {"char*", "outInfoJson"}, {"uint32_t", "bufSize"}},
                               "char info[1024];\nGetModelInfo(model, info, sizeof(info));",
                               cat});

        // ---- Plugin Lifecycle ----
        cat = "Plugin Lifecycle";
        m_categories.push_back(cat);

        m_endpoints.push_back({"PluginInit",
                               "Called once when the plugin DLL is loaded.",
                               "bool",
                               {{"uint32_t", "hostABIVersion"}},
                               "// Return false to abort loading\nreturn hostABIVersion >= 2;",
                               cat});

        m_endpoints.push_back({"PluginShutdown",
                               "Called once when the plugin DLL is about to be unloaded.",
                               "void",
                               {},
                               "// Free all resources\nPluginShutdown();",
                               cat});

        m_endpoints.push_back({"GetABIVersion",
                               "Return the ABI version this plugin was compiled against.",
                               "uint32_t",
                               {},
                               "return EXPLORERLENSSDK_ABI_VERSION;",
                               cat});

        m_endpoints.push_back({"GetPluginDescriptor",
                               "Return a JSON string describing plugin capabilities.",
                               "const char*",
                               {},
                               "return \"{\\\"name\\\":\\\"MyPlugin\\\",\\\"version\\\":\\\"1.0\\\"}\";",
                               cat});
    }

    // -- Utility --

    static inline std::string GetTimestamp()
    {
        SYSTEMTIME st{};
        GetLocalTime(&st);
        char buf[32]{};
        _snprintf_s(buf, _countof(buf), _TRUNCATE, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        return std::string(buf);
    }

    /// Convert a heading to a markdown anchor id.
    static inline std::string AnchorId(const std::string& text)
    {
        std::string id;
        id.reserve(text.size());
        for (char c : text) {
            if (c >= 'A' && c <= 'Z') {
                id += static_cast<char>(c - 'A' + 'a');
            } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                id += c;
            } else if (c == ' ' || c == '_') {
                id += '-';
            }
        }
        return id;
    }

    static inline std::string EscapeHtml(const std::string& s)
    {
        std::string out;
        out.reserve(s.size() + 16);
        for (char c : s) {
            switch (c) {
                case '&':
                    out += "&amp;";
                    break;
                case '<':
                    out += "&lt;";
                    break;
                case '>':
                    out += "&gt;";
                    break;
                case '"':
                    out += "&quot;";
                    break;
                default:
                    out += c;
                    break;
            }
        }
        return out;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
