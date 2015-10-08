#include "libretro.h"
#include "burner.h"

#include <vector>
#include <string>

#include "cd/cd_interface.h"
#include "descriptors.h"

#define FBA_VERSION "v0.2.97.36"

#ifdef _WIN32
   char slash = '\\';
#else
   char slash = '/';
#endif

static retro_environment_t environ_cb;
static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t poll_cb;
static retro_input_state_t input_cb;
static retro_audio_sample_batch_t audio_batch_cb;

// FBARL ---

extern UINT8 NeoSystem;
bool is_neogeo_game = false;

enum neo_geo_modes
{
   /* MVS */
   NEO_GEO_MODE_MVS = 0,

   /* AES */
   NEO_GEO_MODE_AES = 1,

   /* UNIBIOS */
   NEO_GEO_MODE_UNIBIOS = 2,
};

static unsigned int BurnDrvGetIndexByName(const char* name);

static neo_geo_modes g_opt_neo_geo_mode = NEO_GEO_MODE_MVS;
static bool gamepad_controls = true;
static bool newgen_controls = false;
static bool core_aspect_par = false;

#define STAT_NOFIND  0
#define STAT_OK      1
#define STAT_CRC     2
#define STAT_SMALL   3
#define STAT_LARGE   4

#define cpsx 1
#define neogeo 2

static int descriptor_id = 0;

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

#define AUDIO_SAMPLERATE 32000
#define AUDIO_SEGMENT_LENGTH 534 // <-- Hardcoded value that corresponds well to 32kHz audio.

static uint32_t *g_fba_frame;
static int16_t g_audio_buf[AUDIO_SEGMENT_LENGTH * 2];

// libretro globals

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t) {}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_cb = cb; }

static const struct retro_variable vars_generic[] = {
   { "fba-aspect", "Core-provided aspect ratio; DAR|PAR" },
   { "fba-cpu-speed-adjust", "CPU overclock; 100|110|120|130|140|150|160|170|180|190|200" },
   { "fba-controls", "Control scheme; gamepad|arcade" },
   { "fba-neogeo-mode", "Neo Geo mode; mvs|aes|unibios" },
   { "fba-neogeo-controls", "Neo Geo gamepad scheme; classic|newgen" },
   { NULL, NULL },
};

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;
   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars_generic);
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
   {"sp-e.sp1",          0x2723a5b5, 0x04, "MVS USA ver. 5 (6 slot)"        ,  5 },
   {"vs-bios.rom",       0xf0e8f27d, 0x05, "MVS Japan ver. 6 (? slot)"      ,  6 },
   {"sp-j2.sp1",         0xacede59C, 0x06, "MVS Japan ver. 5 (? slot)"      ,  7 },
   {"sp1.jipan.1024",    0x9fb0abe4, 0x07, "MVS Japan ver. 3 (4 slot)"      ,  8 },
   {"sp-45.sp1",         0x03cc9f6a, 0x08, "NEO-MVH MV1C"                   ,  9 },
   {"japan-j3.bin",      0xdff6d41f, 0x09, "MVS Japan (J3)"                 , 10 },
   {"sp-1v1_3db8c.bin",  0x162f0ebe, 0x0d, "Deck ver. 6 (Git Ver 1.3)"      , 11 },
   {NULL, 0, 0, NULL, 0 }
};

static struct RomBiosInfo aes_bioses[] = {
   {"neo-epo.bin",       0xd27a71f1, 0x0b, "AES Asia"                       ,  1 },
   {"neo-po.bin",        0x16d0c132, 0x0a, "AES Japan"                      ,  2 },
   {"neodebug.bin",      0x698ebb7d, 0x0c, "Development Kit"                ,  3 },
   {NULL, 0, 0, NULL, 0 }
};

