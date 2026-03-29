// AnnotationStore.h — Per-File Annotation Store (SQLite-ready, Sync-Ready Schema)
// Copyright (c) 2026 ExplorerLens Project
//
// In-memory annotation store with a SQLite-compatible schema. Each file path maps
// to a set of annotations (stars, tags, text comments). Supports dirty-flag tracking
// for incremental cloud sync.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class AnnotationRecordType { Star, Tag, Comment, Color, Rating };

struct Annotation {
    uint64_t    id           = 0;
    AnnotationRecordType type      = AnnotationRecordType::Tag;
    std::wstring   filePath;
    std::wstring   value;        // tag text, comment text, star=L"1"
    std::wstring   authorId;
    std::chrono::system_clock::time_point createdAt = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point modifiedAt = std::chrono::system_clock::now();
    bool         isDirty     = true;
    bool         isDeleted   = false;

    std::string TypeName() const noexcept {
        switch (type) {
        case AnnotationRecordType::Star:    return "Star";
        case AnnotationRecordType::Tag:     return "Tag";
        case AnnotationRecordType::Comment: return "Comment";
        case AnnotationRecordType::Color:   return "Color";
        case AnnotationRecordType::Rating:  return "Rating";
        }
        return "Unknown";
    }
};

class AnnotationStore {
public:
    static AnnotationStore& Instance() {
        static AnnotationStore inst;
        return inst;
    }

    uint64_t Add(const Annotation& ann) {
        Annotation a = ann;
        a.id = ++m_nextId;
        a.isDirty = true;
        m_annotations.push_back(a);
        return a.id;
    }

    bool Delete(uint64_t id) {
        for (auto& a : m_annotations)
            if (a.id == id && !a.isDeleted) { a.isDeleted = true; a.isDirty = true; return true; }
        return false;
    }

    std::vector<Annotation> GetForFile(const std::wstring& filePath) const {
        std::vector<Annotation> v;
        for (const auto& a : m_annotations)
            if (!a.isDeleted && a.filePath == filePath) v.push_back(a);
        return v;
    }

    std::vector<Annotation> GetDirty() const {
        std::vector<Annotation> v;
        for (const auto& a : m_annotations) if (a.isDirty) v.push_back(a);
        return v;
    }

    void MarkSynced(uint64_t id) {
        for (auto& a : m_annotations) if (a.id == id) { a.isDirty = false; break; }
    }

    int  Count()          const noexcept { return (int)m_annotations.size(); }
    int  DirtyCount()     const noexcept {
        int n = 0;
        for (const auto& a : m_annotations) if (a.isDirty) n++;
        return n;
    }

    void Clear() noexcept { m_annotations.clear(); m_nextId = 0; }

private:
    AnnotationStore() = default;
    std::vector<Annotation> m_annotations;
    uint64_t m_nextId = 0;
};

} // namespace Engine
} // namespace ExplorerLens
