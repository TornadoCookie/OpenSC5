
#pragma once

#pragma once
#ifdef _WIN32
#  ifdef GL_WEBKIT_EXPORTS
#     define GLWEBKIT_API __declspec ( dllexport )
#  else
#     define GLWEBKIT_API __declspec ( dllimport )
#  endif
#else
#  define GLWEBKIT_API
#endif


#ifdef __cplusplus
extern "C" {
#endif

//WebKitLib API
   GLWEBKIT_API bool initWebkit();
   GLWEBKIT_API void* createView(int x, int y);
   GLWEBKIT_API void destroyView(void* v);
   GLWEBKIT_API void updateWebkit(void *v);
   GLWEBKIT_API bool shutdownWebKit();



//View API
   GLWEBKIT_API void setViewUrl(void* v, const char* url);
   GLWEBKIT_API void updateView(void* v);
   GLWEBKIT_API void resize(void* v, int width, int height);
   GLWEBKIT_API void mousemove(void* v, int x, int y);
   GLWEBKIT_API void mousebutton(void* v, int x, int y, int btn, bool depressed);
   GLWEBKIT_API void mousewheel(void* v, int x, int y, int keys, int delta);
   GLWEBKIT_API void keyboard(void* v, int id, bool ischar, bool depressed);
   GLWEBKIT_API void reload(void* v);
   GLWEBKIT_API void updateGLTexture(void* v, unsigned int id);

//Javascript API


#ifdef __cplusplus
}
#endif
