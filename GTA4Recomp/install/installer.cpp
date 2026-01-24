#include "installer.h"
#include <functional>
#include "shader_converter.h"
#include "platform_paths.h"

#include <xxh3.h>

#include "directory_file_system.h"
#include "iso_file_system.h"
#include "xcontent_file_system.h"

#include "hashes/game.h"

static const std::string GameDirectory = "game";
static const std::string GameExecutableFile = "default.xex";
static const std::string ZIPExtension = ".zip";

// Temp directory for extracted ZIP files
static std::filesystem::path g_tempExtractDir;


static std::string fromU8(const std::u8string &str)
{
    return std::string(str.begin(), str.end());
}

static std::string fromPath(const std::filesystem::path &path)
{
    return fromU8(path.u8string());
}

static std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
};

// Extract ZIP file to temp directory and return extracted STFS path
static std::filesystem::path extractZipToTemp(const std::filesystem::path &zipPath)
{
    // Create temp directory if needed
    if (g_tempExtractDir.empty())
    {
        g_tempExtractDir = std::filesystem::temp_directory_path() / "FernandoRecomp_DLC";
    }
    
    // Create unique extraction folder based on ZIP filename
    std::string zipName = zipPath.stem().string();
    std::filesystem::path extractDir = g_tempExtractDir / zipName;
    
    // Clean up any existing extraction
    std::error_code ec;
    if (std::filesystem::exists(extractDir))
    {
        std::filesystem::remove_all(extractDir, ec);
    }
    std::filesystem::create_directories(extractDir, ec);
    
    // Use system unzip command (available on macOS/Linux)
    std::string cmd = fmt::format("unzip -q -o \"{}\" -d \"{}\"", 
        zipPath.string(), extractDir.string());
    int result = std::system(cmd.c_str());
    
    if (result != 0)
    {
        return {};
    }
    
    // Find the STFS file inside (usually in a subfolder like 545407F2/00000002/...)
    for (const auto& entry : std::filesystem::recursive_directory_iterator(extractDir))
    {
        if (entry.is_regular_file() && XContentFileSystem::check(entry.path()))
        {
            return entry.path();
        }
    }
    
    // If no STFS found, return the extract dir itself (might be a folder-based DLC)
    return extractDir;
}

static std::unique_ptr<VirtualFileSystem> createFileSystemFromPath(const std::filesystem::path &path)
{
    // Check for ZIP files first - extract and then parse
    if (toLower(fromPath(path.extension())) == ZIPExtension)
    {
        std::filesystem::path extractedPath = extractZipToTemp(path);
        if (!extractedPath.empty())
        {
            // Try to create file system from extracted content
            if (XContentFileSystem::check(extractedPath))
            {
                return XContentFileSystem::create(extractedPath);
            }
            else if (std::filesystem::is_directory(extractedPath))
            {
                return DirectoryFileSystem::create(extractedPath);
            }
        }
        return nullptr;
    }
    
    if (XContentFileSystem::check(path))
    {
        return XContentFileSystem::create(path);
    }
    else if (toLower(fromPath(path.extension())) == ISOExtension)
    {
        return ISOFileSystem::create(path);
    }
    else if (std::filesystem::is_directory(path))
    {
        return DirectoryFileSystem::create(path);
    }
    else
    {
        return nullptr;
    }
}

static bool checkFile(const FilePair &pair, const uint64_t *fileHashes, const std::filesystem::path &targetDirectory, std::vector<uint8_t> &fileData, Journal &journal, const std::function<bool()> &progressCallback, bool checkSizeOnly) {
    const std::string fileName(pair.first);
    const uint32_t hashCount = pair.second;
    const std::filesystem::path filePath = targetDirectory / fileName;
    if (!std::filesystem::exists(filePath))
    {
        journal.lastResult = Journal::Result::FileMissing;
        journal.lastErrorMessage = fmt::format("File {} does not exist.", fileName);
        return false;
    }

    std::error_code ec;
    size_t fileSize = std::filesystem::file_size(filePath, ec);
    if (ec)
    {
        journal.lastResult = Journal::Result::FileReadFailed;
        journal.lastErrorMessage = fmt::format("Failed to read file size for {}.", fileName);
        return false;
    }

    if (checkSizeOnly)
    {
        journal.progressTotal += fileSize;
    }
    else
    {
        std::ifstream fileStream(filePath, std::ios::binary);
        if (fileStream.is_open())
        {
            fileData.resize(fileSize);
            fileStream.read((char *)(fileData.data()), fileSize);
        }

        if (!fileStream.is_open() || fileStream.bad())
        {
            journal.lastResult = Journal::Result::FileReadFailed;
            journal.lastErrorMessage = fmt::format("Failed to read file {}.", fileName);
            return false;
        }

        // Skip hash validation if no hashes are specified (hashCount == 0)
        if (hashCount > 0)
        {
            uint64_t fileHash = XXH3_64bits(fileData.data(), fileSize);
            bool fileHashFound = false;
            for (uint32_t i = 0; i < hashCount && !fileHashFound; i++)
            {
                fileHashFound = fileHash == fileHashes[i];
            }

            if (!fileHashFound)
            {
                journal.lastResult = Journal::Result::FileHashFailed;
                journal.lastErrorMessage = fmt::format("File {} did not match any of the known hashes.", fileName);
                return false;
            }
        }

        journal.progressCounter += fileSize;
    }

    if (!progressCallback())
    {
        journal.lastResult = Journal::Result::Cancelled;
        journal.lastErrorMessage = "Check was cancelled.";
        return false;
    }

    return true;
}

