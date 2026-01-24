#include <utility>

extern const uint64_t GameHashes[];
extern const std::pair<const char *, uint32_t> GameFiles[];
extern const size_t GameFilesSize;

// In bytes. Sizes from the original gamefiles
static constexpr uint32_t SIZE_DEFAULT_XEX = 11841536;   // 11.3 MB
static constexpr uint32_t SIZE_AUDIO_RPF   = 782336;     // 764 KB  
static constexpr uint32_t SIZE_COMMON_RPF  = 17223680;   // 16.4 MB
static constexpr uint32_t SIZE_XBOX360_RPF = 60323840;   // 57.5 MB

const uint64_t GameHashes[] = {
    0ULL
};

const std::pair<const char *, uint32_t> GameFiles[] = {
    { "default.xex", 0 },
    { "audio.rpf", 0 },
    { "common.rpf", 0 },
    { "xbox360.rpf", 0 },
};

const size_t GameFilesSize = std::size(GameFiles);