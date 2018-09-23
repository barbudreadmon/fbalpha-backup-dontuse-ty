// Cheevos support

#include <string>
#include "libretro.h"
#include "burner.h"
#include "retro_mem.h"

void* MainRamData = NULL;
size_t MainRamSize = 0;
bool bMainRamFound = false;

int StateGetMainRamAcb(BurnArea *pba)
{
	int nHardwareCode = BurnDrvGetHardwareCode();
	if ((nHardwareCode & (HARDWARE_PUBLIC_MASK - HARDWARE_PREFIX_CARTRIDGE)) == HARDWARE_SNK_NEOGEO) {
		if (strcmp(pba->szName, "68K RAM") == 0) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS1
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS1_QSOUND
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS1_GENERIC
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPSCHANGER
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS2) {
		if (strcmp(pba->szName, "CpsRamFF") == 0) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS3) {
		if (strcmp(pba->szName, "Main RAM") == 0) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_PREFIX_KONAMI
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_TAITO
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_IREM
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_DATAEAST
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_PACMAN
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_CAPCOM_MISC
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_GALAXIAN
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_SETA
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_TECHNOS
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_PCENGINE
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_SYSTEMX
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_SYSTEMY
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_SYSTEM16A
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_SYSTEM16B
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_SYSTEM16M
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_SYSTEM18
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_HANGON
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_OUTRUN
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_SYSTEM1
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_SEGA_MISC
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_TOAPLAN) {
		if (strcmp(pba->szName, "All Ram") == 0) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_CAVE) {
		if (strcmp(pba->szName, "RAM") == 0) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_IGS_PGM) {
		if (strcmp(pba->szName, "Z80 RAM") == 0) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_PSIKYO) {
		if (strcmp(pba->szName, "68K RAM") == 0) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_KANEKO) {
		if ( (strcmp(pba->szName, "All Ram") == 0) || (strcmp(pba->szName, "All RAM") == 0) ) {
			MainRamData = pba->Data;
			MainRamSize = pba->nLen;
			bMainRamFound = true;
		}
	}

	return 0;
}

void *retro_get_memory_data(unsigned id)
{
	return id == RETRO_MEMORY_SYSTEM_RAM ? MainRamData : NULL;
}

size_t retro_get_memory_size(unsigned id)
{
	return id == RETRO_MEMORY_SYSTEM_RAM ? MainRamSize : 0;
}
