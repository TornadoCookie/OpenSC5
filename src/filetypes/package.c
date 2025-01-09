#include "filetypes/package.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cpl_endian.h>
#include <raylib.h>
#include "filetypes/wwriff.h"
#include <threadpool.h>
#include <cpl_pthread.h>
#include <ctype.h>
#include <sys/stat.h>
#include "memstream.h"

#ifdef __linux__
#define mkdir(x) mkdir(x, 0777)
#endif

typedef struct PackageHeader {
    char magic[4];              //00
    uint32_t majorVersion;      //04
    uint32_t minorVersion;      //08
    uint32_t unknown[3];        //0C
    uint32_t dateCreated;       //18
    uint32_t dateModified;      //1C
    uint32_t indexMajorVersion; //20
    uint32_t indexEntryCount;   //24
    uint32_t firstIndexEntryOffset; //28
    uint32_t indexSize;         //2C
    uint32_t holeEntryCount;    //30
    uint32_t holeOffset;        //34
    uint32_t holeSize;          //38
    uint32_t indexMinorVersion; //3C
    uint32_t indexOffset;       //40
    uint32_t unknown2;          //44
    unsigned char reserved[24]; //48
                                //5C
} PackageHeader;

typedef struct IndexEntry {
    uint32_t type;
    uint32_t group;
    uint32_t instance;
    uint32_t chunkOffset;
    uint32_t diskSize;
    uint32_t memSize;
    uint16_t compressed;
    uint16_t unknown;
    
    bool isCompressed;
} IndexEntry;

typedef struct IndexData {
    uint32_t null;
} IndexData;

typedef struct Index {
    uint32_t indexType;
    
    IndexData data;
} Index;

static void readuint(uint32_t *ret, FILE *f)
{
    fread(ret, sizeof(uint32_t), 1, f);

    if (feof(f))
    {
        TRACELOG(LOG_FATAL, "Unexpected end of file: %lu.\n", ftell(f));
        exit(EXIT_FAILURE);
    }
}

static bool ProcessPackageData(unsigned char *data, int dataSize, uint32_t dataType, PackageEntry *pkgEntry)
{
    switch (dataType)
    {
        case PKGENTRY_PROP: // Properties files https://simswiki.info/wiki.php?title=Spore:00B1B104
        {
            PropData propData = LoadPropData(data, dataSize);
            pkgEntry->data.propData = propData;
            return !propData.corrupted;
        } break;
        case PKGENTRY_RAST: // Raster file https://simswiki.info/wiki.php?title=Spore:2F4E681C
        {
            RastData rastData = LoadRastData(data, dataSize);
            pkgEntry->data.imgData.img = rastData.img;
            //if (!rastData.corrupted)
            //{
            //    pkgEntry->data.imgData.tex = LoadTextureFromImage(rastData.img);
            //}
            return !rastData.corrupted;            
        } break;
        case PKGENTRY_ER2:
        case PKGENTRY_HTML:
        case PKGENTRY_CSS:
        case PKGENTRY_JSN8:
        case PKGENTRY_SCPT: // Script file format (?)
        case PKGENTRY_TEXT: // "textual" file.
        case PKGENTRY_JSON: // JSON file.
        {
            TRACELOG(LOG_DEBUG, "Text:\n");
            char *str = malloc(dataSize);
            int actualSize = 0;

            // Filter out these disgusting UTF-8 characters.
            for (int i = 0; i < dataSize; i++)
            {
                if (!isprint(data[i]) && data[i] != '\n' && data[i] != '\t') continue;
                str[actualSize] = data[i];
                actualSize++;
            }

            str = realloc(str, actualSize + 1);
            str[actualSize] = 0;

            TRACELOG(LOG_DEBUG, "%s\n", str);
            pkgEntry->data.scriptSource = str;
        } break;
        case PKGENTRY_RULE: // Binary rules file. https://community.simtropolis.com/forums/topic/55521-binary-rules-file-format/
        {
            RulesData rulesData = LoadRulesData(data, dataSize);
            pkgEntry->data.rulesData = rulesData;
            return !rulesData.corrupted;
        } break;
        case PKGENTRY_PNG: // PNG file.
        {
            pkgEntry->data.imgData.img = LoadImageFromMemory(".png", data, dataSize);
            pkgEntry->corrupted = !IsImageValid(pkgEntry->data.imgData.img);
            //if (!pkgEntry->corrupted)
            //{
            //    pkgEntry->data.imgData.tex = LoadTextureFromImage(pkgEntry->data.imgData.img);
            //}
            return !pkgEntry->corrupted;
        } break;
        case PKGENTRY_RW4:
        case PKGENTRY_TTF:
        case PKGENTRY_SWB:
        case PKGENTRY_MOV:
        case PKGENTRY_EXIF:
        {
            return false;
        } break;
        case PKGENTRY_BNK:
        {
            pkgEntry->data.bnkData = LoadBnkData(data, dataSize);
            return !pkgEntry->data.bnkData.corrupted;
        } break;
        case PKGENTRY_GIF:
        {
            pkgEntry->data.gifData.img = LoadImageAnimFromMemory(".gif", data, dataSize, &pkgEntry->data.gifData.frameCount);
            pkgEntry->corrupted = !IsImageValid(pkgEntry->data.gifData.img);
            //if (!pkgEntry->corrupted)
            //{
            //    pkgEntry->data.gifData.tex = LoadTextureFromImage(pkgEntry->data.gifData.img);
            //}
            return !pkgEntry->corrupted;
        } break;
        case PKGENTRY_WEM:
        {
            ExportWWRiffToFile(data, dataSize, TextFormat("corrupted/%#X-%#X-%#X.ogg", pkgEntry->type, pkgEntry->group, pkgEntry->instance));
        } break;
        default:
        {
            TRACELOG(LOG_WARNING, "Unknown data type %#X.", dataType);
            return false;
        } break;
    }

    return true;
}

