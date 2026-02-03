
To build for linux64: <br>
`$ make -j$(nproc)` <br>
To build for win64 (can only build on linux): <br>
`$ PLATFORM=win64 make -j$(nproc)` <br>

Requires GNU make and gcc/mingw. <br>

<br>This program is based on the source code to SimCityPak and SporeModder-FX, as well as several articles, the EAWebKit source code, simtropolis forum posts, and personal research.<br>

What do the programs do:
- test_update: Downloads the SimCity game scripts into ./update. (You first have to run `mkdir update` for it to work.)
- test_package `file`: Prints information in parsing the .package file `file`.
- test_rast, test_rw4, test_heightmap, test_sdelta, test_prop `file`: Prints information in parsing the file format.
- test_crcbin `dir`: Prints information in parsing the directory containins .bin files.
- opensc5_editor: GUI editor based on the design of s3pe.
- opensc5: program which runs UI loaded from a SimCityData (changeable via code)

## Disk space warning ##
Running test_update will download ~80 GB of files from update.prod.simcity.com (the entire server contents.)
Running opensc5_editor will unpack ~1 GB of files into the corrupted folder because the program can't read it.
