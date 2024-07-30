#include <cpl_raylib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>

#ifdef __linux__
#define mkdir(x) mkdir(x, 0777)
#endif

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

#define strlitsize(s) (sizeof(s)-1)

void download_to_file(const char *url, FILE *f)
{
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Simcity-Game/10.3.4.0");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK)
    {
        printf("Failed: %s\n", curl_easy_strerror(code));
    }

    curl_easy_cleanup(curl);
}

static void add_transfer(CURLM *cm, unsigned int i, int *left, const char *packagefilename)
{
    CURL *eh = curl_easy_init();
    printf("Downloading %s...\n", packagefilename);
    FILE *f = fopen(TextFormat("update/%s", packagefilename), "w+");
    curl_easy_setopt(eh, CURLOPT_URL, TextFormat("http://update.prod.simcity.com/%s", packagefilename));
    curl_easy_setopt(eh, CURLOPT_PRIVATE, f);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, f);
    curl_multi_add_handle(cm, eh);
    (*left)++;
}

#define MAX_PARALLEL 10

int main()
{
    struct MemoryStruct chunk;
 
    chunk.memory = malloc(1);  /* grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, "http://update.prod.simcity.com/game_scripts_manifest.html");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Simcity-Game/10.3.4.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode code = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    mkdir("update");

    if (code != CURLE_OK)
    {
        printf("Failed: %s\n", curl_easy_strerror(code));
    }
    else
    {
        printf("game_scripts_manifest.html: Retrieved %lu bytes.\n", chunk.size);
        
        char **packages = NULL;
        int packageCount;

        char *cursor = chunk.memory;
        while ((cursor - chunk.memory) < chunk.size)
        {
            cursor = strchr(cursor, '\"')+1;
            if ((cursor - chunk.memory) > chunk.size) break;
            char *end = strchr(cursor, '\"');
            int length = end - cursor;
            char *packagefilename = malloc(length+1);
            memcpy(packagefilename, cursor, length);
            packagefilename[length] = 0;
            printf("%s\n", packagefilename);

            packageCount++;
            packages = realloc(packages, packageCount * sizeof(char*));
            packages[packageCount - 1] = packagefilename;

            cursor += strlitsize("\">");
            cursor += length;
        }

        CURLM *curlm = curl_multi_init();

        printf("There are %d packages to download\n", packageCount);
        printf("Download in parallel, %d scripts at a time\n", MAX_PARALLEL);

        curl_multi_setopt(curlm, CURLMOPT_MAXCONNECTS, MAX_PARALLEL);
        int transfers = 0;
        int left = 0;

        int pkgOff = 0;

        for (transfers = 0; transfers < packageCount && transfers < MAX_PARALLEL; transfers++)
        {
            char *packagefilename = packages[transfers + pkgOff];
            if (transfers + pkgOff >= packageCount)
            {
                printf("Finished.\n");
                break;
            }
            if (FileExists(TextFormat("update/%s", packagefilename)))
            {
                printf("%s already downloaded.\n", packagefilename);
                pkgOff++;
                transfers--;
                continue;
            }
            else
            {
                add_transfer(curlm, transfers, &left, packagefilename);
            }
        }

        do
        {
            int stillAlive = 1;
            int msgs_left = -1;
            curl_multi_perform(curlm, &stillAlive);
            CURLMsg *msg;

            while ((msg = curl_multi_info_read(curlm, &msgs_left)) != NULL)
            {
                if (msg->msg == CURLMSG_DONE)
                {
                    CURL *e = msg->easy_handle;
                    FILE *f;
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &f);
                    fclose(f);
                    curl_multi_remove_handle(curlm, e);
                    curl_easy_cleanup(e);
                    left--;
                }
                else
                {
                    printf("Error: CURLMsg %d\n", msg->msg);
                }
                if (transfers < packageCount)
                {
                    char *packagefilename = packages[transfers + pkgOff];
                    if (transfers + pkgOff >= packageCount)
                    {
                        printf("Finished.\n");
                        break;
                    }
                    while (FileExists(TextFormat("update/%s", packagefilename)))
                    {
                        printf("%s already downloaded.\n", packagefilename);
                        pkgOff++;
                        if (transfers + pkgOff >= packageCount)
                        {
                            printf("Finished.\n");
                            break;
                        }
                        packagefilename = packages[transfers + pkgOff];
                    }
                    add_transfer(curlm, transfers++, &left, packagefilename);
                }
            }
            if (left)
            {
                curl_multi_wait(curlm, NULL, 0, 1000, NULL);
            }
        } while (left);

        curl_multi_cleanup(curlm);
    }

    curl_global_cleanup();
    return 0;
}
