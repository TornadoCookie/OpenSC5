#ifndef _MEMSTREAM_H_
#define _MEMSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>

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
uint64_t memstream_read64(MemStream *stream);
uint32_t memstream_read32(MemStream *stream);
uint16_t memstream_read16(MemStream *stream);
uint8_t  memstream_read8(MemStream *stream);
float memstream_readfloat(MemStream *stream);
void memstream_skip(MemStream *stream, int bytes);

Vector2 memstream_readv2(MemStream *stream);
Vector3 memstream_readv3(MemStream *stream);
BoundingBox memstream_readbbox(MemStream *stream);

Color memstream_readc3(MemStream *stream); // rgb 0-1

// Big-Endian
uint64_t memstream_readb64(MemStream *stream);
uint32_t memstream_readb32(MemStream *stream);
uint16_t memstream_readb16(MemStream *stream);
float memstream_readbfloat(MemStream *stream);

bool memstream_read_check(MemStream *stream, int size, unsigned char bytes[]); // true if equal

#ifdef __cplusplus
}
#endif

#endif

