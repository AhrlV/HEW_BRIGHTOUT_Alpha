/*=======================================================================


　キーボード入力の記録


========================================================================*/

#ifndef KEY_LOGGER
#define KEY_LOGGER

#include "keyboard.h"

void KeyLogger_Initialize();
void KeyLogger_Update();

bool KeyLogger_isPressed(Keyboard_Keys key);
bool KeyLogger_isTrigger(Keyboard_Keys key);
bool KeyLogger_isRelese(Keyboard_Keys key);


#endif // !KEY_LOGGER
