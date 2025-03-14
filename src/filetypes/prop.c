#include "filetypes/prop.h"
#include <stdint.h>
#include <cpl_endian.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "hash.h"

static float htobefloat(float x)
{
    uint32_t val = *(uint32_t*)&x;
    val = htobe32(val);
    float ret = *(float*)&val;
    return ret;
}

static Vector2 vec2tobe(Vector2 v)
{
    return (Vector2)
    {
        htobefloat(v.x),
        htobefloat(v.y),
    };
}

static Vector3 vec3tobe(Vector3 v)
{
    return (Vector3)
    {
        htobefloat(v.x),
        htobefloat(v.y),
        htobefloat(v.z),
    };
}

static Vector4 vec4tobe(Vector4 v)
{
    return (Vector4)
    {
        htobefloat(v.x),
        htobefloat(v.y),
        htobefloat(v.z),
        htobefloat(v.w),
    };
}

PropData LoadPropData(unsigned char *data, int dataSize)
{
    unsigned char *initData = data;
    uint32_t variableCount = htobe32(*(uint32_t *)data);
    PropData propData = { 0 };

    data += sizeof(uint32_t);

    TRACELOG(LOG_DEBUG, "Properties Info:\n");
    TRACELOG(LOG_DEBUG, "Variable count: %d\n", variableCount);

    propData.variableCount = variableCount;
    propData.variables = malloc(sizeof(PropVariable) * propData.variableCount);

    for (int i = 0; i < variableCount; i++)
    {
        if (data - initData > dataSize && i != variableCount - 1)
        {
            TRACELOG(LOG_DEBUG, "{Corruption Detected: Variable}\n");
            propData.corrupted = true;
            return propData;
        }

        TRACELOG(LOG_DEBUG, "\nVariable %d:\n", i);

        uint32_t identifier = htobe32(*(uint32_t *)data);
        data += sizeof(uint32_t);
        uint16_t type = htobe16(*(uint16_t *)data);
        data += sizeof(uint16_t);
        uint16_t specifier = htobe16(*(uint16_t *)data);
        data += sizeof(uint16_t);

        TRACELOG(LOG_DEBUG, "Identifier: %#x\n", identifier);
        TRACELOG(LOG_DEBUG, "Type: %#x\n", type);
        TRACELOG(LOG_DEBUG, "Specifier: %#x\n", specifier);

        type &= 0xFF;

        if (type == 0 && specifier == 0)
        {
            data += sizeof(uint32_t);
            continue;
        }

        propData.variables[i].identifier = identifier;
        propData.variables[i].type = type;

        int32_t arrayNumber = 1;
        int32_t arraySize = 0;
        bool isArray = false;
        
        if (specifier == 0x80FF) specifier &= ~0x30;

        if ((specifier & 0x30) && (specifier & 0x40) == 0)
        {
            isArray = true;
            arrayNumber = htobe32(*(int32_t *)data);
            data += sizeof(int32_t);

            arraySize = htobe32(*(int32_t *)data);
            data += sizeof(int32_t);
/*
            arraySize &= ~0x9C000000;*/

            arrayNumber &= 0xFF;
            arraySize &= 0xFF;

            TRACELOG(LOG_DEBUG, "Array nmemb: %#x\n", arrayNumber);
            TRACELOG(LOG_DEBUG, "Array item size: %#x\n", arraySize);
        }/*
        else
        {
            printf("Invalid specifier.\n");
            propData.corrupted = true;
            return propData;
        }*/

        void *arrayInitData = data;

       if (arrayNumber & 0x40)
       {
            continue;
       }

       if (type == 0x38) arraySize -= 6;

        propData.variables[i].count = arrayNumber;
        propData.variables[i].values = malloc(sizeof(*propData.variables[i].values) * arrayNumber);

        for (int j = 0; j < arrayNumber; j++)
        {
            if (data - initData > dataSize && j != arrayNumber - 1)
            {
                TRACELOG(LOG_DEBUG, "{Corruption Detected: Array}\n");
                propData.corrupted = true;
                return propData;
            }
            switch (type)
            {
                case 0x20: // key type
                {
                    uint32_t file = htobe32(*(uint32_t *)data);
                    data += sizeof(uint32_t);
                    uint32_t type = htobe32(*(uint32_t *)data);
                    data += sizeof(uint32_t);
                    uint32_t group = htobe32(*(uint32_t *)data);
                    data += sizeof(uint32_t);

                    if (data - initData > dataSize && j != arrayNumber - 1)
                    {
                        TRACELOG(LOG_DEBUG, "{Corruption Detected: Key Type}\n");
                        propData.corrupted = true;
                        return propData;
                    }

                    if (!isArray)
                    {
                        // data += sizeof(uint32_t);
                    }

                    TRACELOG(LOG_DEBUG, "File: %#x\n", file);
                    TRACELOG(LOG_DEBUG, "Type: %#x\n", type);
                    TRACELOG(LOG_DEBUG, "Group: %#x\n", group);

                    propData.variables[i].values[j].keys.file = file;
                    propData.variables[i].values[j].keys.group = group;
                    propData.variables[i].values[j].keys.type = type;
                } break;
                case 9: // int32 type
                {
                    int32_t value = htobe32(*(int32_t *)data);
                    data += sizeof(int32_t);

                    TRACELOG(LOG_DEBUG, "Value: %#x\n", value);

                    propData.variables[i].values[j].int32 = value;
                } break;
                case 0x32: // colorRGB type
                {
                    float r = htobefloat(*(float *)data);
                    data += sizeof(float);
                    float g = htobefloat(*(float *)data);
                    data += sizeof(float);
                    float b = htobefloat(*(float *)data);
                    data += sizeof(float);

                    if (!isArray)
                    {
                        // data += sizeof(uint32_t);
                    }

                    TRACELOG(LOG_DEBUG, "Value: {%f, %f, %f}\n", r, g, b);

                    propData.variables[i].values[j].colorRGB.r = r;
                    propData.variables[i].values[j].colorRGB.g = g;
                    propData.variables[i].values[j].colorRGB.b = b;
                } break;
                case 0x13: // string type
                {
                    uint32_t length = htobe32(*(uint32_t *)data);
                    data += sizeof(uint32_t);

                    length &= 0xFF;

                    TRACELOG(LOG_DEBUG, "Length %d\n", length);

                    if ((data - initData) + length > dataSize)
                    {
                        TRACELOG(LOG_DEBUG, "{Corruption detected.}\n");
                        propData.corrupted = true;
                        return propData;
                    }

                    char *str = malloc(length + 1);

                    for (int i = 0; i < length; i++)
                    {
                        str[i] = data[i * 2 + 1];
                    }

                    data += length * 2;

                    str[length] = 0;

                    TRACELOG(LOG_DEBUG, "Value: %s\n", str);

                    propData.variables[i].values[j].string = str;
                } break;
                case 0x0a: // uint32 type
                {
                    uint32_t value = htobe32(*(uint32_t *)data);
                    data += sizeof(uint32_t);

                    TRACELOG(LOG_DEBUG, "Value: %u\n", value);

                    propData.variables[i].values[j].uint32 = value;
                } break;
                case 0x12: // string8 type
                {
                    uint32_t length = htobe32(*(uint32_t *)data);
                    data += sizeof(uint32_t);

                    if ((data - initData) + length > dataSize)
                    {
                        TRACELOG(LOG_DEBUG, "{Corruption detected.}\n");
                        propData.corrupted = true;
                        return propData;
                    }

                    char *str = malloc(length + 1);
                    memcpy(str, data, length);
                    str[length] = 0;
                    data += length;

                    TRACELOG(LOG_DEBUG, "Value: %s\n", str);

                    propData.variables[i].values[j].string8 = str;
                } break;
                case 0x0d: // float type
                {
                    float value = htobefloat(*(float *)data);
                    data += sizeof(float);

                    TRACELOG(LOG_DEBUG, "Value: %f\n", value);

                    propData.variables[i].values[j].f = value;
                } break;
                case 0x30: // vector2 type
                {
                    // Raylib's vector2 type happens to fit nicely with the description.
                    Vector2 val = vec2tobe(*(Vector2 *)data);
                    data += sizeof(Vector2);

                    TRACELOG(LOG_DEBUG, "Value: {%f, %f}\n", val.x, val.y);

                    propData.variables[i].values[j].vector2 = val;
                } break;
                case 0x31: // vector3 type
                {
                    // Raylib's vector3 type happens to fit nicely with the description.
                    Vector3 val = vec3tobe(*(Vector3 *)data);
                    data += sizeof(Vector3);

                    TRACELOG(LOG_DEBUG, "Value: {%f, %f, %f}\n", val.x, val.y, val.z);

                    propData.variables[i].values[j].vector3 = val;

                    if (!isArray)
                    {
                        // data += sizeof(uint32_t);
                    }
                } break;
                case 0x01: // bool type
                {
                    bool val = *(bool *)data;
                    data += sizeof(bool);

                    TRACELOG(LOG_DEBUG, "Value: %s\n", val ? "true" : "false");

                    propData.variables[i].values[j].b = val;
                } break;
                case 0x22: // texts type
                {
                    if (!isArray)
                    {
                        arrayNumber = *(uint32_t *)data;
                        data += sizeof(uint32_t);
                        arraySize = *(uint32_t *)data;
                        data += sizeof(uint32_t);
                    }

                    uint32_t arrSize = arraySize - 8;

                    uint32_t textsFileSpec = *(uint32_t *)data;
                    data += sizeof(uint32_t);
                    uint32_t textsIdentifier = *(uint32_t *)data;
                    data += sizeof(uint32_t);

                    TRACELOG(LOG_DEBUG, "Texts file spec: %#x\n", textsFileSpec);
                    TRACELOG(LOG_DEBUG, "Texts Identifier: %#x\n", textsIdentifier);

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
                    BoundingBox bbox = {0};

                    memcpy(&bbox, data, sizeof(BoundingBox));
                    data += sizeof(BoundingBox);

                    bbox.min = vec3tobe(bbox.min);
                    bbox.max = vec3tobe(bbox.max);

                    TRACELOG(LOG_DEBUG, "Value: min {%f, %f, %f}, max {%f, %f, %f}\n",
                           bbox.min.x, bbox.min.y, bbox.min.z, bbox.max.x, bbox.max.y, bbox.max.z);

                    propData.variables[i].values[j].bbox = bbox;
                } break;
                case 0x38: // transform type
                {
                    uint16_t unknown1 = *(uint16_t*)data;
                    data += sizeof(uint16_t);

                    TRACELOG(LOG_DEBUG, "Unknown 1: %#x\n", unknown1);

                    if (unknown1 == 0xf00 && j == 0) arraySize += 4;

                    float unknown2[12];

                    for (int i = 0; i < 12; i++)
                    {
                        unknown2[i] = htobefloat(*(float*)data);
                        TRACELOG(LOG_DEBUG, "Unknown2[%d] = %f\n", i, unknown2[i]);
                        data += sizeof(float);
                    }

                    if (unknown1 & 0x0100)
                    {
                        data += sizeof(uint32_t);
                    }

                } break;
                case 0x34: // colorRGBA type
                {
                    float r = htobefloat(*(float *)data);
                    data += sizeof(float);
                    float g = htobefloat(*(float *)data);
                    data += sizeof(float);
                    float b = htobefloat(*(float *)data);
                    data += sizeof(float);
                    float a = htobefloat(*(float *)data);
                    data += sizeof(float);

                    if (!isArray)
                    {
                        // data += sizeof(uint32_t);
                    }

                    TRACELOG(LOG_DEBUG, "Value: {%f, %f, %f, %f}\n", r, g, b, a);

                    propData.variables[i].values[j].colorRGBA.r = r;
                    propData.variables[i].values[j].colorRGBA.g = g;
                    propData.variables[i].values[j].colorRGBA.b = b;
                    propData.variables[i].values[j].colorRGBA.a = a;
                } break;
                case 0x33: // vector4 type
                {
                    // Raylib's vector4 type happens to fit nicely with the description.
                    Vector4 val = vec4tobe(*(Vector4 *)data);
                    data += sizeof(Vector4);

                    TRACELOG(LOG_DEBUG, "Value: {%f, %f, %f, %f}\n", val.x, val.y, val.z, val.w);

                    propData.variables[i].values[j].vector4 = val;
                } break;
                default:
                {
                    TRACELOG(LOG_DEBUG, "Unrecognized variable type.\n");
                    propData.corrupted = true;
                    return propData;
                } break;
            }
        }

        // Weird thing in older versions of the format
        while (data - initData + 4 < dataSize)
        {
            if (*(uint32_t*)data == 0)
            {
                data += 4;
            }
            else
            {
                break;
            }
        }

        if (arraySize > 0)
        {
            //data = arrayInitData + arraySize * arrayNumber;
        }
    }

    return propData;
}

