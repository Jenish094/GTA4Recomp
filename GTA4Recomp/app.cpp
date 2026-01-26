#include "app.h"
#include <gpu/video.h>
#include <install/installer.h>
#include <kernel/function.h>
#include <kernel/memory.h>
#include <os/process.h>
#include <os/logger.h>
#include <patches/patches.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <user/paths.h>
#include <user/registry.h>

static std::thread::id g_mainThreadID = std::this_thread::get_id();

void App::Restart (std::vector::<std::string> restartArgs)
{
    Config::Save();
}

void App::Exit()
{
#ifdef _WIN32
    timeEndPeriod(1);
#endif

    std::_Exit(0);
}

namespace GTA4FrameHooks
{
    static double s_lastFrameTime = 0.0;

    void OnFrameStart()
    {
        if (std::this_thread::get_id() == g_mainThreadId)
        {
            SDL_PumpEvents();
            SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
            GameWindow::Update();
        }
    }
    void OnFrameEnd(double deltaTime)
    {
        App::s_deltaTime = deltaTime;
        App::s_time += deltaTime;
        Video::WaitOnSwapChain();
    }
}
