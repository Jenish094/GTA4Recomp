#pragma once

#include <cstdint>

// replace sub_82120000 with a more modern init sequence
// these addresses were found using a person whos bad at reverse engineering (hint: its me) and IDA pro.
// sub_8218C600 core engine init
// sub_82120EE8 game manager init
// sub_821250B0 memory pool allocator
// sub_82318F60 RAGE string table lookup
// sub_82120FB8 63 subsystem initializations

namespace GameInitGlobals {
    
    // sub_82120000 game init
    constexpr uint32_t INIT_CONTEXT_ADDR   = 0x82126194;
    constexpr uint32_t POOL_PTR_ADDR       = 0x8305C1B0;
    constexpr uint32_t STRING_TABLE_ADDR   = 0x82129140;
    
    // sub_8218C600 core engine init
    constexpr uint32_t CORE_INIT_FLAG      = 0x8312579A;
    constexpr uint32_t GPU_STATE_1         = 0x83084044;
    constexpr uint32_t GPU_STATE_2         = 0x83085784;
    constexpr uint32_t GPU_STATE_3         = 0x83093764;
    constexpr uint32_t GPU_BUFFER_SIZE     = 0x83080A20;
    constexpr uint32_t ENGINE_VTABLE_PTR_1 = 0x830F500C;
    constexpr uint32_t ENGINE_VTABLE_VAL_1 = 0x830350D4;
    constexpr uint32_t ENGINE_VTABLE_PTR_2 = 0x83124E04;
    constexpr uint32_t ENGINE_VTABLE_VAL_2 = 0x82128F40;
    constexpr uint32_t ENGINE_VTABLE_PTR_3 = 0x83124E18;
    constexpr uint32_t ENGINE_VTABLE_PTR_4 = 0x83124CE8;
    constexpr uint32_t ENGINE_VTABLE_VAL_4 = 0x820009E0;
    constexpr uint32_t ENGINE_VTABLE_PTR_5 = 0x83124D9C;
    constexpr uint32_t ENGINE_VTABLE_VAL_5 = 0x820009DC;
    
    // sub_82120EE8 game manager init

    constexpr uint32_t GAME_MANAGER_PTR    = 0x831474B4;
    constexpr uint32_t GAME_MANAGER_SIZE   = 944;
    constexpr uint32_t WORLD_CONTEXT_PTR   = 0x831474B8;
    constexpr uint32_t WORLD_CONTEXT_SIZE  = 352;

    // sub_82120FB8 subsystem init
    constexpr uint32_t SUBSYS_STATE        = 0x83137654;
    constexpr uint32_t SUBSYS_FLAG_1       = 0x83137BB4;
    constexpr uint32_t SUBSYS_FLAG_2       = 0x83137BB6;
    constexpr uint32_t SUBSYS_COUNT = 63;
    
}

struct PPCContext;
namespace GameInit {
    bool InitCoreEngine(PPCContext& ctx, uint8_t* base);
    void InitGameManager(PPCContext& ctx, uint8_t* base);
    uint32_t AllocateFromPool(PPCContext& ctx, uint8_t* base, uint32_t poolPtr);
    void InitSubsystems(PPCContext& ctx, uint8_t* base);
    uint32_t Initialize(PPCContext& ctx, uint8_t* base);  
}
