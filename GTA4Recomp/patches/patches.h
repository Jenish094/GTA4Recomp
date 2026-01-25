#pragma once

#include "gta4_patches.h"
#include "player_limit_patches.h"
// disabled:
// - aspect_ratio_patches.h
// - camera_patches.h

inline void InitPatches()
{
    GTA4Patches::Init();
}
