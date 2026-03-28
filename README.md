
To build for linux64: <br>
`$ make -j$(nproc)` <br>
To build for win32 (w64devkit or similar linux-like environmnent required): <br>
`$ PLATFORM=win32 make -j$(nproc)` <br>

Look at docs/building.md for more info.

## Dependencies
- libcurl (for updater)
- EAWebKit 12 (bundled)
- raylib (bundled)

## Contributing
If you are looking to help the project,
- If you can code in C or C++, the TODO file is full of little things to work on.
- If you know / discovered something about SimCity (or Spore, Darkspore, or The Sims 3) (Spark Engine) feel free to email me or add me on discord, both of which are linked on my GitHub account.
- Testing and reviewing the software is encouraged, although the software isn't quite ready for testing yet and still requires a bit of tinkering to get working.

<br>This program is based on the source code to SimCityPak and SporeModder-FX, as well as several articles, the EAWebKit source code, simtropolis forum posts, and personal research.<br>

What do the programs do:
- updater: Downloads the SimCity 7z files into update/. Use updater --help for more usage info.
- test_package `file`: Prints information in parsing the .package file `file`.
- test_rast, test_rw4, test_heightmap, test_sdelta, test_prop `file`: Prints information in parsing the file format.
- test_crcbin `dir`: Prints information in parsing the directory containins .bin files.
- opensc5_editor: GUI editor based on the design of s3pe.
- opensc5: program which runs UI loaded from a SimCityData (changeable via code)
