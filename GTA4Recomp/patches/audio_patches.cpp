#include <stdafx.h>
#include <api/Fernando.h>
#include <kernel/function.h>
#include <kernel/memory.h>
#include <os/media.h>
#include <os/version.h>
#include <os/logger.h>
#include <patches/audio_patches.h>
#include <user/config.h>
#include <app.h>

int AudioPatches::m_isAttenuationSupported = -1;

namespace gameaudio
{
    static float g_musicVolume = 1.0f;
    static float g_effectsVolume = 1.0f;
    static float g_masterVolume = 1.0f;
    
    static bool g_audioInitialized = false;
    static uint32_t g_audioEnginePtr = 0;
    
    float GetMusicVolume() { return g_musicVolume; }
    void SetMusicVolume(float volume) { g_musicVolume = std::clamp(volume, 0.0f, 1.0f); }
    
    float GetEffectsVolume() { return g_effectsVolume; }
    void SetEffectsVolume(float volume) { g_effectsVolume = std::clamp(volume, 0.0f, 1.0f); }
    
    float GetMasterVolume() { return g_masterVolume; }
    void SetMasterVolume(float volume) { g_masterVolume = std::clamp(volume, 0.0f, 1.0f); }
    
    bool IsInitialized() { return g_audioInitialized; }
    void SetInitialized(bool init) { g_audioInitialized = init; }
}

bool AudioPatches::CanAttenuate()
{
#if _WIN32
    if (m_isAttenuationSupported >= 0)
        return m_isAttenuationSupported;

    auto version = os::version::GetOSVersion();

    m_isAttenuationSupported = version.Major >= 10 && version.Build >= 17763;

    return m_isAttenuationSupported;
#elif __linux__
    return true;
#elif __APPLE__
    // macOS supports audio attenuation
    return true;
#else
    return false;
#endif
}

void AudioPatches::Update(float deltaTime)
{
    const float musicVolume = Config::MusicVolume * Config::MasterVolume;
    const float effectsVolume = Config::EffectsVolume * Config::MasterVolume;

    if (Config::MusicAttenuation && CanAttenuate())
    {
        auto time = 1.0f - expf(2.5f * -deltaTime);

        if (os::media::IsExternalMediaPlaying())
        {
            gameaudio::SetMusicVolume(std::lerp(gameaudio::GetMusicVolume(), 0.0f, time));
        }
        else
        {
            gameaudio::SetMusicVolume(std::lerp(gameaudio::GetMusicVolume(), musicVolume, time));
        }
    }
    else
    {
        gameaudio::SetMusicVolume(musicVolume);
    }

    gameaudio::SetEffectsVolume(effectsVolume);
    gameaudio::SetMasterVolume(Config::MasterVolume);
}

// audio system
void GTA4_AudioEngineInit(PPCRegister& result)
{
    gameaudio::SetInitialized(true);
    LOGN("GTA4 Audio: Engine initialised");
    result.u32 = 1;
}

void GTA4_SetMusicVolume(PPCRegister& volume)
{
    float vol = volume.f32;
    gameaudio::SetMusicVolume(vol);
    Config::MusicVolume = vol;
}

void GTA4_SetEffectsVolume(PPCRegister& volume)
{
    float vol = volume.f32;
    gameaudio::SetEffectsVolume(vol);
    Config::EffectsVolume = vol;
}

void GTA4_GetMusicVolume(PPCRegister& result)
{
    result.f32 = gameaudio::GetMusicVolume();
}

void GTA4_GetEffectsVolume(PPCRegister& result)
{
    result.f32 = gameaudio::GetEffectsVolume();
}

// audio playback
// TODO: Find actual address for audio stream creation
uint32_t GTA4_CreateAudioStream(PPCContext& ctx, uint8_t* base)
{
    ctx.r3.u32 = 0;
    return ctx.r3.u32;
}

// radio

namespace GTA4Radio
{
    static int g_currentStation = 0;
    static float g_radioVolume = 1.0f;
    static bool g_radioEnabled = true;
    
    int GetCurrentStation() { return g_currentStation; }
    void SetCurrentStation(int station) { g_currentStation = station; }
    
    float GetRadioVolume() { return g_radioVolume * gameaudio::GetMusicVolume(); }
    void SetRadioVolume(float volume) { g_radioVolume = std::clamp(volume, 0.0f, 1.0f); }
    
    bool IsRadioEnabled() { return g_radioEnabled; }
    void SetRadioEnabled(bool enabled) { g_radioEnabled = enabled; }
}

void GTA4_SetRadioStation(PPCRegister& station)
{
    int stationId = station.s32;
    GTA4Radio::SetCurrentStation(stationId);
}

void GTA4_SetRadioVolume(PPCRegister& volume)
{
    float vol = volume.f32;
    GTA4Radio::SetRadioVolume(vol);
}

// callback audio config

void AudioConfigChanged()
{
    gameaudio::SetMasterVolume(Config::MasterVolume);
    gameaudio::SetMusicVolume(Config::MusicVolume);
    gameaudio::SetEffectsVolume(Config::EffectsVolume);
}

// Audio system init hooks
extern "C" void __imp__sub_822EEDB8(PPCContext& ctx, uint8_t* base);
extern "C" void __imp__sub_821AB5F8(PPCContext& ctx, uint8_t* base);

// sub_822EEDB8
PPC_FUNC(sub_822EEDB8) {
    __imp__sub_822EEDB8(ctx, base);
        gameaudio::SetInitialized(true);
    LOG_INFO("[Audio] GTA4 audio system initialized");
}

// sub_821AB5F8
PPC_FUNC(sub_821AB5F8) {
    __imp__sub_821AB5F8(ctx, base);
    
    LOG_INFO("[Audio] GTA4 radio system initialized");
}
