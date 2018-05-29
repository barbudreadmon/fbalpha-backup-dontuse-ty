[![Build Status](https://travis-ci.org/libretro/fbalpha.svg?branch=master)](https://travis-ci.org/libretro/fbalpha)
[![Build status](https://ci.appveyor.com/api/projects/status/bdj5xf7t3kgbk1p7/branch/master?svg=true)](https://ci.appveyor.com/project/bparker06/fbalpha/branch/master)

# fba-libretro

## Roms

Use clrmamepro (which runs fine on linux with wine and on mac with crossover) to build valid romsets with dats from the [dats](dats/) directory.
fba dats are NOT mame dats.
Don't report issues if you didn't build a valid romset.
Also, i don't provide a "parent-only" dat file, this is usually a bad idea to only use parent roms (some don't work, and some clones are interesting)

## Emulating consoles

You can emulate consoles (with specific romsets, dats are also in the [dats](dats/) directory) by prefixing the name of the roms with `XXX_` and removing the `zip|7z` extension, or using the `--subsystem XXX` argument in the command line, here is the list of available prefix :
* CBS ColecoVision : `cv`
* MSX 1 : `msx`
* Nec PC-Engine : `pce`
* Nec SuperGrafX : `sgx`
* Nec TurboGrafx-16 : `tg`
* Sega GameGear : `gg`
* Sega Master System : `sms`
* Sega Megadrive : `md`
* Sega SG-1000 : `sg1k`

## Samples

Samples should be put under SYSTEM_DIRECTORY/fba/samples

## hiscore.dat

Copy [hiscore.dat](metadata/hiscore.dat) to SYSTEM_DIRECTORY/fba/

## Mapping

We don't have a convenient tool like the MAME OSD, instead we use the retroarch api to customize mappings, you can do that by going into `Quick menu > Controls`.
For those who don't want to fully customize their mapping, there are 2 convenient presets you can apply by changing the "device type" for a player in this menu :
* **Classic** : it will apply the original neogeo cd "square" mapping in neogeo games, and use L/R as 5th and 6th button for 6 buttons games like Street Fighter II.
* **Modern** : it will apply the King of fighters mapping from Playstation 1 and above in neogeo fighting games, and it will use R1/R2 as 5th and 6th button for 6 buttons games like Street Fighter II (for the same reason as the neogeo games), this is really convenient for most arcade sticks.

## Frequently asked questions

### Where can i find the XXX roms ?
We don't provide links for roms. Google is your friend.

### I can't launch Killer Instinct, Rampage, Mortal Kombat 3 and other MIDWAY games, why ?
Support for those games was removed from the libretro port.
They are buggy in FB Alpha and have serious compatibility issues with this port. 
Let's hope someone will fix them in a future release of FB Alpha.

### Game XXX is not launching, why ?
It is either not supported or you have a bad rom. Build a valid romset with clrmamepro as said above.
There is also a few games marked as not working, try one of their clones.

### Game XXX has graphical glitches, why ?
Most likely the same as above, make sure you have the right romset.
If the problem persist, write a report with a screenshot and the name of the platform you are using.

### Game XXX runs slowly, why ?
Your hardware is probably too slow to run the game with normal settings. Try the following :
* Check if there is a speedhack dipswitch in the core options, set it to "yes".
* Try disabling rewind, runahead, and anything related to the savestates system in retroarch.
* Try lowering audio settings in the core options.
If it is not enough, upgrade or overclock your hardware.

### Game XXX has choppy sound, why ?
Most likely for the same reason as above.

### Game XXX runs faster in MAME2003/MAME2010, why ?
This is not MAME, we are using different code. 
Overall, FB Alpha is slower than old MAME version but more accurate and less buggy.
This libretro port also support various features which are usually buggy or absent in MAME cores (netplay, rewind, retroachievements, ...). It takes some resources.

### Cheat code doesn't work, why ?
This is not implemented yet.

### Neogeo CD doesn't work, why ?
The support is still a work in progress, there are several things to know :
* You need a copy of neocdz.zip and neogeo.zip in your libretro system directory
* You need to add `--subsystem neocd` to the command line
* you can't load `MODE1/2352` tracks at the moment
