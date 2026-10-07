#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define ID_BTN_ATTACH        1001
#define ID_CHK_GOD           1002
#define ID_CHK_HEALTH        1003
#define ID_CHK_AMMO          1004
#define ID_CHK_OXYGEN        1005
#define ID_CHK_MONEY         1006
#define ID_CHK_SHIPHP        1007
#define ID_CHK_SHIPDMG       1008
#define ID_CHK_SHIPAMMO      1009
#define ID_CHK_SHIPWPCD      1010
#define ID_CHK_STEALTH       1011
#define ID_CHK_WANTED        1012
#define ID_CHK_CREWS         1013
#define ID_CHK_POTION        1014
#define ID_CHK_SMOKE         1015
#define ID_CHK_DAYTIME       1016
#define ID_EDIT_SPEED        1017
#define ID_BTN_SPEED         1018
#define ID_BTN_WAYPOINT      1019
#define ID_STATUS            1020
#define ID_TIMER             2001

HWND g_hwnd = NULL;
HWND g_hStatus = NULL;
HANDLE g_hProcess = NULL;
DWORD g_dwPID = 0;
uintptr_t g_modBase = 0;
size_t g_modSize = 0;

bool g_god = false, g_health = false, g_ammo = false, g_oxygen = false;
bool g_money = false, g_shiphp = false, g_shipdmg = false, g_shipammo = false;
bool g_shipwpcd = false, g_stealth = false, g_wanted = false, g_crews = false;
bool g_potion = false, g_smoke = false, g_daytime = false;

uintptr_t addr_god = 0, addr_health = 0, addr_ammo = 0, addr_oxygen = 0;
uintptr_t addr_money = 0, addr_shipdmg = 0, addr_shipammo = 0, addr_shipwpcd = 0;
uintptr_t addr_stealth = 0, addr_wanted = 0, addr_crews = 0, addr_potion = 0;
uintptr_t addr_smoke = 0, addr_daytime = 0, addr_gamespeed = 0, addr_waypoint = 0;
uintptr_t addr_player = 0, addr_shipptr = 0, addr_breakdmg = 0;

BYTE pat_god[] = {0x48,0x8B,0x41,0x20,0xF6,0x40,0x0B,0x80,0x74};
char  msk_god[] = "xxxxxxxxx";
BYTE pat_ammo[] = {0x45,0x85,0xC0,0x74,0x00,0x00,0x8D,0x00,0x00,0x00,0x8B,0x00,0x08};
char  msk_ammo[] = "xxxx??x??x?x?";
BYTE pat_oxygen[] = {0xC5,0xFA,0x10,0x49,0x00,0xC4};
char  msk_oxygen[] = "xxxx?x";
BYTE pat_oxygendrain[] = {0xC5,0xFA,0x11,0x86,0x00,0x00,0x00,0x00,0x80};
char  msk_oxygendrain[] = "xxxx????x";
BYTE pat_money[] = {0x8B,0x54,0x24,0x00,0x49,0x89,0xE9,0xE8};
char  msk_money[] = "xxx?xxxx";
BYTE pat_shipdmg[] = {0x48,0x89,0xF1,0xE8,0x00,0x00,0x00,0x00,0x00,0x00,0x7A,0x10};
char  msk_shipdmg[] = "xxxx??????xx";
BYTE pat_shipammo[] = {0x74,0x00,0x00,0x8D,0x00,0x24,0x00,0x00,0x00,0x00,0xC6};
char  msk_shipammo[] = "x??x?x????x";
BYTE pat_shipwpcd[] = {0x48,0x89,0x83,0x68,0x02,0x00,0x00};
char  msk_shipwpcd[] = "xxxxxxx";
BYTE pat_stealth[] = {0xBD,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x85};
char  msk_stealth[] = "xxxxx??????xx";
BYTE pat_wanted[] = {0x44,0x89,0xC3,0x89,0xD7,0x00,0x00,0x00,0xE8};
char  msk_wanted[] = "xxxxx???x";
BYTE pat_crews[] = {0xC5,0xFA,0x11,0x59,0x0C,0xC5,0xFA,0x11};
char  msk_crews[] = "xxxxxxxx";
BYTE pat_potion[] = {0xBA,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE8};
char  msk_potion[] = "xxxxx??????x";
BYTE pat_smoke[] = {0x45,0x85,0xC0,0x0F};
char  msk_smoke[] = "xxxx";
BYTE pat_daytime[] = {0xC5,0xFA,0x58,0x00,0xC5,0xFA,0x11,0x00,0xC5,0xFA,0x11,0x05};
char  msk_daytime[] = "xxxxxxxxxxxx";
BYTE pat_gamespeed[] = {0x80,0xBB,0x00,0x00,0x00,0x00,0x00,0x74};
char  msk_gamespeed[] = "xx????xx";
BYTE pat_waypoint[] = {0x48,0x8B,0x00,0x00,0x00,0x00,0x00,0xC5,0xF8,0x28,0x3C,0x08};
char  msk_waypoint[] = "xx?????xxxxx";
BYTE pat_player[] = {0x48,0x8B,0x89,0x00,0x00,0x00,0x00,0x48,0x83};
char  msk_player[] = "xxx????xx";
BYTE pat_shipptr[] = {0x41,0xB0,0x01,0xE8};
char  msk_shipptr[] = "xxxx";
BYTE pat_breakdmg[] = {0xC5,0xC2,0x5D,0xCE,0x48,0x83,0xC1,0x08};
char  msk_breakdmg[] = "xxxxxxxx";
BYTE pat_dmg[] = {0xC5,0xFA,0x10,0x4B,0x20,0x88};
char  msk_dmg[] = "xxxxxx";

