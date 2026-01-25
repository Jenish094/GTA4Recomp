// bwoah - Kimi Raikkonen

#include <cstdint>
#include <cstdio>
#include <atomic>
#include <thread>

#include <stdafx.h>
#include <cpu/ppc_context.h>
#include <os/logger.h>

#include "game_init.h"

extern "C" void __imp__sub_8218C600(PPCContext& ctx, uint8_t* base);  // Core engine init
extern "C" void __imp__sub_82120EE8(PPCContext& ctx, uint8_t* base);  // Game manager init
extern "C" void __imp__sub_821250B0(PPCContext& ctx, uint8_t* base);  // Memory pool alloc
extern "C" void __imp__sub_82318F60(PPCContext& ctx, uint8_t* base);  // String table lookup
extern "C" void __imp__sub_82120FB8(PPCContext& ctx, uint8_t* base);  // Subsystem init (63)

// ByteSwap is provided by XenonUtils/byteswap.h in stdafx.h

#define PPC_STORE_U8(addr, val) \
    *(uint8_t*)(base + (addr)) = (uint8_t)(val)

#define PPC_STORE_U16(addr, val) \
    *(uint16_t*)(base + (addr)) = ByteSwap((uint16_t)(val))

#define PPC_STORE_U32(addr, val) \
    *(uint32_t*)(base + (addr)) = ByteSwap((uint32_t)(val))

#define PPC_LOAD_U32(addr) \
    ByteSwap(*(uint32_t*)(base + (addr)))


// this replacement writes all the expected global state and calls the pure logic functions

namespace GameInit {

bool InitCoreEngine(PPCContext& ctx, uint8_t* base)
{
    LOG_WARNING("[GameInit] initcoreengine starting...");
    ctx.r3.u64 = GameInitGlobals::INIT_CONTEXT_ADDR;
    __imp__sub_8218C600(ctx, base);
    
    bool success = (ctx.r3.u32 & 0xFF) != 0;
    LOGF_WARNING("[GameInit] initcoreengine {} (r3={})", 
                 success ? "SUCCEEDED" : "FAILED", ctx.r3.u32);
    
    return success;
}

void InitGameManager(PPCContext& ctx, uint8_t* base)
{
    LOG_WARNING("[GameInit] game manager init starting...");
    
    // For now, call the original function
    // TODO: Replace with direct implementation to avoid audio blocking
    __imp__sub_82120EE8(ctx, base);
    
    LOG_WARNING("[GameInit] game manager init completed");
}

uint32_t AllocateFromPool(PPCContext& ctx, uint8_t* base, uint32_t poolPtr)
{
    ctx.r3.u64 = poolPtr;
    __imp__sub_821250B0(ctx, base);
    return ctx.r3.u32;
}

void InitSubsystems(PPCContext& ctx, uint8_t* base)
{
    LOG_WARNING("[GameInit] initiating subsystems");

    PPC_STORE_U32(GameInitGlobals::SUBSYS_STATE, 0);
    PPC_STORE_U8(GameInitGlobals::SUBSYS_FLAG_1, 0);
    PPC_STORE_U8(GameInitGlobals::SUBSYS_FLAG_2, 0);

    __imp__sub_82120FB8(ctx, base);
    LOG_WARNING("[GameInit] initinged subsystems");
}

uint32_t Initialize(PPCContext& ctx, uint8_t* base)
{
    LOG_WARNING("[GameInit] starting game initialisation bwoah");
    if (!InitCoreEngine(ctx, base)) {
        LOG_WARNING("[GameInit] initcoreengine failed");
        return 0;
    }

    InitGameManager(ctx, base);
    
    LOG_WARNING("[GameInit] memory pool allocation...");
    uint32_t poolPtr = PPC_LOAD_U32(GameInitGlobals::POOL_PTR_ADDR);
    uint32_t allocResult = AllocateFromPool(ctx, base, poolPtr);
    LOGF_WARNING("[GameInit] Allocated from pool: 0x{:08X}", allocResult);
    PPC_STORE_U32(allocResult + 0, 0);
    PPC_STORE_U32(allocResult + 4, 0);
    ctx.r3.u64 = GameInitGlobals::STRING_TABLE_ADDR;
    __imp__sub_82318F60(ctx, base);
    uint32_t stringTable = ctx.r3.u32;
    PPC_STORE_U32(allocResult + 8, stringTable);
    PPC_STORE_U32(allocResult + 12, 0xFFFFFFFF);

    InitSubsystems(ctx, base);
    
    LOG_WARNING("[GameInit] yay it works");
    return 1;
}

}
