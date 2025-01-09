#include "filetypes/rw4.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <cpl_endian.h>
#include "memstream.h"

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

    // Section Manifest
    uint32_t typeCode;
    uint32_t field0;
    uint32_t field1;
    uint32_t offset1;
    uint32_t offset2;
    uint32_t offset3;
    uint32_t offset4;
} RWHeader;

typedef struct RWSectionInfo {
    uint32_t dataOffset;
    uint32_t field4;
    uint32_t size;
    uint32_t alignment;
    uint32_t typeCodeIndex;
    uint32_t typeCode;
} RWSectionInfo;

typedef struct RWKeyframeAnim {
    uint32_t channelNameOffset;
    uint32_t channelCount;
    uint32_t skeletonId;
    uint32_t fieldC;
    uint32_t channelDataOffset;
    uint32_t paddingEndOffset;
    uint32_t channelCount2;
    uint32_t field1c;
    float length;
    uint32_t field24;
    uint32_t flags;
    uint32_t channelInfoOffset;
} RWKeyframeAnim;

typedef struct RWSkeleton {
    uint32_t boneFlagOffset;
    uint32_t boneParentOffset;
    uint32_t boneNameOffset;
    uint32_t count;
    uint32_t skeletonId;
    uint32_t unknown1;
} RWSkeleton;

typedef struct RWSkeletonsInK {
    uint32_t object1;
    uint32_t pFunction;
    uint32_t arrayOffset;
    uint32_t object2;
    uint32_t skeleton;
    uint32_t arrayCount;
} RWSkeletonsInK;

typedef struct RWBBox {
    Vector3 min;
    uint32_t fieldc;
    Vector3 max;
    uint32_t field1c;
} RWBBox;

typedef struct RWMesh {
    uint32_t field0;
    uint32_t primitiveType;
    uint32_t indexBuffer;
    uint32_t triangleCount;
    uint32_t bufferCount;
    uint32_t firstIndex;
    uint32_t primitiveCount;
    uint32_t firstVertex;
    uint32_t vertexCount;
} RWMesh;

typedef struct RWVertexBuffer {
    uint32_t vertexDescription;
    uint32_t field4;
    uint32_t baseVertexIndex;
    uint32_t vertexCount;
    uint32_t field10;
    uint32_t vertexSize;
    uint32_t vertexData;
} RWVertexBuffer;

typedef struct RWIndexBuffer {
    uint32_t dxIndexBuffer;
    uint32_t startIndex;
    uint32_t primitiveCount;
    uint32_t usage;
    uint32_t format;
    uint32_t primitiveType;
    uint32_t indexData;
} RWIndexBuffer;

typedef struct RWVertexElement {
    uint16_t stream;
    uint16_t offset;
    uint8_t type;
    uint8_t method;
    uint8_t usage;
    uint8_t usageIndex;
    uint32_t typeCode;
} RWVertexElement;

typedef struct RWVertexDescription {
    uint32_t field0;
    uint32_t field4;
    uint32_t dxVertexDeclaration;
    uint16_t count;
    uint8_t fielde;
    uint8_t vertexSize;
    uint32_t elementFlags;
    uint32_t field14;
    RWVertexElement *elements;
} RWVertexDescription;

typedef struct RWRaster {
    int32_t textureFormat;
    uint16_t textureFlags;
    uint16_t volumeDepth;
    int32_t dxBaseTexture;
    uint16_t width;
    uint16_t height;
    uint8_t field10;
    uint8_t mipmapLevels;
    uint16_t skipped;
    int32_t field14;
    int32_t field18;
    int32_t textureData;
} RWRaster;

unsigned char *LoadSectionData(RWSectionInfo *sectionInfos, int section, char *initData, int *dataSize)
{
    if (dataSize) *dataSize = sectionInfos[section].size;
    return sectionInfos[section].dataOffset + initData;
}

