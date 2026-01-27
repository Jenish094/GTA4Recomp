#include <cstdio>
#include <fmt/base.h>
#include <stdafx.h>
#include "xam.h"
#include "xdm.h"
#include <hid/hid.h>
#include <hid/mouse_camera.h>
#include <ui/game_window.h>
#include <cpu/guest_thread.h>
#include <ranges>
#include <unordered_set>
#include "xxHashMap.h"
#include <user/paths.h>
#include <SDL.h>

#define SAVE_SYSTEM_DEBUG_LOGGING 1

struct XamListener : KernelObject
{
    uint32_t id{};
    uint64_t areas{};
    std::vector<std::tuple<uint32_t, uint32_t>> notifications;

    XamListener(const XamListener&) = delete;
    XamListener& operator=(const XamListener&) = delete;

    XamListener();
    ~XamListener();
};

struct XamEnumeratorBase : KernelObject
{
    virtual uint32_t Next(void* buffer)
    {
        return -1;
    }
};

template<typename TIterator = std::vector<XHOSTCONTENT_DATA>::iterator>
struct XamEnumerator : XamEnumeratorBase
{
    uint32_t fetch;
    size_t size;
    TIterator position;
    TIterator begin;
    TIterator end;

    XamEnumerator() = default;
    XamEnumerator(uint32_t fetch, size_t size, TIterator begin, TIterator end) : fetch(fetch), size(size), position(begin), begin(begin), end(end)
    {

    }

    uint32_t Next(void* buffer) override
    {
        if (position == end)
        {
            return -1;
        }

        if (buffer == nullptr)
        {
            for (size_t i = 0; i < fetch; i++)
            {
                if (position == end)
                {
                    return i == 0 ? -1 : i;
                }

                ++position;
            }
        }

        for (size_t i = 0; i < fetch; i++)
        {
            if (position == end)
            {
                return i == 0 ? -1 : i;
            }

            memcpy(buffer, &*position, size);

            ++position;
            buffer = (void*)((size_t)buffer + size);
        }

        return fetch;
    }
};

std::array<xxHashMap<XHOSTCONTENT_DATA>, 3> gContentRegistry{};
std::unordered_set<XamListener*> gListeners{};
xxHashMap<std::string> gRootMap;

std::string_view XamGetRootPath(const std::string_view& root)
{
    const auto result = gRootMap.find(StringHash(root));

    if (result == gRootMap.end())
        return "";

    return result->second;
}

void XamRootCreate(const std::string_view& root, const std::string_view& path)
{
    gRootMap.emplace(StringHash(root), path);
}

XamListener::XamListener()
{
    gListeners.insert(this);
}

XamListener::~XamListener()
{
    gListeners.erase(this);
}

XCONTENT_DATA XamMakeContent(uint32_t type, const std::string_view& name)
{
    XCONTENT_DATA data{ 1, type };

    strncpy(data.szFileName, name.data(), sizeof(data.szFileName));

    return data;
}

void XamRegisterContent(const XCONTENT_DATA& data, const std::string_view& root)
{
    const auto idx = data.dwContentType - 1;

    gContentRegistry[idx].emplace(StringHash(data.szFileName), XHOSTCONTENT_DATA{ data }).first->second.szRoot = root;
}

void XamRegisterContent(uint32_t type, const std::string_view name, const std::string_view& root)
{
    XCONTENT_DATA data{ 1, type, {}, "" };

    strncpy(data.szFileName, name.data(), sizeof(data.szFileName));

    XamRegisterContent(data, root);
}

uint32_t XamNotifyCreateListener(uint64_t qwAreas)
{
    auto* listener = CreateKernelObject<XamListener>();

    listener->areas = qwAreas;

    return GetKernelHandle(listener);
}

void XamNotifyEnqueueEvent(uint32_t dwId, uint32_t dwParam)
{
    for (const auto& listener : gListeners)
    {
        if (((1 << MSG_AREA(dwId)) & listener->areas) == 0)
            continue;

        listener->notifications.emplace_back(dwId, dwParam);
    }
}

