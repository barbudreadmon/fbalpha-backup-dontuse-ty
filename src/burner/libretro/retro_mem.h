#ifndef __RETRO_MEM__
#define __RETRO_MEM__

extern unsigned char *MainRamData;
extern size_t MainRamSize;
extern bool bMainRamFound;

int StateGetMainRamAcb(BurnArea *pba);

#endif
