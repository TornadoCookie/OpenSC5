#ifndef _PACKAGE_
#define _PACKAGE_

#include <stdio.h>
#include <stdbool.h>
#include "prop.h"
#include "rules.h"
#include "rast.h"
#include "bnk.h"
#include "rw4.h"

#define PKGENTRY_PROP 0x00B1B104 // PROPerties file
#define PKGENTRY_GMDL 0x00E6BCE5 // Unknown. Found in a property file enumerating type codes.
#define PKGENTRY_PLT  0x011989B7 // Unknown. (PaLeTte?) Found in a property file enumerating type codes.
#define PKGENTRY_SCPT 0x024A0E52 // SCriPT file
#define PKGENTRY_TEXT 0x02FAC0B6 // TEXT file
#define PKGENTRY_HM   0x0376C3DA // Unknown. Found in a property file enumerating type codes.
#define PKGENTRY_SHDR 0x0469A3F7 // Compiled DirectX shader.
#define PKGENTRY_RULE 0x08068AEB // RULEs file
#define PKGENTRY_ER2  0x08068AEC // .er2 (uncompiled rules file)
#define PKGENTRY_JSON 0x0A98EAF0 // JSON file (utf-8)
#define PKGENTRY_BNK  0x0A4D8D09 // wwise soundBaNK. 
#define PKGENTRY_WEM  0x0D9E5710 // Wwise Encoded Music. extract using ww2ogg, using arguments --pcb packed_codebooks_aoTuV_603.bin.
#define PKGENTRY_BEM  0x1A99B06B // Unknown. Found in a property file enumerating type codes.
#define PKGENTRY_BLD  0x2399BE55 // Unknown. (Spore BuiLDing?) Found in a property file enumerating type codes.
#define PKGENTRY_VCL  0x24682294 // Unknown. (Spore VehiCLe?) Found in a property file enumerating type codes.
#define PKGENTRY_TTF  0x276CA4B9 // TTF font. Unable to be viewed in-editor.
#define PKGENTRY_CRT  0x2B978C46 // Unknown. (Spore CReaTure?) Found in a property file enumerating type codes.
#define PKGENTRY_CSS  0x2C978DB6 // Cascading Style Sheets
#define PKGENTRY_RW4  0x2F4E681B // RenderWare 4 model file
#define PKGENTRY_RAST 0x2F4E681C // renderware RASTer image
#define PKGENTRY_JPEG 0x2F7D0002 // JPEG image.
#define PKGENTRY_PNG  0x2F7D0004 // PNG file
#define PKGENTRY_TGA  0x2F7D0006 // TGA image.
#define PKGENTRY_GIF  0x2F7D0007 // GIF image.
#define PKGENTRY_MOV  0x376840D7 // MOV file. Unable to be played in-editor.
#define PKGENTRY_EXIF 0x3F8662EA // EXIF image. Unable to be viewed in-editor.
#define PKGENTRY_FLR  0x438F6347 // Unknown. Found in a property file enumerating type codes.
#define PKGENTRY_UFO  0x476A98C7 // Unknown. (Spore UFO?) Found in a property file enumerating type codes.
#define PKGENTRY_JSN8 0x67771F5C // JSoN file (ascii), most commonly a MUiLE design.
#define PKGENTRY_HTML 0xDD6233D6 // HyperText Markup Language
#define PKGENTRY_SWB  0xEA5118B0 // SWarm Binary file, particles. Unable to be viewed in-editor.

typedef struct PackageEntry {
    unsigned int type;
    unsigned int group;
    unsigned int instance;
    bool corrupted;
    bool compressed;

    unsigned char *dataRaw;
    int dataRawSize;

    unsigned char *dataCompressed;
    int dataCompressedSize;

    union {
        PropData propData;
        RulesData rulesData;
        char *scriptSource;
        struct {
            Image img;
            Texture2D tex;
        } imgData;
        struct {
            Image img;
            Texture2D tex;
            int frameCount;
            int currentFrame;
        } gifData;
        BnkData bnkData;
        RW4Data rw4Data;
    } data;
} PackageEntry;

typedef struct Package {
    unsigned int entryCount;
    PackageEntry *entries;
} Package;

Package LoadPackageFile(FILE *f);
void UnloadPackageFile(Package pkg);

void ExportPackageEntry(PackageEntry entry, const char *filename);

typedef struct PackageSearchParams {
    bool searchInstance;
    bool searchGroup;
    bool searchType;

    char *instance;
    char *group;
    char *type;
} PackageSearchParams;

int *SearchPackage(Package pkg, PackageSearchParams params, int *nResults);

void MergePackages(Package *dest, Package src);
void SetWriteCorruptedPackageEntries(bool val);
void SetTryParseFilesInPackage(bool val);

void ExportPackage(Package pkg, const char *filename);

unsigned char *DecompressDBPF(unsigned char *data, int dataSize, int outDataSize);

#endif
