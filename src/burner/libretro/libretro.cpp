#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "libretro.h"
#include "burner.h"

#include <file/file_path.h>
#include <streams/file_stream.h>

#include "cd/cd_interface.h"

#define FBA_VERSION "v0.2.97.42"

#define RETROPAD_CLASSIC	RETRO_DEVICE_JOYPAD
#define RETROPAD_MODERN		RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 1)

#if defined(_XBOX) || defined(_WIN32)
   char slash = '\\';
#else
   char slash = '/';
#endif

static void log_dummy(enum retro_log_level level, const char *fmt, ...) { }
static const char *print_label(unsigned i);


static void set_controller_infos();
static void set_environment();
static bool apply_dipswitch_from_variables();

static void init_macro_core_options();
static void init_macro_input_descriptors();
static void set_input_descriptors();
static bool apply_macro_from_variables();
static void evaluate_neogeo_bios_mode(const char* drvname);

static retro_environment_t environ_cb;
static retro_log_printf_t log_cb = log_dummy;
static retro_video_refresh_t video_cb;
static retro_input_poll_t poll_cb;
static retro_input_state_t input_cb;
static retro_audio_sample_batch_t audio_batch_cb;

#define BPRINTF_BUFFER_SIZE 512
char bprintf_buf[BPRINTF_BUFFER_SIZE];
static INT32 __cdecl libretro_bprintf(INT32 nStatus, TCHAR* szFormat, ...)
{
   va_list vp;
   va_start(vp, szFormat);
   int rc = vsnprintf(bprintf_buf, BPRINTF_BUFFER_SIZE, szFormat, vp);
   va_end(vp);

   if (rc >= 0)
   {
      retro_log_level retro_log = RETRO_LOG_DEBUG;
      if (nStatus == PRINT_UI)
         retro_log = RETRO_LOG_INFO;
      else if (nStatus == PRINT_IMPORTANT)
         retro_log = RETRO_LOG_WARN;
      else if (nStatus == PRINT_ERROR)
         retro_log = RETRO_LOG_ERROR;
         
      log_cb(retro_log, bprintf_buf);
   }
   
   return rc;
}

INT32 (__cdecl *bprintf) (INT32 nStatus, TCHAR* szFormat, ...) = libretro_bprintf;

// FBARL ---

extern UINT8 NeoSystem;
bool is_neogeo_game = false;
bool allow_neogeo_mode = true;
UINT16 switch_ncode = 0;

enum neo_geo_modes
{
   /* MVS */
   NEO_GEO_MODE_MVS = 0,

   /* AES */
   NEO_GEO_MODE_AES = 1,

   /* UNIBIOS */
   NEO_GEO_MODE_UNIBIOS = 2,

   /* DIPSWITCH */
   NEO_GEO_MODE_DIPSWITCH = 3,
};

#define MAX_KEYBINDS 0x5000
static uint8_t keybinds[MAX_KEYBINDS][4];
static uint8_t axibinds[5][8][3];
bool bAnalogRightMappingDone[5][2][2];

#define RETRO_DEVICE_ID_JOYPAD_EMPTY 255
static UINT8 diag_input_hold_frame_delay = 0;
static int   diag_input_combo_start_frame = 0;
static bool  diag_combo_activated = false;
static bool  one_diag_input_pressed = false;
static bool  all_diag_input_pressed = true;

static UINT8 *diag_input;
static UINT8 diag_input_start[] =       {RETRO_DEVICE_ID_JOYPAD_START,  RETRO_DEVICE_ID_JOYPAD_EMPTY };
static UINT8 diag_input_start_a_b[] =   {RETRO_DEVICE_ID_JOYPAD_START,  RETRO_DEVICE_ID_JOYPAD_A, RETRO_DEVICE_ID_JOYPAD_B, RETRO_DEVICE_ID_JOYPAD_EMPTY };
static UINT8 diag_input_start_l_r[] =   {RETRO_DEVICE_ID_JOYPAD_START,  RETRO_DEVICE_ID_JOYPAD_L, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_EMPTY };
static UINT8 diag_input_select[] =      {RETRO_DEVICE_ID_JOYPAD_SELECT, RETRO_DEVICE_ID_JOYPAD_EMPTY };
static UINT8 diag_input_select_a_b[] =  {RETRO_DEVICE_ID_JOYPAD_SELECT, RETRO_DEVICE_ID_JOYPAD_A, RETRO_DEVICE_ID_JOYPAD_B, RETRO_DEVICE_ID_JOYPAD_EMPTY };
static UINT8 diag_input_select_l_r[] =  {RETRO_DEVICE_ID_JOYPAD_SELECT, RETRO_DEVICE_ID_JOYPAD_L, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_EMPTY };

static unsigned int BurnDrvGetIndexByName(const char* name);

static neo_geo_modes g_opt_neo_geo_mode = NEO_GEO_MODE_MVS;
static bool core_aspect_par = false;

extern INT32 EnableHiscores;

#define STAT_NOFIND  0
#define STAT_OK      1
#define STAT_CRC     2
#define STAT_SMALL   3
#define STAT_LARGE   4

#define cpsx 1
#define neogeo 2

struct ROMFIND
{
   unsigned int nState;
   int nArchive;
   int nPos;
   BurnRomInfo ri;
};

static std::vector<std::string> g_find_list_path;
static ROMFIND g_find_list[1024];
static unsigned g_rom_count;
static unsigned fba_devices[5] = { RETROPAD_CLASSIC, RETROPAD_CLASSIC, RETROPAD_CLASSIC, RETROPAD_CLASSIC, RETROPAD_CLASSIC };

#define AUDIO_SAMPLERATE 48000
INT32 nAudSegLen = 0;

static uint32_t *g_fba_frame;
static int16_t *g_audio_buf;

#define JOY_NEG 0
#define JOY_POS 1

// Mapping of PC inputs to game inputs
struct GameInp* GameInp = NULL;
UINT32 nGameInpCount = 0;
UINT32 nMacroCount = 0;
UINT32 nMaxMacro = 0;
INT32 nAnalogSpeed;
INT32 nFireButtons = 0;
bool bStreetFighterLayout = false;
bool bButtonMapped = false;
bool bVolumeIsFireButton = false;

// libretro globals
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t) {}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_cb = cb; }

static const struct retro_variable var_empty = { NULL, NULL };

// Global core options
static const struct retro_variable var_fba_aspect = { "fba-aspect", "Core-provided aspect ratio; DAR|PAR" };
static const struct retro_variable var_fba_cpu_speed_adjust = { "fba-cpu-speed-adjust", "CPU overclock; 100|110|120|130|140|150|160|170|180|190|200" };
static const struct retro_variable var_fba_diagnostic_input = { "fba-diagnostic-input", "Diagnostic Input; None|Hold Start|Start + A + B|Hold Start + A + B|Start + L + R|Hold Start + L + R|Hold Select|Select + A + B|Hold Select + A + B|Select + L + R|Hold Select + L + R" };
static const struct retro_variable var_fba_hiscores = { "fba-hiscores", "Hiscores; enabled|disabled" };

// Neo Geo core options
static const struct retro_variable var_fba_neogeo_mode = { "fba-neogeo-mode", "Force Neo Geo mode (if available); MVS|AES|UNIBIOS|DIPSWITCH" };

void retro_set_environment(retro_environment_t cb)
{
	environ_cb = cb;
}

struct RomBiosInfo {
	char* filename;
	uint32_t crc;
	uint8_t NeoSystem;
	char* friendly_name;
	uint8_t priority;
};

static struct RomBiosInfo mvs_bioses[] = {
   {"asia-s3.rom",       0x91b64be3, 0x00, "MVS Asia/Europe ver. 6 (1 slot)",  1 },
   {"sp-s2.sp1",         0x9036d879, 0x01, "MVS Asia/Europe ver. 5 (1 slot)",  2 },
   {"sp-s.sp1",          0xc7f2fa45, 0x02, "MVS Asia/Europe ver. 3 (4 slot)",  3 },
   {"sp-u2.sp1",         0xe72943de, 0x03, "MVS USA ver. 5 (2 slot)"        ,  4 },
   {"v2.bin",            0x62f021f4, 0x04, "MVS USA ver. 5 (4 slot)"        ,  5 },
   {"sp-e.sp1",          0x2723a5b5, 0x05, "MVS USA ver. 5 (6 slot)"        ,  6 },
   {"vs-bios.rom",       0xf0e8f27d, 0x06, "MVS Japan ver. 6 (? slot)"      ,  7 },
   {"sp-j2.sp1",         0xacede59C, 0x07, "MVS Japan ver. 5 (? slot)"      ,  8 },
   {"sp1.jipan.1024",    0x9fb0abe4, 0x08, "MVS Japan ver. 3 (4 slot)"      ,  9 },
   {"sp-45.sp1",         0x03cc9f6a, 0x09, "NEO-MVH MV1C"                   , 10 },
   {"japan-j3.bin",      0xdff6d41f, 0x0a, "MVS Japan (J3)"                 , 11 },
   {"sp-1v1_3db8c.bin",  0x162f0ebe, 0x0e, "Deck ver. 6 (Git Ver 1.3)"      , 12 },
   {NULL, 0, 0, NULL, 0 }
};

static struct RomBiosInfo aes_bioses[] = {
   {"neo-epo.bin",       0xd27a71f1, 0x0c, "AES Asia"                       ,  1 },
   {"neo-po.bin",        0x16d0c132, 0x0b, "AES Japan"                      ,  2 },
   {"neodebug.bin",      0x698ebb7d, 0x0d, "Development Kit"                ,  3 },
   {NULL, 0, 0, NULL, 0 }
};

static struct RomBiosInfo uni_bioses[] = {
   {"uni-bios_3_2.rom",  0xa4e8b9b3, 0x0f, "Universe BIOS ver. 3.2"         ,  1 },
   {"uni-bios_3_1.rom",  0x0c58093f, 0x10, "Universe BIOS ver. 3.1"         ,  2 },
   {"uni-bios_3_0.rom",  0xa97c89a9, 0x11, "Universe BIOS ver. 3.0"         ,  3 },
   {"uni-bios_2_3.rom",  0x27664eb5, 0x12, "Universe BIOS ver. 2.3"         ,  4 },
   {"uni-bios_2_3o.rom", 0x601720ae, 0x13, "Universe BIOS ver. 2.3 (alt)"   ,  5 },
   {"uni-bios_2_2.rom",  0x2d50996a, 0x14, "Universe BIOS ver. 2.2"         ,  6 },
   {"uni-bios_2_1.rom",  0x8dabf76b, 0x15, "Universe BIOS ver. 2.1"         ,  7 },
   {"uni-bios_2_0.rom",  0x0c12c2ad, 0x16, "Universe BIOS ver. 2.0"         ,  8 },
   {"uni-bios_1_3.rom",  0xb24b44a0, 0x17, "Universe BIOS ver. 1.3"         ,  9 },
   {"uni-bios_1_2.rom",  0x4fa698e9, 0x18, "Universe BIOS ver. 1.2"         , 10 },
   {"uni-bios_1_2o.rom", 0xe19d3ce9, 0x19, "Universe BIOS ver. 1.2 (alt)"   , 11 },
   {"uni-bios_1_1.rom",  0x5dda0d84, 0x1a, "Universe BIOS ver. 1.1"         , 12 },
   {"uni-bios_1_0.rom",  0x0ce453a0, 0x1b, "Universe BIOS ver. 1.0"         , 13 },
   {NULL, 0, 0, NULL, 0 }
};

#if 0
static struct RomBiosInfo unknown_bioses[] = {
   {"neopen.sp1",        0xcb915e76, 0x1c, "NeoOpen BIOS v0.1 beta"         ,  1 },
   {NULL, 0, 0, NULL, 0 }
};
#endif

static RomBiosInfo *available_mvs_bios = NULL;
static RomBiosInfo *available_aes_bios = NULL;
static RomBiosInfo *available_uni_bios = NULL;

void set_neo_system_bios()
{
   if (g_opt_neo_geo_mode == NEO_GEO_MODE_DIPSWITCH)
   {
      // Nothing to do in DIPSWITCH mode because the NeoSystem variable is changed by the DIP Switch core option
      log_cb(RETRO_LOG_INFO, "DIPSWITCH Neo Geo Mode selected => NeoSystem: 0x%02x.\n", NeoSystem);
   }
   else if (g_opt_neo_geo_mode == NEO_GEO_MODE_MVS)
   {
      NeoSystem &= ~(UINT8)0x1f;
      if (available_mvs_bios)
      {
         NeoSystem |= available_mvs_bios->NeoSystem;
         log_cb(RETRO_LOG_INFO, "MVS Neo Geo Mode selected => Set NeoSystem: 0x%02x (%s [0x%08x] (%s)).\n", NeoSystem, available_mvs_bios->filename, available_mvs_bios->crc, available_mvs_bios->friendly_name);
      }
      else
      {
         // fallback to another bios type if we didn't find the bios selected by the user
         available_mvs_bios = (available_aes_bios) ? available_aes_bios : available_uni_bios;
         if (available_mvs_bios)
         {
            NeoSystem |= available_mvs_bios->NeoSystem;
            log_cb(RETRO_LOG_WARN, "MVS Neo Geo Mode selected but MVS bios not available => fall back to another: 0x%02x (%s [0x%08x] (%s)).\n", NeoSystem, available_mvs_bios->filename, available_mvs_bios->crc, available_mvs_bios->friendly_name);
         }
      }
   }
   else if (g_opt_neo_geo_mode == NEO_GEO_MODE_AES)
   {
      NeoSystem &= ~(UINT8)0x1f;
      if (available_aes_bios)
      {
         NeoSystem |= available_aes_bios->NeoSystem;
         log_cb(RETRO_LOG_INFO, "AES Neo Geo Mode selected => Set NeoSystem: 0x%02x (%s [0x%08x] (%s)).\n", NeoSystem, available_aes_bios->filename, available_aes_bios->crc, available_aes_bios->friendly_name);
      }
      else
      {
         // fallback to another bios type if we didn't find the bios selected by the user
         available_aes_bios = (available_mvs_bios) ? available_mvs_bios : available_uni_bios;
         if (available_aes_bios)
         {
            NeoSystem |= available_aes_bios->NeoSystem;
            log_cb(RETRO_LOG_WARN, "AES Neo Geo Mode selected but AES bios not available => fall back to another: 0x%02x (%s [0x%08x] (%s)).\n", NeoSystem, available_aes_bios->filename, available_aes_bios->crc, available_aes_bios->friendly_name);
         }
      }      
   }
   else if (g_opt_neo_geo_mode == NEO_GEO_MODE_UNIBIOS)
   {
      NeoSystem &= ~(UINT8)0x1f;
      if (available_uni_bios)
      {
         NeoSystem |= available_uni_bios->NeoSystem;
         log_cb(RETRO_LOG_INFO, "UNIBIOS Neo Geo Mode selected => Set NeoSystem: 0x%02x (%s [0x%08x] (%s)).\n", NeoSystem, available_uni_bios->filename, available_uni_bios->crc, available_uni_bios->friendly_name);
      }
      else
      {
         // fallback to another bios type if we didn't find the bios selected by the user
         available_uni_bios = (available_mvs_bios) ? available_mvs_bios : available_aes_bios;
         if (available_uni_bios)
         {
            NeoSystem |= available_uni_bios->NeoSystem;
            log_cb(RETRO_LOG_WARN, "UNIBIOS Neo Geo Mode selected but UNIBIOS not available => fall back to another: 0x%02x (%s [0x%08x] (%s)).\n", NeoSystem, available_uni_bios->filename, available_uni_bios->crc, available_uni_bios->friendly_name);
         }
      }
   }
}

char g_base_name[128];
char g_rom_dir[1024];
char g_save_dir[1024];
char g_system_dir[1024];
extern unsigned int (__cdecl *BurnHighCol) (signed int r, signed int g, signed int b, signed int i);

static bool driver_inited;

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "FB Alpha";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
   info->library_version = FBA_VERSION GIT_VERSION;
   info->need_fullpath = true;
   info->block_extract = true;
   info->valid_extensions = "iso|zip|7z";
}

/////
static INT32 InputTick();
static void InputMake();
static bool init_input();
static void check_variables();

void wav_exit() { }

// FBA stubs
unsigned ArcadeJoystick;

int bDrvOkay;
int bRunPause;
bool bAlwaysProcessKeyboardInput;

bool bDoIpsPatch;
void IpsApplyPatches(UINT8 *, char *) {}