static struct RomBiosInfo uni_bioses[] = {
   {"uni-bios_3_1.rom",  0x0c58093f, 0x0e, "Universe BIOS ver. 3.1"         ,  1 },
   {"uni-bios_3_0.rom",  0xa97c89a9, 0x0f, "Universe BIOS ver. 3.0"         ,  2 },
   {"uni-bios_2_3.rom",  0x27664eb5, 0x10, "Universe BIOS ver. 2.3"         ,  3 },
   {"uni-bios_2_3o.rom", 0x601720ae, 0x11, "Universe BIOS ver. 2.3 (alt)"   ,  4 },
   {"uni-bios_2_2.rom",  0x2d50996a, 0x12, "Universe BIOS ver. 2.2"         ,  5 },
   {"uni-bios_2_1.rom",  0x8dabf76b, 0x13, "Universe BIOS ver. 2.1"         ,  6 },
   {"uni-bios_2_0.rom",  0x0c12c2ad, 0x14, "Universe BIOS ver. 2.0"         ,  7 },
   {"uni-bios_1_3.rom",  0xb24b44a0, 0x15, "Universe BIOS ver. 1.3"         ,  8 },
   {"uni-bios_1_2.rom",  0x4fa698e9, 0x16, "Universe BIOS ver. 1.2"         ,  9 },
   {"uni-bios_1_2o.rom", 0xe19d3ce9, 0x17, "Universe BIOS ver. 1.2 (alt)"   , 10 },
   {"uni-bios_1_1.rom",  0x5dda0d84, 0x18, "Universe BIOS ver. 1.1"         , 11 },
   {"uni-bios_1_0.rom",  0x0ce453a0, 0x19, "Universe BIOS ver. 1.0"         , 12 },
   {NULL, 0, 0, NULL, 0 }
};

static struct RomBiosInfo unknown_bioses[] = {
   {"neopen.sp1",        0xcb915e76, 0x1a, "NeoOpen BIOS v0.1 beta"         ,  1 },
   {NULL, 0, 0, NULL, 0 }
};

static RomBiosInfo *available_mvs_bios = NULL;
static RomBiosInfo *available_aes_bios = NULL;
static RomBiosInfo *available_uni_bios = NULL;

void set_neo_system_bios()
{
   NeoSystem = 0;

   if (g_opt_neo_geo_mode == NEO_GEO_MODE_MVS)
   {
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

char g_rom_dir[1024];
char g_save_dir[1024];
static bool driver_inited;

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "FB Alpha";
   info->library_version = FBA_VERSION;
   info->need_fullpath = true;
   info->block_extract = true;
   info->valid_extensions = "iso|zip";
}

/////
static void poll_input();
static bool init_input();
static void check_variables();

void wav_exit() { }

// Maybe one day we will log into a file
void log_dummy(enum retro_log_level level, const char *fmt, ...) { }

// FBA stubs
unsigned ArcadeJoystick;

int bDrvOkay;
int bRunPause;
bool bAlwaysProcessKeyboardInput;

bool bDoIpsPatch;
void IpsApplyPatches(UINT8 *, char *) {}

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

static unsigned char nPrevDIPSettings[4];
static int nDIPOffset;

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
   BurnDIPInfo bdi;
   struct GameInp *pgi;

   InpDIPSWGetOffset();
   InpDIPSWResetDIPs();

   // TODO: why does this crash on Wii?
#if 0
   for(int i = 0, j = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
   {
      /* 0xFE is the beginning label for a DIP switch entry */
      /* 0xFD are region DIP switches */
      if (bdi.nFlags == 0xFE || bdi.nFlags == 0xFD)
      {
         log_cb(RETRO_LOG_INFO, "DIP switch label: %s.\n", bdi.szText);

         int l = 0;
         for (int k = 0; l < bdi.nSetting; k++)
         {
            BurnDIPInfo bdi_tmp;
            BurnDrvGetDIPInfo(&bdi_tmp, k+i+1);

            if (bdi_tmp.nMask == 0x3F ||
                  bdi_tmp.nMask == 0x30) /* filter away NULL entries */
               continue;

            log_cb(RETRO_LOG_INFO, "DIP switch option: %s.\n", bdi_tmp.szText);
            l++;
         }
         pgi = GameInp + bdi.nInput + nDIPOffset;
         nPrevDIPSettings[j] = pgi->Input.Constant.nConst;
         j++;
      }
   }
#endif

   return 0;
}

int InputSetCooperativeLevel(const bool bExclusive, const bool bForeGround) { return 0; }

void Reinitialise(void) { }

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

      char path[1024];
#ifdef _XBOX
      snprintf(path, sizeof(path), "%s\\%s", g_rom_dir, rom_name);
#else
      snprintf(path, sizeof(path), "%s/%s", g_rom_dir, rom_name);
#endif

      if (ZipOpen(path) != 0)
      {
         log_cb(RETRO_LOG_ERROR, "[FBA] Failed to find archive: %s\n", path);
         return false;
      }
      ZipClose();

      g_find_list_path.push_back(path);
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

         if (index < 0)
         {
            log_cb(RETRO_LOG_WARN, "[FBA] Searching ROM at index %d with CRC 0x%08x and name %s => Not Found\n", i, g_find_list[i].ri.nCrc, rom_name);
            continue;              
         }

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
      snprintf (output, sizeof(output), "%s%c%s.fs", g_save_dir, slash, BurnDrvGetTextA(DRV_NAME));
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
   // restore the NeoSystem because it was changed by during the gameplay
   if (is_neogeo_game)
      set_neo_system_bios();

   struct GameInp* pgi = GameInp;

   for (unsigned i = 0; i < nGameInpCount; i++, pgi++)
   {
      if (pgi->Input.Switch.nCode != FBK_F3)
         continue;

      pgi->Input.nVal = 1;
      *(pgi->Input.pVal) = pgi->Input.nVal;

      break;
   }

   nBurnLayer = 0xff;
   pBurnSoundOut = g_audio_buf;
   nBurnSoundRate = AUDIO_SAMPLERATE;
   //nBurnSoundLen = AUDIO_SEGMENT_LENGTH;
   nCurrentFrame++;

   BurnDrvFrame();
}

