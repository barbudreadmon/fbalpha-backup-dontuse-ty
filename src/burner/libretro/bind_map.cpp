#include "bind_map.h"

#include "libretro.h"

unsigned init_bind_map(key_map bind_map[], bool gamepad_controls, bool newgen_controls)
{
   unsigned counter = 0;
   unsigned incr = 0;

   /* NOTE: The following buttons aren't mapped to the RetroPad:
    *
    * "Dip 1/2/3", "Dips", "Debug Dip", "Debug Dip 1/2", "Region",
    * "Service", "Service 1/2/3/4",
    * "Reset", "Volume Up/Down", "System", "Slots" and "Tilt"
    *
    * Mahjong/Poker controls aren't mapped since they require a keyboard
    * Excite League isn't mapped because it uses 11 buttons
    *
    * L3 is unmapped and could still be used */

   /* Universal controls */

   bind_map[PTR_INCR].bii_name = "Coin";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Coin 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Coin 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "Coin 3";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "Coin 4";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 3;

   bind_map[PTR_INCR].bii_name = "P1 Coin";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Coin";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P3 Coin";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P4 Coin";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   bind_map[PTR_INCR].nCode[1] = 3;

   bind_map[PTR_INCR].bii_name = "Start";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Start 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Start 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "Start 3";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "Start 4";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 3;

   bind_map[PTR_INCR].bii_name = "P1 Start";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Start";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P3 Start";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P4 Start";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 3;

   bind_map[PTR_INCR].bii_name = "P1 start";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 start";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_START;
   bind_map[PTR_INCR].nCode[1] = 1;

   /* Movement controls */

   bind_map[PTR_INCR].bii_name = "Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Up (Cocktail)";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Down (Cocktail)";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Left (Cocktail)";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Right (Cocktail)";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P3 Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P4 Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 3;

   bind_map[PTR_INCR].bii_name = "P4 Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 3;

   bind_map[PTR_INCR].bii_name = "P4 Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 3;

   bind_map[PTR_INCR].bii_name = "P4 Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 3;

   /* Angel Kids, Crazy Climber 2, Bullet, etc. */

   bind_map[PTR_INCR].bii_name = "P1 Left Stick Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Left Stick Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Left Stick Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Left Stick Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right Stick Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L2;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right Stick Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R2;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right Stick Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right Stick Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Rght Stick Up";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L2;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Rght Stick Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R2;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Rght Stick Left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Rght Stick Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Up 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Down 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Left 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Up 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L2;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Down 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R2;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Left 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Up 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Down 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Left 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Right 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Up 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L2;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Down 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R2;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Left 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Right 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P3 Up 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_UP;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Down 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_DOWN;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Left 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_LEFT;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Right 1";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Up 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L2;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Down 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R2;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Left 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Right 2";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
   bind_map[PTR_INCR].nCode[1] = 2;

   /* Analog controls
    *
    * FIXME: Analog controls still refuse to work properly */

   bind_map[PTR_INCR].bii_name = "Left/Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Up/Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Right / left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Up / Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Right / left";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Up / Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P1 Trackball X";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Trackball Y";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Trackball X";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Trackball Y";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "Target Left/Right";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Target Up/Down";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Turn";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Turn";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P1 Bat Swing";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Bat Swing";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P1 Handle";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Throttle";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Gun L-R";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Gun U-D";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "Stick X";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Stick Y";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   /* Light gun controls
    *
    * FIXME: Controls don't seem to work properly */

   bind_map[PTR_INCR].bii_name = "P1 X-Axis";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Y-Axis";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 X-Axis";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Y-Axis";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P3 X-Axis";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "P3 Y-Axis";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 2;

   bind_map[PTR_INCR].bii_name = "Crosshair X";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "Crosshair Y";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Gun X";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P1 Gun Y";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 0;

   bind_map[PTR_INCR].bii_name = "P2 Gun X";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_X;
   bind_map[PTR_INCR].nCode[1] = 1;

   bind_map[PTR_INCR].bii_name = "P2 Gun Y";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_ANALOG_Y;
   bind_map[PTR_INCR].nCode[1] = 1;

   /* Arcade stick friendly mapping */
   if (gamepad_controls == false)
   {
      /* General controls */

      bind_map[PTR_INCR].bii_name = "Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Button";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P1 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 5";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 6";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 5";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 6";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Space Harrier, 1942, Capcom Commando, Heavy Barrel, etc. */

      bind_map[PTR_INCR].bii_name = "Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 5";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 1 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 2 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 3 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 4 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 5 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Fire";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Tri-Pool */

      bind_map[PTR_INCR].bii_name = "Select Game 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Select Game 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Select Game 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Neo Geo */

      bind_map[PTR_INCR].bii_name = "P1 Button A";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button B";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button C";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button D";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Button A";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button B";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button C";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button D";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Street Fighter II, Darkstalkers, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Weak Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Weak Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Weak Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Medium Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Strong Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Weak Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Medium Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Strong Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

     /* Battle K-Road */

      bind_map[PTR_INCR].bii_name = "P1 Weak punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Weak kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Weak punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Medium punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Strong punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Weak kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Medium kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Strong kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

     /* Cyberbots: Full Metal Madness */

      bind_map[PTR_INCR].bii_name = "P1 Low Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 High Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Weapon";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Boost";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Low Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 High Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Weapon";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Boost";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Super Gem Fighter Mini Mix */

      bind_map[PTR_INCR].bii_name = "P1 Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Killer Instinct */

      /*
      bind_map[PTR_INCR].bii_name = "P1 Button A";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button B";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button C";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button X";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button Y";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button Z";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Button A";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button B";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button C";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button X";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button Y";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button Z";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;
      */

      /* Final Fight, Captain Commando, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* The Punisher */

      bind_map[PTR_INCR].bii_name = "P1 Super";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Super";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Saturday Night Slam Masters */

      bind_map[PTR_INCR].bii_name = "P1 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Dungeons & Dragons Tower of Doom/Shadow over Mystara */

      bind_map[PTR_INCR].bii_name = "P1 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Mercs, U.N. Squadron, Mega Twins, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Special";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Special";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Special";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 2;

      /* Dynasty Wars */

      bind_map[PTR_INCR].bii_name = "P1 Attack Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Attack Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Attack Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Attack Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Armed Police Batrider & Battle Bakraid */

      bind_map[PTR_INCR].bii_name = "P1 Shoot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shoot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shoot 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shoot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shoot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shoot 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Pang 3 */

      bind_map[PTR_INCR].bii_name = "P1 Shot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Mighty! Pang, Jong Pai Puzzle Choko and Jyangokushi: Haoh no Saihai */

      bind_map[PTR_INCR].bii_name = "P1 Shot1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shot2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shot3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shot2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Carrier Air Wing, Mars Matrix, Alien vs Predator, etc.
       *
       * NOTE: This button is shared between both shmups and brawlers
       * Alien vs. Predator and Armored Warriors received if statements as a workaround */

      bind_map[PTR_INCR].bii_name = "P1 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Varth, Giga Wing, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Bomb";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Bomb";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Enforce */

      bind_map[PTR_INCR].bii_name = "Laser";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Bomb";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Progear */

      bind_map[PTR_INCR].bii_name = "P1 Auto";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Auto";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Dimahoo */

      bind_map[PTR_INCR].bii_name = "P1 Shot (auto)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot (auto)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Eco Fighters and Pnickies */

      bind_map[PTR_INCR].bii_name = "P1 Turn 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Turn 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Turn 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Turn 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Last Survivor */

      bind_map[PTR_INCR].bii_name = "P1 Turn Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Turn Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Turn Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Turn Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* After Burner, Thunder Blade, etc. */

      bind_map[PTR_INCR].bii_name = "Missile";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Vulcan";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Cannon";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* OutRun, Chase HQ, Super Chase, Cyber Tank, Racing Beat, etc. */

      bind_map[PTR_INCR].bii_name = "Accelerate";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Accelerate";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Accel";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Brake";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Gear";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Nitro";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Turbo";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Super Charger";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Pit In";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Continental Circus */

      bind_map[PTR_INCR].bii_name = "Accelerate 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Accelerate 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R2;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Brake 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Brake 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L2;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Quiz & Dragons, Capcom World 2, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Answer 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Answer 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Answer 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Answer 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Answer 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Answer 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Answer 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Answer 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Super Puzzle Fighter II Turbo */

      bind_map[PTR_INCR].bii_name = "P1 Rotate Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Rotate Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Rotate Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Rotate Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Gals Pinball */

      bind_map[PTR_INCR].bii_name = "Launch Ball / Tilt";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Left Flippers";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Right Flippers";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

   }
   /* Gamepad friendly mapping */
   else
   {
     /* General controls */

      bind_map[PTR_INCR].bii_name = "Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Button";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P1 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 5";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button 6";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 5";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button 6";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Button 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Button 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Button 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Button 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Space Harrier, 1942, Capcom Commando, Heavy Barrel, etc. */

      bind_map[PTR_INCR].bii_name = "Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 5";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 1 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 2 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 3 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 4 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Fire 5 (Cocktail)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Fire";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Fire 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Fire 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Fire 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Tri-Pool */

      bind_map[PTR_INCR].bii_name = "Select Game 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Select Game 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Select Game 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Neo Geo default mapping */
      if (newgen_controls == false)
      {
         /* Official neogeo mapping */
         bind_map[PTR_INCR].bii_name = "P1 Button A";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P1 Button B";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P1 Button C";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P1 Button D";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P2 Button A";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
         bind_map[PTR_INCR].nCode[1] = 1;

         bind_map[PTR_INCR].bii_name = "P2 Button B";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
         bind_map[PTR_INCR].nCode[1] = 1;

         bind_map[PTR_INCR].bii_name = "P2 Button C";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
         bind_map[PTR_INCR].nCode[1] = 1;

         bind_map[PTR_INCR].bii_name = "P2 Button D";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
         bind_map[PTR_INCR].nCode[1] = 1;
      }
      /* NewGen neogeo mapping from DC, PS, Xbox, ... remakes */
      else
      {
         bind_map[PTR_INCR].bii_name = "P1 Button A";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P1 Button B";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P1 Button C";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P1 Button D";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
         bind_map[PTR_INCR].nCode[1] = 0;

         bind_map[PTR_INCR].bii_name = "P2 Button A";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
         bind_map[PTR_INCR].nCode[1] = 1;

         bind_map[PTR_INCR].bii_name = "P2 Button B";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
         bind_map[PTR_INCR].nCode[1] = 1;

         bind_map[PTR_INCR].bii_name = "P2 Button C";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
         bind_map[PTR_INCR].nCode[1] = 1;

         bind_map[PTR_INCR].bii_name = "P2 Button D";
         bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
         bind_map[PTR_INCR].nCode[1] = 1;
      }

      /* Street Fighter II, Darkstalkers, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Weak Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Weak Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Weak Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Medium Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Strong Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Weak Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Medium Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Strong Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Battle K-Road */

      bind_map[PTR_INCR].bii_name = "P1 Weak punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Weak kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Medium kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Strong kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Weak punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Medium punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Strong punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Weak kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Medium kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Strong kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Cyberbots: Full Metal Madness */

      bind_map[PTR_INCR].bii_name = "P1 Low Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 High Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Weapon";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Boost";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Low Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 High Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Weapon";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Boost";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Super Gem Fighter Mini Mix */

      bind_map[PTR_INCR].bii_name = "P1 Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Punch";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Kick";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Killer Instinct */

      /*
      bind_map[PTR_INCR].bii_name = "P1 Button A";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button B";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button C";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button X";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button Y";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Button Z";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Button A";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button B";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button C";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button X";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button Y";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Button Z";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;
      */

      /* Final Fight, Captain Commando, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Attack";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Jump";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* The Punisher */

      bind_map[PTR_INCR].bii_name = "P1 Super";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Super";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Saturday Night Slam Masters */

      bind_map[PTR_INCR].bii_name = "P1 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Pin";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Dungeons & Dragons Tower of Doom/Shadow over Mystara */

      bind_map[PTR_INCR].bii_name = "P1 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P3 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Select";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 3;

      bind_map[PTR_INCR].bii_name = "P4 Use";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Mercs, U.N. Squadron, Mega Twins, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Special";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Special";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Special";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 2;

      /* Dynasty Wars */

      bind_map[PTR_INCR].bii_name = "P1 Attack Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Attack Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Attack Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Attack Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Armed Police Batrider & Battle Bakraid */

      bind_map[PTR_INCR].bii_name = "P1 Shoot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shoot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shoot 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shoot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shoot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shoot 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Pang 3 */

      bind_map[PTR_INCR].bii_name = "P1 Shot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shot 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Mighty! Pang, Jong Pai Puzzle Choko and Jyangokushi: Haoh no Saihai */

      bind_map[PTR_INCR].bii_name = "P1 Shot1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shot2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Shot3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Shot2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Carrier Air Wing, Mars Matrix, Alien vs Predator, etc.
       *
       * NOTE: This button is shared between both shmups and brawlers
       * Alien vs. Predator and Armored Warriors received if statements as a workaround */

      bind_map[PTR_INCR].bii_name = "P1 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P3 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 2;

      bind_map[PTR_INCR].bii_name = "P4 Shot";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 3;

      /* Varth, Giga Wing, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Bomb";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Bomb";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Enforce */

      bind_map[PTR_INCR].bii_name = "Laser";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Bomb";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Progear */

      bind_map[PTR_INCR].bii_name = "P1 Auto";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Auto";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Dimahoo */

      bind_map[PTR_INCR].bii_name = "P1 Shot (auto)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Shot (auto)";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Eco Fighters and Pnickies */

      bind_map[PTR_INCR].bii_name = "P1 Turn 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Turn 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Turn 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Turn 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Last Survivor */

      bind_map[PTR_INCR].bii_name = "P1 Turn Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Turn Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Turn Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Turn Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* After Burner, Thunder Blade, etc. */

      bind_map[PTR_INCR].bii_name = "Missile";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Vulcan";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Cannon";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* OutRun, Chase HQ, Super Chase, Cyber Tank, Racing Beat, etc. */

      bind_map[PTR_INCR].bii_name = "Accelerate";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Accelerate";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Accel";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Brake";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Gear";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Nitro";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Turbo";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Super Charger";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Pit In";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Continental Circus */

      bind_map[PTR_INCR].bii_name = "Accelerate 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Accelerate 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R2;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Brake 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Brake 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_L2;
      bind_map[PTR_INCR].nCode[1] = 0;

      /* Quiz & Dragons, Capcom World 2, etc. */

      bind_map[PTR_INCR].bii_name = "P1 Answer 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Answer 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Answer 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Answer 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Answer 1";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Answer 2";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_X;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Answer 3";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Answer 4";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Super Puzzle Fighter II Turbo */

      bind_map[PTR_INCR].bii_name = "P1 Rotate Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P1 Rotate Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "P2 Rotate Left";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 1;

      bind_map[PTR_INCR].bii_name = "P2 Rotate Right";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 1;

      /* Gals Pinball */

      bind_map[PTR_INCR].bii_name = "Launch Ball / Tilt";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_Y;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Left Flippers";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_B;
      bind_map[PTR_INCR].nCode[1] = 0;

      bind_map[PTR_INCR].bii_name = "Right Flippers";
      bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_A;
      bind_map[PTR_INCR].nCode[1] = 0;
   }
   
   return counter;
}