TCHAR szAppEEPROMPath[MAX_PATH];
TCHAR szAppHiscorePath[MAX_PATH];
TCHAR szAppSamplesPath[MAX_PATH];
TCHAR szAppBlendPath[MAX_PATH];
TCHAR szAppBurnVer[16];

CDEmuStatusValue CDEmuStatus;

const char* isowavLBAToMSF(const int LBA) { return ""; }
int isowavMSFToLBA(const char* address) { return 0; }
TCHAR* GetIsoPath() { return NULL; }
INT32 CDEmuInit() { return 0; }
INT32 CDEmuExit() { return 0; }
INT32 CDEmuStop() { return 0; }
INT32 CDEmuPlay(UINT8 M, UINT8 S, UINT8 F) { return 0; }
INT32 CDEmuLoadSector(INT32 LBA, char* pBuffer) { return 0; }
UINT8* CDEmuReadTOC(INT32 track) { return 0; }
UINT8* CDEmuReadQChannel() { return 0; }
INT32 CDEmuGetSoundBuffer(INT16* buffer, INT32 samples) { return 0; }

// Replace the char c_find by the char c_replace in the destination c string
char* str_char_replace(char* destination, char c_find, char c_replace)
{
   for (unsigned str_idx = 0; str_idx < strlen(destination); str_idx++)
   {
      if (destination[str_idx] == c_find)
         destination[str_idx] = c_replace;
   }

   return destination;
}

std::vector<retro_input_descriptor> normal_input_descriptors;
std::vector<retro_input_descriptor> macro_input_descriptors;

struct macro_core_option_value
{
   unsigned retro_device_id;
   const char* friendly_name;

   macro_core_option_value(unsigned device_id, const char* name):
	   retro_device_id(device_id),
	   friendly_name(name)
   {
   }
};

struct macro_core_option
{
   struct GameInp *pgi;

   char option_name[100];
   char friendly_name[100];

   std::string values_str;
   std::vector<macro_core_option_value> values;
   
   macro_core_option_value *selected_value;
};

static std::vector<macro_core_option> macro_core_options;

static struct GameInp *pgi_reset;
static struct GameInp *pgi_diag;

struct dipswitch_core_option_value
{
   struct GameInp *pgi;
   BurnDIPInfo bdi;
   char friendly_name[100];
};

struct dipswitch_core_option
{
   char option_name[100];
   char friendly_name[100];

   std::string values_str;
   std::vector<dipswitch_core_option_value> values;
};

static int nDIPOffset;

static std::vector<dipswitch_core_option> dipswitch_core_options;

static void InpDIPSWGetOffset (void)
{
   BurnDIPInfo bdi;
   nDIPOffset = 0;

   for(int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
   {
      if (bdi.nFlags == 0xF0)
      {
         nDIPOffset = bdi.nInput;
         log_cb(RETRO_LOG_INFO, "DIP switches offset: %d.\n", bdi.nInput);
         break;
      }
   }
}

void InpDIPSWResetDIPs (void)
{
   int i = 0;
   BurnDIPInfo bdi;
   struct GameInp * pgi = NULL;

   InpDIPSWGetOffset();

   while (BurnDrvGetDIPInfo(&bdi, i) == 0)
   {
      if (bdi.nFlags == 0xFF)
      {
         pgi = GameInp + bdi.nInput + nDIPOffset;
         if (pgi)
            pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
      }
      i++;
   }
}

static int InpDIPSWInit()
{
   log_cb(RETRO_LOG_INFO, "Initialize DIP switches.\n");

   dipswitch_core_options.clear(); 

   BurnDIPInfo bdi;
   struct GameInp *pgi;

   const char * drvname = BurnDrvGetTextA(DRV_NAME);
   
   if (!drvname)
      return 0;
      
   for (int i = 0, j = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
   {
      /* 0xFE is the beginning label for a DIP switch entry */
      /* 0xFD are region DIP switches */
      if ((bdi.nFlags == 0xFE || bdi.nFlags == 0xFD) && bdi.nSetting > 0)
      {
         dipswitch_core_options.push_back(dipswitch_core_option());
         dipswitch_core_option *dip_option = &dipswitch_core_options.back();
         
         // Clean the dipswitch name to creation the core option name (removing space and equal characters)
         char option_name[100];

         // Some dipswitch has no name...
         if (bdi.szText)
         {
            strcpy(option_name, bdi.szText);
         }
         else // ... so, to not hang, we will generate a name based on the position of the dip (DIPSWITCH 1, DIPSWITCH 2...)
         {
            sprintf(option_name, "DIPSWITCH %d", (int)dipswitch_core_options.size());
            log_cb(RETRO_LOG_WARN, "Error in %sDIPList : The DIPSWITCH '%d' has no name. '%s' name has been generated\n", drvname, dipswitch_core_options.size(), option_name);
         }
         
         strncpy(dip_option->friendly_name, option_name, sizeof(dip_option->friendly_name));
         
         str_char_replace(option_name, ' ', '_');
         str_char_replace(option_name, '=', '_');
         
         snprintf(dip_option->option_name, sizeof(dip_option->option_name), "fba-dipswitch-%s-%s", drvname, option_name);

         // Search for duplicate name, and add number to make them unique in the core-options file
         for (int dup_idx = 0, dup_nbr = 1; dup_idx < dipswitch_core_options.size() - 1; dup_idx++) // - 1 to exclude the current one
         {
            if (strcmp(dip_option->option_name, dipswitch_core_options[dup_idx].option_name) == 0)
            {
               dup_nbr++;
               snprintf(dip_option->option_name, sizeof(dip_option->option_name), "fba-dipswitch-%s-%s_%d", drvname, option_name, dup_nbr);
            }
         }

         // Reserve space for the default value
         dip_option->values.reserve(bdi.nSetting + 1); // + 1 for default value
         dip_option->values.assign(bdi.nSetting + 1, dipswitch_core_option_value());

         int values_count = 0;
         bool skip_unusable_option = false;
         for (int k = 0; values_count < bdi.nSetting; k++)
         {
            BurnDIPInfo bdi_value;
            if (BurnDrvGetDIPInfo(&bdi_value, k + i + 1) != 0)
            {
               log_cb(RETRO_LOG_WARN, "Error in %sDIPList for DIPSWITCH '%s': End of the struct was reached too early\n", drvname, dip_option->friendly_name);
               break;
            }
            
            if (bdi_value.nFlags == 0xFE || bdi_value.nFlags == 0xFD)
            {
               log_cb(RETRO_LOG_WARN, "Error in %sDIPList for DIPSWITCH '%s': Start of next DIPSWITCH is too early\n", drvname, dip_option->friendly_name);
               break;
            }
            
            struct GameInp *pgi_value = GameInp + bdi_value.nInput + nDIPOffset;

            // When the pVal of one value is NULL => the DIP switch is unusable. So it will be skipped by removing it from the list
            if (pgi_value->Input.pVal == 0)
            {
               skip_unusable_option = true;
               break;
            }
               
            // Filter away NULL entries
            if (bdi_value.nFlags == 0)
            {
               log_cb(RETRO_LOG_WARN, "Error in %sDIPList for DIPSWITCH '%s': the line '%d' is useless\n", drvname, dip_option->friendly_name, k + 1);
               continue;
            }
            
            dipswitch_core_option_value *dip_value = &dip_option->values[values_count + 1]; // + 1 to skip the default value
            
            BurnDrvGetDIPInfo(&(dip_value->bdi), k + i + 1);
            dip_value->pgi = pgi_value;
            strncpy(dip_value->friendly_name, dip_value->bdi.szText, sizeof(dip_value->friendly_name));

            bool is_default_value = (dip_value->pgi->Input.Constant.nConst & dip_value->bdi.nMask) == (dip_value->bdi.nSetting);

            if (is_default_value)
            {
               dipswitch_core_option_value *default_dip_value = &dip_option->values[0];

               default_dip_value->bdi = dip_value->bdi;
               default_dip_value->pgi = dip_value->pgi;
             
               snprintf(default_dip_value->friendly_name, sizeof(default_dip_value->friendly_name), "%s %s", "(Default)", default_dip_value->bdi.szText);
            }

            values_count++;
         }
         
         if (bdi.nSetting > values_count)
         {
            // Truncate the list at the values_count found to not have empty values
            dip_option->values.resize(values_count + 1); // +1 for default value
            log_cb(RETRO_LOG_WARN, "Error in %sDIPList for DIPSWITCH '%s': '%d' values were intended and only '%d' were found\n", drvname, dip_option->friendly_name, bdi.nSetting, values_count);
         }
         
         // Skip the unusable option by removing it from the list
         if (skip_unusable_option)
         {
            dipswitch_core_options.pop_back();
            continue;
         }

         pgi = GameInp + bdi.nInput + nDIPOffset;
         
         // Create the string values for the core option
         dip_option->values_str.assign(dip_option->friendly_name);
         dip_option->values_str.append("; ");
         
         log_cb(RETRO_LOG_INFO, "'%s' (%d)\n", dip_option->friendly_name, dip_option->values.size() - 1); // -1 to exclude the Default from the DIP Switch count
         for (int dip_value_idx = 0; dip_value_idx < dip_option->values.size(); dip_value_idx++)
         {
            dip_option->values_str.append(dip_option->values[dip_value_idx].friendly_name);
            if (dip_value_idx != dip_option->values.size() - 1)
               dip_option->values_str.append("|");
            
            log_cb(RETRO_LOG_INFO, "   '%s'\n", dip_option->values[dip_value_idx].friendly_name);
         }
         std::basic_string<char>(dip_option->values_str).swap(dip_option->values_str);

         j++;
      }
   }

   evaluate_neogeo_bios_mode(drvname);

   set_environment();
   apply_dipswitch_from_variables();

   return 0;
}


static void evaluate_neogeo_bios_mode(const char* drvname)
{
	if (!is_neogeo_game)
		return;

	bool is_neogeo_needs_specific_bios = false;
	bool is_bios_dipswitch_found = false;

	// search the BIOS dipswitch
	for (int dip_idx = 0; dip_idx < dipswitch_core_options.size(); dip_idx++)
	{
		if (strcasecmp(dipswitch_core_options[dip_idx].friendly_name, "BIOS") == 0)
		{
			is_bios_dipswitch_found = true;
			if (dipswitch_core_options[dip_idx].values.size() > 0)
			{
				// values[0] is the default value of the dipswitch
				// if the default is different than 0, this means that a different Bios is needed
				if (dipswitch_core_options[dip_idx].values[0].bdi.nSetting != 0x00)
				{
					is_neogeo_needs_specific_bios = true;
					break;
				}
			}
		}
	}

	// Games without the BIOS dipswitch don't handle alternative bioses very well
	if (!is_bios_dipswitch_found)
	{
		is_neogeo_needs_specific_bios = true;
	}

	if (is_neogeo_needs_specific_bios)
	{
		// disable the NeoGeo mode core option
		allow_neogeo_mode = false;

		// set the NeoGeo mode to DIPSWITCH to rely on the Default Bios Dipswitch
		g_opt_neo_geo_mode = NEO_GEO_MODE_DIPSWITCH;
	}
}

static void set_controller_infos()
{
	static const struct retro_controller_description controller_description[] = {
		{ "Classic", RETROPAD_CLASSIC },
		{ "Modern", RETROPAD_MODERN }
	};

	std::vector<retro_controller_info> controller_infos(nMaxPlayers+1);

	for (int i = 0; i < nMaxPlayers; i++)
	{
		controller_infos[i].types = controller_description;
		controller_infos[i].num_types = sizeof(controller_description)/sizeof(*controller_description);
	}

	controller_infos[nMaxPlayers].types = NULL;
	controller_infos[nMaxPlayers].num_types = 0;

	environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, controller_infos.data());
}

static void set_environment()
{
	std::vector<const retro_variable*> vars_systems;
	struct retro_vfs_interface_info vfs_iface_info;

	// Add the Global core options
	vars_systems.push_back(&var_fba_aspect);
	vars_systems.push_back(&var_fba_cpu_speed_adjust);
	vars_systems.push_back(&var_fba_hiscores);

	if (pgi_diag)
	{
		vars_systems.push_back(&var_fba_diagnostic_input);
	}

	if (is_neogeo_game)
	{
		// Add the Neo Geo core options
		if (allow_neogeo_mode)
			vars_systems.push_back(&var_fba_neogeo_mode);
	}

	int nbr_vars = vars_systems.size();
	int nbr_dips = dipswitch_core_options.size();
	int nbr_macros = macro_core_options.size();

#if 0
	log_cb(RETRO_LOG_INFO, "set_environment: SYSTEM: %d, DIPSWITCH: %d, MACRO: %d\n", nbr_vars, nbr_dips, nbr_macros);
#endif

	std::vector<retro_variable> vars(nbr_vars + nbr_dips + nbr_macros + 1); // + 1 for the empty ending retro_variable

	int idx_var = 0;

	// Add the System core options
	for (int i = 0; i < nbr_vars; i++, idx_var++)
	{
		vars[idx_var] = *vars_systems[i];
#if 0
		log_cb(RETRO_LOG_INFO, "retro_variable (SYSTEM)    { '%s', '%s' }\n", vars[idx_var].key, vars[idx_var].value);
#endif
	}

	// Add the DIP switches core options
	for (int dip_idx = 0; dip_idx < nbr_dips; dip_idx++)
	{
		// Filter out the BIOS dipswitch if present while the game needs specific bios
		if (!is_neogeo_game || allow_neogeo_mode || strcasecmp(dipswitch_core_options[dip_idx].friendly_name, "BIOS") != 0)
		{
			vars[idx_var].key = dipswitch_core_options[dip_idx].option_name;
			vars[idx_var].value = dipswitch_core_options[dip_idx].values_str.c_str();
#if 0
			log_cb(RETRO_LOG_INFO, "retro_variable (DIPSWITCH) { '%s', '%s' }\n", vars[idx_var].key, vars[idx_var].value);
#endif
			idx_var++;
		}
	}

	// Add the macro inputs core options
	for (int macro_idx = 0; macro_idx < nbr_macros; macro_idx++, idx_var++)
	{
		vars[idx_var].key = macro_core_options[macro_idx].option_name;
		vars[idx_var].value = macro_core_options[macro_idx].values_str.c_str();
#if 0
		log_cb(RETRO_LOG_INFO, "retro_variable (MACRO)     { '%s', '%s' }\n", vars[idx_var].key, vars[idx_var].value);
#endif
	}

	vars[idx_var] = var_empty;
	environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars.data());

	// Initialize VFS
	vfs_iface_info.required_interface_version = FILESTREAM_REQUIRED_VFS_VERSION;
	vfs_iface_info.iface                      = NULL;
	// DISABLED since path_mkdir is not VFS aware. Enable once a workaround to remove it is found
	//if (environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
	//	filestream_vfs_init(&vfs_iface_info);
}

// Update DIP switches value  depending of the choice the user made in core options
static bool apply_dipswitch_from_variables()
{
	bool dip_changed = false;
#if 0
	log_cb(RETRO_LOG_INFO, "Apply DIP switches value from core options.\n");
#endif
	struct retro_variable var = {0};

	for (int dip_idx = 0; dip_idx < dipswitch_core_options.size(); dip_idx++)
	{
		dipswitch_core_option *dip_option = &dipswitch_core_options[dip_idx];

		// Games which needs a specific bios don't handle alternative bioses very well
		if (is_neogeo_game && !allow_neogeo_mode && strcasecmp(dip_option->friendly_name, "BIOS") == 0)
			continue;

		var.key = dip_option->option_name;
		if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) == false)
			continue;

		for (int dip_value_idx = 0; dip_value_idx < dip_option->values.size(); dip_value_idx++)
		{
			dipswitch_core_option_value *dip_value = &(dip_option->values[dip_value_idx]);

			if (strcasecmp(var.value, dip_value->friendly_name) != 0)
				continue;

			int old_nConst = dip_value->pgi->Input.Constant.nConst;

			dip_value->pgi->Input.Constant.nConst = (dip_value->pgi->Input.Constant.nConst & ~dip_value->bdi.nMask) | (dip_value->bdi.nSetting & dip_value->bdi.nMask);
			dip_value->pgi->Input.nVal = dip_value->pgi->Input.Constant.nConst;
			if (dip_value->pgi->Input.pVal)
				*(dip_value->pgi->Input.pVal) = dip_value->pgi->Input.nVal;

			if (dip_value->pgi->Input.Constant.nConst == old_nConst)
			{
#if 0
				log_cb(RETRO_LOG_INFO, "DIP switch at PTR: [%-10d] [0x%02x] -> [0x%02x] - No change - '%s' '%s' [0x%02x]\n",
				dip_value->pgi->Input.pVal, old_nConst, dip_value->pgi->Input.Constant.nConst, dip_option->friendly_name, dip_value->friendly_name, dip_value->bdi.nSetting);
#endif
			}
			else
			{
				dip_changed = true;
#if 0
				log_cb(RETRO_LOG_INFO, "DIP switch at PTR: [%-10d] [0x%02x] -> [0x%02x] - Changed   - '%s' '%s' [0x%02x]\n",
				dip_value->pgi->Input.pVal, old_nConst, dip_value->pgi->Input.Constant.nConst, dip_option->friendly_name, dip_value->friendly_name, dip_value->bdi.nSetting);
#endif
			}
		}
	}

	// Override the NeoGeo bios DIP Switch by the main one (for the moment)
	if (is_neogeo_game)
		set_neo_system_bios();

	return dip_changed;
}