static void check_variables(void)
{
   struct retro_variable var = {0};

   var.key = "fba-neogeo-mode";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "mvs") == 0)
         g_opt_neo_geo_mode = NEO_GEO_MODE_MVS;
      else if (strcmp(var.value, "aes") == 0)
         g_opt_neo_geo_mode = NEO_GEO_MODE_AES;
      else if (strcmp(var.value, "unibios") == 0)
         g_opt_neo_geo_mode = NEO_GEO_MODE_UNIBIOS;
   }

   var.key = "fba-cpu-speed-adjust";
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

   var.key = "fba-controls";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "gamepad") == 0)
         gamepad_controls = true;
      else
         gamepad_controls = false;
   }

   var.key = "fba-neogeo-controls";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
   {
      if (strcmp(var.value, "newgen") == 0)
         newgen_controls = true;
      else
         newgen_controls = false;
   }

      var.key = "fba-aspect";
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
      {
         if (strcmp(var.value, "PAR") == 0)
            core_aspect_par = true;
         else
            core_aspect_par = false;
      }
}

void retro_run()
{
   int width, height;
   BurnDrvGetVisibleSize(&width, &height);
   pBurnDraw = (uint8_t*)g_fba_frame;

   poll_input();

   nBurnLayer = 0xff;
   pBurnSoundOut = g_audio_buf;
   nBurnSoundRate = AUDIO_SAMPLERATE;
   //nBurnSoundLen = AUDIO_SEGMENT_LENGTH;
   nCurrentFrame++;


   BurnDrvFrame();
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
      neo_geo_modes old_g_opt_neo_geo_mode = g_opt_neo_geo_mode;

      check_variables();
      // todo: only reinit when needed
      init_input();

      // todo: only reinit when needed
      struct retro_system_av_info av_info;
      retro_get_system_av_info(&av_info);
      environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
      
      // reset to change the bios
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
   state_size += pba->nLen;
   return 0;
}

size_t retro_serialize_size()
{
   if (state_size)
      return state_size;

   BurnAcb = burn_dummy_state_cb;
   state_size = 0;
   BurnAreaScan(ACB_VOLATILE | ACB_WRITE, 0);
   return state_size;
}

bool retro_serialize(void *data, size_t size)
{
   if (size != state_size)
      return false;

   BurnAcb = burn_write_state_cb;
   write_state_ptr = (uint8_t*)data;
   BurnAreaScan(ACB_VOLATILE | ACB_WRITE, 0);

   return true;
}

