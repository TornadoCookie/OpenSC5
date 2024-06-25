#include "filetypes/package.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cpl_endian.h>
#include <raylib.h>

typedef struct PackageHeader {
    char magic[4];              //00
    uint32_t majorVersion;      //04
    uint32_t minorVersion;      //08
    uint32_t unknown[3];        //0C
    uint32_t dateCreated;       //18
    uint32_t dateModified;      //1C
    uint32_t indexMajorVersion; //20
    uint32_t indexEntryCount;   //24
    uint32_t firstIndexEntryOffset; //28
    uint32_t indexSize;         //2C
    uint32_t holeEntryCount;    //30
    uint32_t holeOffset;        //34
    uint32_t holeSize;          //38
    uint32_t indexMinorVersion; //3C
    uint32_t indexOffset;       //40
    uint32_t unknown2;          //44
    unsigned char reserved[24]; //48
                                //5C
} PackageHeader;

typedef struct IndexEntry {
    uint32_t type;
    uint32_t group;
    uint32_t instance;
    uint32_t chunkOffset;
    uint32_t diskSize;
    uint32_t memSize;
    uint16_t compressed;
    uint16_t unknown;
    
    bool isCompressed;
} IndexEntry;

typedef struct IndexData {
    uint32_t null;
} IndexData;

typedef struct Index {
    uint32_t indexType;
    
    IndexData data;
} Index;

static void readuint(uint32_t *ret, FILE *f)
{
    fread(ret, sizeof(uint32_t), 1, f);

    if (feof(f))
    {
        printf("Unexpected end of file: %lu.\n", ftell(f));
        exit(EXIT_FAILURE);
    }
}

#define SUPPORT_FILEFORMAT_GIF

typedef unsigned char stbi_uc;
#define STBIDEF extern

STBIDEF stbi_uc *stbi_load_gif_from_memory(stbi_uc const *buffer, int len, int **delays, int *x, int *y, int *z, int *comp, int req_comp);

// Load animated image data
//  - Image.data buffer includes all frames: [image#0][image#1][image#2][...]
//  - Number of frames is returned through 'frames' parameter
//  - All frames are returned in RGBA format
//  - Frames delay data is discarded
Image LoadImageAnimFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int *frames)
{
    Image image = { 0 };
    int frameCount = 0;

    // Security check for input data
    if ((fileType == NULL) || (fileData == NULL) || (dataSize == 0)) return image;

#if defined(SUPPORT_FILEFORMAT_GIF)
    if ((strcmp(fileType, ".gif") == 0) || (strcmp(fileType, ".GIF") == 0))
    {
        if (fileData != NULL)
        {
            int comp = 0;
            int *delays = NULL;
            image.data = stbi_load_gif_from_memory(fileData, dataSize, &delays, &image.width, &image.height, &frameCount, &comp, 4);

            image.mipmaps = 1;
            image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

            RL_FREE(delays);        // NOTE: Frames delays are discarded
        }
    }
#else
    if (false) { }
#endif
    else
    {
        image = LoadImageFromMemory(fileType, fileData, dataSize);
        frameCount = 1;
    }

    *frames = frameCount;
    return image;
}