int InputSetCooperativeLevel(const bool bExclusive, const bool bForeGround) { return 0; }

void Reinitialise(void)
{
    // Update the geometry, some games (sfiii2) and systems (megadrive) need it.
    struct retro_system_av_info av_info;
    retro_get_system_av_info(&av_info);
    environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
}

static void ForceFrameStep()
{
   nBurnLayer = 0xff;

   pBurnSoundOut = g_audio_buf;
   
#ifdef FBA_DEBUG
   nFramesEmulated++;
#endif
   nCurrentFrame++;
#ifdef FBA_DEBUG
   nFramesRendered++;
#endif

   BurnDrvFrame();
}

// Non-idiomatic (OutString should be to the left to match strcpy())
// Seems broken to not check nOutSize.
char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int /*nOutSize*/)
{
   if (pszOutString)
   {
      strcpy(pszOutString, pszInString);
      return pszOutString;
   }

   return (char*)pszInString;
}

int QuoteRead(char **, char **, char*) { return 1; }
char *LabelCheck(char *, char *) { return 0; }
const int nConfigMinVersion = 0x020921;

// addition to support loading of roms without crc check
static int find_rom_by_name(char *name, const ZipEntry *list, unsigned elems)
{
   unsigned i = 0;
   for (i = 0; i < elems; i++)
   {
      if( strcmp(list[i].szName, name) == 0 )
      {
         return i;
      }
   }

#if 0
   log_cb(RETRO_LOG_ERROR, "Not found: %s (name = %s)\n", list[i].szName, name);
#endif

   return -1;
}

static int find_rom_by_crc(uint32_t crc, const ZipEntry *list, unsigned elems)
{
   unsigned i = 0;
   for (i = 0; i < elems; i++)
   {
      if (list[i].nCrc == crc)
     {
         return i;
     }
   }

#if 0
   log_cb(RETRO_LOG_ERROR, "Not found: 0x%X (crc: 0x%X)\n", list[i].nCrc, crc);
#endif

   return -1;
}

static RomBiosInfo* find_bios_info(char *szName, uint32_t crc, struct RomBiosInfo* bioses)
{
   for (int i = 0; bioses[i].filename != NULL; i++)
   {
      if (strcmp(bioses[i].filename, szName) == 0 || bioses[i].crc == crc)
      {
         return &bioses[i];
      }
   }

#if 0
   log_cb(RETRO_LOG_ERROR, "Bios not found: %s (crc: 0x%08x)\n", szName, crc);
#endif

   return NULL;
}

static void free_archive_list(ZipEntry *list, unsigned count)
{
   if (list)
   {
      for (unsigned i = 0; i < count; i++)
         free(list[i].szName);
      free(list);
   }
}

static int archive_load_rom(uint8_t *dest, int *wrote, int i)
{
   if (i < 0 || i >= g_rom_count)
      return 1;

   int archive = g_find_list[i].nArchive;

   if (ZipOpen((char*)g_find_list_path[archive].c_str()) != 0)
      return 1;

   BurnRomInfo ri = {0};
   BurnDrvGetRomInfo(&ri, i);

   if (ZipLoadFile(dest, ri.nLen, wrote, g_find_list[i].nPos) != 0)
   {
      ZipClose();
      return 1;
   }

   ZipClose();
   return 0;
}

static void locate_archive(std::vector<std::string>& pathList, const char* const romName)
{
   static char path[1024];

   snprintf(path, sizeof(path), "%s%c%s", g_rom_dir, slash, romName);
   if (ZipOpen(path) == 0)
   {
      g_find_list_path.push_back(path);
      return;
   }
   else
   {
      snprintf(path, sizeof(path), "%s%c%s", g_system_dir, slash, romName);
      if (ZipOpen(path) == 0)
      {
         g_find_list_path.push_back(path);
         return;
      }
   }

   log_cb(RETRO_LOG_ERROR, "[FBA] Failed to find archive: %s, let's continue with other archives...\n", path);
}

// This code is very confusing. The original code is even more confusing :(
static bool open_archive()
{
   memset(g_find_list, 0, sizeof(g_find_list));

   // FBA wants some roms ... Figure out how many.
   g_rom_count = 0;
   while (!BurnDrvGetRomInfo(&g_find_list[g_rom_count].ri, g_rom_count))
      g_rom_count++;

   g_find_list_path.clear();

   // Check if we have said archives.
   // Check if archives are found. These are relative to g_rom_dir.
   char *rom_name;
   for (unsigned index = 0; index < 32; index++)
   {
      if (BurnDrvGetZipName(&rom_name, index))
         continue;

      log_cb(RETRO_LOG_INFO, "[FBA] Archive: %s\n", rom_name);

      locate_archive(g_find_list_path, rom_name);
 
      // Handle non-arcade roms with unofficial zip name (prefixed)
      if (strcmp(rom_name, g_base_name) != 0)
      {
         locate_archive(g_find_list_path, g_base_name);
      }

      // Handle msx bios unofficial zip name (prefixed)
      const char * boardrom = BurnDrvGetTextA(DRV_BOARDROM);
      if (boardrom && strcmp(boardrom, "msx_msx") == 0)
      {
         locate_archive(g_find_list_path, boardrom);
      }
      
      // Handle bios for pgm single pcb board (special case)
      if (strcmp(rom_name, "thegladpcb") == 0 || strcmp(rom_name, "dmnfrntpcb") == 0 || strcmp(rom_name, "svgpcb") == 0)
      {
         locate_archive(g_find_list_path, "pgm");
      }

      ZipClose();
   }

   for (unsigned z = 0; z < g_find_list_path.size(); z++)
   {
      if (ZipOpen((char*)g_find_list_path[z].c_str()) != 0)
      {
         log_cb(RETRO_LOG_ERROR, "[FBA] Failed to open archive %s\n", g_find_list_path[z].c_str());
         return false;
      }

      log_cb(RETRO_LOG_INFO, "[FBA] Parsing archive %s.\n", g_find_list_path[z].c_str());
      
      ZipEntry *list = NULL;
      int count;
      ZipGetList(&list, &count);

      // Try to map the ROMs FBA wants to ROMs we find inside our pretty archives ...
      for (unsigned i = 0; i < g_rom_count; i++)
      {
         if (g_find_list[i].nState == STAT_OK)
            continue;

         if (g_find_list[i].ri.nType == 0 || g_find_list[i].ri.nLen == 0 || g_find_list[i].ri.nCrc == 0)
         {
            g_find_list[i].nState = STAT_OK;
            continue;
         }

         int index = find_rom_by_crc(g_find_list[i].ri.nCrc, list, count);

         BurnDrvGetRomName(&rom_name, i, 0);

         bool bad_crc = false;

         if (index < 0)
         {
            index = find_rom_by_name(rom_name, list, count);
            bad_crc = true;
         }

         if (index < 0)
         {
            log_cb(RETRO_LOG_WARN, "[FBA] Searching ROM at index %d with CRC 0x%08x and name %s => Not Found\n", i, g_find_list[i].ri.nCrc, rom_name);
            continue;              
         }
         
         if (bad_crc)
            log_cb(RETRO_LOG_WARN, "[FBA] Using ROM at index %d with wrong CRC and name %s\n", i, rom_name);

#if 0
         log_cb(RETRO_LOG_INFO, "[FBA] Searching ROM at index %d with CRC 0x%08x and name %s => Found\n", i, g_find_list[i].ri.nCrc, rom_name);
#endif                          
         // Search for the best bios available by category
         if (is_neogeo_game)
         {
            RomBiosInfo *bios;

            // MVS BIOS
            bios = find_bios_info(list[index].szName, list[index].nCrc, mvs_bioses);
            if (bios)
            {
               if (!available_mvs_bios || (available_mvs_bios && bios->priority < available_mvs_bios->priority))
                  available_mvs_bios = bios;
            }

            // AES BIOS
            bios = find_bios_info(list[index].szName, list[index].nCrc, aes_bioses);
            if (bios)
            {
               if (!available_aes_bios || (available_aes_bios && bios->priority < available_aes_bios->priority))
                  available_aes_bios = bios;
            }

            // Universe BIOS
            bios = find_bios_info(list[index].szName, list[index].nCrc, uni_bioses);
            if (bios)
            {
               if (!available_uni_bios || (available_uni_bios && bios->priority < available_uni_bios->priority))
                  available_uni_bios = bios;
            }
         }
         
         // Yay, we found it!
         g_find_list[i].nArchive = z;
         g_find_list[i].nPos = index;
         g_find_list[i].nState = STAT_OK;

         if (list[index].nLen < g_find_list[i].ri.nLen)
            g_find_list[i].nState = STAT_SMALL;
         else if (list[index].nLen > g_find_list[i].ri.nLen)
            g_find_list[i].nState = STAT_LARGE;
      }

      free_archive_list(list, count);
      ZipClose();
   }

   bool is_neogeo_bios_available = false;
   if (is_neogeo_game)
   {
      if (!available_mvs_bios && !available_aes_bios && !available_uni_bios)
      {
         log_cb(RETRO_LOG_WARN, "[FBA] NeoGeo BIOS missing ...\n");
      }
      
      set_neo_system_bios();
      
      // if we have a least one type of bios, we will be able to skip the asia-s3.rom non optional bios
      if (available_mvs_bios || available_aes_bios || available_uni_bios)
      {
         is_neogeo_bios_available = true;
      }
   }

   // Going over every rom to see if they are properly loaded before we continue ...
   for (unsigned i = 0; i < g_rom_count; i++)
   {
      if (g_find_list[i].nState != STAT_OK)
      {
         if(!(g_find_list[i].ri.nType & BRF_OPT))
         {
            // make the asia-s3.rom [0x91B64BE3] (mvs_bioses[0]) optional if we have another bios available
            if (is_neogeo_game && g_find_list[i].ri.nCrc == mvs_bioses[0].crc && is_neogeo_bios_available)
               continue;

            log_cb(RETRO_LOG_ERROR, "[FBA] ROM at index %d with CRC 0x%08x is required ...\n", i, g_find_list[i].ri.nCrc);
            return false;
         }
      }
   }

   BurnExtLoadRom = archive_load_rom;
   return true;
}

void retro_init()
{
   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = log_dummy;

   BurnLibInit();
}

void retro_deinit()
{
   char output[128];

   if (driver_inited)
   {
      snprintf (output, sizeof(output), "%s%cfba%c%s.fs", g_save_dir, slash, slash, BurnDrvGetTextA(DRV_NAME));
      BurnStateSave(output, 0);
      BurnDrvExit();
   }
   driver_inited = false;
   BurnLibExit();
   if (g_fba_frame)
      free(g_fba_frame);
}

void retro_reset()
{
   // restore the NeoSystem because it was changed during the gameplay
   if (is_neogeo_game)
      set_neo_system_bios();

   if (pgi_reset)
   {
      pgi_reset->Input.nVal = 1;
      *(pgi_reset->Input.pVal) = pgi_reset->Input.nVal;
   }

   check_variables();

   apply_dipswitch_from_variables();

   ForceFrameStep();
}

static void check_variables(void)
{
   struct retro_variable var = {0};

   var.key = var_fba_cpu_speed_adjust.key;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "110") == 0)
         nBurnCPUSpeedAdjust = 0x0110;
      else if (strcmp(var.value, "120") == 0)
         nBurnCPUSpeedAdjust = 0x0120;
      else if (strcmp(var.value, "130") == 0)
         nBurnCPUSpeedAdjust = 0x0130;
      else if (strcmp(var.value, "140") == 0)
         nBurnCPUSpeedAdjust = 0x0140;
      else if (strcmp(var.value, "150") == 0)
         nBurnCPUSpeedAdjust = 0x0150;
      else if (strcmp(var.value, "160") == 0)
         nBurnCPUSpeedAdjust = 0x0160;
      else if (strcmp(var.value, "170") == 0)
         nBurnCPUSpeedAdjust = 0x0170;
      else if (strcmp(var.value, "180") == 0)
         nBurnCPUSpeedAdjust = 0x0180;
      else if (strcmp(var.value, "190") == 0)
         nBurnCPUSpeedAdjust = 0x0190;
      else if (strcmp(var.value, "200") == 0)
         nBurnCPUSpeedAdjust = 0x0200;
      else
         nBurnCPUSpeedAdjust = 0x0100;
   }

   var.key = var_fba_aspect.key;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "PAR") == 0)
         core_aspect_par = true;
      else
         core_aspect_par = false;
   }

   if (pgi_diag)
   {
      var.key = var_fba_diagnostic_input.key;
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
      {
         diag_input = NULL;
         diag_input_hold_frame_delay = 0;
         if (strcmp(var.value, "Hold Start") == 0)
         {
            diag_input = diag_input_start;
            diag_input_hold_frame_delay = 60;
         }
         else if(strcmp(var.value, "Start + A + B") == 0)
         {
            diag_input = diag_input_start_a_b;
            diag_input_hold_frame_delay = 0;
         }
         else if(strcmp(var.value, "Hold Start + A + B") == 0)
         {
            diag_input = diag_input_start_a_b;
            diag_input_hold_frame_delay = 60;
         }
         else if(strcmp(var.value, "Start + L + R") == 0)
         {
            diag_input = diag_input_start_l_r;
            diag_input_hold_frame_delay = 0;
         }
         else if(strcmp(var.value, "Hold Start + L + R") == 0)
         {
            diag_input = diag_input_start_l_r;
            diag_input_hold_frame_delay = 60;
         }
         else if(strcmp(var.value, "Hold Select") == 0)
         {
            diag_input = diag_input_select;
            diag_input_hold_frame_delay = 60;
         }
         else if(strcmp(var.value, "Select + A + B") == 0)
         {
            diag_input = diag_input_select_a_b;
            diag_input_hold_frame_delay = 0;
         }
         else if(strcmp(var.value, "Hold Select + A + B") == 0)
         {
            diag_input = diag_input_select_a_b;
            diag_input_hold_frame_delay = 60;
         }
         else if(strcmp(var.value, "Select + L + R") == 0)
         {
            diag_input = diag_input_select_l_r;
            diag_input_hold_frame_delay = 0;
         }
         else if(strcmp(var.value, "Hold Select + L + R") == 0)
         {
            diag_input = diag_input_select_l_r;
            diag_input_hold_frame_delay = 60;
         }
      }
   }

   if (is_neogeo_game)
   {
      if (allow_neogeo_mode)
      {
         var.key = var_fba_neogeo_mode.key;
         if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
         {
            if (strcmp(var.value, "MVS") == 0)
               g_opt_neo_geo_mode = NEO_GEO_MODE_MVS;
            else if (strcmp(var.value, "AES") == 0)
               g_opt_neo_geo_mode = NEO_GEO_MODE_AES;
            else if (strcmp(var.value, "UNIBIOS") == 0)
               g_opt_neo_geo_mode = NEO_GEO_MODE_UNIBIOS;
            else if (strcmp(var.value, "DIPSWITCH") == 0)
               g_opt_neo_geo_mode = NEO_GEO_MODE_DIPSWITCH;
         }
      }
   }
    
   var.key = var_fba_hiscores.key;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "enabled") == 0)
         EnableHiscores = true;
      else
         EnableHiscores = false;
   }
}

