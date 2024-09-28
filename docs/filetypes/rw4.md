
# RW4 File format
This document specifies the format for files that contain RenderWare 4 models, textures, and animation data.

## RWHeader
```
typedef struct RWHeader {
    char magic[28];
    uint32_t type;
    uint32_t unknown1;
    uint32_t sectionCount;
    uint32_t unknown2[2];
    uint32_t sectionInfoOffset;
    uint32_t unknown3[4];
    uint32_t bufferDataOffset;
    uint32_t unknown4[7];
    uint32_t unknownBits;
    uint32_t unknown5[12];
} RWHeader;
```

`magic`: 0x89, 0x52, 0x57, 0x34, 0x77, 0x33, 0x32, 0x00, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x20, 0x04
			, 0x00, 0x34, 0x35, 0x34, 0x00, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00

`type`: Model: 1, Texture: 0x4000000, Special: 0xCAFED00D

`unknown1` is technically a copy of `sectionCount`

| Type code | Name |
| ----------- | ----------- |
| 0x10004 | RWSectionManifest |
| 0x10005 | RWSectionTypes |
| 0x10006 | RWSectionExternalArenas |
| 0x10007 | RWSectionSubReferences |
| 0x10008 | RWSectionAtoms |
| 0x10030 | RWBaseResource |
| 0x20004 | RWVertexDescription |
| 0x20005 | RWVertexBuffer |
| 0x20007 | RWIndexBuffer |
| 0x20009 | RWMesh |

## RWSectionInfo
In the header, there is a field called `sectionInfoOffset`. At this offset resides the section info. The section info is in the form of a list that follows this format, and has a count of `sectionCount`.
```
typedef struct RWSectionInfo {
    uint32_t pData;
    uint32_t field4;
    uint32_t size;
    uint32_t alignment;
    uint32_t typeCodeIndex;
    uint32_t typeCode;
} RWSectionInfo;
```

If the type is RWBaseResource, the data is stored immediately after the section info, so skip that much. Otherwise, the data for the section is stored at the offset `pData`.

## Sections
### RWSectionManifest
```
typedef struct RWSectionManifest {
    // Section Manifest
    uint32_t field0;
    uint32_t field1;
    uint32_t offset1;
    uint32_t offset2;
    uint32_t offset3;
    uint32_t offset4;
} RWSectionManifest;
```
`offset1` points to types (RWSectionTypes).
`offset2` points to external arenas (RWSectionExternalArenas).
`offset3` points to subreferences (RWSectionSubReferences).
`offset4` points to atoms (RWSectionAtoms).

### RWSectionTypes
```
typedef struct RWSectionTypes {
    uint32_t count;
    uint32_t field4;
    uint32_t typeCodes[count];
} RWSectionTypes;
```

### RWSectionExternalArenas
I think you can just skip this one...
```
typedef struct RWSectionExternalArenas {
    uint32_t field0;
    uint32_t field4;
    uint32_t field8;
    uint32_t fieldc;
    uint32_t field10;
    uint32_t field14;
    uint32_t field18;
    uint32_t field1c;
} RWSectionExternalArenas;
```

### RWSectionSubReferences
```
typedef struct RWSectionSubReferences {
    uint32_t count;
    uint32_t field4;
    uint32_t field8;
    uint32_t endOffset;
    uint32_t offset;
    uint32_t countAgain;
} RWSectionSubReferences;
```

at `offset` we have a list of `count` RWSubReference objects.

```
typedef struct RWSubReference {
    uint32_t object; // The object that contains the subreference
    uint32_t offset; // The offset within the object where the subreferenced object starts
} RWSubReference;
```

### RWSectionAtoms

### RWBaseResource
RWBaseResource is just a uint32 `typeCode` followed by the data. The data has no format, as this section is supposed to be given context by being referenced by another section.

### RWVertexDescription
```
typedef struct RWVertexDescription {
    uint32_t field0;
    uint32_t field4;
    uint32_t vertexDeclaration;
    uint16_t count;
    uint8_t fielde;
    uint8_t vertexSize;
    uint32_t elementFlags;
    uint32_t field14;
} RWVertexDescription;
```
Immediately following this are `count` RWVertexElement objects:
```
typedef struct RWVertexElement {
    uint16_t stream;
    uint16_t offset;
    uint8_t type;
    uint8_t method;
    uint8_t usage;
    uint8_t usageIndex;
    uint32_t typeCode;
} RWVertexElement;
```

### RWVertexBuffer
```
typedef struct RWVertexBuffer {
    uint32_t vertexDescription;
    uint32_t field4;
    uint32_t baseVertexIndex;
    uint32_t vertexCount;
    uint32_t field10;
    uint32_t vertexSize;
    uint32_t vertexData;
} RWVertexBuffer;
```
`vertexDescription` and `vertexData` are references to other sections.

### RWIndexBuffer
```
typedef struct RWIndexBuffer {
    uint32_t dxIndexBuffer;
    uint32_t startIndex;
    uint32_t primitiveCount;
    uint32_t usage;
    uint32_t format;
    uint32_t primitiveType;
    uint32_t indexData;
} RWIndexBuffer;
```
`indexData` is a reference to another section.

### RWMesh
```
typedef struct RWMesh {
    uint32_t field0;
    uint32_t primitiveType;
    uint32_t indexBuffer;
    uint32_t triangleCount;
    uint32_t buffersCount;
    uint32_t firstIndex;
    uint32_t primitiveCount;
    uint32_t firstVertex;
    uint32_t vertexCount;
    uint32_t vertexBuffers[buffersCount];
} RWMesh;
```
`vertexBuffers` and `indexBuffer` are references to other sections.
