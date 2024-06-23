#ifndef _RULES_
#define _RULES_

#include <stdbool.h>

typedef struct RulesData {
    bool corrupted;
} RulesData;

RulesData LoadRulesData(unsigned char *data, int dataSize);

#endif