static bool TextStartsWith(const char *t1, const char *startsWith)
{
    return strstr(t1, startsWith) == t1;
}

PropertyNameList LoadPropertyNameList(const char *filename)
{
    FILE *f = fopen(filename, "r");

    PropertyNameList nameList = { 0 };

    if (!f)
    {
        perror("Error opening Properties.txt");
        return nameList;
    }

    char buf[1024];
    int lineNo = 0;

    while (!feof(f))
    {
        lineNo++;
        fgets(buf, 1023, f);
        if (*buf == '#' || *buf == '\n' || *buf == 0xd || *buf == 0) continue;

        if (!TextStartsWith(buf, "property"))
        {
            TRACELOG(LOG_ERROR, "%s: unknown token on line %d (%#x)\n", filename, lineNo, *buf);
        }

        //printf("%s", buf);
        
        nameList.propCount++;
        nameList.propIds = realloc(nameList.propIds, sizeof(unsigned long) * nameList.propCount);
        nameList.propNames = realloc(nameList.propNames, sizeof(const char *) * nameList.propCount);

        char *name = strchr(buf, ' ') + 1;
        int length = strchr(name, ' ') - name;

        if (TextStartsWith(name, "OptionMotionBlur"))
        {
            length = strchr(name, '\t') - name;
        }

        char *nameCopy = malloc(length + 1);
        memcpy(nameCopy, name, length);
        nameCopy[length] = 0;

        nameList.propNames[nameList.propCount - 1] = nameCopy;

        unsigned long id = strtoul(name + length, NULL, 16);
        if (id == 0 && strstr(name, "(hash("))
        {
            char *idStr = strchr(name + length, '(') + 6;
            int idStrLength = strchr(idStr, ')') - idStr;

            char *idCopy = malloc(idStrLength + 1);
            memcpy(idCopy, idStr, idStrLength);
            idCopy[idStrLength] = 0;

            id = TheHash(idCopy);

            free(idCopy);
        }

        nameList.propIds[nameList.propCount - 1] = id;

        TRACELOG(LOG_DEBUG, "property name %s with id %#lX\n", nameCopy, nameList.propIds[nameList.propCount - 1]);
    }

    TRACELOG(LOG_INFO, "Loaded %d properties\n", nameList.propCount);

    fclose(f);

    return nameList;
}