void retro_run()
{
   int width, height;
   BurnDrvGetVisibleSize(&width, &height);
   pBurnDraw = (uint8_t*)g_fba_frame;

   InputMake();

   ForceFrameStep();
   
   unsigned drv_flags = BurnDrvGetFlags();
   uint32_t height_tmp = height;
   size_t pitch_size = nBurnBpp == 2 ? sizeof(uint16_t) : sizeof(uint32_t);

   switch (drv_flags & (BDF_ORIENTATION_FLIPPED | BDF_ORIENTATION_VERTICAL))
   {
      case BDF_ORIENTATION_VERTICAL:
      case BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED:
         nBurnPitch = height * pitch_size;
         height = width;
         width = height_tmp;
         break;
      case BDF_ORIENTATION_FLIPPED:
      default:
         nBurnPitch = width * pitch_size;
   }

   video_cb(g_fba_frame, width, height, nBurnPitch);
   audio_batch_cb(g_audio_buf, nBurnSoundLen);
   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
   {
      bool old_core_aspect_par = core_aspect_par;
      neo_geo_modes old_g_opt_neo_geo_mode = g_opt_neo_geo_mode;

      check_variables();

      apply_dipswitch_from_variables();

      bool macro_updated = apply_macro_from_variables();

      if (macro_updated) // if the reinit_input_performed is true, the 2 following methods was already called in the init_input one
      {
         // Re-create the list of macro input_descriptors with new values
         init_macro_input_descriptors();
         // Re-assign all the input_descriptors to retroarch
         set_input_descriptors();
      }

      // adjust aspect ratio if the needed
      if (old_core_aspect_par != core_aspect_par)
      {
         struct retro_system_av_info av_info;
         retro_get_system_av_info(&av_info);
         environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
      }

      // reset the game if the user changed the bios
      if (old_g_opt_neo_geo_mode != g_opt_neo_geo_mode)
      {
         retro_reset();
      }
   }
}

static uint8_t *write_state_ptr;
static const uint8_t *read_state_ptr;
static unsigned state_size;

static int burn_write_state_cb(BurnArea *pba)
{
   memcpy(write_state_ptr, pba->Data, pba->nLen);
   write_state_ptr += pba->nLen;
   return 0;
}

static int burn_read_state_cb(BurnArea *pba)
{
   memcpy(pba->Data, read_state_ptr, pba->nLen);
   read_state_ptr += pba->nLen;
   return 0;
}

static int burn_dummy_state_cb(BurnArea *pba)
{
   log_cb(RETRO_LOG_INFO, "state debug: name %s, len %d\n", pba->szName, pba->nLen);
   state_size += pba->nLen;
   return 0;
}

size_t retro_serialize_size()
{
   if (state_size)
      return state_size;

   BurnAcb = burn_dummy_state_cb;
   BurnAreaScan(ACB_FULLSCAN, 0);
   return state_size;
}

bool retro_serialize(void *data, size_t size)
{
   if (size != state_size)
      return false;

   BurnAcb = burn_write_state_cb;
   write_state_ptr = (uint8_t*)data;
   BurnAreaScan(ACB_FULLSCAN | ACB_READ, 0);   
   return true;
}

bool retro_unserialize(const void *data, size_t size)
{
   if (size != state_size)
      return false;

   BurnAcb = burn_read_state_cb;
   read_state_ptr = (const uint8_t*)data;
   BurnAreaScan(ACB_FULLSCAN | ACB_WRITE, 0);
   return true;
}

void retro_cheat_reset() {}
void retro_cheat_set(unsigned, bool, const char*) {}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   int width, height;
   BurnDrvGetVisibleSize(&width, &height);
   int maximum = width > height ? width : height;
   struct retro_game_geometry geom = { (unsigned)width, (unsigned)height, (unsigned)maximum, (unsigned)maximum };

   int game_aspect_x, game_aspect_y;
   BurnDrvGetAspect(&game_aspect_x, &game_aspect_y);

   if (game_aspect_x != 0 && game_aspect_y != 0 && !core_aspect_par)
   {
      geom.aspect_ratio = (float)game_aspect_x / (float)game_aspect_y;
      log_cb(RETRO_LOG_INFO, "retro_get_system_av_info: base_width: %d, base_height: %d, max_width: %d, max_height: %d, aspect_ratio: (%d/%d) = %f (core_aspect_par: %d)\n", geom.base_width, geom.base_height, geom.max_width, geom.max_height, game_aspect_x, game_aspect_y, geom.aspect_ratio, core_aspect_par);
   }
   else
   {
      log_cb(RETRO_LOG_INFO, "retro_get_system_av_info: base_width: %d, base_height: %d, max_width: %d, max_height: %d, aspect_ratio: %f\n", geom.base_width, geom.base_height, geom.max_width, geom.max_height, geom.aspect_ratio);
   }

   struct retro_system_timing timing = { (nBurnFPS / 100.0), (nBurnFPS / 100.0) * nAudSegLen };

   info->geometry = geom;
   info->timing   = timing;
}

int VidRecalcPal()
{
   return BurnRecalcPal();
}

// Standard callbacks for 16/24/32 bit color:
static UINT32 __cdecl HighCol15(INT32 r, INT32 g, INT32 b, INT32  /* i */)
{
	UINT32 t;
	t =(r<<7)&0x7c00; // 0rrr rr00 0000 0000
	t|=(g<<2)&0x03e0; // 0000 00gg ggg0 0000
	t|=(b>>3)&0x001f; // 0000 0000 000b bbbb
	return t;
}

static UINT32 __cdecl HighCol16(INT32 r, INT32 g, INT32 b, INT32 /* i */)
{
	UINT32 t;
	t =(r<<8)&0xf800; // rrrr r000 0000 0000
	t|=(g<<3)&0x07e0; // 0000 0ggg ggg0 0000
	t|=(b>>3)&0x001f; // 0000 0000 000b bbbb
	return t;
}

// 24-bit/32-bit
static UINT32 __cdecl HighCol24(INT32 r, INT32 g, INT32 b, INT32  /* i */)
{
	UINT32 t;
	t =(r<<16)&0xff0000;
	t|=(g<<8 )&0x00ff00;
	t|=(b    )&0x0000ff;

	return t;
}

INT32 SetBurnHighCol(INT32 nDepth)
{
	VidRecalcPal();
	
	if (nDepth == 15) {
		enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_0RGB1555;
		if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
		{
			nBurnBpp = 2;
			BurnHighCol = HighCol15;
		}
	}
	
	if (nDepth == 16) {
		enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
		if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
		{
			nBurnBpp = 2;
			BurnHighCol = HighCol16;
		}
	}
	
	if (nDepth == 24) {
		enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
		if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
		{
			nBurnBpp = 3;
			BurnHighCol = HighCol24;
		}
	}
	
	if (nDepth == 32) {
		enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
		if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
		{
			nBurnBpp = 4;
			BurnHighCol = HighCol24;
		}
	}

	return 0;
}

static bool fba_init(unsigned driver, const char *game_zip_name)
{
   nBurnDrvActive = driver;

   if (!open_archive()) {
      log_cb(RETRO_LOG_ERROR, "[FBA] Cannot find driver.\n");
      return false;
   }

   nFMInterpolation = 3;
   nInterpolation = 1;

   init_input();

   // Initialize EEPROM path
   snprintf (szAppEEPROMPath, sizeof(szAppEEPROMPath), "%s%cfba%c", g_save_dir, slash, slash);
   // Create EEPROM path if it does not exist
   path_mkdir(szAppEEPROMPath);
   // Initialize Hiscore path
   snprintf (szAppHiscorePath, sizeof(szAppHiscorePath), "%s%cfba%c", g_system_dir, slash, slash);
   // Initialize Samples path
   snprintf (szAppSamplesPath, sizeof(szAppSamplesPath), "%s%cfba%csamples%c", g_system_dir, slash, slash, slash);

// Neo cd stuff probably needs to go there
//	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOCD) {
//		if (CDEmuInit()) {
//		}
//	}

   InpDIPSWInit();
   BurnDrvInit();

   char input[128];
   snprintf (input, sizeof(input), "%s%cfba%c%s.fs", g_save_dir, slash, slash, BurnDrvGetTextA(DRV_NAME));
   BurnStateLoad(input, 0, NULL);

   int width, height;
   BurnDrvGetVisibleSize(&width, &height);
   unsigned drv_flags = BurnDrvGetFlags();
   if (!(BurnDrvIsWorking())) {
      log_cb(RETRO_LOG_ERROR, "[FBA] Game %s is not marked as working\n", game_zip_name);
      return false;
   }
   size_t pitch_size = nBurnBpp == 2 ? sizeof(uint16_t) : sizeof(uint32_t);
   if (drv_flags & BDF_ORIENTATION_VERTICAL)
      nBurnPitch = height * pitch_size;
   else
      nBurnPitch = width * pitch_size;

   unsigned rotation;
   switch (drv_flags & (BDF_ORIENTATION_FLIPPED | BDF_ORIENTATION_VERTICAL))
   {
      case BDF_ORIENTATION_VERTICAL:
         rotation = 1;
         break;

      case BDF_ORIENTATION_FLIPPED:
         rotation = 2;
         break;

      case BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED:
         rotation = 3;
         break;

      default:
         rotation = 0;
   }
   
   log_cb(RETRO_LOG_INFO, "Game: %s\n", game_zip_name);

   environ_cb(RETRO_ENVIRONMENT_SET_ROTATION, &rotation);

   nAudSegLen = (AUDIO_SAMPLERATE * 100 + (nBurnFPS >> 1)) / nBurnFPS;
   g_audio_buf = (int16_t*)malloc(nAudSegLen<<2 * sizeof(int16_t));
   nBurnSoundRate = AUDIO_SAMPLERATE;
   nBurnSoundLen = nAudSegLen;

#ifdef FRONTEND_SUPPORTS_RGB565
   SetBurnHighCol(16);
#else
   SetBurnHighCol(15);
#endif

   return true;
}

static void extract_basename(char *buf, const char *path, size_t size)
{
   const char *base = strrchr(path, slash);
   if (!base)
      base = path;

   if (*base == slash)
      base++;

   strncpy(buf, base, size - 1);
   buf[size - 1] = '\0';

   char *ext = strrchr(buf, '.');
   if (ext)
      *ext = '\0';
}

static void extract_directory(char *buf, const char *path, size_t size)
{
   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   char *base = strrchr(buf, slash);

   if (base)
      *base = '\0';
   else
    {
       buf[0] = '.';
       buf[1] = '\0';
    }
}

bool retro_load_game(const struct retro_game_info *info)
{
   if (!info)
      return false;

   extract_basename(g_base_name, info->path, sizeof(g_base_name));
   extract_directory(g_rom_dir, info->path, sizeof(g_rom_dir));

   const char *dir = NULL;
   // If save directory is defined use it, ...
   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir)
   {
      strncpy(g_save_dir, dir, sizeof(g_save_dir));
      log_cb(RETRO_LOG_INFO, "Setting save dir to %s\n", g_save_dir);
   }
   else
   {
      // ... otherwise use rom directory
      strncpy(g_save_dir, g_rom_dir, sizeof(g_save_dir));
      log_cb(RETRO_LOG_ERROR, "Save dir not defined => use roms dir %s\n", g_save_dir);
   }

   // If system directory is defined use it, ...
   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
   {
      strncpy(g_system_dir, dir, sizeof(g_system_dir));
      log_cb(RETRO_LOG_INFO, "Setting system dir to %s\n", g_system_dir);
   }
   else
   {
      // ... otherwise use rom directory
      strncpy(g_system_dir, g_rom_dir, sizeof(g_system_dir));
      log_cb(RETRO_LOG_ERROR, "System dir not defined => use roms dir %s\n", g_system_dir);
   }

   unsigned i = BurnDrvGetIndexByName(g_base_name);
   if (i < nBurnDrvCount)
   {
      INT32 width, height;
      const char * boardrom = BurnDrvGetTextA(DRV_BOARDROM);
      is_neogeo_game = (boardrom && strcmp(boardrom, "neogeo") == 0);

      // Define nMaxPlayers early;
      nMaxPlayers = BurnDrvGetMaxPlayers();
      set_controller_infos();

      set_environment();
      check_variables();

      nAudSegLen = (AUDIO_SAMPLERATE * 100 + (6000 >> 1)) / 6000;
      g_audio_buf = (int16_t*)malloc(nAudSegLen<<2 * sizeof(int16_t));
      
      pBurnSoundOut = g_audio_buf;
      nBurnSoundRate = AUDIO_SAMPLERATE;
      nBurnSoundLen = nAudSegLen;

      if (!fba_init(i, g_base_name))
         goto error;

      driver_inited = true;

      BurnDrvGetFullSize(&width, &height);

      g_fba_frame = (uint32_t*)malloc(width * height * sizeof(uint32_t));
      state_size = 0;

      return true;
   }

error:
   log_cb(RETRO_LOG_ERROR, "[FBA] Cannot load this game.\n");
   return false;
}

bool retro_load_game_special(unsigned, const struct retro_game_info*, size_t) { return false; }

void retro_unload_game(void) {}

unsigned retro_get_region() { return RETRO_REGION_NTSC; }

void *retro_get_memory_data(unsigned) { return 0; }
size_t retro_get_memory_size(unsigned) { return 0; }

unsigned retro_api_version() { return RETRO_API_VERSION; }

void retro_set_controller_port_device(unsigned port, unsigned device)
{
	if (port < nMaxPlayers && fba_devices[port] != device)
	{
		fba_devices[port] = device;
		init_input();
	}
}

static const char *print_label(unsigned i)
{
   switch(i)
   {
      case RETRO_DEVICE_ID_JOYPAD_B:
         return "RetroPad B Button";
      case RETRO_DEVICE_ID_JOYPAD_Y:
         return "RetroPad Y Button";
      case RETRO_DEVICE_ID_JOYPAD_SELECT:
         return "RetroPad Select Button";
      case RETRO_DEVICE_ID_JOYPAD_START:
         return "RetroPad Start Button";
      case RETRO_DEVICE_ID_JOYPAD_UP:
         return "RetroPad D-Pad Up";
      case RETRO_DEVICE_ID_JOYPAD_DOWN:
         return "RetroPad D-Pad Down";
      case RETRO_DEVICE_ID_JOYPAD_LEFT:
         return "RetroPad D-Pad Left";
      case RETRO_DEVICE_ID_JOYPAD_RIGHT:
         return "RetroPad D-Pad Right";
      case RETRO_DEVICE_ID_JOYPAD_A:
         return "RetroPad A Button";
      case RETRO_DEVICE_ID_JOYPAD_X:
         return "RetroPad X Button";
      case RETRO_DEVICE_ID_JOYPAD_L:
         return "RetroPad L Button";
      case RETRO_DEVICE_ID_JOYPAD_R:
         return "RetroPad R Button";
      case RETRO_DEVICE_ID_JOYPAD_L2:
         return "RetroPad L2 Button";
      case RETRO_DEVICE_ID_JOYPAD_R2:
         return "RetroPad R2 Button";
      case RETRO_DEVICE_ID_JOYPAD_L3:
         return "RetroPad L3 Button";
      case RETRO_DEVICE_ID_JOYPAD_R3:
         return "RetroPad R3 Button";
      case RETRO_DEVICE_ID_JOYPAD_EMPTY:
         return "None";
      default:
         return "No known label";
   }
}

static bool init_input(void)
{
	switch_ncode = 0;

	normal_input_descriptors.clear();
	for (unsigned i = 0; i < MAX_KEYBINDS; i++) {
		keybinds[i][0] = 0xff;
		keybinds[i][2] = 0;
	}
	for (unsigned i = 0; i < 5; i++) {
		for (unsigned j = 0; j < 8; j++) {
			axibinds[i][j][0] = 0;
			axibinds[i][j][1] = 0;
			axibinds[i][j][2] = 0;
		}
	}

	GameInpInit();
	GameInpDefault();

	init_macro_core_options();

	// Update core option for diagnostic and macro inputs
	set_environment();
	// Read the user core option values
	check_variables();
	apply_macro_from_variables();

	// Now that the macro_core_options are created and core option values are read, we can create the list of macro input_descriptors
	init_macro_input_descriptors();
	// The list of normal and macro input_descriptors are filled, we can assign all the input_descriptors to retroarch
	set_input_descriptors();

	/* serialization quirks for netplay, cps3 seems problematic, neogeo, cps1 and 2 seem to be good to go 
	uint64_t serialization_quirks = RETRO_SERIALIZATION_QUIRK_SINGLE_SESSION;
	if(!strcmp(systemname, "CPS-3"))
	environ_cb(RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS, &serialization_quirks);*/

	return 0;
}

//#define DEBUG_INPUT
//

