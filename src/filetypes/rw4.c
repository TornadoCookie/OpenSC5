#include "filetypes/rw4.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <cpl_endian.h>
#include "memstream.h"

// All of this adapted from
// SporeModder-FX
// /src/sporemodder/file/rw4/*.java
//               ||/view/editors/RWModelViewer.java

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

typedef struct RWSectionTypes {

} RWSectionTypes;

typedef struct RWSectionExternalArenas {

} RWSectionExternalArenas;

typedef struct RWSectionSubReferences {
    uint32_t count;
    uint32_t field4;
    uint32_t field8;
    uint32_t fieldc;
    uint32_t offset;
    uint32_t countAgain;
} RWSectionSubReferences;

typedef struct RWSectionAtoms {

} RWSectionAtoms;

typedef struct RWTriangleKDTreeProcedural {
    RWBBox bbox;
    int32_t field20;
    int32_t field24;
    int32_t triangleCount;
    int32_t field2c;
    int32_t vertexCount;
    uint32_t pTriangles;
    uint32_t pVertices;
    uint32_t p4;
    uint32_t p3;
} RWTriangleKDTreeProcedural;

typedef struct RWCompiledState {
    uint32_t size;
    int32_t primitiveType;
    int32_t flags1;
    int32_t flags2;
    int32_t flags3;
    int32_t field14;
    int32_t rendererID;
    uint32_t padding;
} RWCompiledState;

typedef struct RWMeshCompiledStateLink {
    int32_t mesh;
    int32_t count;
} RWMeshCompiledStateLink;

unsigned char *LoadSectionData(RWSectionInfo *sectionInfos, int section, const char *initData, int *dataSize)
{
    printf("LoadSectionData %d\n", section);
    if (dataSize) *dataSize = sectionInfos[section].size;
    return sectionInfos[section].dataOffset + initData;
}

Mesh LoadMeshRW4(RWMesh rwmesh, unsigned char *data, RWSectionInfo *sectionInfos, const unsigned char *initData)
{
    Mesh mesh = { 0 };

    mesh.triangleCount = rwmesh.triangleCount;
    mesh.vertexCount = rwmesh.vertexCount;

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
        RWVertexElement *texcoordElement = NULL;
        RWVertexElement *normalElement = NULL;

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
            else if (element->typeCode == 2)
            {
                normalElement = element;
            }
            else if (element->typeCode == 6)
            {
                texcoordElement = element;
            }
        }

        float *positions = malloc(3 * vertexCount * sizeof(float));
        float *normals   = malloc(3 * vertexCount * sizeof(float));
        float *texcoords = malloc(2 * vertexCount * sizeof(float));

        for (int k = 0; k < vertexCount; k++)
        {
            vertexData = vertexInitData + ((vertexStart+k) * vertexBuffer->vertexSize + positionElement->offset);
            positions[k*3+0] = *(float*)vertexData;
            vertexData += sizeof(float);
            positions[k*3+1] = *(float*)vertexData;
            vertexData += sizeof(float);
            positions[k*3+2] = *(float*)vertexData;
            vertexData += sizeof(float);

            vertexData = vertexInitData + ((vertexStart+k) * vertexBuffer->vertexSize + normalElement->offset);
            normals[k*3+0] = *(float*)vertexData;
            vertexData += sizeof(float);
            normals[k*3+1] = *(float*)vertexData;
            vertexData += sizeof(float);
            normals[k*3+2] = *(float*)vertexData;
            vertexData += sizeof(float);

            vertexData = vertexInitData + ((vertexStart+k) * vertexBuffer->vertexSize + texcoordElement->offset);
            texcoords[k*2+0] = *(float*)vertexData;
            vertexData += sizeof(float);
            texcoords[k*2+1] = *(float*)vertexData;
            vertexData += sizeof(float);
        }

        mesh.vertices = positions;
        mesh.normals = normals;
        mesh.texcoords = texcoords;

    }

    if (rwmesh.indexBuffer)
    {
        RWIndexBuffer *indexBuffer = LoadSectionData(sectionInfos, rwmesh.indexBuffer, initData, NULL);

        int indexComponents = 1;
        //if (!mesh.normals) indexComponents = 2;

        uint16_t *indices = malloc(mesh.triangleCount * 3 * sizeof(uint16_t));
        unsigned char *indexData = LoadSectionData(sectionInfos, indexBuffer->indexData, initData, NULL);

        data = indexData + rwmesh.firstIndex * 2;
        
        for (int j = 0; j < mesh.triangleCount-1; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                uint16_t index = *(uint16_t*)data;
                data += sizeof(uint16_t);

                index += indexBuffer->startIndex;
                index -= rwmesh.firstVertex;

                indices[j*indexComponents*3 + indexComponents*k] = index;
                //indices[j*indexComponents*3 + indexComponents*k + 1] = index;
                //if (mesh.normals)
                //{
                //    indices[j*indexComponents*3 + indexComponents*k + 2] = index;
                //}
            }
        }

        mesh.indices = indices;
    }

    return mesh;
}

