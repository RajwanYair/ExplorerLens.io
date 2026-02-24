/******************************************************************************
 * libarchive_wrapper.cpp
 * LibArchive Integration for ExplorerLens
 ******************************************************************************/

#include "StdAfx.h"
#include "LENSArchive.h"
#include "libarchive_wrapper.h"

#ifdef ENABLE_LIBARCHIVE_SUPPORT

#include <archive.h>
#include <archive_entry.h>

namespace ExplorerLens {

// Static member initialization
struct archive* LibArchiveWrapper::m_archive = nullptr;
struct archive_entry* LibArchiveWrapper::m_entry = nullptr;
std::string LibArchiveWrapper::m_currentPath;

bool LibArchiveWrapper::OpenArchive(LPCTSTR filepath) {
    if (m_archive) {
        CloseArchive();
    }

    m_archive = archive_read_new();
    if (!m_archive) {
        return false;
    }

    // Enable all compression formats
    archive_read_support_filter_all(m_archive);
    
    // Enable all archive formats
    archive_read_support_format_all(m_archive);

    // Convert filepath to UTF-8 for libarchive
#ifdef UNICODE
    int len = WideCharToMultiByte(CP_UTF8, 0, filepath, -1, nullptr, 0, nullptr, nullptr);
    if (len == 0) {
        CloseArchive();
        return false;
    }
    
    std::string utf8path(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, filepath, -1, &utf8path[0], len, nullptr, nullptr);
    
    int r = archive_read_open_filename(m_archive, utf8path.c_str(), 10240);
#else
    int r = archive_read_open_filename(m_archive, filepath, 10240);
#endif

    if (r != ARCHIVE_OK) {
        CloseArchive();
        return false;
    }

    return true;
}

void LibArchiveWrapper::CloseArchive() {
    if (m_archive) {
        archive_read_close(m_archive);
        archive_read_free(m_archive);
        m_archive = nullptr;
    }
    m_entry = nullptr;
    m_currentPath.clear();
}

int LibArchiveWrapper::GetEntryCount() {
    if (!m_archive) {
        return -1;
    }

    // Libarchive doesn't provide direct count - would need to scan all entries
    // Return -1 to indicate unknown count
    return -1;
}

bool LibArchiveWrapper::ReadNextEntry() {
    if (!m_archive) {
        return false;
    }

    int r = archive_read_next_header(m_archive, &m_entry);
    if (r == ARCHIVE_EOF) {
        return false;
    }

    if (r != ARCHIVE_OK) {
        return false;
    }

    // Store current path
    const char* pathname = archive_entry_pathname(m_entry);
    if (pathname) {
        m_currentPath = pathname;
    }

    return true;
}

const char* LibArchiveWrapper::GetEntryName() {
    if (!m_entry) {
        return nullptr;
    }

    return archive_entry_pathname(m_entry);
}

int64_t LibArchiveWrapper::GetEntrySize() {
    if (!m_entry) {
        return 0;
    }

    return archive_entry_size(m_entry);
}

bool LibArchiveWrapper::IsEntryDirectory() {
    if (!m_entry) {
        return false;
    }

    int filetype = archive_entry_filetype(m_entry);
    return (filetype == AE_IFDIR);
}

SSIZE_T LibArchiveWrapper::ExtractEntryToMemory(void* buffer, size_t size) {
    if (!m_archive || !m_entry || !buffer) {
        return -1;
    }

    SSIZE_T totalRead = 0;
    const void* buff;
    size_t buffSize;
    int64_t offset;

    while (true) {
        int r = archive_read_data_block(m_archive, &buff, &buffSize, &offset);
        
        if (r == ARCHIVE_EOF) {
            break;
        }

        if (r != ARCHIVE_OK) {
            return -1;
        }

        // Check if we have enough space
        if (totalRead + buffSize > size) {
            buffSize = size - totalRead;
        }

        memcpy((char*)buffer + totalRead, buff, buffSize);
        totalRead += buffSize;

        if (totalRead >= (ssize_t)size) {
            break;
        }
    }

    return totalRead;
}

int LibArchiveWrapper::DetectArchiveType(const BYTE* data, size_t size) {
    if (!data || size < 16) {
        return LENSTYPE_NONE;
    }

    // TAR archives (ustar signature at offset 257)
    if (size > 262) {
        if (memcmp(data + 257, "ustar", 5) == 0) {
            return LENSTYPE_TAR;
        }
    }

    // CPIO archives
    if (size > 6) {
        // ASCII CPIO
        if (memcmp(data, "070707", 6) == 0 || memcmp(data, "070701", 6) == 0 || memcmp(data, "070702", 6) == 0) {
            return LENSTYPE_CPIO;
        }
        // Binary CPIO
        if (data[0] == 0xC7 && data[1] == 0x71) {
            return LENSTYPE_CPIO;
        }
    }

    // ISO 9660
    if (size > 0x8000 + 6) {
        if (memcmp(data + 0x8001, "CD001", 5) == 0) {
            return LENSTYPE_ISO;
        }
    }

    // XAR (macOS)
    if (memcmp(data, "xar!", 4) == 0) {
        return LENSTYPE_XAR;
    }

    // AR archives (Unix)
    if (memcmp(data, "!<arch>\n", 8) == 0) {
        return LENSTYPE_AR;
    }

    // Microsoft CAB
    if (memcmp(data, "MSCF", 4) == 0) {
        return LENSTYPE_CAB;
    }

    // 7-Zip (also handled by native unzip, but libarchive can read it)
    if (memcmp(data, "7z\xBC\xAF\x27\x1C", 6) == 0) {
        return LENSTYPE_7Z;
    }

    // RAR (libarchive can read RAR 1.5-4.x)
    if (memcmp(data, "Rar!\x1A\x07\x00", 7) == 0 || memcmp(data, "Rar!\x1A\x07\x01", 7) == 0) {
        // Note: This is old RAR format, libarchive has limited support
        // Modern RAR5 requires different handling
        return LENSTYPE_NONE;  // Let unrar handle it if enabled
    }

    // Compressed TAR detection
    // GZIP (.tar.gz)
    if (data[0] == 0x1F && data[1] == 0x8B) {
        return LENSTYPE_TAR_GZ;
    }

    // BZIP2 (.tar.bz2)
    if (memcmp(data, "BZ", 2) == 0) {
        return LENSTYPE_TAR_BZ2;
    }

    // XZ (.tar.xz)
    if (memcmp(data, "\xFD" "7zXZ\x00", 6) == 0) {
        return LENSTYPE_TAR_XZ;
    }

    // Zstandard (.tar.zst)
    if (memcmp(data, "\x28\xB5\x2F\xFD", 4) == 0) {
        return LENSTYPE_TAR_ZST;
    }

    return LENSTYPE_NONE;
}

bool LibArchiveWrapper::IsSupportedExtension(LPCTSTR ext) {
    if (!ext || !*ext) {
        return false;
    }

    // Convert to lowercase for comparison
    std::wstring wext(ext);
    for (wchar_t& c : wext) {
        c = towlower(c);
    }

    // TAR and compressed TAR
    if (wext == L".tar" || wext == L".tar.gz" || wext == L".tgz" ||
        wext == L".tar.bz2" || wext == L".tbz" || wext == L".tb2" ||
        wext == L".tar.xz" || wext == L".txz" ||
        wext == L".tar.zst" || wext == L".tzst") {
        return true;
    }

    // Other formats
    if (wext == L".cpio" || wext == L".iso" || wext == L".xar" ||
        wext == L".ar" || wext == L".deb" || wext == L".cab") {
        return true;
    }

    return false;
}

} // namespace ExplorerLens

#endif // ENABLE_LIBARCHIVE_SUPPORT

