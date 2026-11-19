#include <Windows.h>
#include "MinHook.h"
#include <Xinput.h>

#if defined _M_X64
#pragma comment(lib, "libMinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook.x86.lib")
#endif

#pragma comment(lib, "Xinput.lib")

typedef int (WINAPI* pFnMessageBoxW)(HWND, LPCWSTR, LPCWSTR, UINT);
pFnMessageBoxW fpMessageBoxW = NULL;
int WINAPI Detour_MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    return fpMessageBoxW(hWnd, L"Hooked!", lpCaption, uType);
}

typedef DWORD (WINAPI * pXInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState) ;

pXInputGetState Original_XInputGetState = NULL;
DWORD WINAPI Detour_XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
    return Original_XInputGetState(dwUserIndex, pState);
}

int HookMessageBoxW()
{
    if (MH_Initialize() != MH_OK)
    {
        return 1;
    }

    if (MH_CreateHookApiEx(L"user32", "MessageBoxW", &Detour_MessageBoxW, (LPVOID*)&fpMessageBoxW, nullptr) != MH_OK)
    {
        return 1;
    }

    if (MH_EnableHook(&MessageBoxW) != MH_OK)
    {
        return 1;
    }

    MessageBoxW(NULL, L"Not hooked...", L"MinHook Sample", MB_OK);

    if (MH_DisableHook(&MessageBoxW) != MH_OK)
    {
        return 1;
    }

    MessageBoxW(NULL, L"Not hooked...", L"MinHook Sample", MB_OK);

    if (MH_Uninitialize() != MH_OK)
    {
        return 1;
    }
    
    return 0;
}

int HookXInputGetState()
{
    // Expected to tell "Hooked!".
    XINPUT_STATE data = { 0 };

    // Initialize MinHook.
    if (MH_Initialize() != MH_OK)
    {
        return 1;
    }

    pXInputGetState pHooked_Target = nullptr;
    MH_STATUS nCode = MH_STATUS::MH_OK;
    
    nCode = MH_CreateHook(&XInputGetState, &Detour_XInputGetState, reinterpret_cast<LPVOID*>(&Original_XInputGetState));
    // Create a hook for MessageBoxW, in disabled state.
    if (nCode != MH_OK)
    {
        return 1;
    }

    nCode = MH_EnableHook(&XInputGetState);
    if (nCode != MH_OK)
    {
        return 1;
    }

    DWORD dwRes = XInputGetState(0, &data);

    // Disable the hook for MessageBoxW.
    if (MH_DisableHook(&XInputGetState) != MH_OK)
    {
        return 1;
    }

    // Expected to tell "Not hooked...".
    dwRes = XInputGetState(0, &data);

    // Uninitialize MinHook.
    if (MH_Uninitialize() != MH_OK)
    {
        return 1;
    }
}

int main()
{

    HookXInputGetState();

    return 0;
}
