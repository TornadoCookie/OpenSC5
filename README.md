
Building:
`$ make -j$(nproc)`

What do the programs do:
- test_update: Downloads the SimCity game scripts into ./update. (You first have to run `mkdir update` for it to work.)
- test_package `file`: Prints information in parsing the .package file `file`.
