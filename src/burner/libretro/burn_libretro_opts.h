#ifndef _LIBRETRO_OPTIMIZATIONS_H_
#define _LIBRETRO_OPTIMIZATIONS_H_

#include "streams/file_stream_transforms.h"

extern unsigned int (__cdecl *BurnHighCol) (signed int r, signed int g, signed int b, signed int i);

void RetroAchievementCallback(char* name, unsigned char *memory, unsigned int length);
#endif
