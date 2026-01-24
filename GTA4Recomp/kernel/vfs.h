#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>
#include <mutex>
#include <memory>
#include "mapped_file.h"

namespace VFS
{
    void Initialize(const std::filesystem::path& extractedRoot);
    bool IsInitialized();
    std::filesystem::path GetExtractedRoot();
    
    /**
     * convert xbox360 paths to host paths
     * @param guestPath the xbox360 path
     * @return the host filesystem path
    */

    std::filesystem::path Resolve(const std::string& guestPath);
    bool Exists(const std::string& guestPath);
    bool IsDirectory(const std::string& guestPath);
    uint64_t GetFileSize(const std::string& guestPath);
    std::string NormalizePath(const std::string& guestPath);
    std::string StripDrivePrefix(const std::string& guestPath);

    struct PathMapping
    {
        std::string guestPrefix;
        std::string hostPrefix;
    };

    void AddPathMapping(const std::string& guestPrefix, const std::string& hostPrefix);
    void ResetPathMappings();
    void RebuildIndex();
    
    struct Stats
    {
        uint64_t totalFiles = 0;
        uint64_t totalDirectories = 0;
        uint64_t totalBytes = 0;
        uint64_t cacheHits = 0;
        uint64_t cacheMisses = 0;
        uint64_t mmapOpens = 0;
    };
    Stats GetStats();
    
    /**
     * Open file with memory mapping.
     * @param guestPath xb360 path to open
     * @return pointer to mappedfile
     */
    std::shared_ptr<MappedFile> OpenMapped(const std::string& guestPath);
    
    /**
     * Check if a file should use memory mapping.
     * @param guestPath xb360 path to check
     * @return true if file is large enough for mmap
     */
    bool ShouldUseMmap(const std::string& guestPath);
}