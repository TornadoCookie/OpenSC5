#pragma once

#if defined(_WIN32)
    #define GLWEBKIT_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define GLWEBKIT_PLATFORM_LINUX // also assumes glibc and x11
#else
    #error "Unknown Platform: you will have issues"
#endif

#include <string>
#include <vector>
#include <EAWebKit/EAWebKit.h>

extern unsigned int frame;

#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebKitView.h>

#include <string>

int getSystemFonts(std::vector<std::string>& fonts);
int add_ttf_font(EA::WebKit::EAWebKitLib* wk, const char* ttfFile);
int init_system_fonts(EA::WebKit::EAWebKitLib* wk);

