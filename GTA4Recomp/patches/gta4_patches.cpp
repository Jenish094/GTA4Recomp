#include "gta4_patches.h"
#include "player_limit_patches.h"
#include <api/Fernando.h>
#include <gpu/video.h>
#include <hid/hid.h>
#include <user/config.h>
#include <app.h>
#include <kernel/function.h>
#include <kernel/memory.h>
#include <os/logger.h>

namespace GTA4Patches
{
    void Init()
    {
        GTA4FPSPatches::Init();
        GTA4ResolutionPatches::Init();
        GTA4InputPatches::Init();
        GTA4GraphicsPatches::Init();
        GTA4AudioPatches::Init();
    }
    
    void Update(double deltaTime)
    {
        GTA4InputPatches::UpdateInput();
        GTA4AudioPatches::UpdateAudio(deltaTime);
    }
}

// fps
namespace GTA4FPSPatches
{
    static constexpr double REFERENCE_DELTA_TIME_30 = 1.0 / 30.0;
    static constexpr double REFERENCE_DELTA_TIME_60 = 1.0 / 60.0;
    
    void Init()
    {
        // physics and animations need to be fixed since the game is ran at 30fps. I believe you need to hook in CTimer::Update to fix the timing
    }
    
    void FixPhysicsDeltaTime(double deltaTime)
    {
        // need to scale physics based on delta time
    }
    
    void FixAnimationSpeed(double deltaTime)
    {
        // need to scale animation speed based on delta time
    }
}

// res
namespace GTA4ResolutionPatches
{
    void Init()
    {
        // need to hook into resolution setting functions to force custom resolutions
    }
    
    uint32_t GetWidth()
    {
        return Video::s_viewportWidth;
    }
    
    uint32_t GetHeight()
    {
        return Video::s_viewportHeight;
    }
    
    float GetAspectRatio()
    {
        return static_cast<float>(GetWidth()) / static_cast<float>(GetHeight());
    }
    
    void FixUIScale()
    {
        // ui elements need to be scaled based on aspect ratio. 16:9 is defualt
    }
}

// input
namespace GTA4InputPatches
{
    void Init()
    {
        // yeah need to do this but xbox controllers should work fine
    }
    
    void UpdateInput()
    {

    }
}

// graphics
namespace GTA4GraphicsPatches
{
    // shadows and reflections need fix
    void Init()
    {

    }
    
    void FixShadows()
    {

    }
    
    void FixReflections()
    {

    }
}

// audio
namespace GTA4AudioPatches
{
    void Init()
    {

    }
}