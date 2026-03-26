// The OpenSC5 updater.
// TornadoCookie 2026

#include <curl/curl.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
//#include <archive.h> HOLD UNTIL 3.7.8 is in debian trixie
//#include <archive_entry.h>
#include <fcntl.h>

#define PATH_SIZE 128
#define strlitsize(s) (sizeof(s)-1)

// need: mkdir, mkstemps
#ifdef __linux__

#include <sys/stat.h>

#define mkdir(x) mkdir(x, 0777)

FILE *openTempFile(char *template, int suffixLen)
{
    int fd = mkstemps(template, suffixLen);
    if (fd == -1)
    {
        perror("failed to open tmp file");
        exit(EXIT_FAILURE);
    }
    return fdopen(fd, "wb+");
}

#elif defined _WIN32

FILE *openTempFile(char *template, int suffixLen)
{
    for (int i = 0; i < strlen(template); i++)
    {
        if (template[i] == 'X')
            template[i] = rand() % 10 + '0';
    }
    return fopen(template, "wb+");
}

#else
#error no backend for platform.
#endif

typedef struct UpdaterOptions {
    const char *server;
    bool showHelp;
    unsigned long timestamp;
    const char *installPath;
    const char *locale;
} UpdaterOptions;

UpdaterOptions getOptions(int argc, char **argv)
{
    UpdaterOptions options = {
        .server = "http://update.prod.simcity.com",
        .showHelp = false,
        .timestamp = 0,
        .installPath = NULL,
        .locale = "ENUS"
    };

    struct option long_options[] = {
        {"server", required_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {"timestamp", required_argument, 0, 't'},
        {"install", required_argument, 0, 'i'},
        {"locale", required_argument, 0, 0},
        {0,0,0,0}
    };

    while (1)
    {
        int option_index = 0;
        int c = getopt_long(argc, argv, "ht:i:", long_options, &option_index);

        if (c == -1)
            break;
        
        switch (c)
        {
            case 0:
            {
                if (option_index == 0) options.server = strdup(optarg);
                else if (option_index == 1) options.showHelp = true;
                else if (option_index == 2) options.timestamp = atoi(optarg);
                else if (option_index == 3) options.installPath = strdup(optarg);
                else if (option_index == 4) options.locale = strdup(optarg);
            } break;
            case 'h':
            {
                options.showHelp = true;
            } break;
            case 't':
            {
                options.timestamp = atoi(optarg);
            } break;
            case 'i':
            {
                options.installPath = strdup(optarg);
            } break;
            default:
            {
                options.showHelp = true;
            } break;
        }
    }
    
    return options;
}

struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

struct MemoryStruct downloadFileToMemory(const char *url)
{
    CURL *curl = curl_easy_init();
    struct MemoryStruct chunk = {
        .memory = NULL,
        .size = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Simcity-Game/10.3.4.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode code = curl_easy_perform(curl);
    
    if (code != CURLE_OK)
    {
        printf("error: %s: %s\n", url, curl_easy_strerror(code));
        exit(EXIT_FAILURE);
    }

    curl_easy_cleanup(curl);

    return chunk;
}

char *downloadFileToTmpFile(const char *url, const char *name, FILE **of)
{
    char template[PATH_SIZE];

    sprintf(template, "update/%sXXXXXX.7z", name);

    FILE *f = openTempFile(template, 3);
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Simcity-Game/10.3.4.0");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK)
    {
        printf("Failed: %s: %s\n", url, curl_easy_strerror(code));
        exit(EXIT_FAILURE);
    }

    curl_easy_cleanup(curl);

    *of = f;
    return strdup(template);
}

typedef struct Package {
    unsigned long timestamp;
    char *name; // on heap
} Package;

const char *getPackageURL(Package pkg, const char *serverURL)
{
    static char out[PATH_SIZE];
    snprintf(out, PATH_SIZE, "%s/%s_%u.7z", serverURL, pkg.name, pkg.timestamp);
    return out;
}

const char *getPackageCRCURL(Package pkg, const char *serverURL)
{
    static char out[PATH_SIZE];
    snprintf(out, PATH_SIZE, "%s/%sCRC_%u.bin", serverURL, pkg.name, pkg.timestamp);
    return out;
}

const char *getPackageValidateURL(Package pkg, const char *serverURL)
{
    static char out[PATH_SIZE];
    snprintf(out, PATH_SIZE, "%s/%sValidate_%u.bin", serverURL, pkg.name, pkg.timestamp);
    return out;
}

Package getPackageFromFileName(const char *fileName)
{
    Package out;
    out.name = malloc(strlen(fileName));
    sscanf(fileName, "%m[^_]_%u.7z", &out.name, &out.timestamp);
    out.name = realloc(out.name, strlen(out.name)+1);
    return out;
}

typedef struct PackageList {
    int packageCount;
    Package *packages;
} PackageList;

void AddPackageToList(PackageList *list, Package pkg)
{
    list->packages = realloc(list->packages, sizeof(Package)*(++list->packageCount));
    list->packages[list->packageCount - 1] = pkg;
}

PackageList parsePackageManifest(const char *data, size_t length)
{
    PackageList manifest = {0, 0};

    const char *cursor = data;
    while ((cursor - data) < length)
    {
        cursor = strchr(cursor, '\"')+1;
        if ((cursor - data) > length) break;
        char *end = strchr(cursor, '\"');
        int length = end - cursor;
        char *packagefilename = malloc(length+1);
        memcpy(packagefilename, cursor, length);
        packagefilename[length] = 0;
        //printf("%s\n", packagefilename);

        
        if (!strstr(packagefilename, ".package") // .packages are EcoGame scripts which are downloaded later
            && !strstr(packagefilename, "iPatch") // iPatch files are for updating an installation which already exists
            && !strstr(packagefilename, ".html") // manifest files
            && !strstr(packagefilename, ".bin") // manifest files
            && !strstr(packagefilename, ".json") // some json file
            && !strstr(packagefilename, ".txt") // health.txt
        )
        {
            Package pkg = getPackageFromFileName(packagefilename);
            AddPackageToList(&manifest, pkg);
        }

        cursor += strlitsize("\">");
        cursor += length;
    }

    return manifest;
}

PackageList getPackagesForTimestamp(PackageList manifest, unsigned long timestamp)
{
    PackageList packages = {0, 0};

    for (int i = 0; i < manifest.packageCount; i++)
    {
        bool pkgInList = false;

        for (int j = 0; j < packages.packageCount; j++)
        {
            if (!strcmp(packages.packages[j].name, manifest.packages[i].name)
                && (packages.packages[j].timestamp < manifest.packages[i].timestamp)
                && (!timestamp ? 1 : manifest.packages[i].timestamp <= timestamp))
            {
                pkgInList = true;
                packages.packages[j] = manifest.packages[i];
            }
        }

        if (!pkgInList)
        {
            AddPackageToList(&packages, manifest.packages[i]);
        }
    }

    return packages;
}

PackageList getDownloadQueue(PackageList manifest, const char *locale, unsigned long timestamp)
{
    PackageList atTime = getPackagesForTimestamp(manifest, timestamp);
    PackageList downloadQueue = {0, 0};

    for (int i = 0; i < atTime.packageCount; i++)
    {
        char pkgLocale[PATH_SIZE] = {0};
        int isLocale = sscanf(atTime.packages[i].name, "SimCityLocale%s", &pkgLocale);

        if (!isLocale || !strncmp(pkgLocale, locale, 4))
        {
            AddPackageToList(&downloadQueue, atTime.packages[i]);
        }
    }

    free(atTime.packages);

    return downloadQueue;
}

typedef struct PackageInstallQueue {
    struct {
        Package pkg;
        char *filename;
        FILE *f;
    } *downloads;
    int downloadCount;
} PackageInstallQueue;

PackageInstallQueue downloadPackages(PackageList downloadQueue, const char *serverURL)
{
    PackageInstallQueue installQueue = {0, 0};

    for (int i = 0; i < downloadQueue.packageCount; i++)
    {
        printf("downloading %s\n", downloadQueue.packages[i].name);

        installQueue.downloads = realloc(installQueue.downloads, sizeof(*installQueue.downloads)*(++installQueue.downloadCount));
        installQueue.downloads[installQueue.downloadCount-1].pkg = downloadQueue.packages[i];
        
        const char *url = getPackageURL(downloadQueue.packages[i], serverURL);
        installQueue.downloads[installQueue.downloadCount - 1].filename = downloadFileToTmpFile(url, downloadQueue.packages[i].name, &(installQueue.downloads[installQueue.downloadCount - 1].f));
    }

    return installQueue;
}

void extract7z(const char *filename, const char *path)
{
    printf("TODO extract 7z\n");
    /*struct archive *archive = archive_read_new();
    struct archive_entry *entry;
    int result = 0;

    archive_read_support_format_7zip(archive);

    if (result = archive_read_open_filename(archive, filename, 4096))
    {
        printf("archive error: %d / %s / %d\n", result, archive_error_string(archive), archive_errno(archive));
        exit(EXIT_FAILURE);
    }

    while ((result = archive_read_next_header(archive, &entry)) == ARCHIVE_OK)
    {
        const char *pathname = archive_entry_pathname(entry);
        char path[PATH_SIZE];

        sprintf(path, "%s/%s", path, pathname);

        printf("extracting %s\n", path);

        int fd = open(path, O_WRONLY);
        archive_read_data_into_fd(archive, fd);
        close(fd);
    }

    if (result != ARCHIVE_EOF)
    {
        printf("archive error: %d / %s / %d\n", result, archive_error_string(archive), archive_errno(archive));
        exit(EXIT_FAILURE);
    }

    archive_read_close(archive);*/
}

void installPackages(PackageInstallQueue installQueue, const char *installPath)
{
    static const char *installPaths[][2] = {
        {"SimCityData", "SimCityData"},
        {"SimCityDataEP1", "SimCityData"},
        {"SimCityLocale", "SimCityData"},
        {"SimCityGreyMarket", "SimCity"},
        {"SimCityMacGreyMarket", "SimCityMAC"},
        {NULL, NULL},
    };

    mkdir(installPath);
    for (int i = 0; installPaths[i][1] != NULL; i++)
    {
        char path[PATH_SIZE];
        sprintf(path, "%s/%s", installPath, installPaths[i][1]);
        mkdir(path);
    }

    for (int i = 0; i < installQueue.downloadCount; i++)
    {
        char path[PATH_SIZE];
        const char *installDir;

        for (int j = 0; installPaths[j][0] != NULL; j++)
        {
            if (!strncmp(installQueue.downloads[i].pkg.name, installPaths[j][0], strlen(installPaths[j][0])))
                installDir = installPaths[j][1];
        }

        sprintf(path, "%s/%s", installPath, installDir);

        printf("extracting %s to %s\n", installQueue.downloads[i].filename, path);
        extract7z(installQueue.downloads[i].filename, path);

        unlink(installQueue.downloads[i].filename);
    }
}

void copyArchives(PackageInstallQueue installQueue)
{
    printf("copying archives...\n");
    for (int i = 0; i < installQueue.downloadCount; i++)
    {
        char filename[PATH_SIZE];
        sprintf(filename, "update/%s.7z", installQueue.downloads[i].pkg.name);
        rename(installQueue.downloads[i].filename, filename);
    }
}

int main(int argc, char **argv)
{
    printf("OpenSC5 Updater v2.0.0\n");

    UpdaterOptions options = getOptions(argc, argv);

    if (options.showHelp)
    {
        printf("Help:\n");
        printf("--server <url>: Use this URL for the update server.\n"
               "    Default: http://update.prod.simcity.com\n\n");
        printf("-t | --timestamp <ts>: Set the timestamp at which to download the archives.\n"
               "    Given in seconds since the Spore Epoch. \n\n"); // TODO when is that?
        printf("-i | --install <path>: Unpack the archives and install to this directory. [broken]\n\n");
        printf("--locale <locale>: set the locale. Default: ENUS\n\n");
        printf("-h | --help: show this help message\n");
        return 0;
    }

    curl_global_init(CURL_GLOBAL_ALL);

    char manifestURL[PATH_SIZE];
    sprintf(manifestURL, "%s/game_scripts_manifest.html", options.server);

    struct MemoryStruct manifestData = downloadFileToMemory(manifestURL);
    PackageList manifest = parsePackageManifest(manifestData.memory, manifestData.size);
    printf("Manifest contains %d packages.\n", manifest.packageCount);

    mkdir("update");

    PackageList downloadQueue = getDownloadQueue(manifest, options.locale, options.timestamp);
    printf("There are %d packages to download.\n", downloadQueue.packageCount);

    PackageInstallQueue installQueue = downloadPackages(downloadQueue, options.server);

    //if (options.installPath)
    //{
    //    installPackages(installQueue, options.installPath);
    //}
    //else
    {
        copyArchives(installQueue);
    }

    curl_global_cleanup();

    return 0;
}
