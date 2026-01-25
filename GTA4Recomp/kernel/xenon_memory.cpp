#include "xenon_memory.h"
#include <os/logger.h>
#include <cstring>

void InitXenonMem(uint8_t* base) {
    memset(base + 0x82000000, 0, 0x20000); // clear the first 128KB region
    memset(base + 0x82020000, 0, 0x100000); // contains init tables and load data
    memset(base + 0x82A90000, 0, 0x10000); //kernel runtime
    memset(base + 0x83000000, 0, 0x1F0000);
}