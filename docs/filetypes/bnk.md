
# BNK file format
BNK is a sound bank file format, it is a container for WEM files.
It is a proprietary file format developed by Audiokinetic Wwise.

This documentation of this format is based on [src/filetypes/bnk.c](../../src/filetypes/bnk.c), part of the OpenSC5 project, which is based on bnkextr: https://github.com/eXpl0it3r/bnkextr.

## Chunks

There are 4 types of chunks in a bnk file:
| Signature | Description |
| ----------- | ----------- |
| BKHD | Primary header. Should be the first chunk in the file. |
| HIRC | Event information, like looping. |
| DIDX | Content indices (Offset/size of each sound in the DATA chunk) |
| DATA | The sound data |

The chunks are read consecutively and each chunk starts with one of these four letter signatures.

### BKHD
The BKHD chunk contains information about the file.
The chunk header is as follows.
```
typedef struct BnkHeader {
    char signature[4]; // BKHD
    uint32_t size;     // 0x18
    uint32_t version;  // 0x41
    uint32_t pointsTo; // Identifier for a file in the same group.
    char unknown[16];
} BnkHeader;
```
There may be other possible values for the `size` and `version` fields, but they don't seem to differ in SimCity.<br>
The `pointsTo` field is another interesting field. In the case of SimCity it may contain the instance id of the BNK file that actually contains the sound data, as this one doesn't have any sound data.

### HIRC
The HIRC chunk contains information about events. (Pause, SetVoicePitch, etc.)
```
typedef struct HircHeader {
    char signature[4]; // HIRC
    uint32_t objectCount;
} HircHeader;
```
`objectCount` is the number of objects.
HIRC objects follow this format:
```
typedef struct HircObject {
    uint16_t unknown;
    unsigned char type; // Corresponds to EventActionType in bnkextr
    uint32_t size;
    uint32_t id;
} HircObject;
```
bnkextr handles *some* HIRC types, but SimCity doesn't use any of these.

### DIDX
The DIDX chunk is the directory of every sound in the sound bank.
```
typedef struct DidxHeader {
    char signature[4]; // DIDX
    uint32_t size;     // The size of the content index buffer
} DidxHeader;
```
Note that `size` represents the size of the content index buffer in bytes, so to get the content index count, you divide by `sizeof(ContentIndex)` (12).
```
typedef struct ContentIndex {
    uint32_t id;     // Unique identifier for the sound
    uint32_t offset; // Offset in bytes of the sound data
    uint32_t size;   // Size in bytes of the sound data
} ContentIndex;
```
`offset` is an offset into the DATA chunk.

### DATA
The DATA chunk is the WEM data of the sounds in the sound bank.
```
typedef struct DataHeader {
    char signature[4]; // DATA
    uint32_t size;     // Size of chunk in bytes
} DataHeader;
```
The WEM format is another proprietary format by Audiokinetic.
