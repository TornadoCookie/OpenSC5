
#include <EAWebKit/EAWebKit>
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef EA::WebKit::EAWebKitLib* (*PF_CreateEAWebkitInstance)(void);

extern "C" EA::WebKit::EAWebKitLib* CreateEAWebkitInstance(void)
{
    std::cout << "OpenSC5 CEAIHook v1.0" << std::endl;

    MessageBoxA(NULL, "Hello from OpenSC5.", "OpenSC5 EAWebKit Hook", MB_OK);
    
    HMODULE wdll = LoadLibraryA("EAWebkit_real.dll");
    if (!wdll)
    {
        std::cout << "ERROR: EAWebkit_real.dll not found." << std::endl;
        return nullptr;
    }

    PF_CreateEAWebkitInstance ceai = (PF_CreateEAWebkitInstance)GetProcAddress(wdll, "CreateEAWebkitInstance");
    if (!ceai)
    {
        std::cout << "ERROR: CreateEAWebkitInstance not found in EAWebkit_real.dll" << std::endl;
        return nullptr;
    }

    return ceai();
}