bool XNotifyGetNext(uint32_t hNotification, uint32_t dwMsgFilter, be<uint32_t>* pdwId, be<uint32_t>* pParam)
{
    auto& listener = *GetKernelObject<XamListener>(hNotification);

    if (dwMsgFilter)
    {
        for (size_t i = 0; i < listener.notifications.size(); i++)
        {
            if (std::get<0>(listener.notifications[i]) == dwMsgFilter)
            {
                if (pdwId)
                    *pdwId = std::get<0>(listener.notifications[i]);

                if (pParam)
                    *pParam = std::get<1>(listener.notifications[i]);

                listener.notifications.erase(listener.notifications.begin() + i);

                return true;
            }
        }
    }
    else
    {
        if (listener.notifications.empty())
            return false;

        if (pdwId)
            *pdwId = std::get<0>(listener.notifications[0]);

        if (pParam)
            *pParam = std::get<1>(listener.notifications[0]);

        listener.notifications.erase(listener.notifications.begin());

        return true;
    }

    return false;
}

uint32_t XamShowMessageBoxUI(uint32_t dwUserIndex, be<uint16_t>* wszTitle, be<uint16_t>* wszText, uint32_t cButtons,
    xpointer<be<uint16_t>>* pwszButtons, uint32_t dwFocusButton, uint32_t dwFlags, be<uint32_t>* pResult, XXOVERLAPPED* pOverlapped)
{
    *pResult = cButtons ? cButtons - 1 : 0;

#if _DEBUG
    assert("XamShowMessageBoxUI encountered!" && false);
#elif _WIN32

    std::vector<std::wstring> texts{};

    texts.emplace_back(reinterpret_cast<wchar_t*>(wszTitle));
    texts.emplace_back(reinterpret_cast<wchar_t*>(wszText));

    for (size_t i = 0; i < cButtons; i++)
        texts.emplace_back(reinterpret_cast<wchar_t*>(pwszButtons[i].get()));

    for (auto& text : texts)
    {
        for (size_t i = 0; i < text.size(); i++)
            ByteSwapInplace(text[i]);
    }

    for (size_t i = 0; i < cButtons; i++)
    {
        wprintf(L"%ls", texts[2 + i].c_str());

        if (i != cButtons - 1)
            wprintf(L" | ");
    }
#endif

    if (pOverlapped)
    {
        pOverlapped->dwCompletionContext = GuestThread::GetCurrentThreadId();
        pOverlapped->Error = 0;
        pOverlapped->Length = -1;
    }

    XamNotifyEnqueueEvent(9, 0);

    return 0;
}

uint32_t XamContentCreateEnumerator(uint32_t dwUserIndex, uint32_t DeviceID, uint32_t dwContentType,
    uint32_t dwContentFlags, uint32_t cItem, be<uint32_t>* pcbBuffer, be<uint32_t>* phEnum)
{
    if (dwUserIndex != 0)
    {
        GuestThread::SetLastError(ERROR_NO_SUCH_USER);
        return 0xFFFFFFFF;
    }

    const auto& registry = gContentRegistry[dwContentType - 1];
    const auto& values = registry | std::views::values;
    auto* enumerator = CreateKernelObject<XamEnumerator<decltype(values.begin())>>(cItem, sizeof(_XCONTENT_DATA), values.begin(), values.end());

    if (pcbBuffer)
        *pcbBuffer = sizeof(_XCONTENT_DATA) * cItem;

    *phEnum = GetKernelHandle(enumerator);

    return 0;
}

// stub for achievement enum, dont know if it works or not just need a working build first
uint32_t XamUserCreateAchievementEnumerator(uint32_t titleId, uint32_t userIndex, uint32_t xuidCount,
    void* pxuid, uint32_t dwStartingIndex, uint32_t cItem, be<uint32_t>* pcbBuffer, be<uint32_t>* phEnum)
{
    if (phEnum)
        *phEnum = 0;
    if (pcbBuffer)
        *pcbBuffer = 0;
    return ERROR_NO_MORE_FILES;
}