static bool copyFile(const FilePair &pair, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, const std::filesystem::path &targetDirectory, bool skipHashChecks, std::vector<uint8_t> &fileData, Journal &journal, const std::function<bool()> &progressCallback) {
    const std::string filename(pair.first);
    const uint32_t hashCount = pair.second;
    if (!sourceVfs.exists(filename))
    {
        journal.lastResult = Journal::Result::FileMissing;
        journal.lastErrorMessage = fmt::format("File {} does not exist in {}.", filename, sourceVfs.getName());
        return false;
    }

    if (!sourceVfs.load(filename, fileData))
    {
        journal.lastResult = Journal::Result::FileReadFailed;
        journal.lastErrorMessage = fmt::format("Failed to read file {} from {}.", filename, sourceVfs.getName());
        return false;
    }

    if (!skipHashChecks && hashCount > 0)
    {
        uint64_t fileHash = XXH3_64bits(fileData.data(), fileData.size());
        bool fileHashFound = false;
        for (uint32_t i = 0; i < hashCount && !fileHashFound; i++)
        {
            fileHashFound = fileHash == fileHashes[i];
        }

        if (!fileHashFound)
        {
            journal.lastResult = Journal::Result::FileHashFailed;
            journal.lastErrorMessage = fmt::format("File {} from {} did not match any of the known hashes.", filename, sourceVfs.getName());
            return false;
        }
    }

    std::filesystem::path targetPath = targetDirectory / std::filesystem::path(std::u8string_view((const char8_t *)(pair.first)));
    std::filesystem::path parentPath = targetPath.parent_path();
    if (!std::filesystem::exists(parentPath))
    {
        std::error_code ec;
        std::filesystem::create_directories(parentPath, ec);
    }
    
    while (!parentPath.empty()) {
        journal.createdDirectories.insert(parentPath);

        if (parentPath != targetDirectory) {
            parentPath = parentPath.parent_path();
        }
        else {
            parentPath = std::filesystem::path();
        }
    }

    std::ofstream outStream(targetPath, std::ios::binary);
    if (!outStream.is_open())
    {
        journal.lastResult = Journal::Result::FileCreationFailed;
        journal.lastErrorMessage = fmt::format("Failed to create file at {}.", fromPath(targetPath));
        return false;
    }

    journal.createdFiles.push_back(targetPath);

    outStream.write((const char *)(fileData.data()), fileData.size());
    if (outStream.bad())
    {
        journal.lastResult = Journal::Result::FileWriteFailed;
        journal.lastErrorMessage = fmt::format("Failed to create file at {}.", fromPath(targetPath));
        return false;
    }

    journal.progressCounter += fileData.size();
    
    if (!progressCallback())
    {
        journal.lastResult = Journal::Result::Cancelled;
        journal.lastErrorMessage = "Installation was cancelled.";
        return false;
    }

    return true;
}

bool Installer::checkGameInstall(const std::filesystem::path &baseDirectory, std::filesystem::path &modulePath)
{
    modulePath = baseDirectory / GameDirectory / GameExecutableFile;

    if (!std::filesystem::exists(modulePath))
        return false;

    return true;
}


bool Installer::checkAllDLC(const std::filesystem::path& baseDirectory)
{
    return true;
}

bool Installer::checkInstallIntegrity(const std::filesystem::path &baseDirectory, Journal &journal, const std::function<bool()> &progressCallback)
{
    // Run the file checks twice: once to fill out the progress counter and the file sizes, and another pass to do the hash integrity checks.
    for (uint32_t checkPass = 0; checkPass < 2; checkPass++)
    {
        bool checkSizeOnly = (checkPass == 0);
        if (!checkFiles({ GameFiles, GameFilesSize }, GameHashes, baseDirectory / GameDirectory, journal, progressCallback, checkSizeOnly))
        {
            return false;
        }

    }

    return true;
}