/**
 * @brief Decompress a RefPack bitstream
 * @param indata - (optional) Pointer to the input RefPack bitstream; may be
 *	NULL
 * @param bytes_read_out - (optional) Pointer to a size_t which will be filled
 *	with the total number of bytes read from the RefPack bitstream; may be
 *	NULL
 * @param outdata - Pointer to the output buffer which will be filled with the
 *	decompressed data; outdata may be NULL only if indata is also NULL
 * @return The value of the "decompressed size" field in the RefPack bitstream,
 *	or 0 if indata is NULL
 *
 * This function is a verbatim translation from x86 assembly into C (with
 * new names and comments supplied) of the RefPack decompression function
 * located at TSOServiceClientD_base+0x724fd in The Sims Online New & Improved
 * Trial.
 *
 * This function ***does not*** perform any bounds-checking on reading or
 * writing. It is inappropriate to use this function on untrusted data obtained
 * from the internet (even though that is exactly what The Sims Online does...).
 * Here are the potential problems:
 * - This function will read past the end of indata if the last command in
 *   indata tells it to.
 * - This function will write past the end of outdata if indata tells it to.
 * - This function will read before the beginning of outdata if indata tells
 *   it to.
 */
size_t refpack_decompress_unsafe(const uint8_t *indata, size_t *bytes_read_out,
	uint8_t *outdata)
{
	const uint8_t *in_ptr;
	uint8_t *out_ptr;
	uint16_t signature;
	uint32_t decompressed_size = 0;
	uint8_t byte_0, byte_1, byte_2, byte_3;
	uint32_t proc_len, ref_len;
	uint8_t *ref_ptr;
	uint32_t i;
 
	in_ptr = indata, out_ptr = outdata;
	if (!in_ptr)
		goto done;
 
	signature = ((in_ptr[0] << 8) | in_ptr[1]), in_ptr += 2;
	if (signature & 0x0100)
		in_ptr += 3; /* skip over the compressed size field */
 
	decompressed_size = ((in_ptr[0] << 16) | (in_ptr[1] << 8) | in_ptr[2]);
	in_ptr += 3;
 
	while (1) {
		byte_0 = *in_ptr++;
		if (!(byte_0 & 0x80)) {
			/* 2-byte command: 0DDRRRPP DDDDDDDD */
			byte_1 = *in_ptr++;
 
			proc_len = byte_0 & 0x03;
			for (i = 0; i < proc_len; i++)
				*out_ptr++ = *in_ptr++;
 
			ref_ptr = out_ptr - ((byte_0 & 0x60) << 3) - byte_1 - 1;
			ref_len = ((byte_0 >> 2) & 0x07) + 3;
			for (i = 0; i < ref_len; i++)
				*out_ptr++ = *ref_ptr++;
		} else if(!(byte_0 & 0x40)) {
			/* 3-byte command: 10RRRRRR PPDDDDDD DDDDDDDD */
			byte_1 = *in_ptr++;
			byte_2 = *in_ptr++;
 
			proc_len = byte_1 >> 6;
			for (i = 0; i < proc_len; i++)
				*out_ptr++ = *in_ptr++;
 
			ref_ptr = out_ptr - ((byte_1 & 0x3f) << 8) - byte_2 - 1;
			ref_len = (byte_0 & 0x3f) + 4;
			for (i = 0; i < ref_len; i++)
				*out_ptr++ = *ref_ptr++;
		} else if(!(byte_0 & 0x20)) {
			/* 4-byte command: 110DRRPP DDDDDDDD DDDDDDDD RRRRRRRR*/
			byte_1 = *in_ptr++;
			byte_2 = *in_ptr++;
			byte_3 = *in_ptr++;
 
			proc_len = byte_0 & 0x03;
			for (i = 0; i < proc_len; i++)
				*out_ptr++ = *in_ptr++;
 
			ref_ptr = out_ptr - ((byte_0 & 0x10) << 12)
				- (byte_1 << 8) - byte_2 - 1;
			ref_len = ((byte_0 & 0x0c) << 6) + byte_3 + 5;
			for (i = 0; i < ref_len; i++)
				*out_ptr++ = *ref_ptr++;
		} else {
			/* 1-byte command: 111PPPPP */
			proc_len = (byte_0 & 0x1f) * 4 + 4;
			if (proc_len <= 0x70) {
				/* no stop flag */
				for (i = 0; i < proc_len; i++)
					*out_ptr++ = *in_ptr++;
			} else {
				/* stop flag */
				proc_len = byte_0 & 0x3;
				for (i = 0; i < proc_len; i++)
					*out_ptr++ = *in_ptr++;
 
				break;
			}
		}
	}
 
done:
	if (bytes_read_out)
		*bytes_read_out = in_ptr - indata;
	return decompressed_size;
}

