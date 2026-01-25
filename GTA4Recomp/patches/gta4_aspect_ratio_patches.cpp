// replacement for sonic 06 to gta4
#include "aspect_ratio_patches.h"
#include <gpu/video.h>

// empty maps
const xxHashMap<CsdModifier> g_csdModifiers{};
const xxHashMap<MovieModifier> g_movieModifiers{};

std::optional<CsdModifier> FindCsdModifier(uint32_t data)
{
    auto it = g_csdModifiers.find(data);
    if (it != g_csdModifiers.end())
        return it->second;
    return std::nullopt;
}

MovieModifier FindMovieModifier(XXH64_hash_t nameHash)
{
    auto it = g_movieModifiers.find(nameHash);
    if (it != g_movieModifiers.end())
        return it->second;
    return {};
}

void AspectRatioPatches::Init()
{
    ComputeOffsets();
}

void AspectRatioPatches::ComputeOffsets()
{
// current res
    auto width = static_cast<float>(Video::s_viewportWidth);
    auto height = static_cast<float>(Video::s_viewportHeight);
    
    // fallback to 720p
    if (width == 0 || height == 0)
    {
        width = 1280.0f;
        height = 720.0f;
    }
    
    // find aspect ratio
    g_aspectRatio = width / height;
    g_aspectRatioMovie = g_aspectRatio;
    
    // calc offsets and scales. default res is 1280x720
    constexpr float ORIGINAL_WIDTH = 1280.0f;
    constexpr float ORIGINAL_HEIGHT = 720.0f;
    
    g_aspectRatioScale = height / ORIGINAL_HEIGHT;
    
    if (g_aspectRatio >= WIDE_ASPECT_RATIO)
    {
        // if its wider than 16:9 add a pillarbox
        float scaledWidth = ORIGINAL_WIDTH * g_aspectRatioScale;
        g_aspectRatioOffsetX = (width - scaledWidth) / 2.0f;
        g_aspectRatioOffsetY = 0.0f;
    }
    else
    {
        // if its narrower than 16:9 add letterbox
        float targetWidth = height * WIDE_ASPECT_RATIO;
        g_aspectRatioScale = width / ORIGINAL_WIDTH;
        float scaledHeight = ORIGINAL_HEIGHT * g_aspectRatioScale;
        g_aspectRatioOffsetX = 0.0f;
        g_aspectRatioOffsetY = (height - scaledHeight) / 2.0f;
    }
    
    g_aspectRatioGameplayScale = g_aspectRatioScale;
    g_aspectRatioNarrowScale = 1.0f;
    g_aspectRatioNarrowMargin = 0.0f;
    g_aspectRatioMultiplayerOffsetX = 0.0f;
    
    g_horzCentre = width / 2.0f;
    g_vertCentre = height / 2.0f;
    
    g_radarMapScale = g_aspectRatioScale;
}
