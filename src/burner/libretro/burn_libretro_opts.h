#ifndef _LIBRETRO_OPTIMIZATIONS_H_
#define _LIBRETRO_OPTIMIZATIONS_H_

#ifdef USE_LIBRETRO_FILE32API
 #include "streams/file_stream_transforms.h"
#endif

extern unsigned int (__cdecl *BurnHighCol) (signed int r, signed int g, signed int b, signed int i);
#endif
