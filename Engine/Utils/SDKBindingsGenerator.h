// SDKBindingsGenerator.h — Plugin SDK Bindings Code Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates language bindings (C#, Python, TypeScript) from the C ABI plugin API
// defined in plugin_api.h, reducing hand-authoring effort for plugin developers.
//
#pragma once
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SDKBindingLanguage {
    CSharp,
    Python,
    TypeScript,
    Rust
};

struct SDKFunction
{
    std::string name;
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> params;  // type, name
    std::string nativeSymbol;
    std::string docComment;
};

struct SDKBindingResult
{
    bool success = false;
    std::string code;
    int functionCount = 0;
    std::string errorMsg;
};

class SDKBindingsGenerator
{
  public:
    explicit SDKBindingsGenerator() = default;

    void AddFunction(SDKFunction fn)
    {
        m_functions.push_back(std::move(fn));
    }

    SDKBindingResult Generate(SDKBindingLanguage lang) const
    {
        switch (lang) {
            case SDKBindingLanguage::CSharp:
                return GenerateCSharp();
            case SDKBindingLanguage::Python:
                return GeneratePython();
            case SDKBindingLanguage::TypeScript:
                return GenerateTypeScript();
            case SDKBindingLanguage::Rust:
                return GenerateRust();
        }
        return {false, {}, 0, "Unknown language"};
    }

    size_t FunctionCount() const noexcept
    {
        return m_functions.size();
    }

    static std::string LanguageName(SDKBindingLanguage lang) noexcept
    {
        switch (lang) {
            case SDKBindingLanguage::CSharp:
                return "CSharp";
            case SDKBindingLanguage::Python:
                return "Python";
            case SDKBindingLanguage::TypeScript:
                return "TypeScript";
            case SDKBindingLanguage::Rust:
                return "Rust";
        }
        return "Unknown";
    }

  private:
    SDKBindingResult GenerateCSharp() const
    {
        std::ostringstream oss;
        oss << "// Auto-generated ExplorerLens SDK bindings — C#\n"
            << "using System.Runtime.InteropServices;\nnamespace ExplorerLens.SDK {\n";
        for (const auto& fn : m_functions) {
            oss << "    [DllImport(\"LENSShell.dll\")]\n"
                << "    public static extern " << fn.returnType << " " << fn.name << "();\n";
        }
        oss << "}\n";
        return {true, oss.str(), static_cast<int>(m_functions.size())};
    }
    SDKBindingResult GeneratePython() const
    {
        std::ostringstream oss;
        oss << "# Auto-generated ExplorerLens SDK bindings — Python\nimport ctypes\n_lib = ctypes.CDLL('LENSShell.dll')\nclass ExplorerLensSDK:\n";
        for (const auto& fn : m_functions) {
            oss << "    def " << fn.name << "(self): return _lib." << fn.name << "()\n";
        }
        if (m_functions.empty())
            oss << "    pass\n";
        return {true, oss.str(), static_cast<int>(m_functions.size())};
    }
    SDKBindingResult GenerateTypeScript() const
    {
        std::ostringstream oss;
        oss << "// Auto-generated ExplorerLens SDK bindings — TypeScript\n";
        for (const auto& fn : m_functions) {
            oss << "export declare function " << fn.name << "(): " << fn.returnType << ";\n";
        }
        return {true, oss.str(), static_cast<int>(m_functions.size())};
    }
    SDKBindingResult GenerateRust() const
    {
        std::ostringstream oss;
        oss << "// Auto-generated ExplorerLens SDK bindings — Rust\nextern \"C\" {\n";
        for (const auto& fn : m_functions) {
            oss << "    pub fn " << fn.name << "();\n";
        }
        oss << "}\n";
        return {true, oss.str(), static_cast<int>(m_functions.size())};
    }

    std::vector<SDKFunction> m_functions;
};

}  // namespace Engine
}  // namespace ExplorerLens
