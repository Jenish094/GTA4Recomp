#include "exports.h"
#include <apu/embedded_player.h>
#include <kernel/function.h>
#include <kernel/heap.h>
#include <app.h>

void GamePlaySound(const char* pName_)
{
    if (EmbeddedPlayer::s_isActive)
    {
        EmbeddedPlayer::Play(pName_);
    }
}

void GamePlaySound(const char* pBankName, const char* pName)
{
    (void)pBankName;
    (void)pName;
}