bool retro_unserialize(const void *data, size_t size)
{
   if (size != state_size)
      return false;
   BurnAcb = burn_read_state_cb;
   read_state_ptr = (const uint8_t*)data;
   BurnAreaScan(ACB_VOLATILE | ACB_READ, 0);

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
      geom.aspect_ratio = (float)game_aspect_x / (float)game_aspect_y;

   log_cb(RETRO_LOG_INFO, "retro_get_system_av_info: base_width: %d, base_height: %d, max_width: %d, max_height: %d, aspect_ratio: %f\n", geom.base_width, geom.base_height, geom.max_width, geom.max_height, geom.aspect_ratio);

#ifdef FBACORES_CPS
   struct retro_system_timing timing = { 59.629403, 59.629403 * AUDIO_SEGMENT_LENGTH };
#else
   struct retro_system_timing timing = { (nBurnFPS / 100.0), (nBurnFPS / 100.0) * AUDIO_SEGMENT_LENGTH };
#endif

   info->geometry = geom;
   info->timing   = timing;
}

int VidRecalcPal()
{
   return BurnRecalcPal();
}

static bool fba_init(unsigned driver, const char *game_zip_name)
{
   nBurnDrvActive = driver;

   if (!open_archive())
      return false;

   nBurnBpp = 2;
   nFMInterpolation = 3;
   nInterpolation = 3;

   char input[128];

   BurnDrvInit();
   snprintf (input, sizeof(input), "%s%c%s.fs", g_save_dir, slash, BurnDrvGetTextA(DRV_NAME));
   BurnStateLoad(input, 0, NULL);

   int width, height;
   BurnDrvGetVisibleSize(&width, &height);
   unsigned drv_flags = BurnDrvGetFlags();
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

   if(
         (strcmp("gunbird2", game_zip_name) == 0) ||
         (strcmp("s1945ii", game_zip_name) == 0) ||
         (strcmp("s1945iii", game_zip_name) == 0) ||
         (strcmp("dragnblz", game_zip_name) == 0) ||
         (strcmp("gnbarich", game_zip_name) == 0) ||
         (strcmp("mjgtaste", game_zip_name) == 0) ||
         (strcmp("tgm2", game_zip_name) == 0) ||
         (strcmp("tgm2p", game_zip_name) == 0) ||
         (strcmp("soldivid", game_zip_name) == 0) ||
         (strcmp("daraku", game_zip_name) == 0) ||
         (strcmp("sbomber", game_zip_name) == 0) ||
         (strcmp("sbombera", game_zip_name) == 0)

         )
   {
      nBurnBpp = 4;
   }

   log_cb(RETRO_LOG_INFO, "Game: %s\n", game_zip_name);

   environ_cb(RETRO_ENVIRONMENT_SET_ROTATION, &rotation);

   VidRecalcPal();

#ifdef FRONTEND_SUPPORTS_RGB565
   if(nBurnBpp == 4)
   {
      enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;

      if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
         log_cb(RETRO_LOG_INFO, "Frontend supports XRGB888 - will use that instead of XRGB1555.\n");
   }
   else
   {
      enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;

      if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
         log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
   }
#endif

   return true;
}

