#pragma once
#include <imgui.h>
#include <csdint>

namespace Style {
    namespace Colors {
        constexpr ImU32 Red = IM_COL32(199, 7, 7, 0.8);
        constexpr ImU32 Background = IM_COL32(139, 0, 0, 0.8);
        constexpr ImU32 BackgroundDark = IM_COL32(100, 0, 0, 0.8);
        constexpr ImU32 BackgroundPanel = IM_COL32(120, 0, 0, 0.8);
        constexpr ImU32 Border = IM_COL32(255, 255, 255, 0.2);

        constexpr ImU32 ProgressBarBG = IM_COL32(80, 0, 0, 0.8);
        constexpr ImU32 ProgressBarFill = IM_COL32(199, 7, 7, 1.0);
        constexpr ImU32 ProgressBarFillAlt = IM_COL32(255, 140, 0, 1.0);

        constexpr ImU32 RedText = IM_COL32(255, 100, 100, 1.0);
        constexpr ImU32 GreenText = IM_COL32(100, 255, 100, 1.0);
        constexpr ImU32 GreyText = IM_COL32(200, 200, 200, 1.0);
        constexpr ImU32 DarkRedText = IM_COL32(150, 0, 0, 1.0);
        constexpr ImU32 YellowText = IM_COL32(255, 255, 100, 1.0);
    };

    inline ImU32 WithAlpha(ImU32 color, float alpha)
    {
        uint8_t r = (color >> IM_COL32_R_SHIFT) & 0xFF;
        uint8_t g = (color >> IM_COL32_G_SHIFT) & 0xFF;
        uint8_t b = (color >> IM_COL32_B_SHIFT) & 0xFF;
        uint8_t a = static_cast<uint8_t>(((color >> IM_COL32_A_SHIFT) & 0xFF) * alpha);
        return IM_COL32(r, g, b, a);
    }
}