#ifndef _PROP_
#define _PROP_

#include <cpl_raylib.h>

#define PROPVAR_BOOL   0x01
#define PROPVAR_INT32  0x09
#define PROPVAR_UINT32 0x0A
#define PROPVAR_FLOAT  0x0D
#define PROPVAR_STR8   0x12
#define PROPVAR_STRING 0x13
#define PROPVAR_KEYS   0x20
#define PROPVAR_TEXTS  0x22
#define PROPVAR_VECT2  0x30
#define PROPVAR_VECT3  0x31
#define PROPVAR_COLRGB 0x32
#define PROPVAR_CRGBA  0x34
#define PROPVAR_TRANS  0x38
#define PROPVAR_BBOX   0x39

typedef struct PropVariable {
    unsigned int identifier;
    unsigned short type;
    unsigned int count;
    union {
        struct {
            unsigned int file;
            unsigned int type;
            unsigned int group;
        } keys;
        int int32;
        struct {
            float r;
            float g;
            float b;
        } colorRGB;
        char *string;
        unsigned int uint32;
        char *string8;
        float f;
        Vector2 vector2;
        Vector3 vector3;
        bool b;
        struct {
            unsigned int fileSpec;
            unsigned int identifier;
        } texts;
        BoundingBox bbox;
        struct {
            float r;
            float b;
            float g;
            float a;
        } colorRGBA;
    } *values;
} PropVariable;

typedef struct PropData {
    unsigned int variableCount;
    PropVariable *variables;
    bool corrupted;
} PropData;

PropData LoadPropData(unsigned char *data, int dataSize);

#endif
