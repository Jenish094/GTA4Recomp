#pragma once

#include <api/Fernando.h>
#include <user/config.h>

class App
{
public:
    static inline bool s_isInit;
    static inline bool s_isSkipLogos;

    static inline GTA4::CGame* s_pApp;

    static inline EPlayerCharacter s_playerCharacter;
    static inline ELanguage s_language;

    static inline double s_deltaTime;
    static inline double s_time = 0.0;

    static void Restart(std::vector<std::string> restartArgs = {});
    static void Exit();
};
