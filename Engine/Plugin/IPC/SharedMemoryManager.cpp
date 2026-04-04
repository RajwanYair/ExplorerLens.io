/******************************************************************************
 * ExplorerLens Shared Memory Manager Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 *****************************************************************************/

#include "SharedMemoryManager.h"
#include <aclapi.h>
#include <sddl.h>
#include <iomanip>
#include <sstream>

namespace ExplorerLens {
namespace IPC {

//============================================================================
// SharedMemorySection Implementation
//============================================================================

SharedMemorySection::SharedMemorySection() = default;

SharedMemorySection::~SharedMemorySection()
{
    Cleanup();
}

SharedMemorySection::SharedMemorySection(SharedMemorySection&& other) noexcept
    : name_(std::move(other.name_))
    , size_(other.size_)
    , handle_(other.handle_)
    , view_(other.view_)
    , read_only_(other.read_only_)
{
    other.handle_ = nullptr;
    other.view_ = nullptr;
    other.size_ = 0;
}

SharedMemorySection& SharedMemorySection::operator=(SharedMemorySection&& other) noexcept
{
    if (this != &other) {
        Cleanup();

        name_ = std::move(other.name_);
        size_ = other.size_;
        handle_ = other.handle_;
        view_ = other.view_;
        read_only_ = other.read_only_;

        other.handle_ = nullptr;
        other.view_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

bool SharedMemorySection::Create(const std::string& name, uint64_t size)
{
    if (IsValid()) {
        return false;  // Already created
    }

    name_ = name;
    size_ = size;

    // Create security descriptor (allow access to current user only)
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;

    // Create a DACL that allows the current user full control
    PSECURITY_DESCRIPTOR pSD = nullptr;
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(
            "D:(A;;GA;;;WD)",  // Allow generic all to World (simplified for now)
            SDDL_REVISION_1, &pSD, nullptr)) {
        return false;
    }

    sa.lpSecurityDescriptor = pSD;

    // Create file mapping
    std::wstring wide_name(name.begin(), name.end());
    handle_ = CreateFileMappingW(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, static_cast<DWORD>(size >> 32),
                                 static_cast<DWORD>(size & 0xFFFFFFFF), wide_name.c_str());

    LocalFree(pSD);

    if (!handle_) {
        return false;
    }

    return true;
}

bool SharedMemorySection::Open(const std::string& name, uint64_t size, bool read_only)
{
    if (IsValid()) {
        return false;  // Already opened
    }

    name_ = name;
    size_ = size;
    read_only_ = read_only;

    // Open existing file mapping
    std::wstring wide_name(name.begin(), name.end());
    DWORD access = read_only ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;

    handle_ = OpenFileMappingW(access, FALSE, wide_name.c_str());

    return handle_ != nullptr;
}

bool SharedMemorySection::Map(bool read_only)
{
    if (!IsValid() || IsMapped()) {
        return false;
    }

    read_only_ = read_only;

    DWORD access = read_only ? FILE_MAP_READ : FILE_MAP_WRITE;
    view_ = MapViewOfFile(handle_, access, 0, 0, static_cast<SIZE_T>(size_));

    return view_ != nullptr;
}

void SharedMemorySection::Unmap()
{
    if (view_) {
        UnmapViewOfFile(view_);
        view_ = nullptr;
    }
}

void SharedMemorySection::Close()
{
    Unmap();

    if (handle_) {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
}

void SharedMemorySection::Cleanup()
{
    Close();
}

bool SharedMemorySection::Write(const void* data, uint64_t offset, uint64_t write_size)
{
    if (!IsMapped() || read_only_) {
        return false;
    }

    if (offset + write_size > size_) {
        return false;  // Out of bounds
    }

    memcpy(static_cast<uint8_t*>(view_) + offset, data, static_cast<size_t>(write_size));
    return true;
}

bool SharedMemorySection::Read(void* data, uint64_t offset, uint64_t read_size)
{
    if (!IsMapped()) {
        return false;
    }

    if (offset + read_size > size_) {
        return false;  // Out of bounds
    }

    memcpy(data, static_cast<const uint8_t*>(view_) + offset, static_cast<size_t>(read_size));
    return true;
}

//============================================================================
// SharedMemoryManager Implementation
//============================================================================

std::string SharedMemoryManager::GenerateName(uint64_t correlation_id)
{
    std::ostringstream oss;
    oss << "Local\\ExplorerLens-Shared-" << std::hex << std::setw(16) << std::setfill('0') << correlation_id;
    return oss.str();
}

std::unique_ptr<SharedMemorySection> SharedMemoryManager::CreateForFile(uint64_t correlation_id, const void* file_data,
                                                                        uint64_t file_size)
{
    auto section = std::make_unique<SharedMemorySection>();
    std::string name = GenerateName(correlation_id);

    if (!section->Create(name, file_size)) {
        return nullptr;
    }

    if (!section->Map(false)) {
        return nullptr;
    }

    if (!section->Write(file_data, 0, file_size)) {
        return nullptr;
    }

    return section;
}

std::unique_ptr<SharedMemorySection> SharedMemoryManager::OpenForFile(const std::string& name, uint64_t expected_size)
{
    auto section = std::make_unique<SharedMemorySection>();

    if (!section->Open(name, expected_size, true)) {
        return nullptr;
    }

    if (!section->Map(true)) {
        return nullptr;
    }

    return section;
}

std::unique_ptr<SharedMemorySection> SharedMemoryManager::CreateForBitmap(uint64_t correlation_id,
                                                                          const void* bitmap_data, uint64_t bitmap_size)
{
    auto section = std::make_unique<SharedMemorySection>();
    std::string name = GenerateName(correlation_id + 1);  // Different ID for bitmap

    if (!section->Create(name, bitmap_size)) {
        return nullptr;
    }

    if (!section->Map(false)) {
        return nullptr;
    }

    if (!section->Write(bitmap_data, 0, bitmap_size)) {
        return nullptr;
    }

    return section;
}

std::unique_ptr<SharedMemorySection> SharedMemoryManager::OpenForBitmap(const std::string& name, uint64_t expected_size)
{
    auto section = std::make_unique<SharedMemorySection>();

    if (!section->Open(name, expected_size, true)) {
        return nullptr;
    }

    if (!section->Map(true)) {
        return nullptr;
    }

    return section;
}

}  // namespace IPC
}  // namespace ExplorerLens
