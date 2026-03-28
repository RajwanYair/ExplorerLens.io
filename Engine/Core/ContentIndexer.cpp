// =============================================================================
// ContentIndexer.cpp — File Content Indexing for Search
// ExplorerLens Engine — Core Module
// =============================================================================

#include "ContentIndexer.h"
#include <algorithm>
#include <chrono>

namespace ExplorerLens {

static uint64_t NowMs() {
 return static_cast<uint64_t>(
 std::chrono::duration_cast<std::chrono::milliseconds>(
 std::chrono::steady_clock::now().time_since_epoch()).count());
}

ContentIndexer::ContentIndexer() {}

std::wstring ContentIndexer::ExtractExtension(const std::wstring& path) const {
 auto pos = path.rfind(L'.');
 if (pos == std::wstring::npos) return L"";
 std::wstring ext = path.substr(pos);
 std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
 return ext;
}

std::wstring ContentIndexer::ExtractFileName(const std::wstring& path) const {
 auto pos = path.rfind(L'\\');
 if (pos == std::wstring::npos) pos = path.rfind(L'/');
 if (pos == std::wstring::npos) return path;
 return path.substr(pos + 1);
}

uint64_t ContentIndexer::AddFile(const std::wstring& filePath) {
 ContentIndexEntry entry;
 entry.id = m_nextId++;
 entry.filePath = filePath;
 entry.fileName = ExtractFileName(filePath);
 entry.extension = ExtractExtension(filePath);
 entry.contentType = ClassifyExtension(entry.extension);
 entry.state = IndexState::Pending;
 entry.indexedAt = 0;
 m_entries.push_back(entry);
 return entry.id;
}

bool ContentIndexer::RemoveFile(uint64_t entryId) {
 for (auto& e : m_entries) {
 if (e.id == entryId) {
 e.state = IndexState::Removed;
 return true;
 }
 }
 return false;
}

bool ContentIndexer::UpdateFile(uint64_t entryId) {
 for (auto& e : m_entries) {
 if (e.id == entryId) {
 e.state = IndexState::Indexed;
 e.indexedAt = NowMs();
 return true;
 }
 }
 return false;
}

uint32_t ContentIndexer::IndexAll() {
 uint32_t indexed = 0;
 for (auto& e : m_entries) {
 if (e.state == IndexState::Pending || e.state == IndexState::Stale) {
 e.state = IndexState::Indexed;
 e.indexedAt = NowMs();
 indexed++;
 }
 }
 return indexed;
}

std::vector<ContentIndexEntry> ContentIndexer::SearchByName(const std::wstring& pattern) const {
 std::vector<ContentIndexEntry> results;
 for (const auto& e : m_entries) {
 if (e.state == IndexState::Indexed && e.fileName.find(pattern) != std::wstring::npos) {
 results.push_back(e);
 }
 }
 return results;
}

std::vector<ContentIndexEntry> ContentIndexer::SearchByType(ContentType type) const {
 std::vector<ContentIndexEntry> results;
 for (const auto& e : m_entries) {
 if (e.state == IndexState::Indexed && e.contentType == type) {
 results.push_back(e);
 }
 }
 return results;
}

std::vector<ContentIndexEntry> ContentIndexer::SearchByExtension(const std::wstring& ext) const {
 std::vector<ContentIndexEntry> results;
 std::wstring lowerExt = ext;
 std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::towlower);
 for (const auto& e : m_entries) {
 if (e.state == IndexState::Indexed && e.extension == lowerExt) {
 results.push_back(e);
 }
 }
 return results;
}

const ContentIndexEntry* ContentIndexer::GetEntry(uint64_t id) const {
 for (const auto& e : m_entries) {
 if (e.id == id) return &e;
 }
 return nullptr;
}

ContentIndexStats ContentIndexer::GetStats() const {
 ContentIndexStats stats;
 stats.totalEntries = static_cast<uint32_t>(m_entries.size());
 for (const auto& e : m_entries) {
 switch (e.state) {
 case IndexState::Indexed: stats.indexedCount++; break;
 case IndexState::Pending: stats.pendingCount++; break;
 case IndexState::Failed: stats.failedCount++; break;
 case IndexState::Stale: stats.staleCount++; break;
 default: break;
 }
 stats.totalSizeBytes += e.fileSize;
 uint32_t ct = static_cast<uint32_t>(e.contentType);
 if (ct < static_cast<uint32_t>(ContentType::Count)) {
 stats.contentTypeCounts[ct]++;
 }
 }
 return stats;
}

uint32_t ContentIndexer::PurgeRemoved() {
 uint32_t purged = 0;
 auto it = std::remove_if(m_entries.begin(), m_entries.end(),
 [&purged](const ContentIndexEntry& e) {
 if (e.state == IndexState::Removed) { purged++; return true; }
 return false;
 });
 m_entries.erase(it, m_entries.end());
 return purged;
}

void ContentIndexer::Clear() {
 m_entries.clear();
 m_nextId = 1;
}

ContentType ContentIndexer::ClassifyExtension(const std::wstring& ext) {
 if (ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".bmp" ||
 ext == L".gif" || ext == L".webp" || ext == L".tiff" || ext == L".avif" ||
 ext == L".heif" || ext == L".heic" || ext == L".jxl" || ext == L".svg") {
 return ContentType::Image;
 }
 if (ext == L".zip" || ext == L".rar" || ext == L".7z" || ext == L".tar" ||
 ext == L".gz" || ext == L".cbz" || ext == L".cbr" || ext == L".cb7") {
 return ContentType::Archive;
 }
 if (ext == L".pdf" || ext == L".doc" || ext == L".docx" || ext == L".epub") {
 return ContentType::Document;
 }
 if (ext == L".mp4" || ext == L".avi" || ext == L".mkv" || ext == L".webm") {
 return ContentType::Video;
 }
 if (ext == L".mp3" || ext == L".flac" || ext == L".wav" || ext == L".ogg") {
 return ContentType::Audio;
 }
 if (ext == L".obj" || ext == L".stl" || ext == L".gltf" || ext == L".glb") {
 return ContentType::Model3D;
 }
 if (ext == L".ttf" || ext == L".otf" || ext == L".woff" || ext == L".woff2") {
 return ContentType::Font;
 }
 return ContentType::Unknown;
}

const wchar_t* ContentIndexer::GetContentTypeName(ContentType type) {
 switch (type) {
 case ContentType::Image: return L"Image";
 case ContentType::Archive: return L"Archive";
 case ContentType::Document: return L"Document";
 case ContentType::Video: return L"Video";
 case ContentType::Audio: return L"Audio";
 case ContentType::Model3D: return L"3D Model";
 case ContentType::Font: return L"Font";
 case ContentType::Unknown: return L"Unknown";
 default: return L"Unknown";
 }
}

const wchar_t* ContentIndexer::GetIndexStateName(IndexState state) {
 switch (state) {
 case IndexState::Pending: return L"Pending";
 case IndexState::Indexed: return L"Indexed";
 case IndexState::Failed: return L"Failed";
 case IndexState::Stale: return L"Stale";
 case IndexState::Removed: return L"Removed";
 default: return L"Unknown";
 }
}

} // namespace ExplorerLens
