# building opensc5
for Linux, you must build EAWebKit12-Linux. on windows you should be able to use the EAWebKit.dll packaged with SimCity, as opensc5 uses the same version (pain included!)

## building EAWebKit 
download my project EAWebKit12-Linux and build from there.

## building opensc5
copy the EAWebKit dll or so. if on windows, rename it to EAWebKitd.dll. also good luck as you won't have EAWebKit debug messages or symbols.
- Linux: `make`
- windows: `make PLATFORM=win64` no guarantees... if you really really want it i give you the freedom to make it