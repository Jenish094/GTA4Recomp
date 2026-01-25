#pragma once

#include <cstdint>

namespace GTA4Input
{
    constexpr uint16_t BUTTON_A = 0x1000;
    constexpr uint16_t BUTTON_B = 0x2000;
    constexpr uint16_t BUTTON_X = 0x4000;
    constexpr uint16_t BUTTON_Y = 0x8000;
    constexpr uint16_t DPAD_UP = 0x0001;
    constexpr uint16_t DPAD_DOWN = 0x0002;
    constexpr uint16_t DPAD_LEFT = 0x0004;
    constexpr uint16_t DPAD_RIGHT = 0x0008;
    constexpr uint16_t LB = 0x0100;
    constexpr uint16_t RB = 0x0200;
    constexpr uint16_t LSTICK = 0x0040;
    constexpr uint16_t RSTICK = 0x0080;
    constexpr uint16_t START = 0x0010;
    constexpr uint16_t BACK = 0x0020;
    constexpr uint8_t LT_THRESHOLD = 30;
    constexpr uint8_t RT_THRESHOLD = 30;
    constexpr int16_t LEFT_STICK_DEADZONE = 7849;
    constexpr int16_t RIGHT_STICK_DEADZONE = 8689;
}

struct GTA4InputState
{
    uint16_t buttons;
    int16_t leftStickX;
    int16_t leftStickY;
    int16_t rightStickX;
    int16_t rightStickY;
    uint8_t leftTrigger;
    uint8_t rightTrigger;
    
    bool isAiming;
    bool isShooting;
    bool isSprinting;
};

void GTA4_InitInput();
void GTA4_UpdateInputState();
void GTA4_UpdatePreviousState();
const GTA4InputState& GTA4_GetInputState();
bool GTA4_IsButtonDown(uint16_t button);
bool GTA4_IsButtonPressed(uint16_t button);
bool GTA4_IsButtonReleased(uint16_t button);
void GTA4_SetVibration(uint16_t leftMotor, uint16_t rightMotor);
void GTA4_StopVibration();
extern "C" uint32_t GTA4_XInputGetState(uint32_t dwUserIndex, void* pState);
