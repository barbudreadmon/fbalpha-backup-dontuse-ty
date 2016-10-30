# fba-libretro

## Roms
Use clrmamepro (which run fine on linux with wine and macos with crossover) to build valid romsets with dats from the dats directory.
fba dats are NOT mame dats.
Don't report issues if you didn't build a valid romset.

## Samples

Samples should be put under SYSTEM_DIRECTORY/fba/samples

## hiscore.dat

Place hiscore.dat from http://highscore.mameworld.info/download.htm (use 'old format hiscore.dat (pre mame v0174)' version) in SAVE_DIRECTORY

## Raspberry Pi 2 users

Use this command to build :
<tt>make -f makefile.libretro platform=rpi2</tt>

## Known bugs

- Building with gcc-6.X (won't fix)
- Unstable load/save states.
