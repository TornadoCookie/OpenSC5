#include <EABase/eabase.h>
#include "char16helpers.h"

char8_t *char16tochar8(const char16_t *str)
{
    static char8_t result[1024];

    for (int i = 0; str[i] != 0 && i < 1024; i++)
    {
        result[i] = str[i];
    }

    return result;
}

char16_t *char8tochar16(char8_t *str)
{
    static char16_t result[1024];

    for (int i = 0; str[i] != 0 && i < 1024; i++)
    {
        result[i] = str[i];
    }

    return result;
}

