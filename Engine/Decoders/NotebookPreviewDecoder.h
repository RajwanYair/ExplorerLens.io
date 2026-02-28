//==============================================================================
// ExplorerLens Engine — Notebook Preview (Jupyter .ipynb)
// Jupyter notebook thumbnail with cell rendering and output preview.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Notebook cell type
enum class NotebookCellType : uint8_t {
 Code,
 Markdown,
 Raw,
 COUNT
};

/// Notebook output type
enum class NotebookOutputType : uint8_t {
 Text, // text/plain
 Image, // image/png, image/jpeg
 HTML, // text/html
 SVG, // image/svg+xml
 LaTeX, // text/latex
 Error, // error traceback
 COUNT
};

/// Notebook kernel language
enum class NotebookKernel : uint8_t {
 Python,
 R,
 Julia,
 JavaScript,
 CSharp,
 Unknown,
 COUNT
};

/// Notebook metadata
struct NotebookMetadata {
 NotebookKernel kernel = NotebookKernel::Python;
 uint32_t cellCount = 0;
 uint32_t codeCells = 0;
 uint32_t markdownCells = 0;
 uint32_t outputCells = 0;
 std::wstring title;
 uint32_t formatVersion = 4; // nbformat
};

/// Notebook preview config
struct NotebookPreviewConfig {
 uint32_t maxCells = 10;
 uint32_t maxOutputLines = 5;
 bool showCellNumbers = true;
 bool syntaxHighlight = true;
 bool showOutputs = true;
 bool renderMarkdown = true;
};

/// Notebook preview decoder
class NotebookPreviewDecoder {
public:
 static const wchar_t* CellTypeName(NotebookCellType t) {
 switch (t) {
 case NotebookCellType::Code: return L"Code";
 case NotebookCellType::Markdown: return L"Markdown";
 case NotebookCellType::Raw: return L"Raw";
 default: return L"Unknown";
 }
 }

 static const wchar_t* OutputTypeName(NotebookOutputType t) {
 switch (t) {
 case NotebookOutputType::Text: return L"Text";
 case NotebookOutputType::Image: return L"Image";
 case NotebookOutputType::HTML: return L"HTML";
 case NotebookOutputType::SVG: return L"SVG";
 case NotebookOutputType::LaTeX: return L"LaTeX";
 case NotebookOutputType::Error: return L"Error";
 default: return L"Unknown";
 }
 }

 static const wchar_t* KernelName(NotebookKernel k) {
 switch (k) {
 case NotebookKernel::Python: return L"Python";
 case NotebookKernel::R: return L"R";
 case NotebookKernel::Julia: return L"Julia";
 case NotebookKernel::JavaScript: return L"JavaScript";
 case NotebookKernel::CSharp: return L"C#";
 case NotebookKernel::Unknown: return L"Unknown";
 default: return L"Unknown";
 }
 }

 static constexpr size_t CellTypeCount() { return static_cast<size_t>(NotebookCellType::COUNT); }
 static constexpr size_t OutputTypeCount() { return static_cast<size_t>(NotebookOutputType::COUNT); }
 static constexpr size_t KernelCount() { return static_cast<size_t>(NotebookKernel::COUNT); }
};

}} // namespace ExplorerLens::Engine

