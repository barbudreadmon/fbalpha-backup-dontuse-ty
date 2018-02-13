[![Build Status](https://travis-ci.org/libretro/fbalpha.svg?branch=master)](https://travis-ci.org/libretro/fbalpha)
[![Build status](https://ci.appveyor.com/api/projects/status/bdj5xf7t3kgbk1p7/branch/master?svg=true)](https://ci.appveyor.com/project/bparker06/fbalpha/branch/master)

# fba-libretro

## Roms
Use clrmamepro (which run fine on linux with wine and macos with crossover) to build valid romsets with dats from the dats directory.
fba dats are NOT mame dats.
Don't report issues if you didn't build a valid romset.

## Samples

Samples should be put under SYSTEM_DIRECTORY/fba/samples

## hiscore.dat

Move hiscore.dat from /metadata/ to /SYSTEM_DIRECTORY/fba/

## Known issues

- There is no support for MIDWAY hardware in this port
- There is no support for unibios 3.3 at the moment (will be included when it is included upstream)