#if defined(FRONTEND_SUPPORTS_RGB565)
static unsigned int HighCol16(int r, int g, int b, int  /* i */)
{
   return (((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | ((b >> 3) & 0x001f));
}
#else
static unsigned int HighCol15(int r, int g, int b, int  /* i */)
{
   return (((r << 7) & 0x7c00) | ((g << 2) & 0x03e0) | ((b >> 3) & 0x001f));
}
#endif


static void init_video()
{
}

static void extract_basename(char *buf, const char *path, size_t size)
{
   const char *base = strrchr(path, '/');
   if (!base)
      base = strrchr(path, '\\');
   if (!base)
      base = path;

   if (*base == '\\' || *base == '/')
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

   char *base = strrchr(buf, '/');
   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
      buf[0] = '\0';
}

bool analog_controls_enabled = false;

bool retro_load_game(const struct retro_game_info *info)
{
   char basename[128];
   extract_basename(basename, info->path, sizeof(basename));
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

   unsigned i = BurnDrvGetIndexByName(basename);
   if (!(i < nBurnDrvCount))
   {
      log_cb(RETRO_LOG_ERROR, "[FBA] Cannot find driver.\n");
      return false;
   }

   const char * boardrom = BurnDrvGetTextA(DRV_BOARDROM);
   is_neogeo_game = (boardrom && strcmp(boardrom, "neogeo") == 0);

   pBurnSoundOut = g_audio_buf;
   nBurnSoundRate = AUDIO_SAMPLERATE;
   nBurnSoundLen = AUDIO_SEGMENT_LENGTH;

   check_variables();

   if (!fba_init(i, basename))
      return false;

   driver_inited = true;
   analog_controls_enabled = init_input();

   int32_t width, height;
   BurnDrvGetFullSize(&width, &height);

   g_fba_frame = (uint32_t*)malloc(width * height * sizeof(uint32_t));

   InpDIPSWInit();

   /* I want to try to implement different core options depending on the emulated system, not working so far
   const char * boardrom   = BurnDrvGetTextA(DRV_BOARDROM);
   if(boardrom && !strcmp(boardrom,"neogeo"))
   {
      environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars_neogeo);
   }
   */


   return true;
}

bool retro_load_game_special(unsigned, const struct retro_game_info*, size_t) { return false; }

void retro_unload_game(void) {}

unsigned retro_get_region() { return RETRO_REGION_NTSC; }

void *retro_get_memory_data(unsigned) { return 0; }
size_t retro_get_memory_size(unsigned) { return 0; }

unsigned retro_api_version() { return RETRO_API_VERSION; }

void retro_set_controller_port_device(unsigned, unsigned) {}

// Input stuff.

// Ref GamcPlayer() in ../gamc.cpp
struct key_map
{
   const char *bii_name;
   unsigned nCode[2];
};
static uint8_t keybinds[0x5000][2];

#define BIND_MAP_COUNT 310

#define RETRO_DEVICE_ID_JOYPAD_RESET      16
#define RETRO_DEVICE_ID_JOYPAD_SERVICE    17
#define RETRO_DEVICE_ID_JOYPAD_DIAGNOSTIC 18
#define RETRO_DEVICE_ID_JOYPAD_DIP_A      19
#define RETRO_DEVICE_ID_JOYPAD_DIP_B      20
#define RETRO_DEVICE_ID_JOYPAD_TEST       21

static const char *print_label(unsigned i)
{
   switch(i)
   {
      case RETRO_DEVICE_ID_JOYPAD_B:
         return "RetroPad Button B";
      case RETRO_DEVICE_ID_JOYPAD_Y:
         return "RetroPad Button Y";
      case RETRO_DEVICE_ID_JOYPAD_SELECT:
         return "RetroPad Button Select";
      case RETRO_DEVICE_ID_JOYPAD_START:
         return "RetroPad Button Start";
      case RETRO_DEVICE_ID_JOYPAD_UP:
         return "RetroPad D-Pad Up";
      case RETRO_DEVICE_ID_JOYPAD_DOWN:
         return "RetroPad D-Pad Down";
      case RETRO_DEVICE_ID_JOYPAD_LEFT:
         return "RetroPad D-Pad Left";
      case RETRO_DEVICE_ID_JOYPAD_RIGHT:
         return "RetroPad D-Pad Right";
      case RETRO_DEVICE_ID_JOYPAD_A:
         return "RetroPad Button A";
      case RETRO_DEVICE_ID_JOYPAD_X:
         return "RetroPad Button X";
      case RETRO_DEVICE_ID_JOYPAD_L:
         return "RetroPad Button L";
      case RETRO_DEVICE_ID_JOYPAD_R:
         return "RetroPad Button R";
      case RETRO_DEVICE_ID_JOYPAD_L2:
         return "RetroPad Button L2";
      case RETRO_DEVICE_ID_JOYPAD_R2:
         return "RetroPad Button R2";
      case RETRO_DEVICE_ID_JOYPAD_L3:
         return "RetroPad Button L3";
      case RETRO_DEVICE_ID_JOYPAD_R3:
         return "RetroPad Button R3";
      default:
         return "No known label";
   }
}

#define PTR_INCR ((incr++ % 3 == 2) ? counter++ : counter)

static bool init_input(void)
{
   GameInpInit();
   GameInpDefault();

   bool has_analog = false;
   struct GameInp* pgi = GameInp;
   for (unsigned i = 0; i < nGameInpCount; i++, pgi++)
   {
      if (pgi->nType == BIT_ANALOG_REL)
      {
         has_analog = true;
         break;
      }
   }

   //needed for Neo Geo button mappings (and other drivers in future)
   const char * parentrom  = BurnDrvGetTextA(DRV_PARENT);
   const char * boardrom   = BurnDrvGetTextA(DRV_BOARDROM);
   const char * drvname    = BurnDrvGetTextA(DRV_NAME);
   INT32        genre      = BurnDrvGetGenreFlags();
   INT32        hardware   = BurnDrvGetHardwareCode();

   log_cb(RETRO_LOG_INFO, "has_analog: %d\n", has_analog);
   if (parentrom)
      log_cb(RETRO_LOG_INFO, "parentrom: %s\n", parentrom);
   if (boardrom)
      log_cb(RETRO_LOG_INFO, "boardrom: %s\n", boardrom);
   if (drvname)
      log_cb(RETRO_LOG_INFO, "drvname: %s\n", drvname);
   log_cb(RETRO_LOG_INFO, "genre: %d\n", genre);
   log_cb(RETRO_LOG_INFO, "hardware: %d\n", hardware);

   /* initialization */
   struct BurnInputInfo bii;
   memset(&bii, 0, sizeof(bii));

   // Bind to nothing.
   for (unsigned i = 0; i < 0x5000; i++)
      keybinds[i][0] = 0xff;

   pgi = GameInp;

   key_map bind_map[BIND_MAP_COUNT];
   unsigned counter = 0;
   unsigned incr = 0;


   /* NOTE: The following buttons aren't mapped to the RetroPad:
    *
    * "Dip 1/2/3", "Dips", "Debug Dip", "Debug Dip 1/2", "Region",
    * "Service", "Service 1/2/3/4", "Diagnostic", "Diagnostics",
    * "Test", "Reset", "Volume Up/Down", "System", "Slots" and "Tilt"
    *
    * Mahjong/Poker controls aren't mapped since they require a keyboard
    * Excite League isn't mapped because it uses 11 buttons
    *
    * L3 and R3 are unmapped and could still be used */

   /* Universal controls */

   bind_map[PTR_INCR].bii_name = "Diagnostic";
   bind_map[PTR_INCR].nCode[0] = RETRO_DEVICE_ID_JOYPAD_R3;
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

   /* Gamepad friendly mapping */
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

      /* bind_map[PTR_INCR].bii_name = "P1 Button A";
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
      bind_map[PTR_INCR].nCode[1] = 1; */

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
   /* Arcade stick friendly mapping */
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

      /* bind_map[PTR_INCR].bii_name = "P1 Button A";
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
      bind_map[PTR_INCR].nCode[1] = 1; */

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

   for(unsigned int i = 0; i < nGameInpCount; i++, pgi++)
   {
      BurnDrvGetInputInfo(&bii, i);

      bool value_found = false;

      for(int j = 0; j < counter; j++)
      {
         if((strcmp(bii.szName,"P1 Select") ==0) && (boardrom && (strcmp(boardrom,"neogeo") == 0)))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
            keybinds[pgi->Input.Switch.nCode][1] = 0;
            value_found = true;
         }
         else if((strcmp(bii.szName,"P2 Select") ==0) && (boardrom && (strcmp(boardrom,"neogeo") == 0)))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_SELECT;
            keybinds[pgi->Input.Switch.nCode][1] = 1;
            value_found = true;
         }

         /* Alien vs. Predator and Armored Warriors both use "Px Shot" which usually serves as the shoot button for shmups
          * To make sure the controls don't overlap with each other if statements are used */

         else if((parentrom && strcmp(parentrom,"avsp") == 0 || strcmp(drvname,"avsp") == 0) && (strcmp(bii.szName,"P1 Shot") ==0))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_X;
            keybinds[pgi->Input.Switch.nCode][1] = 0;
            value_found = true;
         }
         else if((parentrom && strcmp(parentrom,"avsp") == 0 || strcmp(drvname,"avsp") == 0) && (strcmp(bii.szName,"P2 Shot") ==0))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_X;
            keybinds[pgi->Input.Switch.nCode][1] = 1;
            value_found = true;
         }
         else if((parentrom && strcmp(parentrom,"avsp") == 0 || strcmp(drvname,"avsp") == 0) && (strcmp(bii.szName,"P3 Shot") ==0))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_X;
            keybinds[pgi->Input.Switch.nCode][1] = 2;
            value_found = true;
         }
         else if((parentrom && strcmp(parentrom,"armwar") == 0 || strcmp(drvname,"armwar") == 0) && (strcmp(bii.szName,"P1 Shot") ==0))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_X;
            keybinds[pgi->Input.Switch.nCode][1] = 0;
            value_found = true;
         }
         else if((parentrom && strcmp(parentrom,"armwar") == 0 || strcmp(drvname,"armwar") == 0) && (strcmp(bii.szName,"P2 Shot") ==0))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_X;
            keybinds[pgi->Input.Switch.nCode][1] = 1;
            value_found = true;
         }
         else if((parentrom && strcmp(parentrom,"armwar") == 0 || strcmp(drvname,"armwar") == 0) && (strcmp(bii.szName,"P3 Shot") ==0))
         {
            keybinds[pgi->Input.Switch.nCode][0] = RETRO_DEVICE_ID_JOYPAD_X;
            keybinds[pgi->Input.Switch.nCode][1] = 2;
            value_found = true;
         }
         else if(strcmp(bii.szName, bind_map[j].bii_name) == 0)
         {
            keybinds[pgi->Input.Switch.nCode][0] = bind_map[j].nCode[0];
            keybinds[pgi->Input.Switch.nCode][1] = bind_map[j].nCode[1];
            value_found = true;
         }
         else
            value_found = false;

         if (!value_found)
            continue;

         log_cb(RETRO_LOG_INFO, "%s - assigned to key: %s, port: %d.\n", bii.szName, print_label(keybinds[pgi->Input.Switch.nCode][0]),keybinds[pgi->Input.Switch.nCode][1]);
         log_cb(RETRO_LOG_INFO, "%s - has nSwitch.nCode: %x.\n", bii.szName, pgi->Input.Switch.nCode);
         break;
      }

      if(!value_found)
      {
         log_cb(RETRO_LOG_INFO, "WARNING! Button unaccounted for: [%s].\n", bii.szName);
         log_cb(RETRO_LOG_INFO, "%s - has nSwitch.nCode: %x.\n", bii.szName, pgi->Input.Switch.nCode);
      }
   }

   /* add code to select between different descriptors here */
   if(gamepad_controls)
   {
      if(boardrom && !strcmp(boardrom,"neogeo"))
         if(newgen_controls)
            environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, neogeo_gamepad_newgen);
         else
            environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, neogeo_gamepad);
      else
      environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, default_gamepad);
   }
   else
   {
      if(boardrom && !strcmp(boardrom,"neogeo"))
         environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, neogeo_arcade);
      else
         environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, default_arcade);
   }

   return has_analog;
}