bool Installer::computeTotalSize(std::span<const FilePair> filePairs, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, Journal &journal, uint64_t &totalSize)
{
    for (FilePair pair : filePairs)
    {
        const std::string filename(pair.first);
        if (!sourceVfs.exists(filename))
        {
            journal.lastResult = Journal::Result::FileMissing;
            journal.lastErrorMessage = fmt::format("File {} does not exist in {}.", filename, sourceVfs.getName());
            return false;
        }

        totalSize += sourceVfs.getSize(filename);
    }

    return true;
}

bool Installer::checkFiles(std::span<const FilePair> filePairs, const uint64_t *fileHashes, const std::filesystem::path &targetDirectory, Journal &journal, const std::function<bool()> &progressCallback, bool checkSizeOnly)
{
    FilePair validationPair = {};
    uint32_t validationHashIndex = 0;
    uint32_t hashIndex = 0;
    uint32_t hashCount = 0;
    std::vector<uint8_t> fileData;
    for (FilePair pair : filePairs)
    {
        hashIndex = hashCount;
        hashCount += pair.second;

        if (!checkFile(pair, &fileHashes[hashIndex], targetDirectory, fileData, journal, progressCallback, checkSizeOnly))
        {
            return false;
        }
    }

    return true;
}

bool Installer::copyFiles(std::span<const FilePair> filePairs, const uint64_t *fileHashes, VirtualFileSystem &sourceVfs, const std::filesystem::path &targetDirectory, const std::string &validationFile, bool skipHashChecks, Journal &journal, const std::function<bool()> &progressCallback)
{
    std::error_code ec;
    if (!std::filesystem::exists(targetDirectory) && !std::filesystem::create_directories(targetDirectory, ec))
    {
        journal.lastResult = Journal::Result::DirectoryCreationFailed;
        journal.lastErrorMessage = "Unable to create directory at " + fromPath(targetDirectory);
        return false;
    }

    uint32_t hashIndex = 0;
    uint32_t hashCount = 0;
    std::vector<uint8_t> fileData;
    for (FilePair pair : filePairs)
    {
        hashIndex = hashCount;
        hashCount += pair.second;

        if (!copyFile(pair, &fileHashes[hashIndex], sourceVfs, targetDirectory, skipHashChecks, fileData, journal, progressCallback))
        {
            return false;
        }
    }

    return true;
}

bool Installer::parseContent(const std::filesystem::path &sourcePath, std::unique_ptr<VirtualFileSystem> &targetVfs, Journal &journal)
{
    targetVfs = createFileSystemFromPath(sourcePath);
    if (targetVfs != nullptr)
    {
        return true;
    }
    else
    {
        journal.lastResult = Journal::Result::VirtualFileSystemFailed;
        journal.lastErrorMessage = "Unable to open " + fromPath(sourcePath);
        return false;
    }
}

bool Installer::parseSources(const Input &input, Journal &journal, Sources &sources)
{
    journal = Journal();
    sources = Sources();

    //Check if extracted rpf folders exist
    try {
        std::filesystem::path installerDir = std::filesystem::path(__FILE__).parent_path();
        std::filesystem::path privateDir = (installerDir / ".." / ".." / "GTA4RecompLib" / "private").lexically_normal();

        std::vector<std::string> required = { "Xbox360", "common", "audio" };
        for (const auto &name : required)
        {
            std::filesystem::path checkPath = (privateDir / name);
            if (!std::filesystem::exists(checkPath) || !std::filesystem::is_directory(checkPath))
            {
                journal.lastResult = Journal::Result::ValidationFileMissing;
                journal.lastErrorMessage = fmt::format("Required extracted RPF folder '{}' is missing under {}. You need to extract the RPF files.", name, fromPath(privateDir));
                return false;
            }
        }
    }
    catch (...) {
        journal.lastResult = Journal::Result::ValidationFileMissing;
        journal.lastErrorMessage = "Unable to locate GTA4RecompLib/private. You need to extract the RPF files.";
        return false;
    }

    // Parse the contents of the base game.
    if (!input.gameSource.empty())
    {
        if (!parseContent(input.gameSource, sources.game, journal))
        {
            return false;
        }

        // Use actual directory size instead of just the predefined file list
        uint64_t gameSize = sources.game->getTotalSize();
        if (gameSize > 0)
        {
            sources.totalSize += gameSize;
        }
        else
        {
            // Fallback to file list if getTotalSize not supported
            if (!computeTotalSize({ GameFiles, GameFilesSize }, GameHashes, *sources.game, journal, sources.totalSize))
            {
                return false;
            }
        }
    }

    // Add the total size in bytes as the journal progress.
    journal.progressTotal += sources.totalSize;

    return true;
}

