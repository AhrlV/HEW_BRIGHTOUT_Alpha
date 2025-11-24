/*=======================================================================



========================================================================*/
#include "direct3D/key_logger.h"


static Keyboard_State g_PrevState{};
static Keyboard_State g_TriggerState{};
static Keyboard_State g_ReleseState{};


void KeyLogger_Initialize()
{
    Keyboard_Initialize();
}

void KeyLogger_Update()
{
    const Keyboard_State* state = Keyboard_GetState();
    LPBYTE pt = (LPBYTE)&g_TriggerState;
    LPBYTE pn = (LPBYTE)state;
    LPBYTE po = (LPBYTE)&g_PrevState;
    LPBYTE pr = (LPBYTE)&g_ReleseState;

    for (int i = 0; i < sizeof(Keyboard_State); i++) {
        pt[i] = (po[i] ^ pn[i]) & pn[i];
        pr[i] = po[i] & ~pn[i];
    }

    g_PrevState = *state;
}

bool KeyLogger_isPressed(Keyboard_Keys key)
{
    return Keyboard_IsKeyDown(key);
}

bool KeyLogger_isTrigger(Keyboard_Keys key)
{
    return Keyboard_IsKeyDown(key, &g_TriggerState);
}

bool KeyLogger_isRelese(Keyboard_Keys key)
{
    return Keyboard_IsKeyDown(key, &g_ReleseState);
}
