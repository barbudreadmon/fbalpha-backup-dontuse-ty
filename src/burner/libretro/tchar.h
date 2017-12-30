#ifndef __PORT_TYPEDEFS_H
#define __PORT_TYPEDEFS_H

#include <stdint.h>
#include <wchar.h>

#include "libretro.h"
#include "inp_keys.h"

#ifdef _MSC_VER
#include <winapifamily.h>
#include <string.h>
	#define strncasecmp _strnicmp
	#define strcasecmp _stricmp
	#define snprintf _snprintf
	#undef UNICODE
	#undef _UNICODE
#endif

#define _T(x) x
#define _tfopen rfopen
#define _tcstol strtol
#define _tcsstr strstr
#define _istspace(x) isspace(x)
#define _stprintf sprintf
#define _tcslen strlen
#define _tcsicmp(a, b) strcasecmp(a, b)
#define _tcscpy(to, from) strcpy(to, from)
#define _fgetts fgets
#define _strnicmp(s1, s2, n) strncasecmp(s1, s2, n)
#define _tcsncmp strncmp
#define _tcsncpy strncpy
#define _stscanf sscanf
#define _ftprintf fprintf

#ifdef UNICODE //Is there any point in this? Can we not just typedef TCHAR to CHAR?
	typedef wchar_t TCHAR;
#else
	typedef char	TCHAR;
#endif

#define _stricmp(x, y) strcasecmp(x,y)

#ifndef _MSC_VER
	typedef struct { int x, y, width, height; } RECT;
#else
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
	typedef struct { int x, y, width, height; } RECT;
#endif
#endif

#undef __cdecl
#define __cdecl
#endif

/* fastcall only works on x86_32 */
#ifndef FASTCALL
	#undef __fastcall
	#define __fastcall
#else
#ifndef _MSC_VER
	#undef __fastcall
	#define __fastcall __attribute__((fastcall))
#endif
#endif

#define ANSIToTCHAR(str, foo, bar) (str)

/* for Windows / Xbox 360 (below VS2010) - typedefs for missing stdint.h types such as uintptr_t?*/

/*FBA defines*/
#define PUF_TEXT_NO_TRANSLATE	(0)
#define PUF_TYPE_ERROR		(1)

extern TCHAR szAppBurnVer[16];

typedef int HWND;

extern int bDrvOkay;
extern int bRunPause;
extern bool bAlwaysProcessKeyboardInput;
extern HWND hScrnWnd;		// Handle to the screen window

extern void InpDIPSWResetDIPs (void);

/* undefine some system macro */
#ifdef PAGE_SHIFT
 #undef PAGE_SHIFT
#endif
#ifdef PAGE_SIZE
 #undef PAGE_SIZE
#endif
#ifdef PAGE_MASK
 #undef PAGE_MASK
#endif
