#pragma once

#include <Fernando.inl>
#include <hk330/hkArray.h>
#include <hk330/hkpCollidable.h>

namespace hk330
{
    class hkpLinkedCollidable : public hkpCollidable
    {
    public:
        struct CollisionEntry
        {
            FERNANDO_INSERT_PADDING(4);
            xpointer<hkpLinkedCollidable> m_partner;
        };

        hkArray<CollisionEntry> m_collisionEntries;
    };

    FERNANDO_ASSERT_OFFSETOF(hkpLinkedCollidable::CollisionEntry, m_partner, 0x04);

    FERNANDO_ASSERT_OFFSETOF(hkpLinkedCollidable, m_collisionEntries, 0x4C);
}