static inline INT32 CinpState(INT32 nCode)
{
	INT32 id = keybinds[nCode][0];
	UINT32 port = keybinds[nCode][1];
	INT32 idx = keybinds[nCode][2];
	if(idx == 0)
	{
		return input_cb(port, RETRO_DEVICE_JOYPAD, 0, id);
	}
	else
	{
		INT32 s = input_cb(port, RETRO_DEVICE_ANALOG, idx, id);
		INT32 position = keybinds[nCode][3];
		if(s < -1000 && position == JOY_NEG)
			return 1;
		if(s > 1000 && position == JOY_POS)
			return 1;
	}
	return 0;
}

static inline int CinpJoyAxis(int i, int axis)
{
	INT32 idx = axibinds[i][axis][0];
	if(idx != 0xff)
	{
		INT32 id = axibinds[i][axis][1];
		return input_cb(i, RETRO_DEVICE_ANALOG, idx, id);
	}
	else
	{
		INT32 idpos = axibinds[i][axis][1];
		INT32 idneg = axibinds[i][axis][2];
		INT32 spos = input_cb(i, RETRO_DEVICE_JOYPAD, 0, idpos);
		INT32 sneg = input_cb(i, RETRO_DEVICE_JOYPAD, 0, idneg);
		return (spos - sneg) * 32768;
	}
	return 0;
}

static inline int CinpMouseAxis(int i, int axis)
{
   return 0;
}

static bool poll_diag_input()
{
   if (pgi_diag && diag_input)
   {
      one_diag_input_pressed = false;
      all_diag_input_pressed = true;
      
      for (int combo_idx = 0; diag_input[combo_idx] != RETRO_DEVICE_ID_JOYPAD_EMPTY; combo_idx++)
      {
         if (input_cb(0, RETRO_DEVICE_JOYPAD, 0, diag_input[combo_idx]) == false)
            all_diag_input_pressed = false;
         else
            one_diag_input_pressed = true;
      }

      if (diag_combo_activated == false && all_diag_input_pressed)
      {
         if (diag_input_combo_start_frame == 0) // => User starts holding all the combo inputs
            diag_input_combo_start_frame = nCurrentFrame;
         else if ((nCurrentFrame - diag_input_combo_start_frame) > diag_input_hold_frame_delay) // Delays of the hold reached
            diag_combo_activated = true;
      }
      else if (one_diag_input_pressed == false)
      {
         diag_combo_activated = false;
         diag_input_combo_start_frame = 0;
      }
      
      if (diag_combo_activated)
      {
         // Cancel each input of the combo at the emulator side to not interfere when the diagnostic menu will be opened and the combo not yet released
         struct GameInp* pgi = GameInp;
         for (int combo_idx = 0; diag_input[combo_idx] != RETRO_DEVICE_ID_JOYPAD_EMPTY; combo_idx++)
         {
            for (int i = 0; i < nGameInpCount; i++, pgi++)
            {
               if (pgi->nInput == GIT_SWITCH)
               {
                  pgi->Input.nVal = 0;
                  *(pgi->Input.pVal) = pgi->Input.nVal;
               }
            }
         }

         // Activate the diagnostic key
         pgi_diag->Input.nVal = 1;
         *(pgi_diag->Input.pVal) = pgi_diag->Input.nVal;

         // Return true to stop polling game inputs while diagnostic combo inputs is pressed
         return true;
      }
   }

   // Return false to poll game inputs
   return false;
}

static INT32 InputTick()
{
	struct GameInp *pgi;
	UINT32 i;

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		INT32 nAdd = 0;
		if ((pgi->nInput &  GIT_GROUP_SLIDER) == 0) {				// not a slider
			continue;
		}

		if (pgi->nInput == GIT_KEYSLIDER) {
			// Get states of the two keys
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[0]))	{
				nAdd -= 0x100;
			}
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[1]))	{
				nAdd += 0x100;
			}
		}

		if (pgi->nInput == GIT_JOYSLIDER) {
			// Get state of the axis
			nAdd = CinpJoyAxis(pgi->Input.Slider.JoyAxis.nJoy, pgi->Input.Slider.JoyAxis.nAxis);
			nAdd /= 0x100;
		}

		// nAdd is now -0x100 to +0x100

		// Change to slider speed
		nAdd *= pgi->Input.Slider.nSliderSpeed;
		nAdd /= 0x100;

		if (pgi->Input.Slider.nSliderCenter) {						// Attact to center
			INT32 v = pgi->Input.Slider.nSliderValue - 0x8000;
			v *= (pgi->Input.Slider.nSliderCenter - 1);
			v /= pgi->Input.Slider.nSliderCenter;
			v += 0x8000;
			pgi->Input.Slider.nSliderValue = v;
		}

		pgi->Input.Slider.nSliderValue += nAdd;
		// Limit slider
		if (pgi->Input.Slider.nSliderValue < 0x0100) {
			pgi->Input.Slider.nSliderValue = 0x0100;
		}
		if (pgi->Input.Slider.nSliderValue > 0xFF00) {
			pgi->Input.Slider.nSliderValue = 0xFF00;
		}
	}
	return 0;
}

static void InputMake(void)
{
	poll_cb();

	if (poll_diag_input())
		return;

	struct GameInp* pgi;
	UINT32 i;

	InputTick();

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		if (pgi->Input.pVal == NULL) {
			continue;
		}

		switch (pgi->nInput) {
			case 0:									// Undefined
				pgi->Input.nVal = 0;
				break;
			case GIT_CONSTANT:						// Constant value
				pgi->Input.nVal = pgi->Input.Constant.nConst;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH: {						// Digital input
				INT32 s = CinpState(pgi->Input.Switch.nCode);

				if (pgi->nType & BIT_GROUP_ANALOG) {
					// Set analog controls to full
					if (s) {
						pgi->Input.nVal = 0xFFFF;
					} else {
						pgi->Input.nVal = 0x0001;
					}
#ifdef LSB_FIRST
					*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
					*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
				} else {
					// Binary controls
					if (s) {
						pgi->Input.nVal = 1;
					} else {
						pgi->Input.nVal = 0;
					}
					*(pgi->Input.pVal) = pgi->Input.nVal;
				}

				break;
			}
			case GIT_KEYSLIDER:						// Keyboard slider
			case GIT_JOYSLIDER:	{					// Joystick slider
				INT32 nSlider = pgi->Input.Slider.nSliderValue;
				if (pgi->nType == BIT_ANALOG_REL) {
					nSlider -= 0x8000;
					nSlider >>= 4;
				}

				pgi->Input.nVal = (UINT16)nSlider;
#ifdef LSB_FIRST
				*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
				*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
				break;
			}
			case GIT_MOUSEAXIS:						// Mouse axis
				pgi->Input.nVal = (UINT16)(CinpMouseAxis(pgi->Input.MouseAxis.nMouse, pgi->Input.MouseAxis.nAxis) * nAnalogSpeed);
#ifdef LSB_FIRST
				*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
				*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
				break;
			case GIT_JOYAXIS_FULL:	{				// Joystick axis
				INT32 nJoy = CinpJoyAxis(pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);

				if (pgi->nType == BIT_ANALOG_REL) {
					nJoy *= nAnalogSpeed;
					nJoy >>= 13;

					// Clip axis to 8 bits
					if (nJoy < -32768) {
						nJoy = -32768;
					}
					if (nJoy >  32767) {
						nJoy =  32767;
					}
				} else {
					nJoy >>= 1;
					nJoy += 0x8000;

					// Clip axis to 16 bits
					if (nJoy < 0x0001) {
						nJoy = 0x0001;
					}
					if (nJoy > 0xFFFF) {
						nJoy = 0xFFFF;
					}
				}

				pgi->Input.nVal = (UINT16)nJoy;
#ifdef LSB_FIRST
				*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
				*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif

				break;
			}
			case GIT_JOYAXIS_NEG:	{				// Joystick axis Lo
				INT32 nJoy = CinpJoyAxis(pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
				if (nJoy < 32767) {
					nJoy = -nJoy;

					if (nJoy < 0x0000) {
						nJoy = 0x0000;
					}
					if (nJoy > 0xFFFF) {
						nJoy = 0xFFFF;
					}

					pgi->Input.nVal = (UINT16)nJoy;
				} else {
					pgi->Input.nVal = 0;
				}

#ifdef LSB_FIRST
				*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
				*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
				break;
			}
			case GIT_JOYAXIS_POS:	{				// Joystick axis Hi
				INT32 nJoy = CinpJoyAxis(pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
				if (nJoy > 32767) {

					if (nJoy < 0x0000) {
						nJoy = 0x0000;
					}
					if (nJoy > 0xFFFF) {
						nJoy = 0xFFFF;
					}

					pgi->Input.nVal = (UINT16)nJoy;
				} else {
					pgi->Input.nVal = 0;
				}

#ifdef LSB_FIRST
				*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
				*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
				break;
			}
		}
	}

	for (i = 0; i < nMacroCount; i++, pgi++) {
		if (pgi->Macro.nMode == 1 && pgi->Macro.nSysMacro == 0) { // Macro is defined
			if (CinpState(pgi->Macro.Switch.nCode)) {
				for (INT32 j = 0; j < 4; j++) {
					if (pgi->Macro.pVal[j]) {
						*(pgi->Macro.pVal[j]) = pgi->Macro.nVal[j];
					}
				}
			}
		}
	}
}

static unsigned int BurnDrvGetIndexByName(const char* name)
{
   unsigned int ret = ~0U;
   for (unsigned int i = 0; i < nBurnDrvCount; i++) {
      nBurnDrvActive = i;
      if (strcmp(BurnDrvGetText(DRV_NAME), name) == 0) {
         ret = i;
         break;
      }
   }
   return ret;
}

#ifdef ANDROID
#include <wchar.h>

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
   if (pwcs == NULL)
      return strlen(s);
   return mbsrtowcs(pwcs, &s, n, NULL);
}

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
   return wcsrtombs(s, &pwcs, n, NULL);
}

#endif

// Driver Save State module
// If bAll=0 save/load all non-volatile ram to .fs
// If bAll=1 save/load all ram to .fs

// ------------ State len --------------------
static INT32 nTotalLen = 0;

static INT32 __cdecl StateLenAcb(struct BurnArea* pba)
{
   nTotalLen += pba->nLen;

   return 0;
}

static INT32 StateInfo(INT32* pnLen, INT32* pnMinVer, INT32 bAll)
{
   INT32 nMin = 0;
   nTotalLen = 0;
   BurnAcb = StateLenAcb;

   BurnAreaScan(ACB_NVRAM, &nMin);                  // Scan nvram
   if (bAll) {
      INT32 m;
      BurnAreaScan(ACB_MEMCARD, &m);               // Scan memory card
      if (m > nMin) {                           // Up the minimum, if needed
         nMin = m;
      }
      BurnAreaScan(ACB_VOLATILE, &m);               // Scan volatile ram
      if (m > nMin) {                           // Up the minimum, if needed
         nMin = m;
      }
   }
   *pnLen = nTotalLen;
   *pnMinVer = nMin;

   return 0;
}

// State load
INT32 BurnStateLoadEmbed(FILE* fp, INT32 nOffset, INT32 bAll, INT32 (*pLoadGame)())
{
   const char* szHeader = "FS1 ";                  // Chunk identifier

   INT32 nLen = 0;
   INT32 nMin = 0, nFileVer = 0, nFileMin = 0;
   INT32 t1 = 0, t2 = 0;
   char ReadHeader[4];
   char szForName[33];
   INT32 nChunkSize = 0;
   UINT8 *Def = NULL;
   INT32 nDefLen = 0;                           // Deflated version
   INT32 nRet = 0;

   if (nOffset >= 0) {
      fseek(fp, nOffset, SEEK_SET);
   } else {
      if (nOffset == -2) {
         fseek(fp, 0, SEEK_END);
      } else {
         fseek(fp, 0, SEEK_CUR);
      }
   }

   memset(ReadHeader, 0, 4);
   fread(ReadHeader, 1, 4, fp);                  // Read identifier
   if (memcmp(ReadHeader, szHeader, 4)) {            // Not the right file type
      return -2;
   }

   fread(&nChunkSize, 1, 4, fp);
   if (nChunkSize <= 0x40) {                     // Not big enough
      return -1;
   }

   INT32 nChunkData = ftell(fp);

   fread(&nFileVer, 1, 4, fp);                     // Version of FB that this file was saved from

   fread(&t1, 1, 4, fp);                        // Min version of FB that NV  data will work with
   fread(&t2, 1, 4, fp);                        // Min version of FB that All data will work with

   if (bAll) {                                 // Get the min version number which applies to us
      nFileMin = t2;
   } else {
      nFileMin = t1;
   }

   fread(&nDefLen, 1, 4, fp);                     // Get the size of the compressed data block

   memset(szForName, 0, sizeof(szForName));
   fread(szForName, 1, 32, fp);

   if (nBurnVer < nFileMin) {                     // Error - emulator is too old to load this state
      return -5;
   }

   // Check the game the savestate is for, and load it if needed.
   {
      bool bLoadGame = false;

      if (nBurnDrvActive < nBurnDrvCount) {
         if (strcmp(szForName, BurnDrvGetTextA(DRV_NAME))) {   // The save state is for the wrong game
            bLoadGame = true;
         }
      } else {                              // No game loaded
         bLoadGame = true;
      }

      if (bLoadGame) {
         UINT32 nCurrentGame = nBurnDrvActive;
         UINT32 i;
         for (i = 0; i < nBurnDrvCount; i++) {
            nBurnDrvActive = i;
            if (strcmp(szForName, BurnDrvGetTextA(DRV_NAME)) == 0) {
               break;
            }
         }
         if (i == nBurnDrvCount) {
            nBurnDrvActive = nCurrentGame;
            return -3;
         } else {
            if (pLoadGame == NULL) {
               return -1;
            }
            if (pLoadGame()) {
               return -1;
            }
         }
      }
   }

   StateInfo(&nLen, &nMin, bAll);
   if (nLen <= 0) {                           // No memory to load
      return -1;
   }

   // Check if the save state is okay
   if (nFileVer < nMin) {                        // Error - this state is too old and cannot be loaded.
      return -4;
   }

   fseek(fp, nChunkData + 0x30, SEEK_SET);            // Read current frame
   fread(&nCurrentFrame, 1, 4, fp);               //

   fseek(fp, 0x0C, SEEK_CUR);                     // Move file pointer to the start of the compressed block
   Def = (UINT8*)malloc(nDefLen);
   if (Def == NULL) {
      return -1;
   }
   memset(Def, 0, nDefLen);
   fread(Def, 1, nDefLen, fp);                     // Read in deflated block

   nRet = BurnStateDecompress(Def, nDefLen, bAll);      // Decompress block into driver
   if (Def) {
      free(Def);                                 // free deflated block
      Def = NULL;
   }

   fseek(fp, nChunkData + nChunkSize, SEEK_SET);

   if (nRet) {
      return -1;
   } else {
      return 0;
   }
}

// State load
INT32 BurnStateLoad(TCHAR* szName, INT32 bAll, INT32 (*pLoadGame)())
{
   const char szHeader[] = "FB1 ";                  // File identifier
   char szReadHeader[4] = "";
   INT32 nRet = 0;

   FILE* fp = _tfopen(szName, _T("rb"));
   if (fp == NULL) {
      return 1;
   }

   fread(szReadHeader, 1, 4, fp);                  // Read identifier
   if (memcmp(szReadHeader, szHeader, 4) == 0) {      // Check filetype
      nRet = BurnStateLoadEmbed(fp, -1, bAll, pLoadGame);
   }
    fclose(fp);

   if (nRet < 0) {
      return -nRet;
   } else {
      return 0;
   }
}

