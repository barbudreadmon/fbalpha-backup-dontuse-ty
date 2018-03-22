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
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS2) {
		if (strcmp(pba->szName, "CpsRamFF") == 0) {
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