unsigned char *DecompressDBPF(unsigned char *data, int dataSize, int outDataSize)
{
    unsigned char *ret = malloc(outDataSize);
    unsigned char *initData = data;

    refpack_decompress_unsafe(data, NULL, ret);

    return ret;
}

static const char *GetExtensionFromType(unsigned int type)
{
    switch (type)
    {
        case PKGENTRY_PROP: return "prop";
        case PKGENTRY_TEXT:
        case PKGENTRY_SCPT: return "txt";
        case PKGENTRY_RULE: return "rules";
        case PKGENTRY_JSN8:
        case PKGENTRY_JSON: return "json";
        case PKGENTRY_RAST: return "rast";
        case PKGENTRY_RW4: return "rw4";
        case PKGENTRY_PNG: return "png";
        case PKGENTRY_MOV: return "mov";
        case PKGENTRY_EXIF: return "jpg";
        case PKGENTRY_SWB: return "swb";
        case PKGENTRY_BNK: return "bnk";
        case PKGENTRY_WEM: return "wem";
        case PKGENTRY_TTF: return "ttf";
        case PKGENTRY_ER2: return "er2";
        case PKGENTRY_GIF: return "gif";
        default: return "unkn";
    }
}

static bool writeCorrupted = true;

typedef struct DataCycleArgs {
    FILE *f;
    int i;
    IndexEntry *entries;
    Package *pkg;
    pthread_mutex_t *fmutex;
} DataCycleArgs;

