
To build for linux64: <br>
`$ make -j$(nproc)` <br>
To build for win64: <br>
`$ PLATFORM=win64 make -j$(nproc)` <br>

Requires GNU make and gcc/mingw. <br>
Building for windows is currently broken, I'll work on that some time. <br>

<br>This program is based on the source code to SimCityPak and SporeModder-FX.<br>

What do the programs do:
- test_update: Downloads the SimCity game scripts into ./update. (You first have to run `mkdir update` for it to work.)
- test_package `file`: Prints information in parsing the .package file `file`.
- test_crcbin `dir`: Prints information in parsing the directory containins .bin files.

Quick "style guide":
- Source files must not exceed 1000 lines. If it is longer than 1000 lines, break it up into smaller modules.
- Each source file must have it's own header file, unless it has an entrypoint (main() function).
