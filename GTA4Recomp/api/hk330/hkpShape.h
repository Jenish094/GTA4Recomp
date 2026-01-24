#pragma once

#include <Fernando.inl>
#include <hk330/hkReferencedObject.h>

namespace hk330
{
    class hkpShape : public hkReferencedObject
    {
    public:
        be<uint32_t> m_userData;
        be<uint32_t> m_type;
    };

    FERNANDO_ASSERT_OFFSETOF(hkpShape, m_userData, 0x08);
    FERNANDO_ASSERT_OFFSETOF(hkpShape, m_type, 0x0C);
}