uint32_t XeKeysConsoleSignatureVerification(void* signature, uint32_t signatureSize, void* data, uint32_t dataSize)
{
    return 0;
}

uint32_t XamGetPrivateEnumStructureFromHandle(uint32_t hEnum, void** ppEnumData)
{
    if (ppEnumData)
        *ppEnumData = nullptr;
    return ERROR_INVALID_HANDLE;
}

uint32_t XamEnumerate(uint32_t hEnum, uint32_t dwFlags, void* pvBuffer, uint32_t cbBuffer, be<uint32_t>* pcItemsReturned, XXOVERLAPPED* pOverlapped)
{
    auto* enumerator = GetKernelObject<XamEnumeratorBase>(hEnum);
    const auto count = enumerator->Next(pvBuffer);

    if (count == -1)
        return ERROR_NO_MORE_FILES;

    if (pcItemsReturned)
        *pcItemsReturned = count;

    return ERROR_SUCCESS;
}

uint32_t XamContentCreateEx(uint32_t dwUserIndex, const char* szRootName, const XCONTENT_DATA* pContentData,
    uint32_t dwContentFlags, be<uint32_t>* pdwDisposition, be<uint32_t>* pdwLicenseMask,
    uint32_t dwFileCacheSize, uint64_t uliContentSize, PXXOVERLAPPED pOverlapped)
{
#if SAVE_SYSTEM_DEBUG_LOGGING
    printf("[XamContentCreateEx] Root: '%s', Content: '%s'\n", szRootName, pContentData->szFileName);
    printf("[XamContentCreateEx] Type: %u, Flags: 0x%X, User: %u\n", 
           pContentData->dwContentType, dwContentFlags, dwUserIndex);
#endif
    
    const auto& registry = gContentRegistry[pContentData->dwContentType - 1];
    const auto exists = registry.contains(StringHash(pContentData->szFileName));
    const auto mode = dwContentFlags & 0xF;

    if (mode == CREATE_ALWAYS)
    {
#if SAVE_SYSTEM_DEBUG_LOGGING
        printf("[XamContentCreateEx] Mode: CREATE_ALWAYS\n");
#endif
        if (pdwDisposition)
            *pdwDisposition = XCONTENT_NEW;

        if (!exists)
        {
            std::filesystem::path rootPath;

            if (pContentData->dwContentType == XCONTENTTYPE_SAVEDATA)
            {
                // Save data support removed
                return ERROR_INVALID_PARAMETER;
            }
            else if (pContentData->dwContentType == XCONTENTTYPE_DLC)
            {
                rootPath = GetGamePath() / "dlc";
            }
            else
            {
                rootPath = GetGamePath();
            }

            const std::string root = (const char*)rootPath.u8string().c_str();
            XamRegisterContent(*pContentData, root);

            std::error_code ec;
            std::filesystem::create_directory(rootPath, ec);
            
            if (ec)
            {
                printf("[XamContentCreateEx] ERROR: Failed to create directory: %s\n", ec.message().c_str());
            }

            XamRootCreate(szRootName, root);
#if SAVE_SYSTEM_DEBUG_LOGGING
            printf("[XamContentCreateEx] Created new content, root mapped: %s -> %s\n", szRootName, root.c_str());
#endif
        }
        else
        {
            XamRootCreate(szRootName, registry.find(StringHash(pContentData->szFileName))->second.szRoot);
#if SAVE_SYSTEM_DEBUG_LOGGING
            printf("[XamContentCreateEx] Content exists, using registered root\n");
#endif
        }

#if SAVE_SYSTEM_DEBUG_LOGGING
        printf("[XamContentCreateEx] Result: SUCCESS\n");
#endif
        return ERROR_SUCCESS;
    }

    if (mode == OPEN_EXISTING)
    {
#if SAVE_SYSTEM_DEBUG_LOGGING
        printf("[XamContentCreateEx] Mode: OPEN_EXISTING\n");
#endif
        if (exists)
        {
            if (pdwDisposition)
                *pdwDisposition = XCONTENT_EXISTING;

            XamRootCreate(szRootName, registry.find(StringHash(pContentData->szFileName))->second.szRoot);

#if SAVE_SYSTEM_DEBUG_LOGGING
            printf("[XamContentCreateEx] Content found and opened\n");
            printf("[XamContentCreateEx] Result: SUCCESS\n");
#endif
            return ERROR_SUCCESS;
        }
        else
        {
            if (pdwDisposition)
                *pdwDisposition = XCONTENT_NEW;

#if SAVE_SYSTEM_DEBUG_LOGGING
            printf("[XamContentCreateEx] Content not found\n");
            printf("[XamContentCreateEx] Result: ERROR_PATH_NOT_FOUND\n");
#endif
            return ERROR_PATH_NOT_FOUND;
        }
    }

#if SAVE_SYSTEM_DEBUG_LOGGING
    printf("[XamContentCreateEx] Unknown mode: 0x%X\n", mode);
    printf("[XamContentCreateEx] Result: ERROR_PATH_NOT_FOUND\n");
#endif
    return ERROR_PATH_NOT_FOUND;
}

