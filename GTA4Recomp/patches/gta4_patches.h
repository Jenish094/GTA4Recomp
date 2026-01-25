#pragma once

namespace GTA4Patches
{
    void Init();
    void Update(double deltaTime);
}

// fps
namespace GTA4FPSPatches
{
    void Init();
    void FixPhysicsDeltaTime(double deltaTime);
    void FixAnimationSpeed(double deltaTime);
}

namespace GTA4ResolutionPatches
{
    void Init();
    
    uint32_t GetWidth();
    uint32_t GetHeight();
    float GetAspectRatio();
    void FixUIScale();
}

namespace GTA4InputPatches
{
    void Init();
    void UpdateInput();
}

namespace GTA4GraphicsPatches
{
    void Init();
    void FixShadows();
    void FixReflections();
}

namespace GTA4AudioPatches
{
    void Init();
}
