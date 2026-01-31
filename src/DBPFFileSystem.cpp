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

static const char *tempPath = "_ewk/tmp";

static bool isTempPath(const char *path)
{
    return !strncmp(path, tempPath, strlen(tempPath));
}

IDTriad GetIDTriadFromPath(const EA::WebKit::utf8_t *path)
{
// so we can use raylib functions that I love so very much
    const char *cpath = static_cast<const char *>(path);

    if (GetFileExtension(cpath) == NULL)
    {
        TRACELOG(LOG_WARNING, "%s has no extension. This is no file!\n", path);
        return {0, 0, 0};
    }

    const char *extension = GetFileExtension(cpath) + 1;
    const char *name = GetFileNameWithoutExt(cpath);

    unsigned int type = TheHash(extension);
    unsigned int instance = TheHash(name);
    
    if (!strcmp(extension, "json"))
    {
        type = 0xA98EAF0;
    }
    else if (!strcmp(extension, "png"))
    {
        type = 0x2F7D0004;
    }
    else if (!strcmp(extension, "gif"))
    {
        type = 0x2F7D0007;
    }

    //TRACELOG(LOG_WARNING, "GetIDTriadFromPath: %s -> %X-0-%X\n", path, type, instance);

    return {type, 0, instance};
}

struct DBPFFileObject {
    unsigned int index;
    unsigned int offset;

    bool special;
    const char *path;

    bool tempFile;
    EA::WebKit::FileSystem::FileObject tempFObj;
};

DBPFFileObject *getfobj(EA::WebKit::FileSystem::FileObject fileObject)
{
    return (DBPFFileObject*)fileObject;
}

DBPFFileSystem::FileObject DBPFFileSystem::CreateFileObject()
{
    if (!mDefaultFS)
    {
        mDefaultFS = new EA::WebKit::FileSystemDefault;
    }
    return (FileObject)(new DBPFFileObject);
}

void DBPFFileSystem::DestroyFileObject(DBPFFileSystem::FileObject fileObject)
{
    delete (DBPFFileObject*)fileObject;
}

bool DBPFFileSystem::OpenFile(DBPFFileSystem::FileObject fileObject, const utf8_t *path, int openFlags)
{
    TRACELOG(LOG_WARNING, "DBPFFS OPEN %s\n", path);

    DBPFFileObject *fobj = getfobj(fileObject);
    fobj->special = false;
    fobj->tempFile = false;

    if (strlen(path) == 0)
    {
        fobj->offset = 0;
        return false;
    }

    if (!strcmp(path, "/gameevents/attach")
            || !strcmp(path, "/gamedata/batch/"))
    {
        TRACELOG(LOG_WARNING, "DBPFFS: special");
        fobj->special = true;
        fobj->path = strdup(path);
        return true;
    }

    if (isTempPath(path))
    {
        fobj->tempFile = true;
        fobj->tempFObj = mDefaultFS->CreateFileObject();
        return mDefaultFS->OpenFile(fobj->tempFObj, path, openFlags);
    }

    if (openFlags & kWrite)
    {
        TRACELOG(LOG_ERROR, "DBPFFS: Tried to open with write flag: %s.\n", path);
        return false;
    }

    IDTriad triad = GetIDTriadFromPath(path);

    fobj->index = -1;

    for (int i = 0; i < PKG->entryCount; i++)
    {
        if (PKG->entries[i].instance == triad.instance && PKG->entries[i].type == triad.type)
        {
            fobj->index = i;
            //TRACELOG(LOG_WARNING, "DBPFFS: Triad found %X-%X-%X\n", triad.instance, PKG->entries[i].group, triad.type);
            //break;
        }
    }

    if (fobj->index == -1)
    {
        TRACELOG(LOG_ERROR, "DBPFFS: Does not exist: %s\n", path);
        return false;
    }

    SaveFileData(TextFormat("_ewk%s", static_cast<const char *>(path)), PKG->entries[fobj->index].dataRaw, PKG->entries[fobj->index].dataRawSize);

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

    if (fobj->special)
    {
        if (!strcmp(fobj->path, "/gameevents/attach"))
        {
            const char *resp = "{\"gameEventToken\": \"OpenSC5\"}";
            memcpy(buffer, resp, strlen(resp)+1);
            return strlen(resp) + 1;
        }
        else
        {
            TRACELOG(LOG_ERROR, "DBPFFS: no way to read from %s", fobj->path);
            return 0;
        }
    }

    int64_t bytesRead = size;

    PackageEntry entry = PKG->entries[fobj->index];

    if (fobj->offset + size > entry.dataRawSize)
    {
        bytesRead = entry.dataRawSize - fobj->offset;
    }

    memcpy(buffer, entry.dataRaw + fobj->offset, bytesRead);
    
    //TRACELOG(LOG_WARNING, "DBPFFS: %d: read %d asked for %d, off %d (bytes)\n", fileObject, bytesRead, size, fobj->offset);
    
    fobj->offset += bytesRead;

    return bytesRead;
}

bool DBPFFileSystem::WriteFile(DBPFFileSystem::FileObject fileObject, const void *buffer, int64_t size)
{
    DBPFFileObject *fobj = getfobj(fileObject);

    if (fobj->tempFile) return mDefaultFS->WriteFile(fobj->tempFObj, buffer, size);

    TRACELOG(LOG_ERROR, "DBPFFS: Read-Only file system");

    return false;
}

int64_t DBPFFileSystem::GetFileSize(DBPFFileSystem::FileObject fileObject)
{
    DBPFFileObject *fobj = getfobj(fileObject);
    
    if (fobj->special)
    {
        TRACELOG(LOG_WARNING, "DBPFFS: get size of %s", fobj->path);
        return 1024; // TODO
    }

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
    if (isTempPath(path)) return mDefaultFS->DirectoryExists(path);
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
    if (isTempPath(path)) return mDefaultFS->DeleteDirectory(path);
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

bool DBPFFileSystem::GetTempDirectory(utf8_t *path, size_t pathBufferCapacity)
{
    strcpy(path, tempPath);
    return true;
}

