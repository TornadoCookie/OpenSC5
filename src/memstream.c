#include "memstream.h"
#include <string.h>
#include <stdlib.h>
#include <cpl_endian.h>

void memstream_write(MemStream *stream, void *buf, int size)
{
    int oldSize = stream->size;
    stream->size += size;
    stream->buf = realloc(stream->buf, stream->size);
    memcpy(stream->buf + oldSize, buf, size);
}

void memstream_writestream(MemStream *dest, MemStream *src)
{
    memstream_write(dest, src->buf, src->size);
}

#define memstream_write_impl_(bits) \
void memstream_write##bits(MemStream *dest, uint##bits##_t n) \
{ \
    memstream_write(dest, &n, sizeof(n));\
}

memstream_write_impl_(32)
memstream_write_impl_(16)
memstream_write_impl_(8)

MemStream memstream_read_create(unsigned char *data, int dataSize)
{
    return (MemStream)
    {
        .buf = data,
        .size = dataSize,
        .cur = data
    };
}

int memstream_read(MemStream *stream, void *buf, int size)
{
    int toRead = size;
    int canRead = stream->size - (stream->cur - stream->buf);

    if (canRead < toRead)
    {
        toRead = canRead;
    }

    memcpy(buf, stream->cur, toRead);

    stream->cur += toRead;

    if (toRead == 0 && size != 0)
    {
        TraceLog(LOG_FATAL, "[memstream] attempt to read after buffer %p", stream->buf);
    }

    return toRead;
}

#define memstream_read_impl_(bits) \
uint##bits##_t memstream_read##bits(MemStream *stream) \
{ \
    uint##bits##_t val;\
    memstream_read(stream, &val, bits/8); \
    return val; \
}

memstream_read_impl_(64)
memstream_read_impl_(32)
memstream_read_impl_(16)
memstream_read_impl_(8)

float memstream_readfloat(MemStream *stream)
{
    float val;
    memstream_read(stream, &val, sizeof(float));
    return val;
}

void memstream_skip(MemStream *stream, int bytes)
{
    int canSkip = stream->size - (stream->cur - stream->buf);
    if (bytes > canSkip)
    {
        bytes = canSkip;
        TraceLog(LOG_FATAL, "[memstream] skipping more than possible on stream %p", stream->buf);
    }

    stream->cur += bytes;
}

Vector2 memstream_readv2(MemStream *stream)
{
    Vector2 val;
    val.x = memstream_readfloat(stream);
    val.y = memstream_readfloat(stream);
    return val;
}

Vector3 memstream_readv3(MemStream *stream)
{
    Vector3 val;
    val.x = memstream_readfloat(stream);
    val.y = memstream_readfloat(stream);
    val.z = memstream_readfloat(stream);
    return val;
}

BoundingBox memstream_readbbox(MemStream *stream)
{
    BoundingBox val;
    val.min = memstream_readv3(stream);
    val.max = memstream_readv3(stream);
    return val;
}

Color memstream_readc3(MemStream *stream)
{
    Color val;
    val.r = memstream_readfloat(stream)*255;
    val.g = memstream_readfloat(stream)*255;
    val.b = memstream_readfloat(stream)*255;
    val.a = 255;
    return val;
}

#define memstream_readb_impl_(bits) \
uint##bits##_t memstream_readb##bits(MemStream *stream) \
{ \
    return be##bits##toh(memstream_read##bits(stream)); \
} 

memstream_readb_impl_(64)
memstream_readb_impl_(32)
memstream_readb_impl_(16)

float memstream_readbfloat(MemStream *stream)
{
    float val;
    uint32_t u32val = memstream_readb32(stream);
    *(uint32_t*)&val = u32val;
    return val;
}

bool memstream_read_check(MemStream *stream, int size, unsigned char bytes[])
{
    unsigned char *readbytes = malloc(size);

    memstream_read(stream, readbytes, size);

    int result = memcmp(readbytes, bytes, size);

    free(readbytes);

    return !result;
}

