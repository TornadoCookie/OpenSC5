#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>

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

    if (code != CURLE_OK)
    {
        printf("Failed: %s\n", curl_easy_strerror(code));
    }
    else
    {
        printf("Retrieved %lu bytes.\n", chunk.size);
        

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

            const char *path = TextFormat("update/%s", packagefilename);

            if (!FileExists(path))
            {
                FILE *f = fopen(path, "w+");
                download_to_file(TextFormat("http://update.prod.simcity.com/%s", packagefilename), f);
                fclose(f);
            }

            cursor += strlitsize("\">");
            cursor += length;
        }
    }

    curl_easy_cleanup(curl);

    curl_global_cleanup();
    return 0;
}
