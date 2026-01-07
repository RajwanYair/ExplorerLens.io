/******************************************************************************
 * libarchive_wrapper.h
 * LibArchive Integration for DarkThumbs
 * 
 * Provides support for additional archive formats:
 * - TAR (.tar, .tar.gz, .tgz, .tar.bz2, .tbz, .tar.xz, .txz, .tar.zst)
 * - CPIO (.cpio)
 * - ISO 9660 CD/DVD images (.iso)
 * - XAR (.xar) - macOS archives
 * - AR (.ar, .deb) - Unix archives, Debian packages
 * - CAB (.cab) - Microsoft Cabinet archives
 * - RAR (.rar) - via libarchive (read-only, no password support)
 * - 7-Zip (.7z) - via libarchive
 ******************************************************************************/

#pragma once

#include <windows.h>
#include <string>
#include <vector>

// Forward declare libarchive types
struct archive;
struct archive_entry;

namespace DarkThumbs {

class LibArchiveWrapper {
public:
    /**
     * Open an archive file for reading
     * 
     * @param filepath - Path to archive file
     * @return true if opened successfully
     */
    static bool OpenArchive(LPCTSTR filepath);

    /**
     * Close the current archive
     */
    static void CloseArchive();

    /**
     * Get number of entries in archive
     * 
     * @return Entry count, -1 if not open
     */
    static int GetEntryCount();

    /**
     * Read next entry header
     * 
     * @return true if entry read successfully, false if end or error
     */
    static bool ReadNextEntry();

    /**
     * Get current entry name
     * 
     * @return Entry pathname
     */
    static const char* GetEntryName();

    /**
     * Get current entry size (uncompressed)
     * 
     * @return Size in bytes
     */
    static int64_t GetEntrySize();

    /**
     * Check if current entry is a directory
     * 
     * @return true if directory
     */
    static bool IsEntryDirectory();

    /**
     * Extract current entry to memory buffer
     * 
     * @param buffer - Output buffer (caller allocates)
     * @param size - Buffer size
     * @return Bytes read, or -1 on error
     */
    static SSIZE_T ExtractEntryToMemory(void* buffer, size_t size);

    /**
     * Detect archive format from file header
     * 
     * @param data - File header data
     * @param size - Size of header data
     * @return Archive format code (CBXTYPE_*)
     */
    static int DetectArchiveType(const BYTE* data, size_t size);

    /**
     * Check if file extension is supported by libarchive
     * 
     * @param ext - File extension (e.g., ".tar", ".iso")
     * @return true if supported
     */
    static bool IsSupportedExtension(LPCTSTR ext);

private:
    static struct archive* m_archive;
    static struct archive_entry* m_entry;
    static std::string m_currentPath;
};

} // namespace DarkThumbs
