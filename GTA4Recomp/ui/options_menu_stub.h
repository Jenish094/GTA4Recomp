#include "options_menu.h"

void OptionsMenu::Init()
{
}

void OptionsMenu::Draw()
{
}

void OptionsMenu::Open(bool isPause)
{
    s_isPause = isPause;
    s_isVisible = true;
    s_state = OptionsMenuState::Opening;
}

void OptionsMenu::Close()
{
    s_isVisible = false;
    s_state = OptionsMenuState::Closing;
}

bool OptionsMenu::CanClose()
{
    return true;
}

bool OptionsMenu::IsRestartRequired()
{
    return false;
}

void OptionsMenu::SetFlowState(OptionsMenuFlowState flowState)
{
    s_flowState = flowState;
}