# fba-libretro

Build your sets with dats from the dats directory, fba dats are NOT mame dats.

## Samples

Samples should be put under <SYSTEM_DIRECTORY>/fba/samples

## Raspberry Pi 2 users

Use this command to build :
<tt>make -f makefile.libretro platform=rpi2</tt>

## Known bugs

- Irem m72/m92/m107 games and a few others based on the same nec v30 cpu (arm only ?)
- Ironclad neogeo