// Write a savestate as a chunk of an "FB1 " file
// nOffset is the absolute offset from the beginning of the file
// -1: Append at current position
// -2: Append at EOF
INT32 BurnStateSaveEmbed(FILE* fp, INT32 nOffset, INT32 bAll)
{
   const char* szHeader = "FS1 ";                  // Chunk identifier

   INT32 nLen = 0;
   INT32 nNvMin = 0, nAMin = 0;
   INT32 nZero = 0;
   char szGame[33];
   UINT8 *Def = NULL;
   INT32 nDefLen = 0;                           // Deflated version
   INT32 nRet = 0;

   if (fp == NULL) {
      return -1;
   }

   StateInfo(&nLen, &nNvMin, 0);                  // Get minimum version for NV part
   nAMin = nNvMin;
   if (bAll) {                                 // Get minimum version for All data
      StateInfo(&nLen, &nAMin, 1);
   }

   if (nLen <= 0) {                           // No memory to save
      return -1;
   }

   if (nOffset >= 0) {
      fseek(fp, nOffset, SEEK_SET);
   } else {
      if (nOffset == -2) {
         fseek(fp, 0, SEEK_END);
      } else {
         fseek(fp, 0, SEEK_CUR);
      }
   }

   fwrite(szHeader, 1, 4, fp);                     // Chunk identifier
   INT32 nSizeOffset = ftell(fp);                  // Reserve space to write the size of this chunk
   fwrite(&nZero, 1, 4, fp);                     //

   fwrite(&nBurnVer, 1, 4, fp);                  // Version of FB this was saved from
   fwrite(&nNvMin, 1, 4, fp);                     // Min version of FB NV  data will work with
   fwrite(&nAMin, 1, 4, fp);                     // Min version of FB All data will work with

   fwrite(&nZero, 1, 4, fp);                     // Reserve space to write the compressed data size

   memset(szGame, 0, sizeof(szGame));               // Game name
   sprintf(szGame, "%.32s", BurnDrvGetTextA(DRV_NAME));         //
   fwrite(szGame, 1, 32, fp);                     //

   fwrite(&nCurrentFrame, 1, 4, fp);               // Current frame

   fwrite(&nZero, 1, 4, fp);                     // Reserved
   fwrite(&nZero, 1, 4, fp);                     //
   fwrite(&nZero, 1, 4, fp);                     //

   nRet = BurnStateCompress(&Def, &nDefLen, bAll);      // Compress block from driver and return deflated buffer
   if (Def == NULL) {
      return -1;
   }

   nRet = fwrite(Def, 1, nDefLen, fp);               // Write block to disk
   if (Def) {
      free(Def);                                 // free deflated block and close file
      Def = NULL;
   }

   if (nRet != nDefLen) {                        // error writing block to disk
      return -1;
   }

   if (nDefLen & 3) {                           // Chunk size must be a multiple of 4
      fwrite(&nZero, 1, 4 - (nDefLen & 3), fp);      // Pad chunk if needed
   }

   fseek(fp, nSizeOffset + 0x10, SEEK_SET);         // Write size of the compressed data
   fwrite(&nDefLen, 1, 4, fp);                     //

   nDefLen = (nDefLen + 0x43) & ~3;               // Add for header size and align

   fseek(fp, nSizeOffset, SEEK_SET);               // Write size of the chunk
   fwrite(&nDefLen, 1, 4, fp);                     //

   fseek (fp, 0, SEEK_END);                     // Set file pointer to the end of the chunk

   return nDefLen;
}

// State save
INT32 BurnStateSave(TCHAR* szName, INT32 bAll)
{
   const char szHeader[] = "FB1 ";                  // File identifier
   INT32 nLen = 0, nVer = 0;
   INT32 nRet = 0;

   if (bAll) {                                 // Get amount of data
      StateInfo(&nLen, &nVer, 1);
   } else {
      StateInfo(&nLen, &nVer, 0);
   }
   if (nLen <= 0) {                           // No data, so exit without creating a savestate
      return 0;                              // Don't return an error code
   }

   FILE* fp = _tfopen(szName, _T("wb"));
   if (fp == NULL) {
      return 1;
   }

   fwrite(&szHeader, 1, 4, fp);
   nRet = BurnStateSaveEmbed(fp, -1, bAll);
    fclose(fp);

   if (nRet < 0) {
      return 1;
   } else {
      return 0;
   }
}

// Creates core option for the available macros of the game
// These core options will be stored in the macro_core_options list
// Depending of the game, 4 or 6 RetroPad Buttons will be configurable (L, R, L2, R2, L3, R3)
static void init_macro_core_options()
{
   const char * drvname = BurnDrvGetTextA(DRV_NAME);

   macro_core_options.clear(); 

   int nMaxRetroPadButtons = 10; // 10 = RetroPad max available buttons (A, B, X, Y, L, R, L2, R2, L3, R3)
   int nEffectiveFireButtons = nFireButtons;

   if (bStreetFighterLayout && nFireButtons == 8) // Some CPS2 games have fire buttons to control Volume Up and Down (but we will not use them)
      nEffectiveFireButtons = 6;

   unsigned i = nGameInpCount; // why nGameInpCount? cause macros begin just after normal inputs
   struct GameInp* pgi = GameInp + nGameInpCount;

   for(; i < (nGameInpCount + nMacroCount); i++, pgi++)
   {
      // Skip system macros
      if (pgi->Macro.nSysMacro)
      {
         continue;
      }

      // Assign an unique nCode for the macto
      pgi->Macro.Switch.nCode = switch_ncode++;

      macro_core_options.push_back(macro_core_option());
      macro_core_option *macro_option = &macro_core_options.back();

      // Clean the macro name to creation the core option name (removing space and equal characters)
      std::vector<char> option_name(strlen(pgi->Macro.szName) + 1); // + 1 for the '\0' ending
      strcpy(option_name.data(), pgi->Macro.szName);
      str_char_replace(option_name.data(), ' ', '_');
      str_char_replace(option_name.data(), '=', '_');

      macro_option->pgi = pgi;
      strncpy(macro_option->friendly_name, pgi->Macro.szName, sizeof(macro_option->friendly_name));
      snprintf(macro_option->option_name, sizeof(macro_option->option_name), "fba-macro-%s-%s", drvname, option_name.data());

      // Reserve space for the default value
      int remaining_input_available = nMaxRetroPadButtons - nEffectiveFireButtons;

      macro_option->values.push_back(macro_core_option_value(RETRO_DEVICE_ID_JOYPAD_EMPTY, "None"));

      if (remaining_input_available >= 6)
      {
         macro_option->values.push_back(macro_core_option_value(RETRO_DEVICE_ID_JOYPAD_L, "RetroPad L Button"));
         macro_option->values.push_back(macro_core_option_value(RETRO_DEVICE_ID_JOYPAD_R, "RetroPad R Button"));
      }
      if (remaining_input_available >= 4)
      {
         macro_option->values.push_back(macro_core_option_value(RETRO_DEVICE_ID_JOYPAD_L2, "RetroPad L2 Button"));
         macro_option->values.push_back(macro_core_option_value(RETRO_DEVICE_ID_JOYPAD_R2, "RetroPad R2 Button"));
         
         macro_option->values.push_back(macro_core_option_value(RETRO_DEVICE_ID_JOYPAD_L3, "RetroPad L3 Button"));
         macro_option->values.push_back(macro_core_option_value(RETRO_DEVICE_ID_JOYPAD_R3, "RetroPad R3 Button"));
      }

      std::vector<macro_core_option_value, std::allocator<macro_core_option_value> >(macro_option->values).swap(macro_option->values);

      // Create the string values for the macro option
      macro_option->values_str.assign(macro_option->friendly_name);
      macro_option->values_str.append("; ");

      for (int macro_value_idx = 0; macro_value_idx < macro_option->values.size(); macro_value_idx++)
      {
         macro_option->values_str.append(macro_option->values[macro_value_idx].friendly_name);
         if (macro_value_idx != macro_option->values.size() - 1)
            macro_option->values_str.append("|");
      }
      std::basic_string<char>(macro_option->values_str).swap(macro_option->values_str);

      log_cb(RETRO_LOG_INFO, "'%s' (%d)\n", macro_option->values_str.c_str(), macro_option->values.size() - 1); // -1 to exclude the None from the macro count
   }
}

// Initialize the macro input descriptors depending of the choice the user made in core options
// As soon as the user has choosen a RetroPad button for a macro, this macro will be added to the input descriptor and can be used as a regular input
// This means that the auto remapping of RetroArch will be possible also for macros  
static void init_macro_input_descriptors()
{
   macro_input_descriptors.clear();

   for(unsigned i = 0; i < macro_core_options.size(); i++)
   {
      macro_core_option *macro_option = &macro_core_options[i];

      if (!macro_option->selected_value || macro_option->selected_value->retro_device_id == 255)
      	continue;

      unsigned port = 0;
      unsigned index = 0;
      unsigned id = macro_option->selected_value->retro_device_id;

      // "P1 XXX" - try to exclude the "P1 " from the macro name
      int offset_player_x = 0;
      if (strlen(macro_option->friendly_name) > 3 && macro_option->friendly_name[0] == 'P' && macro_option->friendly_name[2] == ' ')
      {
         port = (unsigned)(macro_option->friendly_name[1] - 49);
         offset_player_x = 3;
      }

      // set the port for the macro
      keybinds[macro_option->pgi->Macro.Switch.nCode][1] = port;

      char* description = macro_option->friendly_name + offset_player_x;

      retro_input_descriptor descriptor;
      descriptor.port = port;
      descriptor.device = RETRO_DEVICE_JOYPAD;
      descriptor.index = index;
      descriptor.id = id;
      descriptor.description = description;
      macro_input_descriptors.push_back(descriptor);

      log_cb(RETRO_LOG_INFO, "MACRO [%-15s] Macro.Switch.nCode: 0x%04x Macro.nMode: %d - assigned to key [%-25s] on port %2d.\n",
      	macro_option->friendly_name, macro_option->pgi->Macro.Switch.nCode, macro_option->pgi->Macro.nMode, print_label(id), port);
   }
}

// Set the input descriptors by combininng the two lists of 'Normal' and 'Macros' inputs
static void set_input_descriptors()
{
   std::vector<retro_input_descriptor> input_descriptors(normal_input_descriptors.size() + macro_input_descriptors.size() + 1); // + 1 for the empty ending retro_input_descriptor { 0 }

   unsigned input_descriptor_idx = 0;

   for (unsigned i = 0; i < normal_input_descriptors.size(); i++, input_descriptor_idx++)
   {
      input_descriptors[input_descriptor_idx] = normal_input_descriptors[i];
   }

   for (unsigned i = 0; i < macro_input_descriptors.size(); i++, input_descriptor_idx++)
   {
      input_descriptors[input_descriptor_idx] = macro_input_descriptors[i];
   }

   input_descriptors[input_descriptor_idx].description = NULL;

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, input_descriptors.data());
}

// Activate or deactivate macros depending of the choice the user made in core options
static bool apply_macro_from_variables()
{
   bool macro_changed = false;

   struct retro_variable var = {0};

   for (int macro_idx = 0; macro_idx < macro_core_options.size(); macro_idx++)
   {
      macro_core_option *macro_option = &macro_core_options[macro_idx];

      var.key = macro_option->option_name;
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) == false)
         continue;

      for (int macro_value_idx = 0; macro_value_idx < macro_option->values.size(); macro_value_idx++)
      {
         macro_core_option_value *macro_value = &(macro_option->values[macro_value_idx]);

         if (strcasecmp(var.value, macro_value->friendly_name) != 0)
            continue;

         unsigned old_retro_device_id = keybinds[macro_option->pgi->Macro.Switch.nCode][0];

         if (macro_value->retro_device_id == old_retro_device_id)
         {
            log_cb(RETRO_LOG_INFO, "Macro '%s' unchanged '%s'\n", macro_option->friendly_name, print_label(macro_value->retro_device_id));
            continue;
         }

         macro_changed = true;

         if (macro_value->retro_device_id == RETRO_DEVICE_ID_JOYPAD_EMPTY)
         {
            // deactivate the macro
            macro_option->selected_value = NULL;
            macro_option->pgi->Macro.nMode = 0;
            log_cb(RETRO_LOG_INFO, "Macro '%s' disable from '%s'\n", macro_option->friendly_name, print_label(old_retro_device_id));
         }
         else
         {
            // activate the macro
            macro_option->selected_value = macro_value;
            macro_option->pgi->Macro.nMode = 1;
            log_cb(RETRO_LOG_INFO, "Macro '%s' changed from '%s' to '%s'\n", macro_option->friendly_name, print_label(old_retro_device_id), print_label(macro_value->retro_device_id));
         }

         // set the retro device id for the macro
         keybinds[macro_option->pgi->Macro.Switch.nCode][0] = macro_value->retro_device_id;
      }
   }

   return macro_changed;
}

INT32 GameInpBlank(INT32 bDipSwitch)
{
	UINT32 i = 0;
	struct GameInp* pgi = NULL;

	// Reset all inputs to undefined (even dip switches, if bDipSwitch==1)
	if (GameInp == NULL) {
		return 1;
	}

	// Get the targets in the library for the Input Values
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		struct BurnInputInfo bii;
		memset(&bii, 0, sizeof(bii));
		BurnDrvGetInputInfo(&bii, i);
		if (bDipSwitch == 0 && (bii.nType & BIT_GROUP_CONSTANT)) {		// Don't blank the dip switches
			continue;
		}

		memset(pgi, 0, sizeof(*pgi));									// Clear input

		pgi->nType = bii.nType;											// store input type
		pgi->Input.pVal = bii.pVal;										// store input pointer to value

		if (bii.nType & BIT_GROUP_CONSTANT) {							// Further initialisation for constants/DIPs
			pgi->nInput = GIT_CONSTANT;
			pgi->Input.Constant.nConst = *bii.pVal;
		}
	}

	for (i = 0; i < nMacroCount; i++, pgi++) {
		pgi->Macro.nMode = 0;
		if (pgi->nInput == GIT_MACRO_CUSTOM) {
			pgi->nInput = 0;
		}
	}

	return 0;
}

