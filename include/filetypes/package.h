#ifndef _PACKAGE_
#define _PACKAGE_

#include <stdio.h>
#include <stdbool.h>
#include "prop.h"
#include "rules.h"
#include "rast.h"

#define PKGENTRY_PROP 0x00B1B104 // PROPerties file
#define PKGENTRY_SCPT 0x024A0E52 // SCriPT file
#define PKGENTRY_TEXT 0x02FAC0B6 // TEXT file
#define PKGENTRY_RULE 0x08068AEB // RULEs file
#define PKGENTRY_JSON 0x0A98EAF0 // JSON file (utf-8)
#define PKGENTRY_RW4  0x2F4E681B // RenderWare 4 model file
#define PKGENTRY_RAST 0x2F4E681C // renderware RASTer image
#define PKGENTRY_JSN8 0x67771F5C // JSoN file (ascii)
#define PKGENTRY_PNG  0x2F7D0004 // PNG file
#define PKGENTRY_MOV  0x376840D7 // MOV file. Unable to be played in-editor.

typedef struct PackageEntry {
    unsigned int type;
    unsigned int group;
    unsigned int instance;
    bool corrupted;

    union {
        PropData propData;
        RulesData rulesData;
        char *scriptSource;
        RastData rastData;
        struct {
            Image img;
            Texture2D tex;
        } imgData;
    } data;
} PackageEntry;

typedef struct Package {
    unsigned int entryCount;
    PackageEntry *entries;
} Package;

Package LoadPackageFile(FILE *f);
void UnloadPackageFile(Package pkg);

#endif
