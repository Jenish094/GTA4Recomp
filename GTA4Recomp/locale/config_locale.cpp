#include <user/config.h>

//modifed from UnleashedRecomp


#define CONFIG_DEFINE_LOCALE(name) \
    CONFIG_LOCALE g_##name##_locale = 

#define CONFIG_DEFINE_ENUM_LOCALE(type) \
    CONFIG_ENUM_LOCALE(type) g_##type##_locale = 

CONFIG_DEFINE_LOCALE(Languge)
{
    { ELanguage::English, {"Language", "Change the language used for text and logos."} }
};

CONFIG_DEFINE_ENUM_LOCALE(ELanguage)
{
    {
        ELanguage::English,
        {
            { ELanguage::English,  {"ENGLISH", ""} }
        }
    }
};

CONFIG_DEGINE_LOCALE(VoiceLanguage)
{
    { EVoiceLanguage::English, {"Voice Language", "Change the language used for character voices."} }
};

CONFIG_DEFINE_ENUM_LOCALE(EVoiceLanguage)
{
    {
        ELanguage::English,
        {
            { EVoiceLanguage::English, {"ENGLISH", ""} }
        }
    }
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { true, {"Subtitles", "Enable or disable subtitles during dialogue."} }
};

CONFIG_DEFINE_LOCALE(Hints)
{
    { true,  {"Hints", "Show hints during gameplay"} }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { true,  {"Control Tutorial", "Show control tutorial prompts during gameplay."} }
};

CONFIG_DEFINE_LOCALE(Autosave)
{
    { true,  {"Autosave", "Enable or disable the autosave feature."} }
};

CONFIG_DEFINE_LOCALE(AchievementNotifications)
{
    { true,  {"Achievement Notifications", "Show notifications when achievements are unlocked."} }
};

CONFIG_DEFINE_LOCALE(HorizontalCamera)
{
    { false, {"Horizontal Camera", "Change how the camera moves left and right"} }
}; 

CONFIG_DEFINE_LOCALE(VerticalCamera)
{
    { false, {"Vertical Camera", "Change how the camera moves up and down"} }
};

CONFIG_DEFINE_LOCALE(ECameraRotationMode)
{
    {
        ELanguage::English,
        {
            { ECameraRotationMode::Normal, { "Normal", ""} },
            { ECameraRotationMode::Reverse, { "Reverse", ""} }
        }
    }
};

CONFIG_DEFINE_LOCALE(AllowBackgroundInput)
{
    { ELanguage::English, { "Allow Background Input", "Allow controller input whilst the game window is unfocused." } }
};

CONFIG_DEFINE_LOCALE(MouseSensitivityX)
{
    { ELanguage::English,  { "Mouse Sensitivity X", "Adjust horizontal mouse sensitivity." } }
};

CONFIG_DEFINE_LOCALE(MouseSensitivityY)
{
    { ELanguage::English,  { "Mouse Sensitivity Y", "Adjust vertical mouse sensitivity." } }
};

CONFIG_DEFINE_LOCALE(MouseInvertY)
{
    { ELanguage::English,  { "Invert Mouse Y", "Invert vertical mouse movement." } }
};

CONFIG_DEFINE_LOCALE(MouseSmoothing)
{
    { ELanguage::English,  { "Mouse Smoothing", "Adjust mouse smoothing amount." } }
};

