# the DRM of SimCity
Ugh. DRM. the bane of any modders existence.

## what's encrypted?
the Main exe (SimCity.exe) is encrypted, and nothing else....

## decrypting SimCity.exe (theoretical)
dump the memory of a running SimCity.exe, copy the .text to the original exe and that should work. haven't tested yet.

## how does origin do this?
core/activation.dll tells origin to do it, origin decrypts it using an AES method probably similar to how more modern games do it but with the .text section instead of the .ooa section.

## how to launch our own c++ code?
aside from using the exe decryption method, one could use a fake eawebkit.dll.
EAWebKit has one function that it exports called CreateEAWebKitInstance. with that we can create a dll that has that function, in which we can put whatever code we want alongside calling the actual EAWebKit function. this function is called exactly once at startup after the exe is decrypted. for a pre-decryption hook one could use core/activation.dll ordinal 100.

## what can we do with this hook?
- add some EAWebKit stuff (we have the concrete instance as returned, then wait for something to change in another thread)
- if we know the memory addresses, we can patch variables and functions at runtime, useful for a drop-in c++ mod.
- make a c++ modding framework ?