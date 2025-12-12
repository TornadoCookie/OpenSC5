#include "DBPFFileSystem.hpp"
#include "filetypes/package.h"
#include "tracelog.h"
#include <raylib.h>
#include "hash.h"

static Package *PKG;

extern "C" void SetWebKitPackage(Package *newPKG)
{
    PKG = newPKG;
}

struct IDTriad {
    unsigned int type;
    unsigned int group;
    unsigned int instance;
};

IDTriad GetIDTriadFromPath(const EA::WebKit::utf8_t *path)
{
// so we can use raylib functions that I love so very much
    const char *cpath = static_cast<const char *>(path);

    const char *extension = GetFileExtension(cpath) + 1;
    const char *name = GetFileNameWithoutExt(cpath);

    unsigned int type = TheHash(extension);
    unsigned int instance = TheHash(name);

    TRACELOG(LOG_WARNING, "GetIDTriadFromPath: %s -> %X-0-%X\n", path, type, instance);

    return {type, 0, instance};
}

struct DBPFFileObject {
    unsigned int index;
    unsigned int offset;
};

DBPFFileObject *getfobj(EA::WebKit::FileSystem::FileObject fileObject)
{
    return (DBPFFileObject*)fileObject;
}

DBPFFileSystem::FileObject DBPFFileSystem::CreateFileObject()
{
    return (FileObject)(new DBPFFileObject);
}

void DBPFFileSystem::DestroyFileObject(DBPFFileSystem::FileObject fileObject)
{
    delete (DBPFFileObject*)fileObject;
}

bool DBPFFileSystem::OpenFile(DBPFFileSystem::FileObject fileObject, const utf8_t *path, int openFlags)
{
    if (openFlags & kWrite)
    {
        TRACELOG(LOG_ERROR, "DBPFFS: Tried to open with write flag: %s.\n", path);
        return false;
    }

    TRACELOG(LOG_WARNING, "DBPFFS OPEN %s\n", path);

    DBPFFileObject *fobj = getfobj(fileObject);

    if (strlen(path) == 0)
    {
        fobj->offset = 0;
        return false;
    }

    IDTriad triad = GetIDTriadFromPath(path);

    fobj->index = -1;

    for (int i = 0; i < PKG->entryCount; i++)
    {
        if (PKG->entries[i].instance == triad.instance && PKG->entries[i].type == triad.type)
        {
            fobj->index = i;
        }
    }

    if (fobj->index == -1)
    {
        TRACELOG(LOG_ERROR, "DBPFFS: Does not exist: %s\n", path);
        return false;
    }

    fobj->offset = 0;

    return true;
}

DBPFFileSystem::FileObject DBPFFileSystem::OpenTempFile(const utf8_t *prefix, utf8_t *pDestPath)
{
    TRACELOG(LOG_ERROR, "DBPFFS OpenTempFile");

    return EA::WebKit::FileSystem::kFileObjectInvalid;
}

void DBPFFileSystem::CloseFile(DBPFFileSystem::FileObject fileObject)
{
    // Nothing to do here.
}

// Assuming we return bytes read.
int64_t DBPFFileSystem::ReadFile(DBPFFileSystem::FileObject fileObject, void *buffer, int64_t size)
{
    DBPFFileObject *fobj = getfobj(fileObject);

    int64_t bytesRead = size;

    PackageEntry entry = PKG->entries[fobj->index];

    if (fobj->offset + size > entry.dataRawSize)
    {
        bytesRead = entry.dataRawSize - fobj->offset;
    }

    memcpy(buffer, entry.dataRaw, bytesRead);

    fobj->offset += bytesRead;

    return bytesRead;
}

bool DBPFFileSystem::WriteFile(DBPFFileSystem::FileObject fileObject, const void *buffer, int64_t size)
{
    TRACELOG(LOG_ERROR, "DBPFFS: Read-Only file system");

    return false;
}

int64_t DBPFFileSystem::GetFileSize(DBPFFileSystem::FileObject fileObject)
{
    DBPFFileObject *fobj = getfobj(fileObject);
    PackageEntry entry = PKG->entries[fobj->index];

    return entry.dataRawSize;
}

int64_t DBPFFileSystem::GetFilePosition(DBPFFileSystem::FileObject fileObject)
{
    DBPFFileObject *fobj = getfobj(fileObject);

    return fobj->offset;
}

bool DBPFFileSystem::FileExists(const utf8_t *path)
{
    IDTriad triad = GetIDTriadFromPath(path);

    for (int i = 0; i < PKG->entryCount; i++)
    {
        if (PKG->entries[i].instance == triad.instance && PKG->entries[i].type == triad.type)
        {
            return true;
        }
    }

    return false;
}

bool DBPFFileSystem::DirectoryExists(const utf8_t *path)
{
    TRACELOG(LOG_ERROR, "DirectoryExists: No directories in DBPF\n");
    return false;
}

bool DBPFFileSystem::RemoveFile(const utf8_t *path)
{
    TRACELOG(LOG_ERROR, "RemoveFile: Read-Only DBPF FS\n");
    return false;
}

bool DBPFFileSystem::DeleteDirectory(const utf8_t *path)
{
    TRACELOG(LOG_ERROR, "DeleteDirectory: just... no.");
    return false;
}

bool DBPFFileSystem::GetFileSize(const utf8_t *path, int64_t &size)
{
    IDTriad triad = GetIDTriadFromPath(path);

    for (int i = 0; i < PKG->entryCount; i++)
    {
        if (PKG->entries[i].instance == triad.instance && PKG->entries[i].type == triad.type)
        {
            size = PKG->entries[i].dataRawSize;
            return true;
        }
    }

    return false;
}

bool DBPFFileSystem::GetFileModificationTime(const utf8_t *path, time_t &result)
{
    TRACELOG(LOG_ERROR, "DBPF GetFileModificationTime");
    result = 0;
    return false;
}

bool DBPFFileSystem::MakeDirectory(const utf8_t *path)
{
    TRACELOG(LOG_ERROR, "DeleteDirectory: just... no.");
    return false;
}

bool DBPFFileSystem::GetDataDirectory(utf8_t *path, size_t pathBufferCapacity)
{
    TRACELOG(LOG_ERROR, "FIXME GetDataDirectory");
    return false;
}

