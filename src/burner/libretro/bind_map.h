#ifndef _BIND_MAP_H_
#define _BIND_MAP_H_

#define BIND_MAP_COUNT 305
#define PTR_INCR ((incr++ % 3 == 2) ? counter++ : counter)

// Ref GamcPlayer() in ../gamc.cpp
struct key_map
{
   const char *bii_name; // Generaly comes from struct BurnInputInfo.szName
   unsigned nCode[2];    // nCode[0] = RETRO_DEVICE_ID_XXXX from libretro.h, nCode[1] = port number from 0 to n
};

unsigned init_bind_map(key_map bind_map[], bool gamepad_controls, bool newgen_controls_p1, bool newgen_controls_p2, bool remap_lr_p1, bool remap_lr_p2);

#endif
