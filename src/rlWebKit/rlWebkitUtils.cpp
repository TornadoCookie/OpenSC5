#include "rlWebKit/rlWebKit.h"

#include "rlWebkitUtils.h"

//#include "glad.h"
//#include <GL/glew.h>

#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebkitAllocator.h>
#include <EAWebKit/EAWebKitFileSystem.h>
#include <EAWebKit/EAWebKitClient.h>
#include <EAWebKit/EAWebKitView.h>
#include "EAWebKit/EAWebKitTextInterface.h"

#include <stdio.h>

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
#include <windows.h> // LoadLibraryA
#elif defined(GLWEBKIT_PLATFORM_LINUX)
#include <dirent.h>
#define index _index // avoid name collision with deprecated POSIX func
#endif

#include <assert.h>
#include <array>

#include <vector>

#include <iostream>
#include <algorithm>

#include <raylib.h>

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
int getSystemFonts(std::vector<std::string>& fonts) 
{
    static const LPWSTR fontRegistryPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY hKey;
    LONG result;

    // Open Windows font registry key
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        return 1;
    }

    DWORD maxValueNameSize = 0, maxValueDataSize = 0;
    result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
    if (result != ERROR_SUCCESS) {
        return 1;
    }

    DWORD valueIndex = 0;
    LPSTR valueName = new CHAR[maxValueNameSize];
    LPBYTE valueData = new BYTE[maxValueDataSize];
    DWORD valueNameSize, valueDataSize, valueType;


    // Build full font file path
    char winDir_[MAX_PATH] = "";
    GetWindowsDirectoryA(winDir_, MAX_PATH);
    std::string winDir = std::string(winDir_);
    fonts.clear();

    do {
        valueDataSize = maxValueDataSize;
        valueNameSize = maxValueNameSize;

        result = RegEnumValueA(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

        valueIndex++;

        if (result != ERROR_SUCCESS || valueType != REG_SZ) {
            continue;
        }

        std::string wsValueName(valueName, valueNameSize);
        std::string wsValueData((LPSTR)valueData, valueDataSize-1); //remove trailing \0 in data

        std::string ext = wsValueData.substr(wsValueData.length() - 3);
        std::transform(ext.begin(), ext.end(),ext.begin(), ::tolower);

        if(ext != std::string("ttf") && ext != std::string("ttc") )
           continue;

        if(wsValueData.substr(0, 2) == std::string("C:"))
           continue;

        std::string fontPath = winDir + "\\Fonts\\" + wsValueData;
        fonts.push_back(fontPath);

    } while (result != ERROR_NO_MORE_ITEMS);

    delete[] valueName;
    delete[] valueData;

    RegCloseKey(hKey);
    return 0;
}

#elif defined(GLWEBKIT_PLATFORM_LINUX)

int getSystemFonts(std::vector<std::string>& fonts) 
{
    struct dirent *dp;
    DIR *fontDir = opendir("/usr/share/fonts/truetype");

    if (!fontDir)
    {
        perror("/usr/share/fonts/truetype");
        return 1;
    }

    while ((dp = readdir(fontDir)) != NULL)
    {
        DIR *familyDir = opendir(dp->d_name);
        struct dirent *fdp;

        if (!familyDir) { perror("warning"); continue;}

        while ((fdp = readdir(familyDir)) != NULL)
        {
            std::cout << fdp->d_name << '\n';
            fonts.push_back(fdp->d_name);
        }

        closedir(familyDir);
    }

    closedir(fontDir);

    return 0;
}

#endif

#include <EAIO/EAFileStream.h>

int add_ttf_font(EA::WebKit::EAWebKitLib* wk, const char* ttfFile) 
{
    EA::WebKit::ITextSystem* ts = wk->GetTextSystem();

    // WTF is this hack??? Otherwise it crashes.
    void *block = malloc(1024);
    EA::IO::FileStream *fs = new (block) EA::IO::FileStream(ttfFile);
    fs->Open();

    //Text system will take ownership of this memory
    int numFaces = ts->AddFace(fs);

    return 1;
}

int init_system_fonts(EA::WebKit::EAWebKitLib* wk) 
{
    std::vector<std::string> fonts;
    if (getSystemFonts(fonts)) 
    {
        return 1;
    }
    int fonts_installed = 0;
    for (int i = 0; i < fonts.size(); ++i) 
    {
        add_ttf_font(wk, fonts[i].c_str());
    }
    return 0;
}

unsigned int vPbo[2] = { 0, 0 };
unsigned char* buffer[2] = { 0, 0 };
int index = 0;
int nextIndex = 0;

void updateGLTexture(EA::WebKit::View* v, Texture2D tex)
{
   if (!v)
   {
      return;
   }

   //int w, h;
   EA::WebKit::ISurface* surface = v->GetDisplaySurface();
   //surface->GetContentDimensions(&w, &h);
   //int dataSize = w * h * 4;

   EA::WebKit::ISurface::SurfaceDescriptor sd = {};
   surface->Lock(&sd);
  
   UpdateTexture(tex, sd.mData);
   
   surface->Unlock();   
}

bool evaluateJavaScript(EA::WebKit::View* v, const char* src, EA::WebKit::JavascriptValue* result)
{
   return v->EvaluateJavaScript(src, result);
}

void bindJavascriptObject(EA::WebKit::View* v, const char* name, EA::WebKit::IJSBoundObject* obj)
{
   v->BindJavaScriptObject(name, obj);
}