uint32_t XamContentClose(const char* szRootName, XXOVERLAPPED* pOverlapped)
{
#if SAVE_SYSTEM_DEBUG_LOGGING
    printf("[XamContentClose] Closing root: '%s'\n", szRootName);
#endif
    
    gRootMap.erase(StringHash(szRootName));
    
    if (pOverlapped)
    {
        pOverlapped->dwCompletionContext = GuestThread::GetCurrentThreadId();
        pOverlapped->Error = 0;
        pOverlapped->Length = 0;
    }
    
    return 0;
}

uint32_t XamContentGetDeviceData(uint32_t DeviceID, XDEVICE_DATA* pDeviceData)
{
    pDeviceData->DeviceID = DeviceID;
    pDeviceData->DeviceType = XCONTENTDEVICETYPE_HDD;
    pDeviceData->ulDeviceBytes = 0x40000000;      // 1GB total
    pDeviceData->ulDeviceFreeBytes = 0x40000000;
    pDeviceData->wszName[0] = 'G';
    pDeviceData->wszName[1] = 'T';
    pDeviceData->wszName[2] = 'A';
    pDeviceData->wszName[3] = '4';
    pDeviceData->wszName[4] = '\0';

    return 0;
}

uint32_t XamContentGetDeviceState(uint32_t DeviceID, be<uint32_t>* pState)
{
    if (pState)
        *pState = 1;
    
    return ERROR_SUCCESS;
}

uint32_t XamInputGetCapabilities(uint32_t unk, uint32_t userIndex, uint32_t flags, XAMINPUT_CAPABILITIES* caps)
{
    if (userIndex != 0)
        return ERROR_NO_SUCH_USER;

    uint32_t result = hid::GetCapabilities(userIndex, caps);

    if (result == ERROR_SUCCESS)
    {
        ByteSwapInplace(caps->Flags);
        ByteSwapInplace(caps->Gamepad.wButtons);
        ByteSwapInplace(caps->Gamepad.sThumbLX);
        ByteSwapInplace(caps->Gamepad.sThumbLY);
        ByteSwapInplace(caps->Gamepad.sThumbRX);
        ByteSwapInplace(caps->Gamepad.sThumbRY);
        ByteSwapInplace(caps->Vibration.wLeftMotorSpeed);
        ByteSwapInplace(caps->Vibration.wRightMotorSpeed);
    }

    return result;
}