static bool ProcessPackageData(unsigned char *data, int dataSize, uint32_t dataType, PackageEntry *pkgEntry)
{
    switch (dataType)
    {
        case PKGENTRY_PROP: // Properties files https://simswiki.info/wiki.php?title=Spore:00B1B104
        {
            PropData propData = LoadPropData(data, dataSize);
            pkgEntry->data.propData = propData;
            return !propData.corrupted;
        } break;
        case PKGENTRY_RAST: // Raster file https://simswiki.info/wiki.php?title=Spore:2F4E681C
        {
            RastData rastData = LoadRastData(data, dataSize);
            pkgEntry->data.imgData.img = rastData.img;
            if (!rastData.corrupted)
            {
                pkgEntry->data.imgData.tex = LoadTextureFromImage(rastData.img);
            }
            return !rastData.corrupted;            
        } break;
        case PKGENTRY_TEXT: // "textual" file.
        case PKGENTRY_JSON: // JSON file.
        {
            printf("JSON:\n");
            char *str = malloc(dataSize-2);
            memcpy(str, data+3, dataSize-3);
            str[dataSize-3] = 0;
            printf("%s\n", str);
            pkgEntry->data.scriptSource = str;
        } break;
        case PKGENTRY_RULE: // Binary rules file. https://community.simtropolis.com/forums/topic/55521-binary-rules-file-format/
        {
            RulesData rulesData = LoadRulesData(data, dataSize);
            pkgEntry->data.rulesData = rulesData;
            return !rulesData.corrupted;
        } break;
        case PKGENTRY_HTML:
        case PKGENTRY_CSS:
        case PKGENTRY_JSN8:
        case PKGENTRY_SCPT: // Script file format (?)
        {
            printf("Script info:\n");

            char *str = malloc(dataSize + 1);
            memcpy(str, data, dataSize);
            str[dataSize] = 0;
            printf("Script source: \"%s\"\n", str);
            pkgEntry->data.scriptSource = str;
        } break;
        case PKGENTRY_PNG: // PNG file.
        {
            pkgEntry->data.imgData.img = LoadImageFromMemory(".png", data, dataSize);
            pkgEntry->corrupted = !IsImageReady(pkgEntry->data.imgData.img);
            if (!pkgEntry->corrupted)
            {
                pkgEntry->data.imgData.tex = LoadTextureFromImage(pkgEntry->data.imgData.img);
            }
            return !pkgEntry->corrupted;
        } break;
        case PKGENTRY_SWB:
        case PKGENTRY_MOV:
        case PKGENTRY_EXIF:
        {
            return false;
        } break;
        case PKGENTRY_BNK:
        {
            pkgEntry->data.bnkData = LoadBnkData(data, dataSize);
        } break;
        case PKGENTRY_GIF:
        {
            pkgEntry->data.gifData.img = LoadImageAnimFromMemory(".gif", data, dataSize, &pkgEntry->data.gifData.frameCount);
            pkgEntry->corrupted = !IsImageReady(pkgEntry->data.gifData.img);
            if (!pkgEntry->corrupted)
            {
                pkgEntry->data.gifData.tex = LoadTextureFromImage(pkgEntry->data.gifData.img);
            }
            return !pkgEntry->corrupted;
        } break;
        default:
        {
            printf("Unknown data type %#X.\n", dataType);
            return false;
        } break;
    }

    return true;
}

