#ifndef _RWK_DBPF_FS_
#define _RWK_DBPF_FS_

#include "filetypes/package.h"

#ifdef __cplusplus

#include <EAWebKit/EAWebKit>

class DBPFFileSystem : public EA::WebKit::FileSystem {
    
    typedef EA::WebKit::FileSystem::FileObject FileObject;
    typedef EA::WebKit::utf8_t utf8_t;

    FileObject CreateFileObject();
    void DestroyFileObject(FileObject fileObject);
    bool OpenFile(FileObject fileObject, const utf8_t *path, int openFlags);
    FileObject OpenTempFile(const utf8_t *prefix, utf8_t *pDestPath);
    void CloseFile(FileObject fileObject);
    int64_t ReadFile(FileObject fileObject, void *buffer, int64_t size);
    bool WriteFile(FileObject fileObject, const void *buffer, int64_t size);
    int64_t GetFileSize(FileObject fileObject);
    int64_t GetFilePosition(FileObject fileObject);

    bool FileExists(const utf8_t *path);
    bool DirectoryExists(const utf8_t *path);
    bool RemoveFile(const utf8_t *path);
    bool DeleteDirectory(const utf8_t *path);
    bool GetFileSize(const utf8_t *path, int64_t &size);
    bool GetFileModificationTime(const utf8_t *path, time_t &result);
    bool MakeDirectory(const utf8_t *path);
    bool GetDataDirectory(utf8_t *path, size_t pathBufferCapacity);
};

extern "C" {
#endif

void SetWebKitPackage(Package *pkg);

#ifdef __cplusplus
}
#endif

#endif

