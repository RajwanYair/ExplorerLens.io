/******************************************************************************
 * ExplorerLens Shared Memory Manager
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Manages shared memory sections for transferring large files between
 * Engine and PluginHost processes.
 *****************************************************************************/

#pragma once

#include "PluginIPCProtocol.h"
#include <Windows.h>
#include <string>
#include <memory>

namespace ExplorerLens {
namespace IPC {

//============================================================================
// Shared Memory Section
//============================================================================

class SharedMemorySection {
public:
    SharedMemorySection();
    ~SharedMemorySection();
    
    // Non-copyable
    SharedMemorySection(const SharedMemorySection&) = delete;
    SharedMemorySection& operator=(const SharedMemorySection&) = delete;
    
    // Movable
    SharedMemorySection(SharedMemorySection&& other) noexcept;
    SharedMemorySection& operator=(SharedMemorySection&& other) noexcept;
    
    // Create a new shared memory section (server side)
    bool Create(const std::string& name, uint64_t size);
    
    // Open an existing shared memory section (client side)
    bool Open(const std::string& name, uint64_t size, bool read_only = true);
    
    // Map the section into the process address space
    bool Map(bool read_only = true);
    
    // Unmap the section
    void Unmap();
    
    // Close the section handle
    void Close();
    
    // Check if section is valid
    bool IsValid() const { return handle_ != nullptr; }
    bool IsMapped() const { return view_ != nullptr; }
    
    // Get mapped memory pointer
    void* GetData() { return view_; }
    const void* GetData() const { return view_; }
    
    // Get size
    uint64_t GetSize() const { return size_; }
    
    // Get name
    const std::string& GetName() const { return name_; }
    
    // Write data to shared memory
    bool Write(const void* data, uint64_t offset, uint64_t size);
    
    // Read data from shared memory
    bool Read(void* data, uint64_t offset, uint64_t size);

private:
    void Cleanup();
    
    std::string name_;
    uint64_t size_ = 0;
    HANDLE handle_ = nullptr;
    void* view_ = nullptr;
    bool read_only_ = true;
};

//============================================================================
// Shared Memory Manager
//============================================================================

class SharedMemoryManager {
public:
    // Generate unique name for shared memory section
    static std::string GenerateName(uint64_t correlation_id);
    
    // Create shared memory for file data (server side)
    static std::unique_ptr<SharedMemorySection> CreateForFile(
        uint64_t correlation_id,
        const void* file_data,
        uint64_t file_size);
    
    // Open shared memory for file data (client side)
    static std::unique_ptr<SharedMemorySection> OpenForFile(
        const std::string& name,
        uint64_t expected_size);
    
    // Create shared memory for bitmap data (server side)
    static std::unique_ptr<SharedMemorySection> CreateForBitmap(
        uint64_t correlation_id,
        const void* bitmap_data,
        uint64_t bitmap_size);
    
    // Open shared memory for bitmap data (client side)
    static std::unique_ptr<SharedMemorySection> OpenForBitmap(
        const std::string& name,
        uint64_t expected_size);
    
    // Check if file size requires shared memory
    static bool RequiresSharedMemory(uint64_t size) {
        return size > SHARED_MEMORY_THRESHOLD;
    }

private:
    // Create security descriptor for shared memory
    static SECURITY_ATTRIBUTES* CreateSecurityAttributes();
    static void FreeSecurityAttributes(SECURITY_ATTRIBUTES* sa);
};

} // namespace IPC
} // namespace ExplorerLens