//#define DEBUG_INPUT
//

static inline int CinpJoyAxis(int i, int axis)
{
   switch(axis)
   {
      case 0:
         return input_cb(i, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT,
               RETRO_DEVICE_ID_ANALOG_X);
      case 1:
         return input_cb(i, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT,
               RETRO_DEVICE_ID_ANALOG_Y);
      case 2:
         return 0;
      case 3:
         return input_cb(i, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT,
               RETRO_DEVICE_ID_ANALOG_X);
      case 4:
         return input_cb(i, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT,
               RETRO_DEVICE_ID_ANALOG_Y);
      case 5:
         return 0;
      case 6:
         return 0;
      case 7:
         return 0;
   }
   return 0;
}

static inline int CinpMouseAxis(int i, int axis)
{
   return 0;
}

static void poll_input(void)
{
   poll_cb();

   struct GameInp* pgi = GameInp;

   for (int i = 0; i < nGameInpCount; i++, pgi++)
   {
      int nAdd = 0;

      if ((pgi->nInput & GIT_GROUP_SLIDER) == 0)                           // not a slider
         continue;

      if (pgi->nInput == GIT_KEYSLIDER)
      {
         // Get states of the two keys
         if (input_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
            nAdd -= 0x100;
         if (input_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
            nAdd += 0x100;
      }

      // nAdd is now -0x100 to +0x100

      // Change to slider speed
      nAdd *= pgi->Input.Slider.nSliderSpeed;
      nAdd /= 0x100;

      if (pgi->Input.Slider.nSliderCenter)
      {                                          // Attact to center
         int v = pgi->Input.Slider.nSliderValue - 0x8000;
         v *= (pgi->Input.Slider.nSliderCenter - 1);
         v /= pgi->Input.Slider.nSliderCenter;
         v += 0x8000;
         pgi->Input.Slider.nSliderValue = v;
      }

      pgi->Input.Slider.nSliderValue += nAdd;
      // Limit slider
      if (pgi->Input.Slider.nSliderValue < 0x0100)
         pgi->Input.Slider.nSliderValue = 0x0100;
      if (pgi->Input.Slider.nSliderValue > 0xFF00)
         pgi->Input.Slider.nSliderValue = 0xFF00;
   }

   pgi = GameInp;

   for (unsigned i = 0; i < nGameInpCount; i++, pgi++)
   {
      switch (pgi->nInput)
      {
         case GIT_CONSTANT: // Constant value
            {
               pgi->Input.nVal = pgi->Input.Constant.nConst;
               *(pgi->Input.pVal) = pgi->Input.nVal;
            }
            break;
         case GIT_SWITCH:
            {
               // Digital input
               INT32 id = keybinds[pgi->Input.Switch.nCode][0];
               unsigned port = keybinds[pgi->Input.Switch.nCode][1];

               bool state = input_cb(port, RETRO_DEVICE_JOYPAD, 0, id);

#if 0
               log_cb(RETRO_LOG_INFO, "GIT_SWITCH: %s, port: %d, pressed: %d.\n", print_label(id), port, state);
#endif

               if (pgi->nType & BIT_GROUP_ANALOG)
               {
                  // Set analog controls to full
                  if (state)
                     pgi->Input.nVal = 0xFFFF;
                  else
                     pgi->Input.nVal = 0x0001;
#ifdef LSB_FIRST
                  *(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
                  *((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
               }
               else
               {
                  // Binary controls
                  if (state)
                     pgi->Input.nVal = 1;
                  else
                     pgi->Input.nVal = 0;
                  *(pgi->Input.pVal) = pgi->Input.nVal;
               }
               break;
            }
         case GIT_KEYSLIDER:                  // Keyboard slider
#if 0
            log_cb(RETRO_LOG_INFO, "GIT_JOYSLIDER\n");
#endif
            {
               int nSlider = pgi->Input.Slider.nSliderValue;
               if (pgi->nType == BIT_ANALOG_REL) {
                  nSlider -= 0x8000;
                  nSlider >>= 4;
               }

               pgi->Input.nVal = (unsigned short)nSlider;
#ifdef LSB_FIRST
               *(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
               *((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
               break;
            }
         case GIT_MOUSEAXIS:                  // Mouse axis
            {
               pgi->Input.nVal = (UINT16)(CinpMouseAxis(pgi->Input.MouseAxis.nMouse, pgi->Input.MouseAxis.nAxis) * nAnalogSpeed);
#ifdef LSB_FIRST
               *(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
               *((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
            }
            break;
         case GIT_JOYAXIS_FULL:
            {            // Joystick axis
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
         case GIT_JOYAXIS_NEG:
            {            // Joystick axis Lo
               INT32 nJoy = CinpJoyAxis(pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
               if (nJoy < 32767)
               {
                  nJoy = -nJoy;

                  if (nJoy < 0x0000)
                     nJoy = 0x0000;
                  if (nJoy > 0xFFFF)
                     nJoy = 0xFFFF;

                  pgi->Input.nVal = (UINT16)nJoy;
               }
               else
                  pgi->Input.nVal = 0;

#ifdef LSB_FIRST
               *(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
               *((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
               break;
            }
         case GIT_JOYAXIS_POS:
            {            // Joystick axis Hi
               INT32 nJoy = CinpJoyAxis(pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
               if (nJoy > 32767)
               {

                  if (nJoy < 0x0000)
                     nJoy = 0x0000;
                  if (nJoy > 0xFFFF)
                     nJoy = 0xFFFF;

                  pgi->Input.nVal = (UINT16)nJoy;
               }
               else
                  pgi->Input.nVal = 0;

#ifdef LSB_FIRST
               *(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
               *((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
               break;
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
