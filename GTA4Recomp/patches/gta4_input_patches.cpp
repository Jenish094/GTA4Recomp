// input translations
#include <stdafx.h>
#include <hid/hid.h>
#include <user/config.h>
#include <os/logger.h>
#include <kernel/xdm.h>

// xbox 360 controls
namespace GTA4Input
{
    // ABXY
    constexpr uint16_t BUTTON_A = 0x1000;
    constexpr uint16_t BUTTON_B = 0x2000; 
    constexpr uint16_t BUTTON_X = 0x4000;
    constexpr uint16_t BUTTON_Y = 0x8000;
    
    // DPAD
    constexpr uint16_t DPAD_UP = 0x0001;
    constexpr uint16_t DPAD_DOWN = 0x0002;
    constexpr uint16_t DPAD_LEFT = 0x0004;
    constexpr uint16_t DPAD_RIGHT = 0x0008;
    
    // LB RB L R
    constexpr uint16_t LB = 0x0100;
    constexpr uint16_t RB = 0x0200;
    constexpr uint16_t LSTICK = 0x0040;
    constexpr uint16_t RSTICK = 0x0080;
    
    // menu and view
    constexpr uint16_t START = 0x0010;
    constexpr uint16_t BACK = 0x0020;
    
    // LTRT
    constexpr uint8_t LT_THRESHOLD = 30;       // On foot: Aim/Lock-on | In car: Brake/Reverse
    constexpr uint8_t RT_THRESHOLD = 30;       // On foot: Shoot | In car: Accelerate
    
    // Deadzones based off gta4 default settings
    constexpr int16_t LEFT_STICK_DEADZONE = 7849;   // ~24% of max
    constexpr int16_t RIGHT_STICK_DEADZONE = 8689;  // ~26.5% of max
}

struct GTA4InputState
{
    uint16_t buttons = 0;
    int16_t leftStickX = 0;
    int16_t leftStickY = 0;
    int16_t rightStickX = 0;
    int16_t rightStickY = 0;
    uint8_t leftTrigger = 0;
    uint8_t rightTrigger = 0;
    
    bool isAiming = false;
    bool isShooting = false;
    bool isSprinting = false;
};

static GTA4InputState g_gta4InputState;

static int16_t ApplyDeadzone(int16_t value, int16_t deadzone = GTA4Input::LEFT_STICK_DEADZONE)
{
    if (abs(value) < deadzone)
        return 0;
    
    int16_t sign = value > 0 ? 1 : -1;
    int32_t magnitude = abs(value) - deadzone;
    int32_t scaledMax = 32767 - deadzone;
    
    return static_cast<int16_t>(sign * (magnitude * 32767 / scaledMax));
}

void GTA4_UpdateInputState()
{
    XAMINPUT_STATE state;
    if (hid::GetState(0, &state) != ERROR_SUCCESS)
    {
        memset(&g_gta4InputState, 0, sizeof(g_gta4InputState));
        return;
    }
    
    auto& pad = state.Gamepad;
    auto& gta = g_gta4InputState;
    
    gta.buttons = pad.wButtons;
    gta.leftStickX = ApplyDeadzone(pad.sThumbLX);
    gta.leftStickY = ApplyDeadzone(pad.sThumbLY);
    gta.rightStickX = ApplyDeadzone(pad.sThumbRX, 8689);
    gta.rightStickY = ApplyDeadzone(pad.sThumbRY, 8689);
    gta.leftTrigger = pad.bLeftTrigger;
    gta.rightTrigger = pad.bRightTrigger;
    gta.isAiming = pad.bLeftTrigger > GTA4Input::LT_THRESHOLD;
    gta.isShooting = pad.bRightTrigger > GTA4Input::RT_THRESHOLD;
    gta.isSprinting = (pad.wButtons & GTA4Input::BUTTON_A) != 0;
}

const GTA4InputState& GTA4_GetInputState()
{
    return g_gta4InputState;
}

bool GTA4_IsButtonDown(uint16_t button)
{
    return (g_gta4InputState.buttons & button) != 0;
}

static uint16_t g_prevButtons = 0;

bool GTA4_IsButtonPressed(uint16_t button)
{
    bool wasDown = (g_prevButtons & button) != 0;
    bool isDown = (g_gta4InputState.buttons & button) != 0;
    return isDown && !wasDown;
}

bool GTA4_IsButtonReleased(uint16_t button)
{
    bool wasDown = (g_prevButtons & button) != 0;
    bool isDown = (g_gta4InputState.buttons & button) != 0;
    return !isDown && wasDown;
}

void GTA4_UpdatePreviousState()
{
    g_prevButtons = g_gta4InputState.buttons;
}

void GTA4_SetVibration(uint16_t leftMotor, uint16_t rightMotor)
{
    XAMINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = leftMotor;
    vibration.wRightMotorSpeed = rightMotor;
    hid::SetState(0, &vibration);
}

void GTA4_StopVibration()
{
    GTA4_SetVibration(0, 0);
}

extern "C" uint32_t GTA4_XInputGetState(uint32_t dwUserIndex, void* pState)
{
    if (dwUserIndex != 0 || !pState)
        return ERROR_DEVICE_NOT_CONNECTED;
    
    GTA4_UpdateInputState();
    
    auto* gameState = reinterpret_cast<XAMINPUT_STATE*>(pState);
    
    XAMINPUT_STATE hostState;
    if (hid::GetState(0, &hostState) == ERROR_SUCCESS)
    {
        *gameState = hostState;
        return ERROR_SUCCESS;
    }
    
    return ERROR_DEVICE_NOT_CONNECTED;
}

void GTA4_InitInput()
{
    LOGN("GTA IV Input system initialised");
    memset(&g_gta4InputState, 0, sizeof(g_gta4InputState));
    g_prevButtons = 0;
}
