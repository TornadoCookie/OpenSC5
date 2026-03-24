#include "ClientFileSystem.hpp"
#include "tracelog.h"
#include <raylib.h>
#include <string>

struct ClientFileObject {
    const ClientFileSystem::utf8_t *mPath;
    std::string mInput;
    std::string mOutput;
};

ClientFileSystem::FileObject ClientFileSystem::CreateFileObject()
{
    return (FileObject)(new ClientFileObject);
}

void ClientFileSystem::DestroyFileObject(ClientFileSystem::FileObject fileObject)
{
    delete (ClientFileObject*)fileObject;
}

static ClientFileObject *getfobj(ClientFileSystem::FileObject fileObject)
{
    return (ClientFileObject*)fileObject;
}

bool ClientFileSystem::OpenFile(ClientFileSystem::FileObject fileObject, const utf8_t *path, int openFlags)
{
    ClientFileObject *fobj = getfobj(fileObject);

    TRACELOG(LOG_WARNING, "CLIENT OPEN %s\n", path);

    fobj->mPath = path;

    return true;
}

int64_t ClientFileSystem::ReadFile(ClientFileSystem::FileObject fileObject, void *buffer, int64_t size)
{
    TRACELOG(LOG_WARNING, "CLIENT READ\n");

    return 0;
}

bool ClientFileSystem::WriteFile(ClientFileSystem::FileObject fileObject, const void *buffer, int64_t size)
{
    TRACELOG(LOG_WARNING, "CLIENT WRITE\n");

    return 0;
}

ClientFileSystem::FileObject ClientFileSystem::OpenTempFile(const utf8_t *prefix, utf8_t *pDestPath)
{
    TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
    return EA::WebKit::FileSystem::kFileObjectInvalid;
}
    
void ClientFileSystem::CloseFile(ClientFileSystem::FileObject fileObject)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
}

int64_t ClientFileSystem::GetFileSize(ClientFileSystem::FileObject fileObject)
{
    ClientFileObject *fobj = getfobj(fileObject);
	TRACELOG(LOG_WARNING, "CLIENT GET SIZE %s\n", fobj->mPath);
	return 1024;
}

bool ClientFileSystem::GetFileSize(const utf8_t *path, int64_t &size)
{
    TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
    return false;
}
    
int64_t ClientFileSystem::GetFilePosition(ClientFileSystem::FileObject fileObject)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return 0;
}

bool ClientFileSystem::FileExists(const utf8_t *path)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}
    
bool ClientFileSystem::DirectoryExists(const utf8_t *path)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}
    
bool ClientFileSystem::RemoveFile(const utf8_t *path)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}
    
bool ClientFileSystem::DeleteDirectory(const utf8_t *path)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}
    
bool ClientFileSystem::GetFileModificationTime(const utf8_t *path, time_t &result)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}
    
bool ClientFileSystem::MakeDirectory(const utf8_t *path)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}    
    
bool ClientFileSystem::GetDataDirectory(utf8_t *path, size_t pathBufferCapacity)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}

bool ClientFileSystem::GetTempDirectory(utf8_t *path, size_t pathBufferCapacity)
{
	TRACELOG(LOG_WARNING, "CLIENT UNSUPPORTED ACTION");
	return false;
}

