#include "filetypes/rules.h"
#include <stdint.h>
#include <string.h>
#include <cpl_endian.h>
#include <stdio.h>

typedef struct RulesFileHeader {
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint8_t unknown4;
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

RulesData LoadRulesData(unsigned char *data, int dataSize)
{
    RulesData rulesData = {0};

    RulesFile file = {0};

    printf("Rules info:\n");

    unsigned char *initData = data;

    // C isn't letting us misalign our memory like this, unknown4 is being treated like it's 4 bytes long.
    // That is why we can't just memcpy like that.
    data += sizeof(uint32_t); // unknown1
    data += sizeof(uint32_t); // unknown2
    data += sizeof(uint32_t); // unknown3
    data++;                   // unknown4

    file.header.ruleCount = *(uint32_t *)data;
    data += sizeof(uint32_t);

    file.header.ruleCount = htobe32(file.header.ruleCount);

    printf("Rule count: %#x\n", file.header.ruleCount);

    // file.rules = malloc(file.header.ruleCount * sizeof(RulesFileRule));

    for (int i = 0; i < file.header.ruleCount; i++)
    {
        RulesFileRule rule = {0};

        memcpy(&rule, data, sizeof(RulesFileRule));
        data += sizeof(RulesFileRule);

        rule.ruleName = htobe32(rule.ruleName);
        rule.startOffset = htobe32(rule.startOffset);
        // rule.endOffset = htobe32(rule.endOffset);
        rule.extraCount = htobe32(rule.extraCount);

        printf("\nRule %d:\n", i);
        printf("Name: %#x\n", rule.ruleName);
        printf("Start Offset: %d\n", rule.startOffset);
        printf("End Offset: %d\n", rule.endOffset);
        printf("Extra Count: %d\n", rule.extraCount);

        data += 12 * rule.extraCount;

        if (data - initData > dataSize)
        {
            printf("Invalid data.\n");
            rulesData.corrupted = true;
            return rulesData;
        }
    }

    data += sizeof(uint32_t);
    uint32_t unknown1count = *(uint32_t *)data;
    printf("unknown1count: %d\n", unknown1count);
    if (unknown1count > 1000)
    {
        printf("I'm gonna assume this is an invalid value. Please FIX!!!\n");
        rulesData.corrupted = true;
        return rulesData;
    }
    data += sizeof(uint32_t);
    data += unknown1count * 0x5c;

    data += sizeof(uint32_t);
    uint32_t unknown2count = htobe32(*(uint32_t *)data);
    printf("unknown2count: %d\n", unknown2count);
    /*if (unknown2count > 1000)
    {
        printf("I'm gonna assume this is an invalid value. Please FIX!!!\n");
        return false;
    }*/
    if (unknown2count == -1)
        unknown2count = 0;
    data += sizeof(uint32_t);
    data += unknown2count * 0x70;

    data += sizeof(uint32_t);
    uint32_t unknown3count = *(uint32_t *)data;
    printf("unknown3count: %d\n", unknown3count);
    data += sizeof(uint32_t);
    if (unknown3count)
    {
        printf("Unknown 3 has positive count.\n");
        printf("offset: %#lx\n", data - initData);
        rulesData.corrupted = true;
        return rulesData;
    }
    // data += unknown3count;

    data += sizeof(uint32_t);
    uint32_t unknown4count = *(uint32_t *)data;
    printf("unknown4count: %d\n", unknown4count);
    data += sizeof(uint32_t);
    if (unknown4count)
    {
        printf("Unknown 4 has positive count.\n");
        printf("offset: %#lx\n", data - initData);
        rulesData.corrupted = true;
        return rulesData;
    }
    // data += unknown4count;

    data += sizeof(uint32_t);
    // data += sizeof(uint32_t);
    uint32_t codeLength = htobe32(*(uint32_t *)data);
    data += sizeof(uint32_t);
    printf("codeLength: %d\n", codeLength);

    printf("offset: %#lx\n", data - initData);

    return rulesData;
}