static unsigned char *DecompressDBPF(unsigned char *data, int dataSize, int outDataSize)
{
    unsigned char *ret = malloc(outDataSize);
    unsigned char *initData = data;

    uint8_t compressionType = data[0];
    uint32_t uncompressedSize;

    printf("Compression Type: %#x\n", compressionType);

    if (compressionType & 0x80)
    {
        printf("Unrecognized compression type.\n");
        free(ret);
        return NULL;
    }

    memcpy(&uncompressedSize, data + 2, 3);
    //printf("Uncompressed Size: %d\n", uncompressedSize);

    data += 5;

    unsigned char *retCursor = ret;

    while (initData - data <= dataSize)
    {
        uint8_t byte0 = *data;
        data++;
        int numPlainText = 0;
        int numToCopy = 0;
        int copyOffset = 0;
        //printf("Control character: %#x\n", byte0);
        if (byte0 >= 0xE0 && byte0 <= 0xFB)
        {
            numPlainText = ((byte0 & 0x1F) + 1) * 4;
        }
        else if (byte0 >= 0x00 && byte0 <= 0x7F)
        {
            uint8_t byte1 = *data;
            data++;
            numPlainText = byte0 & 0x03;
            numToCopy = ((byte0 & 0x1C) >> 2) + 3;
            copyOffset = (((byte0 & 0x60) << 3) | byte1) + 1;
        }
        else if (byte0 >= 0x80 && byte0 <= 0xBF)
        {
            uint8_t byte1 = *data;
            data++;
            uint8_t byte2 = *data;
            data++;

            numPlainText = (byte1 >> 6);
            numToCopy = (byte0 & 0x3F) + 4;
            copyOffset = (((byte1 & 0x3F) << 8) | byte2) + 1;
        }
        else if (byte0 >= 0xFC & byte0 <= 0xFF)
        {
            numPlainText = byte0 & 0x03;
            numToCopy = 0; 
        }
        else if (byte0 >= 0xC0 && byte0 <= 0xDF)
        {
            uint8_t byte1 = *data;
            data++;
            uint8_t byte2 = *data;
            data++;
            uint8_t byte3 = *data;
            data++;

            numPlainText = byte0 & 0x03;
            numToCopy = (((byte0 & 0x0C) << 6) | byte3) + 5;
            copyOffset = ((((byte0 & 0x10) << 4) | byte1 << 8) | byte2) + 1;
        }
        else
        {
            printf("Unrecognized control character %#x.\n", byte0);
            return NULL;
        }

        memcpy(retCursor, data, numPlainText);
        retCursor += numPlainText;
        data += numPlainText;

        if (retCursor - copyOffset < ret)
        {
            printf("Invalid copyOffset. Ret=%p, retCursor - copyOffset = %p.\n", ret, retCursor - copyOffset - 1);
        }

        memcpy(retCursor, retCursor - copyOffset, numToCopy);
        retCursor += numToCopy;

        if (byte0 >= 0xFC & byte0 <= 0xFF) break;
    }

    return ret;
}

static const char *GetExtensionFromType(unsigned int type)
{
    switch (type)
    {
        case PKGENTRY_PROP: return "prop";
        case PKGENTRY_TEXT:
        case PKGENTRY_SCPT: return "txt";
        case PKGENTRY_RULE: return "rules";
        case PKGENTRY_JSN8:
        case PKGENTRY_JSON: return "json";
        case PKGENTRY_RAST: return "rast";
        case PKGENTRY_RW4: return "rw4";
        case PKGENTRY_PNG: return "png";
        case PKGENTRY_MOV: return "mov";
        case PKGENTRY_EXIF: return "jpg";
        case PKGENTRY_SWB: return "swb";
        default: return "unkn";
    }
}

