#include <api/Fernando.h>
#include <user/config.h>

static Sonicteam::SaveDataTaskXENON::SaveDataOperation g_currentAlert{};

// Sonicteam::SaveDataTaskXENON::RunOperation (speculatory)
PPC_FUNC_IMPL(__imp__sub_8238CB18);
PPC_FUNC(sub_8238CB18)
{
    return;
    // removed save data support due to unimplemntation
}

void SaveAlertThreeOptionRemoveDeviceSelect(PPCRegister& r5)
{
    auto options = (uint64_t*)g_memory.Translate(r5.u32 + 8);

    // The "Select storage device." option is always the
    // second index for the three option alert windows.
    options[2] = 0;
}
