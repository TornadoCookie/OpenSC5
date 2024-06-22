#include "filetypes/package.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <endian.h>
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

typedef struct RasterFileHeader {
    uint32_t type;
    uint32_t width;
    uint32_t height;
    uint32_t mipmapct;
    uint32_t pixelwidth;
    uint32_t pixelformat;
} RasterFileHeader;

typedef struct RasterFileImage {
    uint32_t blocksize;
    char *data;
} RasterFileImage;

typedef struct RasterFile {
    RasterFileHeader header;
    RasterFileImage *images;
} RasterFile;

typedef struct RulesFileHeader {
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint8_t unknown4:8;
    uint32_t ruleCount;
} RulesFileHeader;

typedef struct RulesFileRuleExtra {
    char unknown[12];
} RulesFileRuleExtra;

typedef struct RulesFileRule {
    uint32_t ruleName;
    char unknown1[32];
    uint32_t startOffset;
    char unknown2[12];
    uint32_t endOffset;
    char unknown3[92];
    uint32_t extraCount;
    char unknown4[8];
} RulesFileRule;

typedef struct RulesFile {
    RulesFileHeader header;
    RulesFileRule *rules;
    RulesFileRuleExtra **extras;
} RulesFile;

float htobefloat(float x)
{
    uint32_t val = *(uint32_t*)&x;
    val = htobe32(val);
    float ret = *(float*)&val;
    return ret;
}

Vector2 vec2tobe(Vector2 v)
{
    return (Vector2)
    {
        htobefloat(v.x),
        htobefloat(v.y),
    };
}

Vector3 vec3tobe(Vector3 v)
{
    return (Vector3)
    {
        htobefloat(v.x),
        htobefloat(v.y),
        htobefloat(v.z),
    };
}

