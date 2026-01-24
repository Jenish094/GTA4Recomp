#pragma once

#include <Fernando.inl>

namespace hk330
{
    class hkReferencedObject
    {
    public:
        xpointer<void> m_pVftable;
        be<uint16_t> m_memSizeAndFlags;
        be<uint16_t> m_referenceCount;
    };

    FERNANDO_ASSERT_OFFSETOF(hkReferencedObject, m_pVftable, 0x00);
    FERNANDO_ASSERT_OFFSETOF(hkReferencedObject, m_memSizeAndFlags, 0x04);
    FERNANDO_ASSERT_OFFSETOF(hkReferencedObject, m_referenceCount, 0x06);
}