RW4Data LoadRW4Data(unsigned char *data, int dataSize)
{
    RW4Data rw4data = { 0 };

    const unsigned char *initData = data;

    printf("RW4 Info:\n");

    RWHeader header = { 0 };
    memcpy(&header, data, sizeof(RWHeader));
    data += sizeof(RWHeader);

    header.sectionCount &= 0xFF;

    printf("Type: %#x\n", header.type);

    if (header.type == 0x1)
    {
        rw4data.type = RW4_MODEL;
    }
    else
    {
        rw4data.type = RW4_TEXTURE;
    }

    printf("Section Count: %#x\n", header.sectionCount);
    printf("Section Info Offset: %d\n", header.sectionInfoOffset);
    printf("Buffer Data Offset: %d\n", header.bufferDataOffset);
    
    printf("Section manifest:\n");
    printf("Offset 1: %d\n", header.offset1);
    printf("Offset 2: %d\n", header.offset2);
    printf("Offset 3: %d\n", header.offset3);
    printf("Offset 4: %d\n", header.offset4);

    data = initData + header.offset3;
    RWSectionSubReferences *subRefs = (RWSectionSubReferences*)data;
    printf("Subreference info:\n");
    printf("Count: %d\n", subRefs->count);
    printf("Offset: %d\n", subRefs->offset);

    rw4data.corrupted = true;

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

        if (sectionInfo.typeCode == 0x10030)
        {
            sectionInfo.dataOffset += header.bufferDataOffset;
        }


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
                
                printf("Mesh Info:\n");
                printf("Primitive Type: %d\n", rwmesh.primitiveType);
                printf("Index buffer: [Section %d]\n", rwmesh.indexBuffer);
                printf("Triangle Count: %d\n", rwmesh.triangleCount);
                printf("Buffer Count: %d\n", rwmesh.bufferCount);
                printf("First Index: %d\n", rwmesh.firstIndex);
                printf("Primitive Count: %d\n", rwmesh.primitiveCount);
                printf("First Vertex: %d\n", rwmesh.firstVertex);
                printf("Vertex Count: %d\n", rwmesh.vertexCount);

                

                //rw4data.data.mdlData.mdl.meshCount++;
                //rw4data.data.mdlData.mdl.meshes = realloc(rw4data.data.mdlData.mdl.meshes, rw4data.data.mdlData.mdl.meshCount * sizeof(Mesh));
                //rw4data.data.mdlData.mdl.meshes[rw4data.data.mdlData.mdl.meshCount - 1] = mesh;
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
            case 0x80003: // TriangleKDTreeProcedural
            {
                RWTriangleKDTreeProcedural kdt;
                memcpy(&kdt, data, sizeof(RWTriangleKDTreeProcedural));

                printf("TriangleKDTreeProcedural info:\n");
                printf("Triangle count: %d\n", kdt.triangleCount);
                printf("Vertex count: %d\n", kdt.vertexCount);
                printf("Triangle offset: %d\n", kdt.pTriangles);
                printf("Vertex offset: %d\n", kdt.pVertices);
                printf("p4: %d\n", kdt.p4);
                printf("p3: %d\n", kdt.p3);

                printf("TODO\n");
            } break;
            case 0x2001a: // MeshCompiledStateLink
            {
                RWMeshCompiledStateLink csl;
                memcpy(&csl, data, sizeof(RWMeshCompiledStateLink));
                data += sizeof(RWMeshCompiledStateLink);

                printf("MeshCompiledStateLink info:\n");
                printf("Mesh: [Section %d]\n", csl.mesh);
                printf("Count: %d\n", csl.count);

                for (int j = 0; j < csl.count; j++)
                {
                    int32_t compiledState = *(int32_t*)data;
                    data += sizeof(int32_t);

                    printf("Compiled State %d: [Section %d]\n", j, compiledState);
                } 
            } break;
            case 0x2000b: //CompiledState
            {
                RWCompiledState compiledState;
                memcpy(&compiledState, data, sizeof(RWCompiledState));
                data += sizeof(RWCompiledState);

                const int FLAG_MODELTOWORLD       = 0x000001;
                const int FLAG_SHADER_DATA        = 0x000008;
                const int FLAG_MATERIAL_COLOR     = 0x000010;
                const int FLAG_AMBIENT_COLOR      = 0x000020;
                const int FLAG_USE_BOOLEANS       = 0x008000;
                const int FLAG_VERTEX_DESCRIPTION = 0x100000;

                const int FLAG3_TEXTURE_SLOTS   = 0x01FFFF;
                const int FLAG3_RENDER_STATES   = 0x020000;
                const int FLAG3_PALETTE_ENTRIES = 0x100000;
                
                printf("CompiledState info:\n");
                printf("Size: %d\n", compiledState.size);
                printf("Primitive Type: %d\n", compiledState.primitiveType);
                printf("Flags 1: %#x\n", compiledState.flags1);
                printf("Flags 2: %#x\n", compiledState.flags2);
                printf("Flags 3: %#x\n", compiledState.flags3);
                printf("Renderer ID: %#x\n", compiledState.rendererID);

                if (compiledState.flags1 & FLAG_MODELTOWORLD)
                {
                    printf("TODO FLAG_MODELTOWORLD\n");
                    rw4data.corrupted = true;
                }

                RWVertexDescription *vertexDescription;

                if (compiledState.flags1 & FLAG_VERTEX_DESCRIPTION)
                {
                    vertexDescription = data;
                    data += sizeof(RWVertexDescription) - sizeof(void*);
                    data += sizeof(RWVertexElement) * vertexDescription->count;
                }

                if (compiledState.flags1 & FLAG_MATERIAL_COLOR)
                {
                    printf("TODO FLAG_MATERIAL_COLOR\n");
                    rw4data.corrupted = true;
                }

                if (compiledState.flags1 & FLAG_AMBIENT_COLOR)
                {
                    printf("TODO FLAG_AMBIENT_COLOR\n");
                    rw4data.corrupted = true;
                }

                if (compiledState.flags1 & 0x3FC0)
                {
                    printf("TODO 0x3FC0\n");
                    rw4data.corrupted = true;
                }

                if (compiledState.flags1 & FLAG_USE_BOOLEANS)
                {
                    data += 17; // 17 unknown booleans
                }

                if (compiledState.flags1 & 0xF0000)
                {
                    printf("TODO 0xF0000\n");
                    rw4data.corrupted = true;
                }

                if (compiledState.field14)
                {
                    printf("TODO field14 %#x\n", compiledState.field14);
                    rw4data.corrupted = true;
                }

                if (compiledState.flags3 & FLAG3_RENDER_STATES)
                {
                    printf("TODO FLAG3_RENDER_STATES\n");
                    rw4data.corrupted = true;
                }

                int32_t paletteEntriesIndex = *(int32_t*)data;
                data += sizeof(int32_t);

                if (compiledState.flags3 & FLAG3_PALETTE_ENTRIES)
                {
                    printf("TODO FLAG3_PALETTE_ENTRIES\n");
                    rw4data.corrupted = true;
                }

                if (compiledState.flags3 & FLAG3_TEXTURE_SLOTS)
                {
                    int32_t samplerIndex = *(int32_t*)data;
                    data += sizeof(int32_t);

                    printf("Texture Slot info:\n");

                    while (samplerIndex != -1)
                    {
                        int32_t raster = *(int32_t*)data;
                        data += sizeof(int32_t);
                        
                        int32_t stageStatesMask = *(int32_t*)data;
                        data += sizeof(int32_t);

                        printf("Sampler Index: %d\n", samplerIndex);
                        printf("Raster: [Section %#x]\n", raster);
                        printf("Stage States Mask: %d\n", stageStatesMask);

                        if (stageStatesMask)
                        {
                            int32_t state = *(int32_t*)data;
                            data += sizeof(int32_t);

                            while (state != -1)
                            {
                                int unkn = *(int32_t*)data;
                                data += sizeof(int32_t);

                                // clogs output
                                //printf("State=%#x, data=%#x\n", state, unkn);
                                
                                state = *(int32_t*)data;
                                data += sizeof(int32_t);
                            }
                        }

                        int32_t samplerStatesMask = *(int32_t*)data;
                        data += sizeof(int32_t);

                        if (samplerStatesMask)
                        {
                            int state = *(int32_t*)data;
                            data += sizeof(int32_t);

                            while (state != -1)
                            {
                                int unkn = *(int32_t*)data;
                                data += sizeof(int32_t);

                                state = *(int32_t*)data;
                                data += sizeof(int32_t);
                            }
                        }

                        samplerIndex = *(int32_t*)data;
                        data += sizeof(int32_t);
                    }
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
                
                rw4data.corrupted = false;

                if (raster.textureFlags & 0x1000)
                {
                    printf("Cubemaps not supported.\n");
                    rw4data.corrupted = true;
                }

                if (raster.width % 4 != 0 || raster.height % 4 != 0)
                {
                    printf("Invalid size.\n");
                    rw4data.corrupted = true;
                    return rw4data;
                }

                int textureDataSize;
                unsigned char *textureData = LoadSectionData(sectionInfos, raster.textureData, initData, &textureDataSize);

                int blockSize = 0;
                int rgbBitCount = 0;
                int pfFlags = 0;
                bool isCompressed = false;
                
                if (raster.textureFormat == 21)
                {
                    // A8R8G8B8
                    pfFlags |= 0x40 | 0x1; // RGB | ALPHAPIXELS
                    rgbBitCount = 32;
                }
                else if (!memcmp(&raster.textureFormat, "DXT1", 4))
                {
                    blockSize = 8;
                }
                else if (!memcmp(&raster.textureFormat, "DXT5", 4))
                {
                    blockSize = 16;
                }
                else if (raster.textureFormat == 0x74)
                {
                    // TODO not supported by ANYTHING bro
                    // D3DFMT_A32B32G32R32F
                    blockSize = 16;
                }
                else
                {
                    printf("Unimplemented texture format %d.\n", raster.textureFormat);
                    rw4data.corrupted = true;
                }

                int pitchOrLinearSize = 0;

                if (blockSize == 0)
                {
                    pitchOrLinearSize = raster.width * raster.height * rgbBitCount / 8;
                }
                else
                {
#define max(x, y) (((x)>(y))?(x):(y))
                    pitchOrLinearSize = max(1, ((raster.width + 3) / 4) * max(1, (raster.height + 3) / 4)) * blockSize;
                    if (raster.textureFormat == 0x74)
                    {
                        pitchOrLinearSize = raster.width * raster.height * raster.volumeDepth * blockSize;
                    }
                    isCompressed = true;
                }

                int fourCC = 0;

                if (isCompressed)
                {
                    pfFlags |= 0x4; //FOURCC
                    fourCC = raster.textureFormat;
                }

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
                memstream_write32(&ddsStream, fourCC); // FourCC
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

                SaveFileData("rw4_out.dds", ddsStream.buf, ddsStream.size);

                Image img = LoadImageFromMemory(".dds", ddsStream.buf, ddsStream.size);
                
                if (rw4data.type == RW4_TEXTURE)
                {
                    rw4data.data.texData.img = img;
                }
                else if (rw4data.type == RW4_MODEL)
                {
                    
                }

                //rw4data.type = RW4_TEXTURE;
                //rw4data.data.texData.img = img;

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