CONFIG_DEFINE_LOCALE(ControllerIcons)
{
    { ELanguage::English,  { "Controller Icons", "Change the icons to match your controller." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EControllerIcons)
{
    {
        ELanguage::English,
        {
            { EControllerIcons::Auto,        { "Auto", "Auto: the game will determine which icons to use based on the current input device." } },
            { EControllerIcons::Xbox,        { "Xbox", "" } },
            { EControllerIcons::PlayStation, { "PlayStation", "" } }
        }
    }
};
CONFIG_DEFINE_LOCALE(LightDash)
{
    { ELanguage::English,  { "Light Dash", "Change how Light Dash is activated for Sonic and Shadow." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ELightDash)
{
    {
        ELanguage::English,
        {
            { ELightDash::X, { "Press ${picture(button_x)}", "" } },
            { ELightDash::Y, { "Press ${picture(button_y)}", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(SlidingAttack)
{
    { ELanguage::English,  { "Sliding Attack", "Change how the Sliding Attack is activated for Sonic." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ESlidingAttack)
{
    {
        ELanguage::English,
        {
            { ESlidingAttack::B, { "Hold ${picture(button_b)}", "" } },
            { ESlidingAttack::X, { "Release ${picture(button_x)}", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(MasterVolume)
{
    { ELanguage::English,  { "Master Volume", "Adjust the overall volume." } }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English,  { "Music Volume", "Adjust the volume for the music." } }
};

CONFIG_DEFINE_LOCALE(EffectsVolume)
{
    { ELanguage::English,  { "Effects Volume", "Adjust the volume for sound effects." } }
};

CONFIG_DEFINE_LOCALE(ChannelConfiguration)
{
    { ELanguage::English,  { "Channel Configuration", "Change the output mode for your audio device." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EChannelConfiguration)
{
    {
        ELanguage::English,
        {
            { EChannelConfiguration::Stereo,   { "Stereo", "" } },
            { EChannelConfiguration::Surround, { "Surround", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(MuteOnFocusLost)
{
    { ELanguage::English,  { "Mute on Focus Lost", "Mute the game's audio when the window is not in focus." } }
};

CONFIG_DEFINE_LOCALE(MusicAttenuation)
{
    { ELanguage::English,  { "Music Attenuation", "Fade out the game's music when external media is playing." } }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English,  { "Window Size", "Adjust the size of the game window in windowed mode." } }
};

CONFIG_DEFINE_LOCALE(Monitor)
{
    { ELanguage::English,  { "Monitor", "Change which monitor to display the game on." } }
};

CONFIG_DEFINE_LOCALE(AspectRatio)
{
    { ELanguage::English,  { "Aspect Ratio", "Change the aspect ratio." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAspectRatio)
{
    {
        ELanguage::English,
        {
            { EAspectRatio::Auto,     { "Auto", "Auto: the aspect ratio will dynamically adjust to the window size." } },
            { EAspectRatio::Original, { "Original", "Original: locks the game to a widescreen aspect ratio." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English,  { "Resolution Scale", "Adjust the internal resolution of the game." } }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English,  { "Fullscreen", "Toggle between borderless fullscreen or windowed mode." } }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English,  { "V-Sync", "Synchronize the game to the refresh rate of the display to prevent screen tearing." } }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English,  { "FPS", "Set the max frame rate the game can run at. WARNING: this may introduce glitches at frame rates other than 60 FPS." } }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English,  { "Brightness", "Adjust the brightness level." } }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English,  { "Anti-Aliasing", "Adjust the amount of smoothing applied to jagged edges." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAntiAliasing)
{
    {
        ELanguage::English,
        {
            { EAntiAliasing::Off, { "Off", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(TransparencyAntiAliasing)
{
    { ELanguage::English,  { "Transparency Anti-Aliasing", "Apply anti-aliasing to alpha transparent textures." } }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English,  { "Shadow Resolution", "Set the resolution of real-time shadows." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EShadowResolution) {};

CONFIG_DEFINE_LOCALE(ReflectionResolution)
{
    { ELanguage::English,  { "Reflection Resolution", "Set the resolution of real-time reflections." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EReflectionResolution)
{
    {
        ELanguage::English,
        {
            { EReflectionResolution::Eighth, { "12.5%", "" } },
            { EReflectionResolution::Quarter, { "25%", "" } },
            { EReflectionResolution::Half, { "50%", "" } },
            { EReflectionResolution::Full, { "100%", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(RadialBlur)
{
    { ELanguage::English,  { "Radial Blur", "Change the quality of the radial blur." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ERadialBlur)
{
    {
        ELanguage::English,
        {
            { ERadialBlur::Off,      { "Off", "" } },
            { ERadialBlur::Original, { "Original", "" } },
            { ERadialBlur::Enhanced, { "Enhanced", "Enhanced: uses more samples for smoother radial blur." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(CutsceneAspectRatio)
{
    { ELanguage::English,  { "Cutscene Aspect Ratio", "Change the aspect ratio of the real-time cutscenes." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ECutsceneAspectRatio)
{
    {
        ELanguage::English,
        {
            { ECutsceneAspectRatio::Original, { "Original", "Original: locks cutscenes to their original 16:9 aspect ratio." } },
            { ECutsceneAspectRatio::Unlocked, { "Unlocked", "Unlocked: allows cutscenes to adjust their aspect ratio to the window size. WARNING: this will introduce visual oddities past the original 16:9 aspect ratio." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(UIAlignmentMode)
{
    { ELanguage::English,  { "UI Alignment Mode", "Change how the UI aligns with the display." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EUIAlignmentMode)
{
    {
        ELanguage::English,
        {
            { EUIAlignmentMode::Edge,    { "Edge", "Edge: the UI will align with the edges of the display." } },
            { EUIAlignmentMode::Centre,  { "Center", "Center: the UI will align with the center of the display." } }
        }
    }
};