bool Installer::install(const Sources &sources, const std::filesystem::path &targetDirectory, bool skipHashChecks, Journal &journal, std::chrono::seconds endWaitTime, const std::function<bool()> &progressCallback)
{
    // Install files in reverse order of importance. In case of a process crash or power outage, this will increase the likelihood of the installation
    // missing critical files required for the game to run. These files are used as the way to detect if the game is installed.

    if ((sources.game == nullptr))
    {
        journal.lastResult = Journal::Result::FileMissing;
        journal.lastErrorMessage = "No game source provided.";
        return false;
    }

    // Install the base game.
    if (!copyFiles({ GameFiles, GameFilesSize }, GameHashes, *sources.game, targetDirectory / GameDirectory, GameExecutableFile, skipHashChecks, journal, progressCallback))
    {
        return false;
    }
    
    
    // Ensure platform directories exist
    PlatformPaths::EnsureDirectoriesExist();
    
    // Auto-copy AES key if found in source or game directory
    // Check multiple possible locations for the AES key
    std::filesystem::path aesKeyDest = PlatformPaths::GetAesKeyPath();
    if (!std::filesystem::exists(aesKeyDest))
    {
        std::error_code ec;
        // Try to find aes_key.bin in common locations
        std::vector<std::filesystem::path> aesKeySearchPaths = {
            targetDirectory / GameDirectory / "aes_key.bin",
            targetDirectory / "aes_key.bin",
            std::filesystem::current_path() / "aes_key.bin",
            std::filesystem::current_path().parent_path() / "aes_key.bin"
        };
        
        // Also try source VFS
        if (sources.game && sources.game->exists("aes_key.bin"))
        {
            std::vector<uint8_t> keyData;
            if (sources.game->load("aes_key.bin", keyData) && keyData.size() >= 32)
            {
                std::ofstream outFile(aesKeyDest, std::ios::binary);
                if (outFile.is_open())
                {
                    outFile.write(reinterpret_cast<const char*>(keyData.data()), keyData.size());
                }
            }
        }
        else
        {
            // Search filesystem paths
            for (const auto& searchPath : aesKeySearchPaths)
            {
                if (std::filesystem::exists(searchPath, ec) && std::filesystem::file_size(searchPath, ec) >= 32)
                {
                    std::filesystem::copy_file(searchPath, aesKeyDest, std::filesystem::copy_options::overwrite_existing, ec);
                    if (!ec) break;
                }
            }
        }
    }
    

    // Convert shaders to platform-native format
    {
        ShaderConversionJournal shaderJournal;
        ShaderPlatform platform = ShaderConverter::detectPlatform();
        
        // Only convert if cache doesn't exist or is outdated
        auto shaderCacheDir = PlatformPaths::GetShaderCacheDirectory();
        std::filesystem::path gameDir = targetDirectory / GameDirectory;

        if (!ShaderConverter::isCacheValid(shaderCacheDir, gameDir))
        {
            ShaderConverter::convertShaders(gameDir, shaderCacheDir, platform, shaderJournal, progressCallback);
            
            // Shader conversion failure is non-fatal - game can still run with runtime compilation
            if (shaderJournal.lastResult != ShaderConversionJournal::Result::Success &&
                shaderJournal.lastResult != ShaderConversionJournal::Result::NoShadersFound)
            {
                // Log warning but continue
            }
        }
    }
    
    // Clean up temp files
    PlatformPaths::CleanupTemp();
    
    for (uint32_t i = 0; i < 2; i++)
    {
        if (!progressCallback())
        {
            journal.lastResult = Journal::Result::Cancelled;
            journal.lastErrorMessage = "Installation was cancelled.";
            return false;
        }

        if (i == 0)
        {
            // Wait the specified amount of time to allow the consumer of the callbacks to animate, halt or cancel the installation for a while after it's finished.
            std::this_thread::sleep_for(endWaitTime);
        }
    }

    return true;
}

void Installer::rollback(Journal &journal)
{
    std::error_code ec;
    for (const auto &path : journal.createdFiles)
    {
        std::filesystem::remove(path, ec);
    }

    for (auto it = journal.createdDirectories.rbegin(); it != journal.createdDirectories.rend(); it++)
    {
        std::filesystem::remove(*it, ec);
    }
}

bool Installer::parseGame(const std::filesystem::path &sourcePath)
{
    std::unique_ptr<VirtualFileSystem> sourceVfs = createFileSystemFromPath(sourcePath);
    if (sourceVfs == nullptr)
    {
        return false;
    }

    return sourceVfs->exists(GameExecutableFile);
}

