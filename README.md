
To build for linux64: <br>
`$ make -j$(nproc)` <br>
To build for win64 (can only build on linux): <br>
`$ PLATFORM=win64 make -j$(nproc)` <br>

## Dependencies
- libcurl (for updater)
- EAWebKit 12 (bundled)
- raylib (bundled)

<br>This program is based on the source code to SimCityPak and SporeModder-FX, as well as several articles, the EAWebKit source code, simtropolis forum posts, and personal research.<br>

What do the programs do:
- updater: Downloads the SimCity 7z files into update/. Use updater --help for more usage info.
- test_package `file`: Prints information in parsing the .package file `file`.
- test_rast, test_rw4, test_heightmap, test_sdelta, test_prop `file`: Prints information in parsing the file format.
- test_crcbin `dir`: Prints information in parsing the directory containins .bin files.
- opensc5_editor: GUI editor based on the design of s3pe.
- opensc5: program which runs UI loaded from a SimCityData (changeable via code)
