#include <stdio.h>
#include <stdlib.h>

#define DllImport   __declspec( dllimport )
#define DllExport   __declspec( dllexport )

/* chat HELP */
DllExport void LaunchActivation(void)
{
    printf("LONG LIVE SIMCITY, hello from Core/Activation.dll\n");
    exit(EXIT_SUCCESS);
}

