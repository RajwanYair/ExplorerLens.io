#include "BatchProcessingEngine.h"

namespace DarkThumbs { namespace Engine {

BatchJob BatchProcessingEngine::s_emptyJob = {};

BatchProcessingEngine::BatchProcessingEngine() = default;

const wchar_t* BatchProcessingEngine::GetOperationName(BatchOperation op) {
    switch (op) {
        case BatchOperation::GenerateThumbnails: return L"Generate Thumbnails";
        case BatchOperation::ConvertFormats:     return L"Convert Formats";
        case BatchOperation::ValidateFiles:      return L"Validate Files";
        case BatchOperation::CleanCache:         return L"Clean Cache";
        case BatchOperation::ExportMetadata:     return L"Export Metadata";
        default:                                 return L"Unknown";
    }
}

const wchar_t* BatchProcessingEngine::GetStatusName(BatchStatus status) {
    switch (status) {
        case BatchStatus::Queued:    return L"Queued";
        case BatchStatus::Running:   return L"Running";
        case BatchStatus::Paused:    return L"Paused";
        case BatchStatus::Completed: return L"Completed";
        case BatchStatus::Failed:    return L"Failed";
        case BatchStatus::Cancelled: return L"Cancelled";
        default:                     return L"Unknown";
    }
}

size_t BatchProcessingEngine::CreateJob(const std::wstring& name, BatchOperation op,
                                         const std::vector<std::wstring>& files) {
    BatchJob job;
    job.name = name;
    job.operation = op;
    job.status = BatchStatus::Queued;
    job.inputFiles = files;
    job.progress.totalFiles = static_cast<uint32_t>(files.size());
    m_jobs.push_back(std::move(job));
    return m_jobs.size() - 1;
}

const BatchJob& BatchProcessingEngine::GetJob(size_t index) const {
    if (index >= m_jobs.size()) return s_emptyJob;
    return m_jobs[index];
}

bool BatchProcessingEngine::RunJob(size_t index) {
    if (index >= m_jobs.size()) return false;
    auto& job = m_jobs[index];
    job.status = BatchStatus::Running;

    // Simulate processing
    for (uint32_t i = 0; i < job.progress.totalFiles; ++i) {
        job.progress.processedFiles = i + 1;
        job.progress.progressPercent = (static_cast<double>(i + 1) / job.progress.totalFiles) * 100.0;
        if (m_progressCb) m_progressCb(job.progress);
    }
    job.status = BatchStatus::Completed;
    return true;
}

bool BatchProcessingEngine::CancelJob(size_t index) {
    if (index >= m_jobs.size()) return false;
    m_jobs[index].status = BatchStatus::Cancelled;
    return true;
}

}} // namespace DarkThumbs::Engine
