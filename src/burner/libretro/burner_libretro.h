#ifndef _BURNER_LIBRETRO_H
#define _BURNER_LIBRETRO_H

#include "gameinp.h"
#include "input/inp_keys.h"
#include "cd_interface.h"

#include "streams/file_stream_transforms.h"
#include "compat/msvc.h"

extern int bDrvOkay;
extern int bRunPause;
extern bool bAlwaysProcessKeyboardInput;

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

extern char g_rom_dir[MAX_PATH];

extern void InpDIPSWResetDIPs (void);

#endif
