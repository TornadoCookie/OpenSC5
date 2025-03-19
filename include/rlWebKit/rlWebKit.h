
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


namespace EA
{
   namespace WebKit
   {
      class View;
      class JavascriptValue;
      class IJSBoundObject;
   }
}




#ifdef __cplusplus
extern "C" {
#endif

//WebKitLib API
   GLWEBKIT_API bool initWebkit();
   GLWEBKIT_API EA::WebKit::View* createView(int x, int y);
   GLWEBKIT_API void destroyView(EA::WebKit::View* v);
   GLWEBKIT_API void updateWebkit(EA::WebKit::View *v);
   GLWEBKIT_API bool shutdownWebKit();



//View API
   GLWEBKIT_API void setViewUrl(EA::WebKit::View* v, const char* url);
   GLWEBKIT_API void updateView(EA::WebKit::View* v);
   GLWEBKIT_API void resize(EA::WebKit::View* v, int width, int height);
   GLWEBKIT_API void mousemove(EA::WebKit::View* v, int x, int y);
   GLWEBKIT_API void mousebutton(EA::WebKit::View* v, int x, int y, int btn, bool depressed);
   GLWEBKIT_API void mousewheel(EA::WebKit::View* v, int x, int y, int keys, int delta);
   GLWEBKIT_API void keyboard(EA::WebKit::View* v, int id, bool ischar, bool depressed);
   GLWEBKIT_API void reload(EA::WebKit::View* v);
   GLWEBKIT_API void updateGLTexture(EA::WebKit::View* v, unsigned int id);
   GLWEBKIT_API bool evaluateJavaScript(EA::WebKit::View* v, const char* src, EA::WebKit::JavascriptValue* result  = 0);
   GLWEBKIT_API void bindJavascriptObject(EA::WebKit::View* v, const char* name, EA::WebKit::IJSBoundObject* obj);

//Javascript API


#ifdef __cplusplus
}
#endif