uint32_t XamInputGetState(uint32_t userIndex, uint32_t flags, XAMINPUT_STATE* state)
{
    if (userIndex != 0)
        return ERROR_NO_SUCH_USER;

    memset(state, 0, sizeof(*state));

    if (hid::IsInputAllowed())
        hid::GetState(userIndex, state);

    auto keyboardState = SDL_GetKeyboardState(NULL);
    auto mouseState = SDL_GetMouseState(nullptr, nullptr);

    // Keyboard to controller
    if (GameWindow::s_isFocused && !keyboardState[SDL_SCANCODE_LALT])
    {
        // Left stick
        if (keyboardState[Config::Key_LeftStickUp])
            state->Gamepad.sThumbLY = 32767;
        if (keyboardState[Config::Key_LeftStickDown])
            state->Gamepad.sThumbLY = -32768;
        if (keyboardState[Config::Key_LeftStickLeft])
            state->Gamepad.sThumbLX = -32768;
        if (keyboardState[Config::Key_LeftStickRight])
            state->Gamepad.sThumbLX = 32767;

        // Mouse to right stick
        if (keyboardState[Config::Key_RightStickUp])
            state->Gamepad.sThumbRY = 32767;
        if (keyboardState[Config::Key_RightStickDown])
            state->Gamepad.sThumbRY = -32768;
        if (keyboardState[Config::Key_RightStickLeft])
            state->Gamepad.sThumbRX = -32768;
        if (keyboardState[Config::Key_RightStickRight])
            state->Gamepad.sThumbRX = 32767;

        // Triggers to mouse
        if (keyboardState[Config::Key_LeftTrigger])
            state->Gamepad.bLeftTrigger = 0xFF;
        if (keyboardState[Config::Key_RightTrigger])
            state->Gamepad.bRightTrigger = 0xFF;

        // DPAD
        if (keyboardState[Config::Key_DPadUp])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_UP;
        if (keyboardState[Config::Key_DPadDown])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_DOWN;
        if (keyboardState[Config::Key_DPadLeft])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_LEFT;
        if (keyboardState[Config::Key_DPadRight])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_RIGHT;

        // start/back
        if (keyboardState[Config::Key_Start])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_START;
        if (keyboardState[Config::Key_Back])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_BACK;

        // LB/RB
        if (keyboardState[Config::Key_LeftBumper])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_LEFT_SHOULDER;
        if (keyboardState[Config::Key_RightBumper])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_RIGHT_SHOULDER;

        // ABXY
        if (keyboardState[Config::Key_A])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_A;
        if (keyboardState[Config::Key_B])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_B;
        if (keyboardState[Config::Key_X])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_X;
        if (keyboardState[Config::Key_Y])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_Y;
        // additional buttons just mapped to whatever they are in the pc version
        // R reload
        if (keyboardState[Config::Key_Reload])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_B;
        
        // C look back
        if (keyboardState[Config::Key_LookBehind])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_RIGHT_THUMB;
        
        // h horn
        if (keyboardState[Config::Key_Horn])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_LEFT_THUMB;
        
        // g headlight
        if (keyboardState[Config::Key_Headlight])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_X;
        
        // Left/Right radio
        if (keyboardState[Config::Key_RadioNext])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_RIGHT;
        if (keyboardState[Config::Key_RadioPrev])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_LEFT;
        
        // Mouse
        if (hid::g_inputDevice == hid::EInputDevice::Mouse || MouseCamera::IsActive())
        {
            if (mouseState & SDL_BUTTON_LMASK)
                state->Gamepad.bRightTrigger = 0xFF;
            if (mouseState & SDL_BUTTON_RMASK)
                state->Gamepad.bLeftTrigger = 0xFF;
            if (mouseState & SDL_BUTTON_MMASK)
                state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_Y;
            if (mouseState & SDL_BUTTON_X1MASK)
                state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_LEFT_SHOULDER;
            if (mouseState & SDL_BUTTON_X2MASK)
                state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_RIGHT_SHOULDER;
            int32_t wheelDelta = hid::GetMouseWheelDelta();
            if (wheelDelta > 0)
            {
                state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_RIGHT;
                hid::ResetMouseWheelDelta();
            }
            else if (wheelDelta < 0)
            {
                state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_LEFT;
                hid::ResetMouseWheelDelta();
            }
            int16_t mouseX, mouseY;
            MouseCamera::GetAnalogValues(mouseX, mouseY);
            if (MouseCamera::IsActive())
            {
                state->Gamepad.sThumbRX = mouseX;
                state->Gamepad.sThumbRY = mouseY;
            }
        }
    }

    state->Gamepad.wButtons &= ~hid::g_prohibitedButtons;

    if (hid::g_isLeftStickProhibited)
    {
        state->Gamepad.sThumbLX = 0;
        state->Gamepad.sThumbLY = 0;
    }

    if (hid::g_isRightStickProhibited)
    {
        state->Gamepad.sThumbRX = 0;
        state->Gamepad.sThumbRY = 0;
    }

    ByteSwapInplace(state->Gamepad.wButtons);
    ByteSwapInplace(state->Gamepad.sThumbLX);
    ByteSwapInplace(state->Gamepad.sThumbLY);
    ByteSwapInplace(state->Gamepad.sThumbRX);
    ByteSwapInplace(state->Gamepad.sThumbRY);

    return ERROR_SUCCESS;
}