bool MatchPattern(const BYTE* data, const BYTE* pat, const char* mask, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (mask[i] == 'x' && data[i] != pat[i]) return false;
    }
    return true;
}

uintptr_t PatternScan(HANDLE hProc, uintptr_t start, size_t size, const BYTE* pat, const char* mask) {
    size_t plen = strlen(mask);
    const size_t CHUNK = 0x10000;
    std::vector<BYTE> buf(CHUNK + plen);
    for (size_t offset = 0; offset < size; offset += CHUNK) {
        size_t toRead = (std::min)(CHUNK + plen, size - offset);
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProc, (LPCVOID)(start + offset), buf.data(), toRead, &bytesRead) || bytesRead < plen)
            continue;
        for (size_t i = 0; i + plen <= bytesRead; i++) {
            if (MatchPattern(buf.data() + i, pat, mask, plen))
                return start + offset + i;
        }
    }
    return 0;
}

DWORD FindProcess(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe = {sizeof(pe)};
    DWORD pid = 0;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, name) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

uintptr_t GetModuleBase(DWORD pid, const wchar_t* modName, size_t* outSize) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32W me = {sizeof(me)};
    uintptr_t base = 0;
    if (Module32FirstW(snap, &me)) {
        do {
            if (_wcsicmp(me.szModule, modName) == 0) {
                base = (uintptr_t)me.modBaseAddr;
                if (outSize) *outSize = me.modBaseSize;
                break;
            }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return base;
}

bool WriteBytes(uintptr_t addr, const BYTE* data, size_t len) {
    if (!g_hProcess || !addr) return false;
    DWORD old;
    VirtualProtectEx(g_hProcess, (LPVOID)addr, len, PAGE_EXECUTE_READWRITE, &old);
    SIZE_T written = 0;
    BOOL ok = WriteProcessMemory(g_hProcess, (LPVOID)addr, data, len, &written);
    VirtualProtectEx(g_hProcess, (LPVOID)addr, len, old, &old);
    return ok && written == len;
}

bool NopBytes(uintptr_t addr, size_t len) {
    std::vector<BYTE> nops(len, 0x90);
    return WriteBytes(addr, nops.data(), len);
}

bool WriteMem(uintptr_t addr, const void* buf, size_t len) {
    return WriteBytes(addr, (const BYTE*)buf, len);
}

void SetStatus(const wchar_t* msg) {
    if (g_hStatus) SetWindowTextW(g_hStatus, msg);
}

void ApplyGod(bool on) {
    if (!addr_god) return;
    if (on) {
        BYTE patch[] = {0x90,0x90};
        WriteBytes(addr_god + 0x08, patch, 2);
    } else {
        BYTE orig[] = {0x74};
        WriteBytes(addr_god + 0x08, orig, 1);
    }
}

void ApplyHealth(bool on) {
    if (!addr_god) return;
    if (on) {
        BYTE patch[] = {0x90,0x90,0x90,0x90,0x90,0x90};
        WriteBytes(addr_god + 0x14, patch, 6);
    }
}

void ApplyAmmo(bool on) {
    if (!addr_ammo) return;
    if (on) {
        BYTE patch[] = {0x90,0x90};
        WriteBytes(addr_ammo + 0x03, patch, 2);
    }
}

void ApplyOxygen(bool on) {
    if (!addr_oxygen) return;
    if (on) {
        BYTE patch[] = {0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};
        WriteBytes(addr_oxygen, patch, 8);
    }
}

void ApplyMoney(bool on) {
    if (!addr_money) return;
    if (on) {
        int val = 999999;
        WriteMem(addr_money + 0x03, &val, 4);
    }
}

void ApplyShipHP(bool on) {
    if (!addr_shipdmg) return;
    if (on) {
        BYTE patch[] = {0x90,0x90,0x90,0x90,0x90};
        WriteBytes(addr_shipdmg + 0x0A, patch, 5);
    }
}

void ApplyShipDmg(bool on) {
    if (!addr_breakdmg) return;
    if (on) {
        float mult = 10.0f;
        WriteMem(addr_breakdmg + 0x08, &mult, 4);
    }
}

void ApplyShipAmmo(bool on) {
    if (!addr_shipammo) return;
    if (on) {
        BYTE patch[] = {0x90,0x90};
        WriteBytes(addr_shipammo, patch, 2);
    }
}

void ApplyShipWPCD(bool on) {
    if (!addr_shipwpcd) return;
    if (on) {
        BYTE patch[] = {0x31,0xC0,0x90,0x90,0x90};
        WriteBytes(addr_shipwpcd + 0x07, patch, 5);
    }
}

void ApplyStealth(bool on) {
    if (!addr_stealth) return;
    if (on) {
        BYTE patch[] = {0x90,0x90,0x90,0x90,0x90,0x90};
        WriteBytes(addr_stealth + 0x0B, patch, 6);
    }
}

void ApplyWanted(bool on) {
    if (!addr_wanted) return;
    if (on) {
        BYTE patch[] = {0x31,0xFF,0x90};
        WriteBytes(addr_wanted + 0x03, patch, 3);
    }
}

void ApplyCrews(bool on) {
    if (!addr_crews) return;
    if (on) {
        float val = 100.0f;
        WriteMem(addr_crews + 0x04, &val, 4);
    }
}

void ApplyPotion(bool on) {
    if (!addr_potion) return;
    if (on) {
        BYTE patch[] = {0x90,0x90,0x90,0x90,0x90};
        WriteBytes(addr_potion + 0x05, patch, 5);
    }
}

void ApplySmoke(bool on) {
    if (!addr_smoke) return;
    if (on) {
        BYTE patch[] = {0x90,0x90,0x90,0x90,0x90,0x90};
        WriteBytes(addr_smoke + 0x03, patch, 6);
    }
}

void ApplyDaytime(bool on) {
    if (!addr_daytime) return;
    if (on) {
        BYTE patch[] = {0x90,0x90,0x90,0x90};
        WriteBytes(addr_daytime + 0x04, patch, 4);
    }
}

void ApplySpeed(float mult) {
    if (!addr_gamespeed) return;
    WriteMem(addr_gamespeed + 0x02, &mult, 4);
}

void DoWaypoint() {
    if (!addr_waypoint || !addr_player) return;
    float pos[3] = {0};
    if (ReadProcessMemory(g_hProcess, (LPCVOID)(addr_player + 0x50), pos, 12, NULL)) {
        WriteMem(addr_waypoint + 0x08, pos, 12);
        SetStatus(L"Waypoint set");
    }
}

bool Attach() {
    if (g_hProcess) {
        CloseHandle(g_hProcess);
        g_hProcess = NULL;
    }
    g_dwPID = FindProcess(L"ACBlackFlag.exe");
    if (!g_dwPID)
        g_dwPID = FindProcess(L"ACBlackFlag_Plus.exe");
    if (!g_dwPID) {
        SetStatus(L"Process not found");
        return false;
    }
    g_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, g_dwPID);
    if (!g_hProcess) {
        SetStatus(L"OpenProcess failed");
        return false;
    }
    g_modBase = GetModuleBase(g_dwPID, L"ACBlackFlag.exe", &g_modSize);
    if (!g_modBase)
        g_modBase = GetModuleBase(g_dwPID, L"ACBlackFlag_Plus.exe", &g_modSize);
    if (!g_modBase || !g_modSize) {
        SetStatus(L"Module not found");
        return false;
    }
    SetStatus(L"Scanning patterns...");
    addr_god = PatternScan(g_hProcess, g_modBase, g_modSize, pat_god, msk_god);
    addr_ammo = PatternScan(g_hProcess, g_modBase, g_modSize, pat_ammo, msk_ammo);
    addr_oxygen = PatternScan(g_hProcess, g_modBase, g_modSize, pat_oxygendrain, msk_oxygendrain);
    if (!addr_oxygen)
        addr_oxygen = PatternScan(g_hProcess, g_modBase, g_modSize, pat_oxygen, msk_oxygen);
    addr_money = PatternScan(g_hProcess, g_modBase, g_modSize, pat_money, msk_money);
    addr_shipdmg = PatternScan(g_hProcess, g_modBase, g_modSize, pat_shipdmg, msk_shipdmg);
    addr_shipammo = PatternScan(g_hProcess, g_modBase, g_modSize, pat_shipammo, msk_shipammo);
    addr_shipwpcd = PatternScan(g_hProcess, g_modBase, g_modSize, pat_shipwpcd, msk_shipwpcd);
    addr_stealth = PatternScan(g_hProcess, g_modBase, g_modSize, pat_stealth, msk_stealth);
    addr_wanted = PatternScan(g_hProcess, g_modBase, g_modSize, pat_wanted, msk_wanted);
    addr_crews = PatternScan(g_hProcess, g_modBase, g_modSize, pat_crews, msk_crews);
    addr_potion = PatternScan(g_hProcess, g_modBase, g_modSize, pat_potion, msk_potion);
    addr_smoke = PatternScan(g_hProcess, g_modBase, g_modSize, pat_smoke, msk_smoke);
    addr_daytime = PatternScan(g_hProcess, g_modBase, g_modSize, pat_daytime, msk_daytime);
    addr_gamespeed = PatternScan(g_hProcess, g_modBase, g_modSize, pat_gamespeed, msk_gamespeed);
    addr_waypoint = PatternScan(g_hProcess, g_modBase, g_modSize, pat_waypoint, msk_waypoint);
    addr_player = PatternScan(g_hProcess, g_modBase, g_modSize, pat_player, msk_player);
    addr_shipptr = PatternScan(g_hProcess, g_modBase, g_modSize, pat_shipptr, msk_shipptr);
    addr_breakdmg = PatternScan(g_hProcess, g_modBase, g_modSize, pat_breakdmg, msk_breakdmg);

    int found = 0;
    if (addr_god) found++;
    if (addr_ammo) found++;
    if (addr_oxygen) found++;
    if (addr_money) found++;
    if (addr_shipdmg) found++;
    if (addr_shipammo) found++;
    if (addr_shipwpcd) found++;
    if (addr_stealth) found++;
    if (addr_wanted) found++;
    if (addr_crews) found++;
    if (addr_potion) found++;
    if (addr_smoke) found++;
    if (addr_daytime) found++;
    if (addr_gamespeed) found++;
    if (addr_waypoint) found++;
    if (addr_player) found++;
    if (addr_shipptr) found++;
    if (addr_breakdmg) found++;

    wchar_t msg[128];
    swprintf_s(msg, L"Attached PID %lu | Patterns: %d/18", g_dwPID, found);
    SetStatus(msg);
    return true;
}

