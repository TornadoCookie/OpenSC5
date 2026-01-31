#ifndef _CLIENTFS_
#define _CLIENTFS_

#include <EAWebKit/EAWebKit>

class ClientFileSystem : public EA::WebKit::FileSystem {

public:
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

    bool GetTempDirectory(utf8_t *path, size_t pathBufferCapacity);

};

#endif

