// AnnotationDiffViewer.h — Annotation Diff Viewer (Provenance & Change History)
// Copyright (c) 2026 ExplorerLens Project
//
// Computes and displays the diff between annotation snapshots, enabling audit trails
// and provenance tracking for collaborative annotation workflows.
//
#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

enum class DiffOperation { Add, Remove, Modify, Unchanged };

struct AnnotationDiffEntry {
    DiffOperation op        = DiffOperation::Unchanged;
    std::wstring  field;
    std::wstring  oldValue;
    std::wstring  newValue;
    std::wstring  author;
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::string OpName() const noexcept {
        switch (op) {
        case DiffOperation::Add:       return "Add";
        case DiffOperation::Remove:    return "Remove";
        case DiffOperation::Modify:    return "Modify";
        case DiffOperation::Unchanged: return "Unchanged";
        }
        return "Unknown";
    }
};

struct AnnotationDiffResult {
    std::vector<AnnotationDiffEntry> entries;
    int additions  = 0;
    int removals   = 0;
    int changes    = 0;
    bool IsClean() const noexcept { return additions == 0 && removals == 0 && changes == 0; }
};

struct AnnotationSnapshot {
    std::wstring filePath;
    int          starRating  = 0;
    std::vector<std::wstring> tags;
    std::wstring comment;
    uint32_t     colorLabel  = 0;
    std::wstring author;
};

class AnnotationDiffViewer {
public:
    explicit AnnotationDiffViewer() = default;

    AnnotationDiffResult Diff(const AnnotationSnapshot& before,
                              const AnnotationSnapshot& after,
                              const std::wstring& author = {}) const {
        AnnotationDiffResult result;
        if (before.starRating != after.starRating) {
            result.entries.push_back({ DiffOperation::Modify, L"starRating",
                std::to_wstring(before.starRating), std::to_wstring(after.starRating), author });
            result.changes++;
        }
        if (before.comment != after.comment) {
            result.entries.push_back({ DiffOperation::Modify, L"comment",
                before.comment, after.comment, author });
            result.changes++;
        }
        if (before.colorLabel != after.colorLabel) {
            result.entries.push_back({ DiffOperation::Modify, L"colorLabel",
                std::to_wstring(before.colorLabel), std::to_wstring(after.colorLabel), author });
            result.changes++;
        }
        // Tags diff
        for (const auto& t : after.tags) {
            bool found = false;
            for (const auto& bt : before.tags) if (bt == t) { found = true; break; }
            if (!found) { result.entries.push_back({ DiffOperation::Add, L"tag", {}, t, author }); result.additions++; }
        }
        for (const auto& t : before.tags) {
            bool found = false;
            for (const auto& at : after.tags) if (at == t) { found = true; break; }
            if (!found) { result.entries.push_back({ DiffOperation::Remove, L"tag", t, {}, author }); result.removals++; }
        }
        return result;
    }

    std::string FormatDiff(const AnnotationDiffResult& diff) const {
        std::ostringstream oss;
        oss << "Diff: +" << diff.additions << " -" << diff.removals << " ~" << diff.changes;
        return oss.str();
    }
};

} // namespace Engine
} // namespace ExplorerLens