static bool ProcessPackageData(unsigned char *data, int dataSize, uint32_t dataType, PackageEntry *pkgEntry)
{
    switch (dataType)
    {
        case 0x00B1B104: // Properties files https://simswiki.info/wiki.php?title=Spore:00B1B104
        {
            uint32_t variableCount = htobe32(*(uint32_t*)data);
            PropData propData;

            data += sizeof(uint32_t);

            printf("Properties Info:\n");
            printf("Variable count: %d\n", variableCount);

            propData.variableCount = variableCount;
            propData.variables = malloc(sizeof(PropVariable) * propData.variableCount);
            
            for (int i = 0; i < variableCount; i++)
            {
                printf("\nVariable %d:\n", i);

                uint32_t identifier = htobe32(*(uint32_t*)data);
                data += sizeof(uint32_t);
                uint16_t type = htobe16(*(uint16_t*)data);
                data += sizeof(uint16_t);
                uint16_t specifier = htobe16(*(uint16_t*)data);
                data += sizeof(uint16_t);

                printf("Identifier: %#x\n", identifier);
                printf("Type: %#x\n", type);
                printf("Specifier: %#x\n", specifier);

                propData.variables[i].identifier = identifier;
                propData.variables[i].type = type;

                int32_t arrayNumber = 1;
                int32_t arraySize = 0;
                bool isArray = false;

                if ((specifier & 0x30) == 0)
                {
                    isArray = false;
                }
                else if ((specifier & 0x40) == 0)
                {
                    isArray = true;
                    arrayNumber = htobe32(*(int32_t*)data);
                    data += sizeof(int32_t);

                    arraySize = htobe32(*(int32_t*)data);
                    data += sizeof(int32_t);
                    
                    arraySize &= ~0x9C000000;

                    printf("Array nmemb: %d\n", arrayNumber);
                    printf("Array item size: %d\n", arraySize);
                }
                else
                {
                    printf("Invalid specifier.\n");
                    return false;
                }

                propData.variables[i].count = arrayNumber;
                propData.variables[i].values = malloc(sizeof(*propData.variables[i].values) * arrayNumber);

                for (int j = 0; j < arrayNumber; j++)
                {
                    switch (type)
                    {
                        case 0x20: // key type
                        {
                            uint32_t file = *(uint32_t*)data;
                            data += sizeof(uint32_t);
                            uint32_t type = *(uint32_t*)data;
                            data += sizeof(uint32_t);
                            uint32_t group = *(uint32_t*)data;
                            data += sizeof(uint32_t);

                            if (!isArray)
                            {
                                //data += sizeof(uint32_t);
                            }

                            printf("File: %#x\n", file);
                            printf("Type: %#x\n", type);
                            printf("Group: %#x\n", group);

                            propData.variables[i].values[j].keys.file = file;
                            propData.variables[i].values[j].keys.group = group;
                            propData.variables[i].values[j].keys.type = type;
                        } break;
                        case 9: // int32 type
                        {
                            int32_t value = htobe32(*(int32_t*)data);
                            data += sizeof(int32_t);

                            printf("Value: %#x\n", value);

                            propData.variables[i].values[j].int32 = value;
                        } break;
                        case 0x32: // colorRGB type
                        {
                            float r = htobefloat(*(float*)data);
                            data += sizeof(float);
                            float g = htobefloat(*(float*)data);
                            data += sizeof(float);
                            float b = htobefloat(*(float*)data);
                            data += sizeof(float);

                            if (!isArray)
                            {
                                //data += sizeof(uint32_t);
                            }

                            printf("Value: {%f, %f, %f}\n", r, g, b);

                            propData.variables[i].values[j].colorRGB.r = r;
                            propData.variables[i].values[j].colorRGB.g = g;
                            propData.variables[i].values[j].colorRGB.b = b;
                        } break;
                        case 0x13: // string type
                        {
                            uint32_t length = htobe32(*(uint32_t*)data);
                            data += sizeof(uint32_t);

                            printf("Length %d\n", length);

                            char *str = malloc(length + 1);

                            for (int i = 0; i < length; i++)
                            {
                                str[i] = data[i*2+1];
                            }

                            data += length * 2;

                            str[length] = 0;

                            printf("Value: %s\n", str);

                            propData.variables[i].values[j].string = str;
                        } break;
                        case 0x0a: // uint32 type
                        {
                            uint32_t value = htobe32(*(uint32_t*)data);
                            data += sizeof(uint32_t);

                            printf("Value: %u\n", value);

                            propData.variables[i].values[j].uint32 = value;
                        } break;
                        case 0x12: // string8 type
                        {
                            uint32_t length = htobe32(*(uint32_t*)data);
                            data += sizeof(uint32_t);

                            char *str = malloc(length + 1);
                            memcpy(str, data, length);
                            str[length] = 0;
                            data += length;

                            printf("Value: %s\n", str);

                            propData.variables[i].values[j].string8 = str;
                        } break;
                        case 0x0d: // float type
                        {
                            float value = htobefloat(*(float*)data);
                            data += sizeof(float);

                            printf("Value: %f\n", value);

                            propData.variables[i].values[j].f = value;
                        } break;
                        case 0x30: // vector2 type
                        {
                            // Raylib's vector2 type happens to fit nicely with the description.
                            Vector2 val = vec2tobe(*(Vector2*)data);
                            data += sizeof(Vector2);

                            printf("Value: {%f, %f}\n", val.x, val.y);

                            propData.variables[i].values[j].vector2 = val;
                        } break;
                        case 0x31: // vector3 type
                        {
                            // Raylib's vector3 type happens to fit nicely with the description.
                            Vector3 val = vec3tobe(*(Vector3*)data);
                            data += sizeof(Vector3);

                            printf("Value: {%f, %f, %f}\n", val.x, val.y, val.z);

                            propData.variables[i].values[j].vector3 = val;

                            if (!isArray)
                            {
                                //data += sizeof(uint32_t);
                            }
                        } break;
                        case 0x01: // bool type
                        {
                            bool val = *(bool*)data;
                            data += sizeof(bool);

                            printf("Value: %s\n", val?"true":"false");

                            propData.variables[i].values[j].b = val;
                        } break;
                        case 0x22: // texts type
                        {
                            if (!isArray)
                            {
                                arrayNumber = *(uint32_t*)data;
                                data += sizeof(uint32_t);
                                arraySize = *(uint32_t*)data;
                                data += sizeof(uint32_t);
                            }

                            uint32_t arrSize = arraySize - 8;

                            uint32_t textsFileSpec = *(uint32_t*)data;
                            data += sizeof(uint32_t);
                            uint32_t textsIdentifier = *(uint32_t*)data;
                            data += sizeof(uint32_t);

                            printf("Texts file spec: %#x\n", textsFileSpec);
                            printf("Texts Identifier: %#x\n", textsIdentifier);

                            propData.variables[i].values[j].texts.fileSpec = textsFileSpec;
                            propData.variables[i].values[j].texts.identifier = textsIdentifier;

/*
                            char *str = malloc(arrSize + 1);

                            for (int i = 0; i < arrSize /2; i++)
                            {
                                str[i] = data[i*2+1];
                            }
                            str[arrSize] = 0;

                            printf("Value: %s\n", str);
                            data += arrSize;*/

                        } break;
                        case 0x39: // bbox type
                        {
                            // Raylib's BoundingBox type happens to fit nicely with the description.
                            BoundingBox bbox = { 0 };

                            memcpy(&bbox, data, sizeof(BoundingBox));
                            data += sizeof(BoundingBox);

                            bbox.min = vec3tobe(bbox.min);
                            bbox.max = vec3tobe(bbox.max);

                            if (arrayNumber != 1)
                            {
                                printf("Array number expected to be 1.\n");
                                return false;
                            }

                            printf("Value: min {%f, %f, %f}, max {%f, %f, %f}\n", 
                                bbox.min.x, bbox.min.y, bbox.min.z, bbox.max.x, bbox.max.y, bbox.max.z);
                            
                            propData.variables[i].values[j].bbox = bbox;
                        } break;
                        default:
                        {
                            printf("Unrecognized variable type.\n");
                            return false;
                        } break;
                    }
                }
            }
            pkgEntry->data.propData = propData;
        } break;
        case 0x2F4E681C: // Raster file https://simswiki.info/wiki.php?title=Spore:2F4E681C
        {
            printf("Raster info:\n");
            RasterFile file = { 0 };

            memcpy(&file.header, data, sizeof(RasterFileHeader));
            data += sizeof(RasterFileHeader);

            file.header.type = htobe32(file.header.type);
            file.header.width = htobe32(file.header.width);
            file.header.height = htobe32(file.header.height);
            file.header.mipmapct = htobe32(file.header.mipmapct);
            file.header.pixelwidth = htobe32(file.header.pixelwidth);
            file.header.pixelformat = htobe32(file.header.pixelformat);

            printf("Type: %d\n", file.header.type);
            printf("Width: %d\n", file.header.width);
            printf("Height: %d\n", file.header.height);
            printf("Mipmap Count: %d\n", file.header.mipmapct);
            printf("PixelWidth: %d\n", file.header.pixelwidth);
            printf("PixelFormat: %#x\n", file.header.pixelformat);

            return false;

            file.images = malloc(sizeof(RasterFileImage) * file.header.mipmapct);

            for (int i = 0; i < file.header.mipmapct; i++)
            {
                RasterFileImage img = { 0 };

                img.blocksize = *(uint32_t*)data;
                data += sizeof(uint32_t);

                printf("Image %d: Blocksize %d\n", i, img.blocksize);
            }
            
        } break;
        case 0x0A98EAF0: // JSON file.
        {
            printf("JSON:\n");
            printf("%s\n", data);
        } break;
        case 0x08068AEB: // Binary rules file. https://community.simtropolis.com/forums/topic/55521-binary-rules-file-format/
        {
            RulesFile file = { 0 };

            printf("Rules info:\n");

            unsigned char *initData = data;

            // C isn't letting us misalign our memory like this, unknown4 is being treated like it's 4 bytes long.
            // That is why we can't just memcpy like that.
            data += sizeof(uint32_t); // unknown1
            data += sizeof(uint32_t); // unknown2
            data += sizeof(uint32_t); // unknown3
            data ++;                  // unknown4

            file.header.ruleCount = *(uint32_t*)data;
            data += sizeof(uint32_t);

            file.header.ruleCount = htobe32(file.header.ruleCount);

            printf("Rule count: %#x\n", file.header.ruleCount);

            //file.rules = malloc(file.header.ruleCount * sizeof(RulesFileRule));

            for (int i = 0; i < file.header.ruleCount; i++)
            {
                RulesFileRule rule = { 0 };

                memcpy(&rule, data, sizeof(RulesFileRule));
                data += sizeof(RulesFileRule);

                if (data - initData > dataSize)
                {
                    printf("Invalid data.\n");
                    return false;
                }

                rule.startOffset = htobe32(rule.startOffset);
                //rule.endOffset = htobe32(rule.endOffset);
                //rule.extraCount = htobe32(rule.extraCount);

                printf("\nRule %d:\n", i);
                printf("Name: %#x\n", rule.ruleName);
                printf("Start Offset: %d\n", rule.startOffset);
                printf("End Offset: %d\n", rule.endOffset);
                printf("Extra count: %d\n", rule.extraCount);

                if (rule.extraCount == 0xffff)
                {
                    rule.extraCount = 0;
                }

                data += rule.extraCount * sizeof(RulesFileRuleExtra);
            }

            data += sizeof(uint32_t);
            uint32_t unknown1count = *(uint32_t*)data;
            printf("unknown1count: %d\n", unknown1count);
            if (unknown1count > 1000)
            {
                printf("I'm gonna assume this is an invalid value. Please FIX!!!\n");
                return false;
            }
            data += sizeof(uint32_t);
            data += unknown1count * 0x5c;

            data += sizeof(uint32_t);
            uint32_t unknown2count = htobe32(*(uint32_t*)data);
            printf("unknown2count: %d\n", unknown2count);
            if (unknown2count > 1000)
            {
                printf("I'm gonna assume this is an invalid value. Please FIX!!!\n");
                return false;
            }
            data += sizeof(uint32_t);
            data += unknown2count * 0x70;

            data += sizeof(uint32_t);
            uint32_t unknown3count = *(uint32_t*)data;
            printf("unknown3count: %d\n", unknown3count);
            data += sizeof(uint32_t);
            if (unknown3count)
            {
                printf("Unknown 3 has positive count.\n");
                printf("offset: %#lx\n", data - initData);
                return false;
            }
            //data += unknown3count;

            data += sizeof(uint32_t);
            uint32_t unknown4count = *(uint32_t*)data;
            printf("unknown4count: %d\n", unknown4count);
            data += sizeof(uint32_t);
            if (unknown4count)
            {
                printf("Unknown 4 has positive count.\n");
                printf("offset: %#lx\n", data - initData);
                return false;
            }
            //data += unknown4count;

            data += sizeof(uint32_t);
            //data += sizeof(uint32_t);
            uint32_t codeLength = htobe32(*(uint32_t*)data);
            data += sizeof(uint32_t);
            printf("codeLength: %d\n", codeLength);

            printf("offset: %#lx\n", data - initData);
            
        } break;
        case 0x24A0E52: // Script file format (?)
        {
            printf("Script info:\n");

            char *str = malloc(dataSize + 1);
            memcpy(str, data, dataSize);
            str[dataSize] = 0;
            printf("Script source: \"%s\"\n", str);
            pkgEntry->data.scriptSource = str;
        } break;
        default:
        {
            printf("Unknown data type %#X.\n", dataType);
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

    if (compressionType != 0x10)
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
                int toPrint = entry.memSize;
                if (!ProcessPackageData(uncompressed, entry.memSize, entry.type, &(pkg.entries[i]))) toPrint = entry.memSize;
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
            int toPrint = entry.diskSize;
            if (!ProcessPackageData(data, entry.diskSize, entry.type, &(pkg.entries[i]))) toPrint = entry.diskSize;
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
