#ifndef _CHAR16HELPERS_
#define _CHAR16HELPERS_

#ifdef __cplusplus
extern "C" {
#endif

char8_t *char16tochar8(const char16_t *str);
char16_t *char8tochar16(char8_t *str);

#ifdef __cplusplus 
}
#endif

#endif