uint32_t XamInputSetState(uint32_t userIndex, uint32_t flags, XAMINPUT_VIBRATION* vibration)
{
    if (userIndex != 0)
        return ERROR_NO_SUCH_USER;

    if (!hid::IsInputDeviceController())
        return ERROR_SUCCESS;

    ByteSwapInplace(vibration->wLeftMotorSpeed);
    ByteSwapInplace(vibration->wRightMotorSpeed);

    return hid::SetState(userIndex, vibration);
}

// profile Settings Stubs
uint32_t XamUserReadProfileSettings(uint32_t dwTitleId, uint32_t dwUserIndex, uint32_t dwNumSettingIds,
    const uint32_t* pdwSettingIds, be<uint32_t>* pcbResults, void* pResults, XXOVERLAPPED* pOverlapped)
{
#if SAVE_SYSTEM_DEBUG_LOGGING
    printf("[XamUserReadProfileSettings] TitleId: 0x%X, User: %u, NumSettings: %u\n",
           dwTitleId, dwUserIndex, dwNumSettingIds);
#endif
// havent done ts yet
    if (pcbResults)
        *pcbResults = 0;
    
    if (pOverlapped)
    {
        pOverlapped->dwCompletionContext = GuestThread::GetCurrentThreadId();
        pOverlapped->Error = 0;
        pOverlapped->Length = 0;
    }
    
    return ERROR_SUCCESS;
}

uint32_t XamUserWriteProfileSettings(uint32_t dwUserIndex, uint32_t dwNumSettings,
    const void* pSettings, XXOVERLAPPED* pOverlapped)
{
#if SAVE_SYSTEM_DEBUG_LOGGING
    printf("[XamUserWriteProfileSettings] User: %u, NumSettings: %u\n",
           dwUserIndex, dwNumSettings);
#endif
    // havent done ts yet either
    if (pOverlapped)
    {
        pOverlapped->dwCompletionContext = GuestThread::GetCurrentThreadId();
        pOverlapped->Error = 0;
        pOverlapped->Length = 0;
    }
    
    return ERROR_SUCCESS;
}

// always returns signed in user0
PPC_FUNC(__imp__XamUserGetSigninState)
{
    uint32_t dwUserIndex = ctx.r3.u32;
    
    if (dwUserIndex != 0)
        ctx.r3.u32 = 0;
    else
        ctx.r3.u32 = 1; //signed in and logged into xbl
}

uint32_t XamUserGetSigninInfo(uint32_t dwUserIndex, uint32_t dwFlags, void* pInfo)
{
    if (dwUserIndex != 0)
        return ERROR_NO_SUCH_USER;
    return ERROR_SUCCESS;
}