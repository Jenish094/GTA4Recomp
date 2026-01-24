#pragma once

#include <Fernando.inl>
#include <hk330/hkpCdBody.h>
#include <hk330/hkpTypedBroadPhaseHandle.h>

namespace hk330
{
    class hkpCollidable : public hkpCdBody
    {
    public:
        FERNANDO_INSERT_PADDING(4);
        hkpTypedBroadPhaseHandle m_broadPhaseHandle;
        FERNANDO_INSERT_PADDING(0x28);
        be<float> m_allowedPenetrationDepth;
    };

    FERNANDO_ASSERT_OFFSETOF(hkpCollidable, m_broadPhaseHandle, 0x14);
    FERNANDO_ASSERT_OFFSETOF(hkpCollidable, m_allowedPenetrationDepth, 0x48);
}