static void GameInpInitMacros()
{
	struct GameInp* pgi;
	struct BurnInputInfo bii;

	INT32 nPunchx3[4] = {0, 0, 0, 0};
	INT32 nPunchInputs[4][3];
	INT32 nKickx3[4] = {0, 0, 0, 0};
	INT32 nKickInputs[4][3];

	INT32 nNeogeoButtons[4][4];
	INT32 nPgmButtons[10][16];

	bStreetFighterLayout = false;
	bVolumeIsFireButton = false;
	nMacroCount = 0;

	nFireButtons = 0;

	memset(&nNeogeoButtons, 0, sizeof(nNeogeoButtons));
	memset(&nPgmButtons, 0, sizeof(nPgmButtons));

	for (UINT32 i = 0; i < nGameInpCount; i++) {
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.szName == NULL) {
			bii.szName = "";
		}

		bool bPlayerInInfo = (toupper(bii.szInfo[0]) == 'P' && bii.szInfo[1] >= '1' && bii.szInfo[1] <= '4'); // Because some of the older drivers don't use the standard input naming.
		bool bPlayerInName = (bii.szName[0] == 'P' && bii.szName[1] >= '1' && bii.szName[1] <= '4');

		if (bPlayerInInfo || bPlayerInName) {
			INT32 nPlayer = 0;

			if (bPlayerInName)
				nPlayer = bii.szName[1] - '1';
			if (bPlayerInInfo && nPlayer == 0)
				nPlayer = bii.szInfo[1] - '1';

			if (nPlayer == 0) {
				if (strncmp(" fire", bii.szInfo + 2, 5) == 0) {
					nFireButtons++;
				}
			}
			
			if ((strncmp("Volume", bii.szName, 6) == 0) && (strncmp(" fire", bii.szInfo + 2, 5) == 0)) {
				bVolumeIsFireButton = true;
			}
			if (_stricmp(" Weak Punch", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 1;
				nPunchInputs[nPlayer][0] = i;
			}
			if (_stricmp(" Medium Punch", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 2;
				nPunchInputs[nPlayer][1] = i;
			}
			if (_stricmp(" Strong Punch", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 4;
				nPunchInputs[nPlayer][2] = i;
			}
			if (_stricmp(" Weak Kick", bii.szName + 2) == 0) {
				nKickx3[nPlayer] |= 1;
				nKickInputs[nPlayer][0] = i;
			}
			if (_stricmp(" Medium Kick", bii.szName + 2) == 0) {
				nKickx3[nPlayer] |= 2;
				nKickInputs[nPlayer][1] = i;
			}
			if (_stricmp(" Strong Kick", bii.szName + 2) == 0) {
				nKickx3[nPlayer] |= 4;
				nKickInputs[nPlayer][2] = i;
			}
			
			if ((BurnDrvGetHardwareCode() & (HARDWARE_PUBLIC_MASK - HARDWARE_PREFIX_CARTRIDGE)) == HARDWARE_SNK_NEOGEO) {
				if (_stricmp(" Button A", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][0] = i;
				}
				if (_stricmp(" Button B", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][1] = i;
				}
				if (_stricmp(" Button C", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][2] = i;
				}
				if (_stricmp(" Button D", bii.szName + 2) == 0) {
					nNeogeoButtons[nPlayer][3] = i;
				}
			}

			//if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_IGS_PGM) {
			{ // Use nPgmButtons for Autofire too -dink
				if ((_stricmp(" Button 1", bii.szName + 2) == 0) || (_stricmp(" fire 1", bii.szInfo + 2) == 0)) {
					nPgmButtons[nPlayer][0] = i;
				}
				if ((_stricmp(" Button 2", bii.szName + 2) == 0) || (_stricmp(" fire 2", bii.szInfo + 2) == 0)) {
					nPgmButtons[nPlayer][1] = i;
				}
				if ((_stricmp(" Button 3", bii.szName + 2) == 0) || (_stricmp(" fire 3", bii.szInfo + 2) == 0)) {
					nPgmButtons[nPlayer][2] = i;
				}
				if ((_stricmp(" Button 4", bii.szName + 2) == 0) || (_stricmp(" fire 4", bii.szInfo + 2) == 0)) {
					nPgmButtons[nPlayer][3] = i;
				}
				if ((_stricmp(" Button 5", bii.szName + 2) == 0) || (_stricmp(" fire 5", bii.szInfo + 2) == 0)) {
					nPgmButtons[nPlayer][4] = i;
				}
				if ((_stricmp(" Button 6", bii.szName + 2) == 0) || (_stricmp(" fire 6", bii.szInfo + 2) == 0)) {
					nPgmButtons[nPlayer][5] = i;
				}
			}
		}
	}

	pgi = GameInp + nGameInpCount;
	
	{ // Autofire!!!
			for (INT32 nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++) {
				for (INT32 i = 0; i < nFireButtons; i++) {
					pgi->nInput = GIT_MACRO_AUTO;
					pgi->nType = BIT_DIGITAL;
					pgi->Macro.nMode = 0;
					pgi->Macro.nSysMacro = 15; // 15 = Auto-Fire mode
					if ((BurnDrvGetHardwareCode() & (HARDWARE_PUBLIC_MASK - HARDWARE_PREFIX_CARTRIDGE)) == HARDWARE_SEGA_MEGADRIVE) {
						if (i < 3) {
							sprintf(pgi->Macro.szName, "P%d Auto-Fire Button %c", nPlayer+1, i+'A'); // A,B,C
						} else {
							sprintf(pgi->Macro.szName, "P%d Auto-Fire Button %c", nPlayer+1, i+'X'-3); // X,Y,Z
						}
					} else {
						sprintf(pgi->Macro.szName, "P%d Auto-Fire Button %d", nPlayer+1, i+1);
					}
					if ((BurnDrvGetHardwareCode() & (HARDWARE_PUBLIC_MASK - HARDWARE_PREFIX_CARTRIDGE)) == HARDWARE_SNK_NEOGEO) {
						BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][i]);
					} else {
						BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][i]);
					}
					pgi->Macro.pVal[0] = bii.pVal;
					pgi->Macro.nVal[0] = 1;
					nMacroCount++;
					pgi++;
				}
			}
	}

	for (INT32 nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++) {
		if (nPunchx3[nPlayer] == 7) {		// Create a 3x punch macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i 3× Punch", nPlayer + 1);
			for (INT32 j = 0; j < 3; j++) {
				BurnDrvGetInputInfo(&bii, nPunchInputs[nPlayer][j]);
				pgi->Macro.pVal[j] = bii.pVal;
				pgi->Macro.nVal[j] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nKickx3[nPlayer] == 7) {		// Create a 3x kick macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i 3× Kick", nPlayer + 1);
			for (INT32 j = 0; j < 3; j++) {
				BurnDrvGetInputInfo(&bii, nKickInputs[nPlayer][j]);
				pgi->Macro.pVal[j] = bii.pVal;
				pgi->Macro.nVal[j] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nFireButtons == 4 && (BurnDrvGetHardwareCode() & (HARDWARE_PUBLIC_MASK - HARDWARE_PREFIX_CARTRIDGE)) == HARDWARE_SNK_NEOGEO) {
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons AB", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons AC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons AD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons BC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons BD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons CD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ABC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ABD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ACD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons BCD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons ABCD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[3] = bii.pVal;
			pgi->Macro.nVal[3] = 1;
			nMacroCount++;
			pgi++;
		}
		
		if (nFireButtons == 4 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_IGS_PGM) {
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 12", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 13", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 14", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 23", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 24", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 34", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 123", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 124", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;
			
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 134", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 234", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			nMacroCount++;
			pgi++;

			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;
			sprintf(pgi->Macro.szName, "P%i Buttons 1234", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[2] = bii.pVal;
			pgi->Macro.nVal[2] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][3]);
			pgi->Macro.pVal[3] = bii.pVal;
			pgi->Macro.nVal[3] = 1;
			nMacroCount++;
			pgi++;
		}
	}

	if ((nPunchx3[0] == 7) && (nKickx3[0] == 7)) {
		bStreetFighterLayout = true;
	}
	if (nFireButtons >= 5 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS2 && !bVolumeIsFireButton) {
		bStreetFighterLayout = true;
	}
}

INT32 GameInpInit()
{
	INT32 nRet = 0;
	// Count the number of inputs
	nGameInpCount = 0;
	nMacroCount = 0;
	nMaxMacro = nMaxPlayers * 52;

	for (UINT32 i = 0; i < 0x1000; i++) {
		nRet = BurnDrvGetInputInfo(NULL,i);
		if (nRet) {														// end of input list
			nGameInpCount = i;
			break;
		}
	}

	// Allocate space for all the inputs
	INT32 nSize = (nGameInpCount + nMaxMacro) * sizeof(struct GameInp);
	GameInp = (struct GameInp*)malloc(nSize);
	if (GameInp == NULL) {
		return 1;
	}
	memset(GameInp, 0, nSize);

	GameInpBlank(1);

	InpDIPSWResetDIPs();

	GameInpInitMacros();

	nAnalogSpeed = 0x0100;

	return 0;
}

// Analog to analog mapping
INT32 GameInpAnalog2RetroInpAnalog(struct GameInp* pgi, UINT32 nJoy, UINT8 nAxis, UINT32 nKey, UINT32 nIndex, char *szn, UINT8 nInput = GIT_JOYAXIS_FULL, INT32 nSliderValue = 0x8000, INT16 nSliderSpeed = 0x0E00, INT16 nSliderCenter = 10)
{
	if(bButtonMapped) return 0;
	switch (nInput)
	{
		case GIT_JOYAXIS_FULL:
		{
			pgi->nInput = GIT_JOYAXIS_FULL;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
			axibinds[nJoy][nAxis][0] = nIndex;
			axibinds[nJoy][nAxis][1] = nKey;
			retro_input_descriptor descriptor;
			descriptor.port = nJoy;
			descriptor.device = RETRO_DEVICE_ANALOG;
			descriptor.index = nIndex;
			descriptor.id = nKey;
			descriptor.description = szn;
			normal_input_descriptors.push_back(descriptor);
			break;
		}
		case GIT_JOYSLIDER:
		{
			pgi->nInput = GIT_JOYSLIDER;
			pgi->Input.Slider.nSliderValue = nSliderValue;
			pgi->Input.Slider.nSliderSpeed = nSliderSpeed;
			pgi->Input.Slider.nSliderCenter = nSliderCenter;
			pgi->Input.Slider.JoyAxis.nAxis = nAxis;
			pgi->Input.Slider.JoyAxis.nJoy = (UINT8)nJoy;
			axibinds[nJoy][nAxis][0] = nIndex;
			axibinds[nJoy][nAxis][1] = nKey;
			retro_input_descriptor descriptor;
			descriptor.port = nJoy;
			descriptor.device = RETRO_DEVICE_ANALOG;
			descriptor.index = nIndex;
			descriptor.id = nKey;
			descriptor.description = szn;
			normal_input_descriptors.push_back(descriptor);
			break;
		}
		// I'm not sure the 2 following settings are needed in the libretro port
		case GIT_JOYAXIS_NEG:
		{
			pgi->nInput = GIT_JOYAXIS_NEG;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
			axibinds[nJoy][nAxis][0] = nIndex;
			axibinds[nJoy][nAxis][1] = nKey;
			retro_input_descriptor descriptor;
			descriptor.port = nJoy;
			descriptor.device = RETRO_DEVICE_ANALOG;
			descriptor.index = nIndex;
			descriptor.id = nKey;
			descriptor.description = szn;
			normal_input_descriptors.push_back(descriptor);
			break;
		}
		case GIT_JOYAXIS_POS:
		{
			pgi->nInput = GIT_JOYAXIS_POS;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
			axibinds[nJoy][nAxis][0] = nIndex;
			axibinds[nJoy][nAxis][1] = nKey;
			retro_input_descriptor descriptor;
			descriptor.port = nJoy;
			descriptor.device = RETRO_DEVICE_ANALOG;
			descriptor.index = nIndex;
			descriptor.id = nKey;
			descriptor.description = szn;
			normal_input_descriptors.push_back(descriptor);
			break;
		}
	}
	bButtonMapped = true;
	return 0;
}

// Digital to digital mapping
INT32 GameInpDigital2RetroInpKey(struct GameInp* pgi, UINT32 nJoy, UINT32 nKey, char *szn)
{
	if(bButtonMapped) return 0;
	pgi->nInput = GIT_SWITCH;
	pgi->Input.Switch.nCode = (UINT16)(switch_ncode++);
	keybinds[pgi->Input.Switch.nCode][0] = nKey;
	keybinds[pgi->Input.Switch.nCode][1] = nJoy;
	retro_input_descriptor descriptor;
	descriptor.port = nJoy;
	descriptor.device = RETRO_DEVICE_JOYPAD;
	descriptor.index = 0;
	descriptor.id = nKey;
	descriptor.description = szn;
	normal_input_descriptors.push_back(descriptor);
	bButtonMapped = true;
	return 0;
}

// 2 digital to 1 analog mapping
// Need to be run once for each of the 2 pgi (the 2 game inputs)
// nJoy (player) and nKey (axis) needs to be the same for each of the 2 buttons
// position is either JOY_POS or JOY_NEG (the position expected on axis to trigger the button)
// szn is the descriptor text
INT32 GameInpDigital2RetroInpAnalogRight(struct GameInp* pgi, UINT32 nJoy, UINT32 nKey, UINT32 position, char *szn)
{
	if(bButtonMapped) return 0;
	pgi->nInput = GIT_SWITCH;
	pgi->Input.Switch.nCode = (UINT16)(switch_ncode++);
	keybinds[pgi->Input.Switch.nCode][0] = nKey;
	keybinds[pgi->Input.Switch.nCode][1] = nJoy;
	keybinds[pgi->Input.Switch.nCode][2] = RETRO_DEVICE_INDEX_ANALOG_RIGHT;
	keybinds[pgi->Input.Switch.nCode][3] = position;
	bAnalogRightMappingDone[nJoy][nKey][position] = true;
	if(bAnalogRightMappingDone[nJoy][nKey][JOY_POS] && bAnalogRightMappingDone[nJoy][nKey][JOY_NEG]) {
		retro_input_descriptor descriptor;
		descriptor.port = nJoy;
		descriptor.device = RETRO_DEVICE_ANALOG;
		descriptor.index = RETRO_DEVICE_INDEX_ANALOG_RIGHT;
		descriptor.id = nKey;
		descriptor.description = szn;
		normal_input_descriptors.push_back(descriptor);
	}
	bButtonMapped = true;
	return 0;
}

// 1 analog to 2 digital mapping
// Needs pgi, player, axis, 2 buttons retropad id and 2 descriptions
INT32 GameInpAnalog2RetroInpDualKeys(struct GameInp* pgi, UINT32 nJoy, UINT8 nAxis, UINT32 nKeyPos, UINT32 nKeyNeg, char *sznpos, char *sznneg)
{
	if(bButtonMapped) return 0;
	pgi->nInput = GIT_JOYAXIS_FULL;
	pgi->Input.JoyAxis.nAxis = nAxis;
	pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
	axibinds[nJoy][nAxis][0] = 0xff;
	axibinds[nJoy][nAxis][1] = nKeyPos;
	axibinds[nJoy][nAxis][2] = nKeyNeg;

	retro_input_descriptor descriptor;

	descriptor.port = nJoy;
	descriptor.device = RETRO_DEVICE_JOYPAD;
	descriptor.index = 0;
	descriptor.id = nKeyPos;
	descriptor.description = sznpos;
	normal_input_descriptors.push_back(descriptor);

	descriptor.port = nJoy;
	descriptor.device = RETRO_DEVICE_JOYPAD;
	descriptor.index = 0;
	descriptor.id = nKeyNeg;
	descriptor.description = sznneg;
	normal_input_descriptors.push_back(descriptor);

	bButtonMapped = true;
	return 0;
}

