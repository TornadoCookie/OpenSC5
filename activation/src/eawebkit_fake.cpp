
#include "../lib/libEAWebKitd-win32/include/EAWebKit/EAWebKit"
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef EA::WebKit::EAWebKitLib* (*PF_CreateEAWebkitInstance)(void);

extern "C" __attribute__ ((visibility ("default"))) EA::WebKit::EAWebKitLib* CreateEAWebkitInstance(void)
{
    std::cout << "OpenSC5 CEAIHook v1.0" << std::endl;
    
    MODULE wdll = LoadLibraryA("EAWebkit_real.dll");
    if (!wdll)
    {
        std::cout << "ERROR: EAWebkit_real.dll not found." << std::endl;
        return nullptr;
    }

    PF_CreateEAWebkitInstance ceai = GetProcAddress(wdll, "CreateEAWebkitInstance");
    if (!ceai)
    {
        std::cout << "ERROR: CreateEAWebkitInstance not found in EAWebkit_real.dll" << std::endl;
        return nullptr;
    }

    return ceai();
}