RW4Data LoadRW4Data(unsigned char *data, int dataSize)
{
    RW4Data rw4data = { 0 };

    unsigned char *initData = data;

    printf("RW4 Info:\n");

    RWHeader header = { 0 };
    memcpy(&header, data, sizeof(RWHeader));
    data += sizeof(RWHeader);

    header.sectionCount &= 0xFF;

    printf("Type: %#x\n", header.type);
    printf("Section Count: %#x\n", header.sectionCount);
    printf("Section Info Offset: %d\n", header.sectionInfoOffset);
    printf("Buffer Data Offset: %d\n", header.bufferDataOffset);
    
    printf("Section manifest:\n");
    printf("Offset 1: %d\n", header.offset1);
    printf("Offset 2: %d\n", header.offset2);
    printf("Offset 3: %d\n", header.offset3);
    printf("Offset 4: %d\n", header.offset4);

    RWSectionInfo *sectionInfos = malloc(sizeof(RWSectionInfo) * header.sectionCount);

    printf("Section info:\n");
    data = initData + header.sectionInfoOffset;
    for (int i = 0; i < header.sectionCount; i++)
    {
        RWSectionInfo sectionInfo = { 0 };
        memcpy(&sectionInfo, data, sizeof(RWSectionInfo));
        data += sizeof(RWSectionInfo);

        printf("\nSection %d:\n", i);
        printf("Data Offset: %d\n", sectionInfo.dataOffset);
        printf("Size: %d\n", sectionInfo.size);
        printf("Alignment: %d\n", sectionInfo.alignment);
        printf("Type Code Index: %d\n", sectionInfo.typeCodeIndex);
        printf("Type Code: %#x\n", sectionInfo.typeCode);

        sectionInfos[i] = sectionInfo;
    }

    for (int i = 0; i < header.sectionCount; i++) {
        RWSectionInfo sectionInfo = sectionInfos[i];
        data = initData + sectionInfo.dataOffset;

        printf("\nSection %d:\n", i);

        switch (sectionInfo.typeCode)
        {
            case 0x70001: // KeyframeAnim
            {
                RWKeyframeAnim keyframeAnim;
                memcpy(&keyframeAnim, data, sizeof(RWKeyframeAnim));

                printf("Keyframe Anim Info:\n");
                printf("Channel name offset: %d\n", keyframeAnim.channelNameOffset);
                printf("Channel count: %d\n", keyframeAnim.channelCount);
                printf("Skeleton Id: %#x\n", keyframeAnim.skeletonId);
                printf("Channel Data offset: %d\n", keyframeAnim.channelDataOffset);
                printf("Padding end offset: %d\n", keyframeAnim.paddingEndOffset);
                printf("Length: %f\n", keyframeAnim.length);
                printf("Flags: %#x\n", keyframeAnim.flags);
                printf("Channel Info Offset: %d\n", keyframeAnim.channelInfoOffset);
                printf("TODO\n");
            } break;
            case 0xff0001: // Animations
            {
                data += sizeof(uint32_t);
                uint32_t count = *(uint32_t*)data;
                data += sizeof(uint32_t);

                printf("Animations Info:\n");
                printf("Count: %d\n", count);
                for (int i = 0; i < count; i++)
                {
                    uint32_t animationIndex = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    uint32_t section = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    printf("Animation %#x: [Section %d]\n", animationIndex, section);
                }
            } break;
            case 0x70002: // Skeleton
            {
                RWSkeleton skeleton;
                memcpy(&skeleton, data, sizeof(RWSkeleton));
                data += sizeof(RWSkeleton);

                printf("Skeleton Info:\n");
                printf("Bone Flag Offset: %d\n", skeleton.boneFlagOffset);
                printf("Bone Parent Offset: %d\n", skeleton.boneParentOffset);
                printf("Bone Name Offset: %d\n", skeleton.boneNameOffset);
                printf("Bone Count: %d\n", skeleton.count);
                printf("Skeleton Id: %#x\n", skeleton.skeletonId);
                printf("TODO\n");
            } break;
            case 0x7000b: // SkeletonsInK
            {
                RWSkeletonsInK skeletonsink;
                memcpy(&skeletonsink, data, sizeof(RWSkeletonsInK));
                data += sizeof(RWSkeletonsInK);

                printf("SkeletonsInK Info:\n");
                printf("Array Offset: %d\n", skeletonsink.arrayOffset);
                printf("Skeleton: [Section %d]\n", skeletonsink.skeleton);
                printf("Count: %d\n", skeletonsink.arrayCount);

                printf("TODO\n");

            } break;
            case 0x80005: // BBox
            {
                RWBBox bbox;
                memcpy(&bbox, data, sizeof(RWBBox));
                data += sizeof(RWBBox);

                printf("Bounding Box Information:\n");
                printf("Min: {%f, %f, %f}\n", bbox.min.x, bbox.min.y, bbox.min.z);
                printf("Max: {%f, %f, %f}\n", bbox.max.x, bbox.max.y, bbox.max.z);
            } break;
            case 0x20009: // Mesh
            {
                RWMesh rwmesh;
                Mesh mesh = { 0 };

                memcpy(&rwmesh, data, sizeof(RWMesh));
                data += sizeof(RWMesh);

                mesh.triangleCount = rwmesh.triangleCount;
                mesh.vertexCount = rwmesh.vertexCount;
                
                printf("Mesh Info:\n");
                printf("Primitive Type: %d\n", rwmesh.primitiveType);
                printf("Index buffer: [Section %d]\n", rwmesh.indexBuffer);
                printf("Triangle Count: %d\n", rwmesh.triangleCount);
                printf("Buffer Count: %d\n", rwmesh.bufferCount);
                printf("First Index: %d\n", rwmesh.firstIndex);
                printf("Primitive Count: %d\n", rwmesh.primitiveCount);
                printf("First Vertex: %d\n", rwmesh.firstVertex);
                printf("Vertex Count: %d\n", rwmesh.vertexCount);

                for (int j = 0; j < rwmesh.bufferCount; j++)
                {
                    uint32_t buffer = *(uint32_t*)data;
                    data += sizeof(uint32_t);
                    printf("Buffer %d: [Section %d]\n", j, buffer);

                    RWVertexBuffer *vertexBuffer = (RWVertexBuffer*)LoadSectionData(sectionInfos, buffer, initData, NULL);
                    
                    unsigned char *vertexData = LoadSectionData(sectionInfos, vertexBuffer->vertexData, initData, NULL);
                    unsigned char *vertexInitData = vertexData;

                    int vertexCount = rwmesh.vertexCount;
                    int vertexStart = rwmesh.firstVertex;

                    RWVertexElement *positionElement = NULL;

                    RWVertexDescription *description = (RWVertexDescription*)LoadSectionData(sectionInfos, vertexBuffer->vertexDescription, initData, NULL);
                    
                    for (int k = 0; k < description->count; k++)
                    {
                        unsigned char *descData = (unsigned char *)description + sizeof(RWVertexDescription) - sizeof(void*);
                        descData += sizeof(RWVertexElement) * k;
                        RWVertexElement *element = (RWVertexElement*)descData;
                        if (element->typeCode == 0)
                        {
                            positionElement = element;
                        }
                    }

                    float *positions = malloc(3 * vertexCount * sizeof(float));

                    SaveFileData("helpme.bin", vertexData, sectionInfos[vertexBuffer->vertexData].size);

                    for (int k = 0; k < vertexCount; k++)
                    {
                        printf("Load vertex %d: ", k);
                        vertexData = vertexInitData + (8 * vertexBuffer->vertexSize + vertexCount) + 0x10 *k; //((vertexStart+k) * vertexBuffer->vertexSize + positionElement->offset);
                        printf("vertexData %p, off %#x, ", vertexData, vertexData - vertexInitData);
                        positions[k*3+0] = *(float*)vertexData;
                        vertexData += sizeof(float);
                        positions[k*3+1] = *(float*)vertexData;
                        vertexData += sizeof(float);
                        positions[k*3+2] = *(float*)vertexData;
                        vertexData += sizeof(float);
                        printf("{%f, %f, %f}.\n", positions[k*3+0], positions[k*3+1], positions[k*3+2]);
                    }

                    mesh.vertices = positions;

                }

                //rw4data.model.meshCount++;
                //rw4data.model.meshes = realloc(rw4data.model.meshes, rw4data.model.meshCount * sizeof(Mesh));
                //rw4data.model.meshes[rw4data.model.meshCount - 1] = mesh;
            } break;
            case 0x20005: // VertexBuffer
            {
                RWVertexBuffer vertexBuffer;
                memcpy(&vertexBuffer, data, sizeof(RWVertexBuffer));
                data += sizeof(RWVertexBuffer);

                printf("Vertex Buffer Info:\n");
                printf("Vertex Description: [Section %d]\n", vertexBuffer.vertexDescription);
                printf("Base Vertex Index: %d\n", vertexBuffer.baseVertexIndex);
                printf("Vertex Count: %d\n", vertexBuffer.vertexCount);
                printf("Vertex Size: %d\n", vertexBuffer.vertexSize);
                printf("Vertex Data: [Section %d]\n", vertexBuffer.vertexData);
            } break;
            case 0x10030: // BaseResource
            {
                printf("Base Resource info:\n");
                printf("Size: %d\n", sectionInfo.size);
            } break;
            case 0x20007: // IndexBuffer
            {
                RWIndexBuffer indexBuffer;
                memcpy(&indexBuffer, data, sizeof(RWIndexBuffer));
                data += sizeof(RWIndexBuffer);

                printf("Index Buffer Info:\n");
                printf("DirectX Index Buffer: %d\n", indexBuffer.dxIndexBuffer);
                printf("Start Index: %d\n", indexBuffer.startIndex);
                printf("Primitive Count: %d\n", indexBuffer.primitiveCount);
                printf("Usage: %d\n", indexBuffer.usage);
                printf("Format: %d\n", indexBuffer.format);
                printf("Primitive Type: %d\n", indexBuffer.primitiveType);
                printf("Index Data: [Section %d]\n", indexBuffer.indexData);
            } break;
            case 0x20004: // VertexDescription
            {
                RWVertexDescription vertexDescription;
                memcpy(&vertexDescription, data, sizeof(RWVertexDescription));
                data += sizeof(RWVertexDescription);

                printf("Vertex Description Info:\n");
                printf("DirectX Vertex Declaration: %d\n", vertexDescription.dxVertexDeclaration);
                printf("Count: %d\n", vertexDescription.count);
                printf("Vertex Size: %d\n", vertexDescription.vertexSize);
                printf("Element Flags: %#x\n", vertexDescription.elementFlags);

                vertexDescription.elements = malloc(sizeof(RWVertexElement) * vertexDescription.count);
                memcpy(vertexDescription.elements, data, sizeof(RWVertexElement) * vertexDescription.count);

                for (int j = 0; j < vertexDescription.count; j++)
                {
                    RWVertexElement element = vertexDescription.elements[j];
                    printf("\nElement %d:\n", j);
                    printf("Stream: %d\n", element.stream);
                    printf("Offset: %d\n", element.offset);
                    printf("Type: %d\n", element.type);
                    printf("Method: %d\n", element.method);
                    printf("Usage: %d\n", element.usage);
                    printf("Usage index: %d\n", element.usageIndex);
                    printf("Type Code: %#x\n", element.typeCode);
                }
            } break;
            case 0x20003: // Raster
            {
                RWRaster raster;
                memcpy(&raster, data, sizeof(RWRaster));
                data += sizeof(RWRaster);

                printf("Raster info:\n");
                printf("Texture Format: %d\n", raster.textureFormat);
                printf("Texture Flags: %#x\n", raster.textureFlags);
                printf("Volume Depth: %d\n", raster.volumeDepth);
                //printf("dxBaseTexture: %#x\n", raster.dxBaseTexture);
                printf("width: %d\n", raster.width);
                printf("height: %d\n", raster.height);
                printf("mipmaps: %d\n", raster.mipmapLevels);
                printf("textureData: [Section %d]\n", raster.textureData);

                if (raster.textureFormat != 21)
                {
                    printf("Unimplemented texture format %d.\n", raster.textureFormat);
                    rw4data.corrupted = true;
                }

                if (raster.textureFlags & 0x1000)
                {
                    printf("Cubemaps not supported.\n");
                    rw4data.corrupted = true;
                }

                int textureDataSize;
                unsigned char *textureData = LoadSectionData(sectionInfos, raster.textureData, initData, &textureDataSize);

                int blockSize = 0;
                int rgbBitCount = 0;
                int pfFlags = 0;
                
                if (raster.textureFormat == 21)
                {
                    // A8R8G8B8
                    pfFlags |= 0x40 | 0x1; // RGB | ALPHAPIXELS
                    rgbBitCount = 32;
                }

                int pitchOrLinearSize = raster.width * raster.height * rgbBitCount / 8;

                MemStream ddsStream = { 0 };

                char padding[256] = { 0 }; // 0-filled padding

                memstream_write32(&ddsStream, 0x20534444L); // signature
                memstream_write32(&ddsStream, 0x7c); // size (default 0x7c)
                const int DEFAULT_FLAGS = 0x80000 | 0x20000 | 0x1000 | 0x4 | 0x2 | 0x1; // LINEARSIZE | MIPMAPCOUNT | PIXELFORMAT | WIDTH | HEIGHT | CAPS
                                                                                
                memstream_write32(&ddsStream, DEFAULT_FLAGS); // flags
                memstream_write32(&ddsStream, raster.height); // height
                memstream_write32(&ddsStream, raster.width); // width
                memstream_write32(&ddsStream, pitchOrLinearSize); // pitchOrLinearSize
                memstream_write32(&ddsStream, 0); // depth
                memstream_write32(&ddsStream, raster.mipmapLevels); // mipmapCount
                memstream_write(&ddsStream, padding, 44); // padding

                // PixelFormat
                memstream_write32(&ddsStream, 32); // size (default 32)
                memstream_write32(&ddsStream, pfFlags); // flags
                memstream_write32(&ddsStream, 0); // FourCC (not compressed so 0)
                memstream_write32(&ddsStream, rgbBitCount); // rgbBitCount
                memstream_write32(&ddsStream, 0x00FF0000); // maskRed
                memstream_write32(&ddsStream, 0x0000FF00); // maskGreen
                memstream_write32(&ddsStream, 0x000000FF); // maskBlue
                memstream_write32(&ddsStream, 0xFF000000); // maskAlpha

                memstream_write32(&ddsStream, 0); // caps
                memstream_write32(&ddsStream, 0); // caps2
                memstream_write32(&ddsStream, 0); // caps3
                memstream_write32(&ddsStream, 0); // caps4
                memstream_write32(&ddsStream, 0); // padding 4 bytes

                // DDS Data
                memstream_write(&ddsStream, textureData, textureDataSize);

                Image img = LoadImageFromMemory(".dds", ddsStream.buf, ddsStream.size);
                
                rw4data.type = RW4_TEXTURE;
                rw4data.data.texData.img = img;

                ExportImage(img, "rw4_out.png");

            } break;
            default:
            {
                printf("Unrecognized type code %#x.\n", sectionInfo.typeCode);

                //FILE *f = fopen(TextFormat("corrupted/RW4-%#X-%#X.unkn", sectionInfo.typeCode, i), "wb");
                //fwrite(data, 1, sectionInfo.size, f);
                //fclose(f);
            } break;
        }
    }

    return rw4data;
}
