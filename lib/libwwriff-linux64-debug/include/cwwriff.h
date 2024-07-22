#ifndef _CWWRIFF_H
#define _CWWRIFF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

#define VERSION "0.24"

typedef enum {
    NO_FORCE_PACKET_FORMAT,
    FORCE_MOD_PACKETS,
    FORCE_NO_MOD_PACKETS,
} CForcePacketFormat;

typedef struct WWRiff WWRiff;

WWRiff *WWRiff_Create(const char *name, const char *codebooks_name, bool inline_codebooks,
                      bool full_setup, CForcePacketFormat force_packet_format);

void WWRiff_PrintInfo(WWRiff *wwriff);

void WWRiff_GenerateOGG(WWRiff *wwriff, const char *outfilename);

#ifdef __cplusplus
}
#endif

#endif
