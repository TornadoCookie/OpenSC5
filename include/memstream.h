#ifndef _MEMSTREAM_H_
#define _MEMSTREAM_H_

#include <stdint.h>

typedef struct MemStream {
    void *buf;
    int size;
    void *cur;
} MemStream;

void memstream_write(MemStream *stream, void *buf, int size);
void memstream_writestream(MemStream *dest, MemStream *src);
void memstream_write32(MemStream *dest, uint32_t n);
void memstream_write16(MemStream *dest, uint16_t n);
void memstream_write8(MemStream *dest, uint8_t n);

MemStream memstream_read_create(unsigned char *data, int dataSize);
int memstream_read(MemStream *stream, void *buf, int size);
uint32_t memstream_read32(MemStream *stream);
uint16_t memstream_read16(MemStream *stream);
uint8_t  memstream_read8(MemStream *stream);

#endif

