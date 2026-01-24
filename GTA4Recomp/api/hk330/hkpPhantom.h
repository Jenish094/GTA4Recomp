#pragma once

#include <hk330/hkArray.h>
#include <hk330/hkpWorldObject.h>

namespace hk330
{
    class hkpPhantom : public hkpWorldObject
    {
    public:
        hkArray<xpointer<void>> m_overlapListeners;
        hkArray<xpointer<void>> m_phantomListeners;
    };

    FERNANDO_ASSERT_OFFSETOF(hkpPhantom, m_overlapListeners, 0x8C);
    FERNANDO_ASSERT_OFFSETOF(hkpPhantom, m_phantomListeners, 0x98);
}
