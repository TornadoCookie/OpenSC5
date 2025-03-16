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

typedef struct RuleGlobalRuleInfo {
    uint32_t name;
    uint32_t null1;
    uint32_t start;
    uint32_t unkn0;
    uint32_t unkn1[3];
    uint32_t bool1[2];
    uint32_t null2[2];
    uint32_t always4;
    uint32_t null3[3];
    uint32_t sometimes4;
    uint32_t null4;
    uint32_t sometimes4_2;
    uint32_t bool3;
    uint32_t referenceToOther;
    uint32_t null5;
    uint32_t alwaysffffffff;
    uint32_t null6;
    uint32_t bool4;
    uint32_t referenceToOther2;
    uint32_t null7;
    uint32_t endOff;
    uint32_t bool5;
} RuleGlobalRuleInfo;

typedef struct RuleHeader {
        uint32_t null1;
        uint32_t always10be;
        uint32_t null2[10];
        //float timeTriggerPeriod;
        //uint32_t timeTriggerName;
        //uint32_t timeTriggerCount;
} RuleHeader;

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
    // Have you heard of __attribute__ ((packed)) dumbass?
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
        rule.endOffset = htobe32(rule.endOffset);
        rule.extraCount = htobe32(rule.extraCount);

        printf("\nUnit Rule %d:\n", i);
        printf("Name: %#x\n", rule.ruleName);
        printf("Start Offset: %#x\n", rule.startOffset);
        printf("End Offset: %#x\n", rule.endOffset);
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
    uint32_t globalRuleCount = htobe32(*(uint32_t *)data);
    printf("Global Rule Count: %d @ %#x\n", globalRuleCount, data - initData);
    if (globalRuleCount == -1)
        globalRuleCount = 0;
    data += sizeof(uint32_t);
    RuleGlobalRuleInfo *globalRuleInfo = (RuleGlobalRuleInfo *)data;

    for (int i = 0; i < globalRuleCount; i++)
    {
        globalRuleInfo[i].start = be32toh(globalRuleInfo[i].start);
        globalRuleInfo[i].endOff = be32toh(globalRuleInfo[i].endOff);
        if (globalRuleInfo[i].start == 0xFFFFFFFF)
        {
            globalRuleInfo[i].start = be32toh(globalRuleInfo[i].unkn0);
        }
        if (globalRuleInfo[i].endOff == 0xFFFFFFFF)
        {
            globalRuleInfo[i].endOff = be32toh(globalRuleInfo[i].unkn1[1]);
        }

        RuleGlobalRuleInfo globalRule = globalRuleInfo[i];

        printf("Global Rule %d:\n", i);
        printf("Name: %#x\n", globalRule.name);
        printf("Start: %#x\n", globalRule.start);
        printf("End: %#x\n", globalRule.endOff);
    }

    data += globalRuleCount * sizeof(RuleGlobalRuleInfo);

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

    // 0x20 unkn bytes
    data += 0x4;
    unsigned char *ruleOff = data;

    printf("offset: %#lx\n", data - initData);

    // global rules
    for (int i = 0; i < globalRuleCount; i++)
    {
        data = ruleOff + globalRuleInfo[i].start;
        
        RuleHeader *ruleHeader = (RuleHeader *)data;
        data += sizeof(RuleHeader);

        printf("\nglobalRule %#x @ %#x to %#x\n", globalRuleInfo[i].name, globalRuleInfo[i].start + ruleOff - initData, globalRuleInfo[i].endOff + ruleOff - initData);
        //printf("    timeTrigger %#x %f -count %d\n", ruleHeader->timeTriggerName, ruleHeader->timeTriggerPeriod, ruleHeader->timeTriggerCount);

        uint32_t x = 0;
        while (data < ruleOff + globalRuleInfo[i].endOff)
        {
            x = *(uint32_t*)data;
            data += sizeof(uint32_t);

            if (x == 0) continue;
            switch(x)
            {
                case 0x3f800000: //global checks
                {
                    while (data < ruleOff + globalRuleInfo[i].endOff)
                    {
                        uint32_t var = *(uint32_t*)data;
                        if (var == 0xFFFFFFFF || var == 0x84) break;
                        data += sizeof(uint32_t);

                        uint32_t val = *(uint32_t*)data;
                        data += sizeof(uint32_t);

                        if (*(uint32_t*)data == val)
                        {
                            data += sizeof(uint32_t);
                            printf("    global %#x atLeast %d\n", var, val);
                        }
                        else if (*(uint32_t*)data == 0x00F423F)
                        {
                            data += sizeof(uint32_t);
                            printf("    global %#x out !%#x@global\n", var, var);
                        }
                        else
                        {
                            printf("    global %#x is %d\n", var, val);
                        }
                    }
                } break;
                case 0xFFFFFFFF: // global inout
                {
                    while (data < ruleOff + globalRuleInfo[i].endOff)
                    {
                        uint32_t var = *(uint32_t*)data;
                        if (var == 0x84) break;
                        data += sizeof(uint32_t);
                        uint32_t unkn = *(uint32_t*)data;
                        data += sizeof(uint32_t);
                        uint32_t val = *(uint32_t*)data;
                        data += sizeof(uint32_t);

                        printf("    global %#x out %d (unkn %d)\n", var, val, unkn);
                    }
                } break;
                case 0xFFFFFFFE: // global out
                {
                    uint32_t var;
                    uint32_t unkn;
                    uint32_t val;
                    var = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    unkn = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    val = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    printf("    global %#x out %d (unkn %d)\n", var, val, unkn);
                } break;
                case 0x000F423F: // global out !@global
                {
                    uint32_t var = *(uint32_t*)(data-12);
                    uint32_t unkn = *(uint32_t*)(data-8);
                    printf("    global %#x out !%#x@global (unkn %d)\n", var, var, unkn);
                } break;
                case 0x84: // successEvent uiEvent
                {
                    uint32_t name = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    uint8_t unkn = *(uint8_t*)data;
                    data++;
                    printf("    successEvent uiEvent %#x (unkn %#x)\n", name, unkn);
                } break;
                default:
                {
                    printf("unkn byte %#x\n", x);
                } break;
            }
        }

        
    }

    return rulesData;
}
