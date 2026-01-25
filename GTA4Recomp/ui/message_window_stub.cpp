#include "message_window.h"

void MessageWindow::Draw()
{
// peak    
}

bool MessageWindow::Open(std::string text, int* result, std::span<std::string> buttons, int defaultButtonIndex, int cancelButtonIndex)
{
    if (result)
        *result = defaultButtonIndex;
    return MSG_CLOSED;
}

void MessageWindow::Close()
{
    s_isVisible = false;
}