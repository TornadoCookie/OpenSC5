#include "memstream.h"
#include <string.h>
#include <stdlib.h>

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

    return toRead;
}

#define memstream_read_impl_(bits) \
uint##bits##_t memstream_read##bits(MemStream *stream) \
{ \
    uint##bits##_t val;\
    memstream_read(stream, &val, bits/8); \
    return val; \
}

memstream_read_impl_(32)
memstream_read_impl_(16)
memstream_read_impl_(8)