static void datacycle(void *param)
{
    DataCycleArgs *args = param;
    IndexEntry *entries = args->entries;
    int i = args->i;
    FILE *f = args->f;
    Package pkg = *args->pkg;

    IndexEntry entry = entries[i];

    TRACELOG(LOG_DEBUG, "\nEntry %d:\n", i);

    printf("Locking fmutex.\n");
    pthread_mutex_lock(args->fmutex);

    if (fseek(f, entry.chunkOffset, SEEK_SET) == -1)
    {
        perror("Unexpected error occurred");
    }

    unsigned char *data = malloc(entry.diskSize);
    fread(data, 1, entry.diskSize, f);

    if (feof(f))
    {
        TRACELOG(LOG_ERROR, "Unexpected end of file.\n");
    }

    printf("Unlocking fmutex.\n");
    pthread_mutex_unlock(args->fmutex);

    if (entry.isCompressed)
    {
        pkg.entries[i].compressed = true;
        unsigned char *uncompressed = DecompressDBPF(data, entry.diskSize, entry.memSize);
        if (uncompressed)
        {
            int toPrint = 10;
            pkg.entries[i].dataRaw = uncompressed;
            pkg.entries[i].dataRawSize = entry.memSize;
            
            if (!ProcessPackageData(uncompressed, entry.memSize, entry.type, &(pkg.entries[i])))
            {
                if (writeCorrupted) ExportPackageEntry(pkg.entries[i], TextFormat("corrupted/%#X-%#X-%#X.%s", entry.type, entry.group, entry.instance, GetExtensionFromType(entry.type)));
                pkg.entries[i].corrupted = true;
            }

            for (int i = 0; i < toPrint; i++)
            {
                //TRACELOG(LOG_DEBUG, "%#x\n", uncompressed[i]);
            }
        }
        pkg.entries[i].dataCompressed = data;
        pkg.entries[i].dataCompressedSize = entry.diskSize;
    }
    else
    {
        int toPrint = 10;
        pkg.entries[i].dataRaw = data;
        pkg.entries[i].dataRawSize = entry.diskSize;

        if (!ProcessPackageData(data, entry.diskSize, entry.type, &(pkg.entries[i])))
        {
            if (writeCorrupted) ExportPackageEntry(pkg.entries[i], TextFormat("corrupted/%#X-%#X-%#X.%s", entry.type, entry.group, entry.instance, GetExtensionFromType(entry.type)));
            pkg.entries[i].corrupted = true;
        }

        for (int i = 0; i < toPrint; i++)
        {
            //TRACELOG(LOG_DEBUG, "%#x\n", data[i]);
        }
    }
}

