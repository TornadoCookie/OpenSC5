
# DBPF v3.0 specification

DBPF (DataBase Packed File) is a format used for storing game assets in every mainline maxis game developed since 2003.
It is used in:
- SimCity 4  (2003)
- The Sims 2 (2004)
- Spore      (2008)
- The Sims 3 (2009)
- Darkspore  (2011)
- SimCity 5  (2013)

This document specifies the DBPF format used in SimCity (version 3.0, index version 1.3.)

This file format was reverse engineered with the help of the following wiki pages:
- https://simswiki.info/wiki.php?title=Spore:DBPF
- https://simswiki.info/index.php?title=Sims_3:DBPF

as well as some experimentation on my part.

## Terminology
A DBPF file can be thought of as an archive, like a zip or tar file, meaning it is a file that represents a collection of files. However, unlike zip or tar, DBPF doesn't have "filenames." Instead, DBPF uses 3 32-bit big endian integers to differentiate files:
- **Type** - A code representing the file type of the file. Think of it as the file's extension.
- **Group** - An id to group files that are used in the same parts of the game together. Think of it as the name of the top level folder that the file is in.
- **Instance** - A unique id for the file. Usually the hash of the original filename. This would be the file name.

## File Header
```
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
```
This looks like a lot of fields, but the following fields seem to have no use:
- `dateCreated`, `dateModified`: Should really be unknown, taken from spore reference.
- `firstIndexEntryOffset`: ignore.
- `holeEntryCount`, `holeOffset`, `holeSize`: Also taken from spore reference.

Because we are dealing with packages for SimCity 5, we can make some assumptions:
- `majorVersion` will always be 3.
- `minorVersion` will always be 0.
- `indexMajorVersion` will always be 1.
- `indexMinorVersion` will always be 3.

## The Index
The index is the database of the database packed file. It can be found at the offset `indexOffset`. It is very frequently at the end of the file, which unfortunately prevents us from reading the entire database in one linear read.<br>
The format of the start of the index is as follows:
```
uint32_t indexType;
// indexType has some flags that make the index smaller in certain cases.
// The definitions of these flags are as follows:
// #define INDEX_ALLTYPESEQUAL  1
// #define INDEX_ALLGROUPSEQUAL 2
// #define INDEX_UNKNOWNFLAG    4
// Each of these flags specify whether to read an extra uint32 for a id, group, or unknown.

// if indexType & INDEX_ALLTYPESEQUAL, read indexTypeId.
// indexTypeId represents every file having the same type, this one.
uint32_t indexTypeId;

// if indexType & INDEX_ALLGROUPSEQUAL, read indexGroupContainer.
// indexGroupContainer represents every file having the same group, this one.
uint32_t indexGroupContainer

// if indexType & INDEX_UNKNOWNFLAG, read indexUnknown.
uint32_t indexUnknown.
```
I have chosen not to format this in a struct as it can't be read as a flat table because of the flags.

### Index Entries
After the above format comes the index entries. The format of an index entry is as follows:
```
// We have some values that depend on indexType from before.
// If we didn't read indexTypeId, read the type of the file.
uint32_t type;

// If we didn't read indexGroupContainer, read the group of the file.
uint32_t group;

// If we didn't read indexUnknown, read an unknown value.
uint32_t unknown; // we can just throw this away...
```
After these two members, we have the rest of the index entry data which can be represented as a flat table.
```
typedef struct IndexEntry {
    uint32_t instance;    // The instance of the file (see terminology)
    uint32_t chunkOffset; // The offset of the file's data
    uint32_t diskSize;    // The size of the file's data on disk (compressed size, may be logically ANDed with 0x80000000 for some reason)
    uint32_t memSize;     // The size of the file's data once uncompressed
    uint16_t compressionType; // 0x0000: Uncompressed, 0xFFFF: DBPF compression
    uint16_t unknown;
} IndexEntry;
```

## DBPF Compression
If you saw the above section, you probable read that a `compressionType` of 0xFFFF means "DBPF compression." This is a proprietary compression format used exclusively by this format. TODO: Document this?

## What do the codes mean
- Look [here](../../include/filetypes/package.h) for a list of type codes and their meanings.
- Look [here](../../Properties.txt) for a partial list of instance IDs taken from SimCity.