Package LoadPackageFile(FILE *f)
{
    Package pkg = { 0 };
    PackageHeader header;
    fread(&header, sizeof(PackageHeader), 1, f);

    printf("Header:\n");
    printf("Magic: %.4s\n", header.magic);
    printf("Major Version #: %d\n", header.majorVersion);
    printf("Minor Version #: %d\n", header.minorVersion);
    printf("Index Entry Count: %d\n", header.indexEntryCount);
    printf("Index Size: %d\n", header.indexSize);
    printf("Index Major Version: %d\n", header.indexMajorVersion);
    printf("Index Minor Version: %d\n", header.indexMinorVersion);
    printf("Index Offset: %d\n", header.indexOffset);

    fseek(f, header.indexOffset, SEEK_SET);
    Index index = { 0 };

    readuint(&index.indexType, f);

    printf("\nIndex information:\n");
    printf("Index Type: %d\n", index.indexType);

    IndexData indexData = index.data;

    //readuint(&indexData.null, f);

    uint32_t indexTypeId = 0xCAFEBABE;
    if ((index.indexType & (1 << 0)) == 1 << 0)
    {
        readuint(&indexTypeId, f);
    }

    uint32_t indexGroupContainer = 0xCAFEBABE;
    if ((index.indexType & (1 << 1)) == 1 << 1)
    {
        readuint(&indexGroupContainer, f);
    }

    uint32_t indexUnknown = 0xCAFEBABE;
    if ((index.indexType & (1 << 2)) == 1 << 2)
    {
        readuint(&indexUnknown, f);
    }

    pkg.entryCount = header.indexEntryCount;

    IndexEntry *entries = malloc(sizeof(IndexEntry) * header.indexEntryCount);

    pkg.entries = malloc(sizeof(PackageEntry) * pkg.entryCount);

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        IndexEntry entry = { 0 };
        PackageEntry pkgEntry = { 0 };

        if ((index.indexType & (1 << 0)) == 1 << 0)
        {
            entry.type = indexTypeId;
        }
        else
        {
            readuint(&entry.type, f);
        }

        if ((index.indexType & (1 << 1)) == 1 << 1)
        {
            entry.group = indexGroupContainer;
        }
        else
        {
            readuint(&entry.group, f);
        }

        if ((index.indexType & (1 << 2)) == 1 << 2)
        {
            uint32_t unk = indexUnknown;
        }
        else
        {
            uint32_t unk;
            readuint(&unk, f);
        }

        readuint(&entry.instance, f);

        readuint(&entry.chunkOffset, f);

        readuint(&entry.diskSize, f);
        entry.diskSize &= ~0x80000000;
        readuint(&entry.memSize, f);
        fread(&entry.compressed, sizeof(uint16_t), 1, f);
        fread(&entry.unknown, sizeof(uint16_t), 1, f);

        entry.isCompressed = (entry.compressed == 0xFFFF);
        if (entry.compressed != 0xFFFF && entry.compressed != 0x0000)
        {
            printf("Error: Invalid value for compressed: %#x\n", entry.compressed);
        }

        printf("\nEntry %d:\n", i);
        printf("Type: %#X\n", entry.type);
        printf("Group: %#X\n", entry.group);
        printf("Instance: %#X\n", entry.instance);
        printf("Chunk Offset: %u\n", entry.chunkOffset);
        printf("Disk Size: %u\n", entry.diskSize);
        printf("Mem Size: %u\n", entry.memSize);
        printf("Compressed? %s\n", entry.isCompressed?"yes":"no");

        pkgEntry.type = entry.type;
        pkgEntry.group = entry.group;
        pkgEntry.instance = entry.instance;

        entries[i] = entry;
        pkg.entries[i] = pkgEntry;
    }

    printf("\nData Cycle.\n");

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        IndexEntry entry = entries[i];

        printf("\nEntry %d:\n", i);

        if (fseek(f, entry.chunkOffset, SEEK_SET) == -1)
        {
            perror("Unexpected error occurred");
        }

        unsigned char *data = malloc(entry.diskSize);

        fread(data, 1, entry.diskSize, f);

        if (feof(f))
        {
            printf("Unexpected end of file.\n");
        }

        if (entry.isCompressed)
        {
            unsigned char *uncompressed = DecompressDBPF(data, entry.diskSize, entry.memSize);
            if (uncompressed)
            {
                int toPrint = 10;
                if (!ProcessPackageData(uncompressed, entry.memSize, entry.type, &(pkg.entries[i])))
                {
                    FILE *f = fopen(TextFormat("corrupted/%#X-%#X-%#X.%s", entry.type, entry.group, entry.instance, GetExtensionFromType(entry.type)), "wb");
                    fwrite(uncompressed, 1, entry.memSize, f);
                    fclose(f);
                    pkg.entries[i].corrupted = true;
                }
                for (int i = 0; i < toPrint; i++)
                {
                    printf("%#x ", uncompressed[i]);
                }
                puts("");
                free(uncompressed);
            }
        }
        else
        {
            int toPrint = 10;
            if (!ProcessPackageData(data, entry.diskSize, entry.type, &(pkg.entries[i])))
            {
                FILE *f = fopen(TextFormat("corrupted/%#X-%#X-%#X.%s", entry.type, entry.group, entry.instance, GetExtensionFromType(entry.type)), "wb");
                fwrite(data, 1, entry.diskSize, f);
                fclose(f);
                pkg.entries[i].corrupted = true;
            }
            for (int i = 0; i < toPrint; i++)
            {
                printf("%#x ", data[i]);
            }
            puts("");
        }

        free(data);
    }

    free(entries);

    return pkg;
}

void UnloadPackageFile(Package pkg)
{
    free(pkg.entries);
}
