#ifndef _WWRIFF_H_
#define _WWRIFF_H_

#include <cpl_raylib.h>

void ExportWWRiffToFile(unsigned char *data, int dataSize, const char *filename);
Music LoadWWRiffMusic(unsigned char *data, int dataSize);
Wave LoadWWRiffWave(unsigned char *data, int dataSize);

#endif

