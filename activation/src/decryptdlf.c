#include "aes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

uint8_t *ReadFile(const char *fileName, long *fsz)
{
    FILE *f = fopen(fileName, "rb");
    fseek(f, 0, SEEK_END);
    *fsz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *mem = malloc(*fsz);
    fread(mem, *fsz, 1, f);
    fclose(f);

    return mem;
}

void print_hex(const char *prefix, uint8_t *bytes, int len)
{
    printf("%s", prefix);
    for (int i = 0; i < len; i++)
    {
        if (i % 0x10 == 0) printf(" ");
        printf("%02X", bytes[i]);
    }
    printf("\n");
}

// https://github.com/BigApex/rse-ooa-decrypt

const uint8_t DLF_KEY[16] = {65, 50, 114, 45, 208, 130, 239, 176, 220, 100, 87, 197, 118, 104, 202, 9};
const uint8_t IV[16] = {0};

void GetKeyFromDLF(const char *dlfFile, uint8_t *outKeys)
{
    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, DLF_KEY, IV);

    long fsz;
    uint8_t *dlf = ReadFile(dlfFile, &fsz);

    const char *xml = dlf + 0x41;

    AES_CBC_decrypt_buffer(&ctx, xml, fsz - 0x41);

    char *cipherKey = strstr(xml, "<CipherKey>") + strlen("<CipherKey>");
    *strchr(cipherKey, '<') = 0;

    //printf("cipherKey: %s, %d\n", cipherKey, strlen(cipherKey));

    uint8_t *cipherKeyRaw = base64_decode(cipherKey);

    memcpy(outKeys, cipherKeyRaw, 128);
}

void test_key_iv(uint8_t *textStart, uint8_t *key, uint8_t *iv)
{
    

    //int i = 0xc20;
    for (int i = 0; i < 0x1000; i+=0x10)
    {
        struct AES_ctx ctx;

        AES_init_ctx_iv(&ctx, key, iv);

        uint8_t *block = malloc(16);
        memcpy(block, textStart + i, 16);
        AES_CBC_decrypt_buffer(&ctx, block, 16);

        if (*block == 0x83 && block[1] == 0xEC)
        {
            printf("Offset %#x: ", i);
            for (int j = 0; j < 16; j++)
            {
                printf("%02X", block[j]);
            }
            printf("\n");
        }

        

        free(block);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Invalid arguments.\n");
        return 0;
    }

    uint8_t candidates[128];
    GetKeyFromDLF("71480.dlf", candidates);

    print_hex("Key/IV candidates: ", candidates, 128);

    long exeSize;
    uint8_t *exe = ReadFile(argv[1], &exeSize);

    // get start of encrypted code
    const unsigned CODE_OFF = 0x400;
    uint8_t *textStart = exe + CODE_OFF;

    const int codeSize = 0x00ced0f7-0x00401000 - CODE_OFF;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            //if (i == j) continue;

            printf("Key %d, IV %d:\n", i, j);
            test_key_iv(textStart, candidates+i*16, candidates+j*16);
        }

        printf("Key %d, IV All-Zeroes:\n", i);
        test_key_iv(textStart, candidates+i*16, IV);
    }

    //fwrite(exe, exeSize, 1, stdout);

    return 0;
}