// [WIP]
// All inputs which needs special handling need to go in the next function
INT32 GameInpSpecialOne(struct GameInp* pgi, INT32 nPlayer, char* szi, char *szn, char *description)
{
	const char * parentrom	= BurnDrvGetTextA(DRV_PARENT);
	const char * drvname	= BurnDrvGetTextA(DRV_NAME);
	//const char * boardrom   = BurnDrvGetTextA(DRV_BOARDROM);
	const char * systemname = BurnDrvGetTextA(DRV_SYSTEM);
	//INT32        genre      = BurnDrvGetGenreFlags();
	//INT32        family     = BurnDrvGetFamilyFlags();
	//INT32        hardware   = BurnDrvGetHardwareCode();
	
	// Fix part of issue #102 (Crazy Fight)
	// Can't really manage to have a decent mapping on this one if you don't have a stick/pad with the following 2 rows of 3 buttons :
	// Y X R1
	// B A R2
	if ((parentrom && strcmp(parentrom, "crazyfgt") == 0) ||
		(drvname && strcmp(drvname, "crazyfgt") == 0)
	) {
		if (strcmp("top-left", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
		}
		if (strcmp("top-center", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_X, description);
		}
		if (strcmp("top-right", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
		if (strcmp("bottom-left", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
		if (strcmp("bottom-center", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
		}
		if (strcmp("bottom-right", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R2, description);
		}
	}

	// Fix part of issue #102 (Bikkuri Card)
	// Coin 2 overwrited Coin 1, which is probably part of the issue
	// I managed to pass the payout tests once, but i don't know how
	if ((parentrom && strcmp(parentrom, "bikkuric") == 0) ||
		(drvname && strcmp(drvname, "bikkuric") == 0)
	) {
		if (strcmp("Coin 2", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_START, description);
		}
	}

	// Fix part of issue #102 (Jackal)
	if ((parentrom && strcmp(parentrom, "jackal") == 0) ||
		(drvname && strcmp(drvname, "jackal") == 0)
	) {
		if (strcmp("Rotate Left", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_L, description);
		}
		if (strcmp("Rotate Right", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
	}

	// Fix part of issue #102 (Last Survivor)
	if ((parentrom && strcmp(parentrom, "lastsurv") == 0) ||
		(drvname && strcmp(drvname, "lastsurv") == 0)
	) {
		if (strcmp("Turn Left", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_L, description);
		}
		if (strcmp("Turn Right", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
	}

	// Recommandation from http://neosource.1emulation.com/forums/index.php?topic=2991.0 (Power Drift)
	if ((parentrom && strcmp(parentrom, "pdrift") == 0) ||
		(drvname && strcmp(drvname, "pdrift") == 0)
	) {
		if (strcmp("Steering", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 0, RETRO_DEVICE_ID_ANALOG_X, RETRO_DEVICE_INDEX_ANALOG_LEFT, description, GIT_JOYSLIDER, 0x8000, 0x0800, 10);
		}
	}

	// Car steer a little too much with default setting + use L/R for Shift Down/Up (Super Monaco GP)
	if ((parentrom && strcmp(parentrom, "smgp") == 0) ||
		(drvname && strcmp(drvname, "smgp") == 0)
	) {
		if (strcmp("Left/Right", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 0, RETRO_DEVICE_ID_ANALOG_X, RETRO_DEVICE_INDEX_ANALOG_LEFT, description, GIT_JOYSLIDER, 0x8000, 0x0C00, 10);
		}
		if (strcmp("Shift Down", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_L, description);
		}
		if (strcmp("Shift Up", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
	}

	// Fix issue #133 (Night striker)
	if ((parentrom && strcmp(parentrom, "nightstr") == 0) ||
		(drvname && strcmp(drvname, "nightstr") == 0)
	) {
		if (strcmp("Stick Y", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 1, RETRO_DEVICE_ID_ANALOG_Y, RETRO_DEVICE_INDEX_ANALOG_LEFT, description, GIT_JOYSLIDER, 0x8000, 0x0700, 0);
		}
	}
	
	// Fix part of issue #102 (Hang On Junior)
	if ((parentrom && strcmp(parentrom, "hangonjr") == 0) ||
		(drvname && strcmp(drvname, "hangonjr") == 0)
	) {
		if (strcmp("Accelerate", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
	}
	
	// Fix part of issue #102 (Out Run)
	if ((parentrom && strcmp(parentrom, "outrun") == 0) ||
		(drvname && strcmp(drvname, "outrun") == 0)
	) {
		if (strcmp("Gear", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
	}
	
	// Fix part of issue #102 (Golden Axe)
	// Use same layout as megadrive cores
	if ((parentrom && strcmp(parentrom, "goldnaxe") == 0) ||
		(drvname && strcmp(drvname, "goldnaxe") == 0)
	) {
		if (strcmp("Fire 1", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, "Magic");
		}
		if (strcmp("Fire 2", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, "Attack");
		}
		if (strcmp("Fire 3", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, "Jump");
		}
	}
	
	// Fix part of issue #102 (Major League)
	if ((parentrom && strcmp(parentrom, "mjleague") == 0) ||
		(drvname && strcmp(drvname, "mjleague") == 0)
	) {
		if (strcmp("Bat Swing", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
		if (strcmp("Fire 1", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
		}
		if (strcmp("Fire 2", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
		}
		if (strcmp("Fire 3", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_X, description);
		}
		if (strcmp("Fire 4", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
		if (strcmp("Fire 5", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_L, description);
		}
	}
	
	// Fix part of issue #102 (Chase HQ)
	if ((parentrom && strcmp(parentrom, "chasehq") == 0) ||
		(drvname && strcmp(drvname, "chasehq") == 0)
	) {
		if (strcmp("Brake", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
		}
		if (strcmp("Accelerate", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
		if (strcmp("Turbo", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
		}
		if (strcmp("Gear", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
	}
	
	// Fix part of issue #102 (SDI - Strategic Defense Initiative)
	if ((parentrom && strcmp(parentrom, "sdi") == 0) ||
		(drvname && strcmp(drvname, "sdi") == 0)
	) {
		if (strcmp("Target L/R", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 0, RETRO_DEVICE_ID_ANALOG_X, RETRO_DEVICE_INDEX_ANALOG_RIGHT, description);
		}
		if (strcmp("Target U/D", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 1, RETRO_DEVICE_ID_ANALOG_Y, RETRO_DEVICE_INDEX_ANALOG_RIGHT, description);
		}
		if (strcmp("Fire 1", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
	}
	
	// Fix part of issue #102 (Blades of Steel)
	if ((parentrom && strcmp(parentrom, "bladestl") == 0) ||
		(drvname && strcmp(drvname, "bladestl") == 0)
	) {
		if (strcmp("Trackball X", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 0, RETRO_DEVICE_ID_ANALOG_X, RETRO_DEVICE_INDEX_ANALOG_RIGHT, description);
		}
		if (strcmp("Trackball Y", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 1, RETRO_DEVICE_ID_ANALOG_Y, RETRO_DEVICE_INDEX_ANALOG_RIGHT, description);
		}
		if (strcmp("Button 1", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
		if (strcmp("Button 2", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R2, description);
		}
		if (strcmp("Button 3", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_L, description);
		}
	}
	
	// Fix part of issue #102 (Forgotten Worlds)
	if ((parentrom && strcmp(parentrom, "forgottn") == 0) ||
		(drvname && strcmp(drvname, "forgottn") == 0)
	) {
		if (strcmp("Turn (analog)", description) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 0, RETRO_DEVICE_ID_ANALOG_X, RETRO_DEVICE_INDEX_ANALOG_RIGHT, description);
		}
		if (strcmp("Attack", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
		if (strcmp("Turn - (digital)", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
		if (strcmp("Turn + (digital)", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
		}
	}
	
	// Fix part of issue #102 (VS Block Breaker)
	if ((parentrom && strcmp(parentrom, "vblokbrk") == 0) ||
		(drvname && strcmp(drvname, "vblokbrk") == 0)
	) {
		if (strcmp("Paddle", description) == 0) {
			GameInpAnalog2RetroInpDualKeys(pgi, nPlayer, 0, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_L, "Paddle Up", "Paddle Down");
		}
	}
	
	// Fix part of issue #102 (Puzz Loop 2)
	if ((parentrom && strcmp(parentrom, "pzloop2") == 0) ||
		(drvname && strcmp(drvname, "pzloop2") == 0)
	) {
		if (strcmp("Paddle", description) == 0) {
			GameInpAnalog2RetroInpDualKeys(pgi, nPlayer, 0, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_L, "Paddle Up", "Paddle Down");
		}
	}
	
	// Fix part of issue #102 (After burner 1 & 2)
	if ((parentrom && strcmp(parentrom, "aburner2") == 0) ||
		(drvname && strcmp(drvname, "aburner2") == 0)
	) {
		if (strcmp("Throttle", description) == 0) {
			GameInpAnalog2RetroInpDualKeys(pgi, nPlayer, 2, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_L, "Speed Up", "Speed Down");
		}
	}
	
	// Fix part of issue #156 (Alien vs Predator)
	// Use a layout more similar to the cabinet, with jump in the middle
	if ((parentrom && strcmp(parentrom, "avsp") == 0) ||
		(drvname && strcmp(drvname, "avsp") == 0)
	) {
		if (strcmp("Attack", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
		}
		if (strcmp("Jump", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
		if (strcmp("Shot", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
		}
	}
	
	// Handle megadrive
	if ((systemname && strcmp(systemname, "Sega Megadrive") == 0)) {
		// Street Fighter 2 mapping (which is the only 6 button megadrive game ?)
		// Use same layout as arcade
		if ((parentrom && strcmp(parentrom, "md_sf2") == 0) ||
			(drvname && strcmp(drvname, "md_sf2") == 0)
		) {
			if (strcmp("Button A", description) == 0) {
				GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, "Weak Kick");
			}
			if (strcmp("Button B", description) == 0) {
				GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, "Medium Kick");
			}
			if (strcmp("Button C", description) == 0) {
				GameInpDigital2RetroInpKey(pgi, nPlayer, (fba_devices[nPlayer] == RETROPAD_MODERN ? RETRO_DEVICE_ID_JOYPAD_R2 : RETRO_DEVICE_ID_JOYPAD_R), "Strong Kick");
			}
			if (strcmp("Button X", description) == 0) {
				GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, "Weak Punch");
			}
			if (strcmp("Button Y", description) == 0) {
				GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_X, "Medium Punch");
			}
			if (strcmp("Button Z", description) == 0) {
				GameInpDigital2RetroInpKey(pgi, nPlayer, (fba_devices[nPlayer] == RETROPAD_MODERN ? RETRO_DEVICE_ID_JOYPAD_R : RETRO_DEVICE_ID_JOYPAD_L), "Strong Punch");
			}
		}
		// Generic megadrive mapping
		// Use same layout as megadrive cores
		if (strcmp("Button A", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
		}
		if (strcmp("Button B", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
		if (strcmp("Button C", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
		}
	}
	
	// Handle MSX
	if ((systemname && strcmp(systemname, "MSX") == 0)) {
		if (strcmp("Button 1", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
		}
		if (strcmp("Button 2", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
		}
		if (strcmp("Key F1", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_START, description);
		}
		if (strcmp("Key F2", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_SELECT, description);
		}
		if (strcmp("Key F3", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
		}
		if (strcmp("Key F4", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_X, description);
		}
		if (strcmp("Key F5", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
		}
		if (strcmp("Key F6", description) == 0) {
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_L, description);
		}
		if (strcmp("Key UP", description) == 0) {
			GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_Y, JOY_NEG, "Key UP / Key DOWN");
		}
		if (strcmp("Key DOWN", description) == 0) {
			GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_Y, JOY_POS, "Key UP / Key DOWN");
		}
		if (strcmp("Key LEFT", description) == 0) {
			GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_X, JOY_NEG, "Key LEFT / Key RIGHT");
		}
		if (strcmp("Key RIGHT", description) == 0) {
			GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_X, JOY_POS, "Key LEFT / Key RIGHT");
		}
	}

	// Fix part of issue #102 (Twin stick games)
	if ((strcmp("Up 2", description) == 0) ||
		(strcmp("Up (right)", description) == 0) ||
		(strcmp("Right Up", description) == 0)
	) {
		GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_Y, JOY_NEG, "Up / Down (Right Stick)");
	}
	if ((strcmp("Down 2", description) == 0) ||
		(strcmp("Down (right)", description) == 0) ||
		(strcmp("Right Down", description) == 0)
	) {
		GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_Y, JOY_POS, "Up / Down (Right Stick)");
	}
	if ((strcmp("Left 2", description) == 0) ||
		(strcmp("Left (right)", description) == 0) ||
		(strcmp("Right Left", description) == 0)
	) {
		GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_X, JOY_NEG, "Left / Right (Right Stick)");
	}
	if ((strcmp("Right 2", description) == 0) ||
		(strcmp("Right (right)", description) == 0) ||
		(strcmp("Right Right", description) == 0)
	) {
		GameInpDigital2RetroInpAnalogRight(pgi, nPlayer, RETRO_DEVICE_ID_ANALOG_X, JOY_POS, "Left / Right (Right Stick)");
	}
	
	// Default racing games's Steering control to the joyslider type of analog control
	// Joyslider is some sort of "wheel" emulation
	if ((BurnDrvGetGenreFlags() & GBF_RACING)) {
		if (strcmp("x-axis", szi + 3) == 0) {
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 0, RETRO_DEVICE_ID_ANALOG_X, RETRO_DEVICE_INDEX_ANALOG_LEFT, description, GIT_JOYSLIDER);
		}
	}

	return 0;
}

// Handle mapping of an input
// Will delegate to GameInpSpecialOne for cases which needs "fine tuning"
// Use GameInp2RetroInp for the actual mapping
static INT32 GameInpAutoOne(struct GameInp* pgi, char* szi, char *szn)
{
	bool bPlayerInInfo = (toupper(szi[0]) == 'P' && szi[1] >= '1' && szi[1] <= '4'); // Because some of the older drivers don't use the standard input naming.
	bool bPlayerInName = (szn[0] == 'P' && szn[1] >= '1' && szn[1] <= '4');

	if (bPlayerInInfo || bPlayerInName) {
		INT32 nPlayer = -1;

		if (bPlayerInName)
			nPlayer = szn[1] - '1';
		if (bPlayerInInfo && nPlayer == -1)
			nPlayer = szi[1] - '1';

		char* szb = szi + 3;

		// "P1 XXX" - try to exclude the "P1 " from the szName
		INT32 offset_player_x = 0;
		if (strlen(szn) > 3 && szn[0] == 'P' && szn[2] == ' ')
			offset_player_x = 3;
		char* description = szn + offset_player_x;

		bButtonMapped = false;
		GameInpSpecialOne(pgi, nPlayer, szi, szn, description);
		if(bButtonMapped) return 0;

		if (strncmp("select", szb, 6) == 0)
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_SELECT, description);
		if (strncmp("coin", szb, 4) == 0)
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_SELECT, description);
		if (strncmp("start", szb, 5) == 0)
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_START, description);
		if (strncmp("up", szb, 2) == 0)
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_UP, description);
		if (strncmp("down", szb, 4) == 0)
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_DOWN, description);
		if (strncmp("left", szb, 4) == 0)
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_LEFT, description);
		if (strncmp("right", szb, 5) == 0)
			GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_RIGHT, description);
		if (strncmp("x-axis", szb, 6) == 0)
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 0, RETRO_DEVICE_ID_ANALOG_X, RETRO_DEVICE_INDEX_ANALOG_LEFT, description);
		if (strncmp("y-axis", szb, 6) == 0)
			GameInpAnalog2RetroInpAnalog(pgi, nPlayer, 1, RETRO_DEVICE_ID_ANALOG_Y, RETRO_DEVICE_INDEX_ANALOG_LEFT, description);

		if (strncmp("fire ", szb, 5) == 0) {
			char *szf = szb + 5;
			INT32 nButton = strtol(szf, NULL, 0);
			// The "Modern" mapping on neogeo (which mimic mapping from remakes and further opus of the series)
			// doesn't make sense and is kinda harmful on games other than kof, fatal fury and samourai showdown
			// So we restrain it to those 3 series.
			if ((BurnDrvGetFamilyFlags() & (FBF_KOF | FBF_FATFURY | FBF_SAMSHO)) && fba_devices[nPlayer] == RETROPAD_MODERN) {
				switch (nButton) {
					case 1:
						GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
						break;
					case 2:
						GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
						break;
					case 3:
						GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_X, description);
						break;
					case 4:
						GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
						break;
				}
			} else {
				// Handle 6 buttons fighting games with 3xPunchs and 3xKicks
				if (bStreetFighterLayout) {
					switch (nButton) {
						case 1:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
							break;
						case 2:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_X, description);
							break;
						case 3:
							GameInpDigital2RetroInpKey(pgi, nPlayer, (fba_devices[nPlayer] == RETROPAD_MODERN ? RETRO_DEVICE_ID_JOYPAD_R : RETRO_DEVICE_ID_JOYPAD_L), description);
							break;
						case 4:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
							break;
						case 5:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
							break;
						case 6:
							GameInpDigital2RetroInpKey(pgi, nPlayer, (fba_devices[nPlayer] == RETROPAD_MODERN ? RETRO_DEVICE_ID_JOYPAD_R2 : RETRO_DEVICE_ID_JOYPAD_R), description);
							break;
					}
				// Handle generic mapping of everything else
				} else {
					switch (nButton) {
						case 1:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_B, description);
							break;
						case 2:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_A, description);
							break;
						case 3:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_Y, description);
							break;
						case 4:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_X, description);
							break;
						case 5:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_R, description);
							break;
						case 6:
							GameInpDigital2RetroInpKey(pgi, nPlayer, (fba_devices[nPlayer] == RETROPAD_MODERN ? RETRO_DEVICE_ID_JOYPAD_R2 : RETRO_DEVICE_ID_JOYPAD_L), description);
							break;
						case 7:
							GameInpDigital2RetroInpKey(pgi, nPlayer, (fba_devices[nPlayer] == RETROPAD_MODERN ? RETRO_DEVICE_ID_JOYPAD_L : RETRO_DEVICE_ID_JOYPAD_R2), description);
							break;
						case 8:
							GameInpDigital2RetroInpKey(pgi, nPlayer, RETRO_DEVICE_ID_JOYPAD_L2, description);
							break;
					}
				}
			}
		}
	}

	// Store the pgi that controls the reset input
	if (strcmp(szi, "reset") == 0) {
		pgi->nInput = GIT_SWITCH;
		pgi->Input.Switch.nCode = (UINT16)(switch_ncode++);
		pgi_reset = pgi;
	}

	// Store the pgi that controls the diagnostic input
	if (strcmp(szi, "diag") == 0) {
		pgi->nInput = GIT_SWITCH;
		pgi->Input.Switch.nCode = (UINT16)(switch_ncode++);
		pgi_diag = pgi;
	}
	return 0;
}

// Auto-configure any undefined inputs to defaults
INT32 GameInpDefault()
{
	struct GameInp* pgi;
	struct BurnInputInfo bii;
	UINT32 i;

	pgi_reset = NULL;
	pgi_diag = NULL;

	// Fill all inputs still undefined
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		if (pgi->nInput) {											// Already defined - leave it alone
			continue;
		}

		// Get the extra info about the input
		bii.szInfo = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szInfo == NULL) {
			bii.szInfo = "";
		}

		// Dip switches - set to constant
		if (bii.nType & BIT_GROUP_CONSTANT) {
			pgi->nInput = GIT_CONSTANT;
			continue;
		}

		GameInpAutoOne(pgi, bii.szInfo, bii.szName);
	}

	// Fill in macros still undefined
	/*
	for (i = 0; i < nMacroCount; i++, pgi++) {
		if (pgi->nInput != GIT_MACRO_AUTO || pgi->Macro.nMode) {	// Already defined - leave it alone
			continue;
		}

		GameInpAutoOne(pgi, pgi->Macro.szName, pgi->Macro.szName);
	}
	*/

	return 0;
}
