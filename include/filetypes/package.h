#ifndef _PACKAGE_
#define _PACKAGE_

#include <stdio.h>
#include <stdbool.h>
#include "prop.h"
#include "rules.h"

#define PKGENTRY_PROP 0x00B1B104
#define PKGENTRY_SCPT 0x024A0E52
#define PKGENTRY_RULE 0x08068AEB
#define PKGENTRY_JSON 0x0A98EAF0
#define PKGENTRY_RAST 0x2F4E681C

typedef struct PackageEntry {
    unsigned int type;
    unsigned int group;
    unsigned int instance;
    bool corrupted;

    union {
        PropData propData;
        RulesData rulesData;
        char *scriptSource;
    } data;
} PackageEntry;

typedef struct Package {
    unsigned int entryCount;
    PackageEntry *entries;
} Package;

Package LoadPackageFile(FILE *f);
void UnloadPackageFile(Package pkg);

#endif