Package LoadPackageFile(FILE *f)
{
    Package pkg = { 0 };
    PackageHeader header;
    fread(&header, sizeof(PackageHeader), 1, f);

    mkdir("corrupted");

    TRACELOG(LOG_DEBUG, "Header:\n");
    TRACELOG(LOG_DEBUG, "Magic: %.4s\n", header.magic);
    TRACELOG(LOG_DEBUG, "Major Version #: %d\n", header.majorVersion);
    TRACELOG(LOG_DEBUG, "Minor Version #: %d\n", header.minorVersion);
    TRACELOG(LOG_DEBUG, "Index Entry Count: %d\n", header.indexEntryCount);
    TRACELOG(LOG_DEBUG, "Index Size: %d\n", header.indexSize);
    TRACELOG(LOG_DEBUG, "Index Major Version: %d\n", header.indexMajorVersion);
    TRACELOG(LOG_DEBUG, "Index Minor Version: %d\n", header.indexMinorVersion);
    TRACELOG(LOG_DEBUG, "Index Offset: %d\n", header.indexOffset);

    fseek(f, header.indexOffset, SEEK_SET);
    Index index = { 0 };

    readuint(&index.indexType, f);

    TRACELOG(LOG_DEBUG, "\nIndex information:\n");
    TRACELOG(LOG_DEBUG, "Index Type: %d\n", index.indexType);

    IndexData indexData = index.data;

    //readuint(&indexData.null, f);

    uint32_t indexTypeId = 0xCAFEBABE;
    if ((index.indexType & (1 << 0)) == 1 << 0)
    {
        readuint(&indexTypeId, f);
    }

    uint32_t indexGroupContainer = 0xCAFEBABE;
    if ((index.indexType & (1 << 1)) == 1 << 1)
    {
        readuint(&indexGroupContainer, f);
    }

    uint32_t indexUnknown = 0xCAFEBABE;
    if ((index.indexType & (1 << 2)) == 1 << 2)
    {
        readuint(&indexUnknown, f);
    }

    pkg.entryCount = header.indexEntryCount;

    IndexEntry *entries = malloc(sizeof(IndexEntry) * header.indexEntryCount);

    pkg.entries = malloc(sizeof(PackageEntry) * pkg.entryCount);

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        IndexEntry entry = { 0 };
        PackageEntry pkgEntry = { 0 };

        if ((index.indexType & (1 << 0)) == 1 << 0)
        {
            entry.type = indexTypeId;
        }
        else
        {
            readuint(&entry.type, f);
        }

        if ((index.indexType & (1 << 1)) == 1 << 1)
        {
            entry.group = indexGroupContainer;
        }
        else
        {
            readuint(&entry.group, f);
        }

        if ((index.indexType & (1 << 2)) == 1 << 2)
        {
            uint32_t unk = indexUnknown;
        }
        else
        {
            uint32_t unk;
            readuint(&unk, f);
        }

        readuint(&entry.instance, f);

        readuint(&entry.chunkOffset, f);

        readuint(&entry.diskSize, f);
        entry.diskSize &= ~0x80000000;
        readuint(&entry.memSize, f);
        fread(&entry.compressed, sizeof(uint16_t), 1, f);
        fread(&entry.unknown, sizeof(uint16_t), 1, f);

        entry.isCompressed = (entry.compressed == 0xFFFF);
        if (entry.compressed != 0xFFFF && entry.compressed != 0x0000)
        {
            TRACELOG(LOG_ERROR, "Error: Invalid value for compressed: %#x\n", entry.compressed);
        }

        TRACELOG(LOG_DEBUG, "\nEntry %d:\n", i);
        TRACELOG(LOG_DEBUG, "Type: %#X\n", entry.type);
        TRACELOG(LOG_DEBUG, "Group: %#X\n", entry.group);
        TRACELOG(LOG_DEBUG, "Instance: %#X\n", entry.instance);
        TRACELOG(LOG_DEBUG, "Chunk Offset: %u\n", entry.chunkOffset);
        TRACELOG(LOG_DEBUG, "Disk Size: %u\n", entry.diskSize);
        TRACELOG(LOG_DEBUG, "Mem Size: %u\n", entry.memSize);
        TRACELOG(LOG_DEBUG, "Compressed? %s\n", entry.isCompressed?"yes":"no");

        pkgEntry.type = entry.type;
        pkgEntry.group = entry.group;
        pkgEntry.instance = entry.instance;

        entries[i] = entry;
        pkg.entries[i] = pkgEntry;
    }

    TRACELOG(LOG_DEBUG, "\nData Cycle.\n");

    InitThreadpool(-1);
    pthread_mutex_t fmutex;
    pthread_mutex_init(&fmutex, NULL);

    DataCycleArgs *argList = malloc(sizeof(DataCycleArgs) * header.indexEntryCount);

    for (int i = 0; i < header.indexEntryCount; i++)
    {
        DataCycleArgs args;
        args.f = f;
        args.i = i;
        args.pkg = &pkg;
        args.entries = entries;
        args.fmutex = &fmutex;
        argList[i] = args;
        NewThreadpoolTask(datacycle, &argList[i]);
    }

    WaitForThreadpoolTasksDone();
    CloseThreadpool();
    pthread_mutex_destroy(&fmutex);

    free(entries);

    return pkg;
}

void UnloadPackageFile(Package pkg)
{
    free(pkg.entries);
}

void ExportPackageEntry(PackageEntry entry, const char *filename)
{
    switch (entry.type)
    {
        case PKGENTRY_ER2:
        case PKGENTRY_HTML:
        case PKGENTRY_CSS:
        case PKGENTRY_JSN8:
        case PKGENTRY_SCPT: // Script file format (?)
        case PKGENTRY_TEXT: // "textual" file.
        case PKGENTRY_JSON: // JSON file.
        {
            SaveFileData(filename, entry.data.scriptSource, strlen(entry.data.scriptSource));
        } break;
        default: SaveFileData(filename, entry.dataRaw, entry.dataRawSize); break;
    }
}