void OnTimer() {
    if (!g_hProcess) return;
    DWORD code = 0;
    if (!GetExitCodeProcess(g_hProcess, &code) || code != STILL_ACTIVE) {
        CloseHandle(g_hProcess);
        g_hProcess = NULL;
        SetStatus(L"Process closed");
        return;
    }
    if (g_god) ApplyGod(true);
    if (g_health) ApplyHealth(true);
    if (g_ammo) ApplyAmmo(true);
    if (g_oxygen) ApplyOxygen(true);
    if (g_money) ApplyMoney(true);
    if (g_shiphp) ApplyShipHP(true);
    if (g_shipdmg) ApplyShipDmg(true);
    if (g_shipammo) ApplyShipAmmo(true);
    if (g_shipwpcd) ApplyShipWPCD(true);
    if (g_stealth) ApplyStealth(true);
    if (g_wanted) ApplyWanted(true);
    if (g_crews) ApplyCrews(true);
    if (g_potion) ApplyPotion(true);
    if (g_smoke) ApplySmoke(true);
    if (g_daytime) ApplyDaytime(true);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        int y = 8;
        CreateWindowW(L"BUTTON", L"Attach to ACBlackFlag.exe", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, y, 300, 26, hwnd, (HMENU)ID_BTN_ATTACH, NULL, NULL);
        y += 30;
        CreateWindowW(L"BUTTON", L"God Mode", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_GOD, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Infinite Health", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            165, y, 145, 20, hwnd, (HMENU)ID_CHK_HEALTH, NULL, NULL);
        y += 22;
        CreateWindowW(L"BUTTON", L"Infinite Ammo", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_AMMO, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Infinite Oxygen", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            165, y, 145, 20, hwnd, (HMENU)ID_CHK_OXYGEN, NULL, NULL);
        y += 22;
        CreateWindowW(L"BUTTON", L"Money Multiplier", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_MONEY, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Ship Infinite HP", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            165, y, 145, 20, hwnd, (HMENU)ID_CHK_SHIPHP, NULL, NULL);
        y += 22;
        CreateWindowW(L"BUTTON", L"Ship Damage x10", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_SHIPDMG, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Ship Infinite Ammo", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            165, y, 145, 20, hwnd, (HMENU)ID_CHK_SHIPAMMO, NULL, NULL);
        y += 22;
        CreateWindowW(L"BUTTON", L"Ship No Weapon CD", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_SHIPWPCD, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Stealth Mode", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            165, y, 145, 20, hwnd, (HMENU)ID_CHK_STEALTH, NULL, NULL);
        y += 22;
        CreateWindowW(L"BUTTON", L"No Wanted Level", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_WANTED, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Max Crew", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            165, y, 145, 20, hwnd, (HMENU)ID_CHK_CREWS, NULL, NULL);
        y += 22;
        CreateWindowW(L"BUTTON", L"Infinite Potions", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_POTION, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Infinite Smoke Bombs", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            165, y, 145, 20, hwnd, (HMENU)ID_CHK_SMOKE, NULL, NULL);
        y += 22;
        CreateWindowW(L"BUTTON", L"Freeze Daytime", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, y, 145, 20, hwnd, (HMENU)ID_CHK_DAYTIME, NULL, NULL);
        y += 26;
        CreateWindowW(L"STATIC", L"Game Speed:", WS_CHILD | WS_VISIBLE, 10, y + 2, 75, 18, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"EDIT", L"1.0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            90, y, 50, 20, hwnd, (HMENU)ID_EDIT_SPEED, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Set", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            145, y, 40, 20, hwnd, (HMENU)ID_BTN_SPEED, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Set Waypoint", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            195, y, 100, 20, hwnd, (HMENU)ID_BTN_WAYPOINT, NULL, NULL);
        y += 28;
        g_hStatus = CreateWindowW(L"STATIC", L"Ready - Attach to game", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, y, 300, 20, hwnd, (HMENU)ID_STATUS, NULL, NULL);
        EnumChildWindows(hwnd, [](HWND h, LPARAM f) -> BOOL {
            SendMessageW(h, WM_SETFONT, (WPARAM)f, TRUE);
            return TRUE;
        }, (LPARAM)hFont);
        SetTimer(hwnd, ID_TIMER, 200, NULL);
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_BTN_ATTACH) {
            Attach();
        } else if (id == ID_CHK_GOD) {
            g_god = (IsDlgButtonChecked(hwnd, ID_CHK_GOD) == BST_CHECKED);
            ApplyGod(g_god);
        } else if (id == ID_CHK_HEALTH) {
            g_health = (IsDlgButtonChecked(hwnd, ID_CHK_HEALTH) == BST_CHECKED);
            ApplyHealth(g_health);
        } else if (id == ID_CHK_AMMO) {
            g_ammo = (IsDlgButtonChecked(hwnd, ID_CHK_AMMO) == BST_CHECKED);
            ApplyAmmo(g_ammo);
        } else if (id == ID_CHK_OXYGEN) {
            g_oxygen = (IsDlgButtonChecked(hwnd, ID_CHK_OXYGEN) == BST_CHECKED);
            ApplyOxygen(g_oxygen);
        } else if (id == ID_CHK_MONEY) {
            g_money = (IsDlgButtonChecked(hwnd, ID_CHK_MONEY) == BST_CHECKED);
            ApplyMoney(g_money);
        } else if (id == ID_CHK_SHIPHP) {
            g_shiphp = (IsDlgButtonChecked(hwnd, ID_CHK_SHIPHP) == BST_CHECKED);
            ApplyShipHP(g_shiphp);
        } else if (id == ID_CHK_SHIPDMG) {
            g_shipdmg = (IsDlgButtonChecked(hwnd, ID_CHK_SHIPDMG) == BST_CHECKED);
            ApplyShipDmg(g_shipdmg);
        } else if (id == ID_CHK_SHIPAMMO) {
            g_shipammo = (IsDlgButtonChecked(hwnd, ID_CHK_SHIPAMMO) == BST_CHECKED);
            ApplyShipAmmo(g_shipammo);
        } else if (id == ID_CHK_SHIPWPCD) {
            g_shipwpcd = (IsDlgButtonChecked(hwnd, ID_CHK_SHIPWPCD) == BST_CHECKED);
            ApplyShipWPCD(g_shipwpcd);
        } else if (id == ID_CHK_STEALTH) {
            g_stealth = (IsDlgButtonChecked(hwnd, ID_CHK_STEALTH) == BST_CHECKED);
            ApplyStealth(g_stealth);
        } else if (id == ID_CHK_WANTED) {
            g_wanted = (IsDlgButtonChecked(hwnd, ID_CHK_WANTED) == BST_CHECKED);
            ApplyWanted(g_wanted);
        } else if (id == ID_CHK_CREWS) {
            g_crews = (IsDlgButtonChecked(hwnd, ID_CHK_CREWS) == BST_CHECKED);
            ApplyCrews(g_crews);
        } else if (id == ID_CHK_POTION) {
            g_potion = (IsDlgButtonChecked(hwnd, ID_CHK_POTION) == BST_CHECKED);
            ApplyPotion(g_potion);
        } else if (id == ID_CHK_SMOKE) {
            g_smoke = (IsDlgButtonChecked(hwnd, ID_CHK_SMOKE) == BST_CHECKED);
            ApplySmoke(g_smoke);
        } else if (id == ID_CHK_DAYTIME) {
            g_daytime = (IsDlgButtonChecked(hwnd, ID_CHK_DAYTIME) == BST_CHECKED);
            ApplyDaytime(g_daytime);
        } else if (id == ID_BTN_SPEED) {
            wchar_t buf[32];
            GetDlgItemTextW(hwnd, ID_EDIT_SPEED, buf, 32);
            float v = (float)_wtof(buf);
            if (v < 0.1f) v = 0.1f;
            if (v > 20.0f) v = 20.0f;
            ApplySpeed(v);
            SetStatus(L"Game speed set");
        } else if (id == ID_BTN_WAYPOINT) {
            DoWaypoint();
        }
        break;
    }
    case WM_TIMER:
        if (wParam == ID_TIMER) OnTimer();
        break;
    case WM_DESTROY:
        KillTimer(hwnd, ID_TIMER);
        if (g_hProcess) CloseHandle(g_hProcess);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);
    WNDCLASSEXW wc = {sizeof(wc)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ACBlackFlagResyncedTrainer";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExW(&wc);
    g_hwnd = CreateWindowExW(0, L"ACBlackFlagResyncedTrainer", L"AC Black Flag Resynced Trainer v1.0-v1.0.x",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 340, 420, NULL, NULL, hInst, NULL);
    ShowWindow(g_hwnd, nShow);
    UpdateWindow(g_hwnd);
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
