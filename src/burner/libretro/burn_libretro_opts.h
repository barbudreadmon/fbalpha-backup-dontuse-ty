#ifndef _LIBRETRO_OPTIMIZATIONS_H_
#define _LIBRETRO_OPTIMIZATIONS_H_

extern char g_rom_dir[1024];
extern char g_save_dir[1024];
extern char g_system_dir[1024];

extern unsigned int (__cdecl *BurnHighCol) (signed int r, signed int g, signed int b, signed int i);
#endif