static bool TextStartsWith(const char *t1, const char *startsWith)
{
    return strstr(t1, startsWith) == t1;
}

int *SearchPackage(Package pkg, PackageSearchParams params, int *nResults)
{
    int *results = NULL;

    *nResults = 0;

    for (int i = 0; i < pkg.entryCount; i++)
    {
        bool isCorrect = true;
        if (params.searchInstance)
        {
            if (!TextStartsWith(TextFormat("%#X", pkg.entries[i].instance), params.instance)) isCorrect = false;
        }
        if (params.searchGroup)
        {
            if (!TextStartsWith(TextFormat("%#X", pkg.entries[i].group), params.group)) isCorrect = false;
        }
        if (params.searchType)
        {
            if (!TextStartsWith(TextFormat("%#X", pkg.entries[i].type), params.type)) isCorrect = false;
        }
        if (!isCorrect) continue;

        (*nResults)++;
        results = realloc(results, sizeof(int) * (*nResults));
        results[(*nResults) - 1] = i;
    }

    return results;
}

void MergePackages(Package *dest, Package src)
{
    int startIndex = dest->entryCount;

    dest->entryCount += src.entryCount;
    dest->entries = realloc(dest->entries, dest->entryCount * sizeof(PackageEntry));

    memcpy(dest->entries + startIndex, src.entries, src.entryCount * sizeof(PackageEntry));
}

void SetWriteCorruptedPackageEntries(bool val)
{
    writeCorrupted = val;
}

void ExportPackage(Package pkg, const char *filename)
{
    FILE *f = fopen(filename, "wb");

    PackageHeader header = { .magic = "DBPF" };
    header.majorVersion = 3;
    header.minorVersion = 0;
    header.indexEntryCount = pkg.entryCount;
    header.indexMajorVersion = 0;
    header.indexMinorVersion = 3;

    IndexEntry *indexEntries = malloc(sizeof(IndexEntry) * pkg.entryCount);

    // TODO check if all entries have same group or type

    MemStream dataStream = { 0 };

    for (int i = 0; i < pkg.entryCount; i++)
    {
        indexEntries[i].type = pkg.entries[i].type;
        indexEntries[i].group = pkg.entries[i].group;
        indexEntries[i].instance = pkg.entries[i].instance;
        indexEntries[i].chunkOffset = dataStream.size + sizeof(PackageHeader);

        // TODO: compress data
        memstream_write(&dataStream, pkg.entries[i].dataRaw, pkg.entries[i].dataRawSize);
        indexEntries[i].diskSize = pkg.entries[i].dataRawSize; // TODO bitwise OR with 0x80000000?
        indexEntries[i].memSize = pkg.entries[i].dataRawSize;
        indexEntries[i].compressed = 0x0000;
        indexEntries[i].unknown = 0x0000;
    }

    header.indexOffset = dataStream.size + sizeof(PackageHeader);

    MemStream indexStream = { 0 };

    uint32_t indexType = 0;
    memstream_write(&indexStream, &indexType, sizeof(uint32_t));

    for (int i = 0; i < pkg.entryCount; i++)
    {
        memstream_write(&indexStream, &indexEntries[i].type, 4);
        memstream_write(&indexStream, &indexEntries[i].group, 4);
        uint32_t unk = 0;
        memstream_write(&indexStream, &unk, 4);
        memstream_write(&indexStream, &indexEntries[i].instance, 4);
        memstream_write(&indexStream, &indexEntries[i].chunkOffset, 4);
        memstream_write(&indexStream, &indexEntries[i].diskSize, 4);
        memstream_write(&indexStream, &indexEntries[i].memSize, 4);
        memstream_write(&indexStream, &indexEntries[i].compressed, 2);
        memstream_write(&indexStream, &indexEntries[i].unknown, 2);
    }

    header.indexSize = indexStream.size;

    fwrite(&header, sizeof(PackageHeader), 1, f);
    fwrite(dataStream.buf, 1, dataStream.size, f);
    fwrite(indexStream.buf, 1, indexStream.size, f);

}
