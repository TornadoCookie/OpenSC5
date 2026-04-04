#include "rlWebKit/rlWebKit.h"

#include "rlWebkitUtils.h"
#include "rlWebkitRenderer.h"
#include "rlWebkitThreading.h"
#include "rlWebkitClient.h"

#include <EABase/eabase.h>
#include <EAWebKit/EAWebKit>
#include <EAWebKit/EAWebKitDll.h>
#include <EAText/EAText.h>

#include "DBPFFileSystem.hpp"

//#include <DirtySDK/dirtysock/netconn.h>

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
#include <windows.h>
#include <bcrypt.h>
#elif defined(GLWEBKIT_PLATFORM_LINUX)
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#endif

#include <list>
#include <iostream>
#include "tracelog.h"
#include <raylib.h>

EA::WebKit::EAWebKitLib* wk = nullptr;

typedef EA::WebKit::EAWebKitLib* (*PF_CreateEAWebkitInstance)(void);

#if defined(GLWEBKIT_PLATFORM_WINDOWS)

// Callbacks
double timerCallback() 
{ 
    LARGE_INTEGER frequency;
    ::QueryPerformanceFrequency(&frequency);

LARGE_INTEGER start;
    ::QueryPerformanceCounter(&start);

    return static_cast<double>(start.QuadPart) / (double)frequency.QuadPart;
}

double monotonicTimerCallback() 
{
    return timerCallback();
};

void cryptographicallyRandomValueCallback(unsigned char *buffer, size_t length)
{
   //deprecated API, using newer api below
//     HCRYPTPROV hCryptProv = 0;
//     CryptAcquireContext(&hCryptProv, 0, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
//     CryptGenRandom(hCryptProv, length, buffer);
//     CryptReleaseContext(hCryptProv, 0);

    BCRYPT_ALG_HANDLE algorithm;
    NTSTATUS ret;
    ret = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RNG_ALGORITHM, 0, 0);
    if(ret != 0)
    {
       std::cout << "Failed to initialize cryto algorithm provider.  Return code: " << ret << std::endl;
       return;
    }

    ret = BCryptGenRandom(algorithm, buffer, length, 0);
    if(ret != 0)
    {
       std::cout << "Failed to get cryto random number.  Return code: " << ret << std::endl;
       return;
    }
    
    //return true;  // Returns true if no error, else false
}

void* stackBaseCallback() 
{
   //taken from: https://github.com/adobe/webkit/blob/master/Source/WTF/wtf/StackBounds.cpp#L228
   PNT_TIB64 pTib = reinterpret_cast<PNT_TIB64>(NtCurrentTeb());
   return reinterpret_cast<void*>(pTib->StackBase);
}

PF_CreateEAWebkitInstance get_CreateEAWebKitInstance()
{
#ifdef _DEBUG
   HMODULE wdll = LoadLibraryA("EAWebkitd.dll");
#else
   HMODULE wdll = LoadLibraryA("EAWebkit.dll");
#endif // _DEBUG
if(wdll != nullptr)
   {
      return reinterpret_cast<PF_CreateEAWebkitInstance>(GetProcAddress(wdll, "CreateEAWebkitInstance"));
   }

   return nullptr;
}

#elif defined(GLWEBKIT_PLATFORM_LINUX)

// Callbacks
double timerCallback()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    double t = (double)ts.tv_sec + ((double)ts.tv_nsec) / 1000000000;

    return t;
}

double monotonicTimerCallback()
{
    return timerCallback();
}

void cryptographicallyRandomValueCallback(unsigned char *buffer, size_t length)
{
    // use /dev/random because it is more secure
    FILE *f = fopen("/dev/random", "rb");

    if (!f) return;

    if (fread(buffer, 1, length, f) != length) return;

    fclose(f);

    //return true;
}

#include <pthread.h>

void *stackBaseCallback()
{
    void* stackBase = 0;
    size_t stackSize = 0;

    pthread_t thread = pthread_self();
    pthread_attr_t sattr;
    pthread_attr_init(&sattr);
//#if HAVE(PTHREAD_NP_H) || OS(NETBSD)
    // e.g. on FreeBSD 5.4, neundorf@kde.org
//    pthread_attr_get_np(thread, &sattr);
//#else
    // FIXME: this function is non-portable; other POSIX systems may have different np alternatives
    pthread_getattr_np(thread, &sattr);
//#endif
    int rc = pthread_attr_getstack(&sattr, &stackBase, &stackSize);
    (void)rc; // FIXME: Deal with error code somehow? Seems fatal.
    //ASSERT(stackBase);
    pthread_attr_destroy(&sattr);
    //m_bound = stackBase;
    return static_cast<char*>(stackBase) + stackSize; 
}

static EA::WebKit::EAWebKitLib *linux_CreateEAWebkitInstance()
{
    static EA::WebKit::EAWebKitLib concreteInstance;
    return &concreteInstance;
}

PF_CreateEAWebkitInstance get_CreateEAWebKitInstance()
{
    return linux_CreateEAWebkitInstance;
}

#endif

//void getCookiesCallback(const char16_t* pUrl, EA::WebKit::EASTLFixedString16Wrapper& result, uint32_t flags)
//{
//   std::cout << __FUNCTION__ << std::endl;
//}

//bool setCookieCallback(const EA::WebKit::CookieEx& cookie)
//{
//   std::cout << __FUNCTION__ << std::endl;
//   return false;  
//}

struct EA::WebKit::AppCallbacks callbacks = {
   timerCallback,
   stackBaseCallback,
   NULL, //atomicIncrementCallback
   NULL, //atomicDecrementCallback
   cryptographicallyRandomValueCallback, //cryptRandomValueCallback
};

static GLWebkitClient wkC;
static StdThreadSystem threadSystem;
static DBPFFileSystem fileSystem;

// init the systems: using DefaultAllocator, DefaultFileSystem, no text/font support, DefaultThreadSystem
struct EA::WebKit::AppSystems systems = {
    NULL,// mAllocator
    &fileSystem,// mFileSystem
    NULL,// mTextSystem
    &threadSystem,// mThreadSystem
    &wkC,// mEAWebkitClient
};

static char16_t *tochar16(const char *str)
{
    char16_t *ret = new char16_t[strlen(str)+1];

    for (int i = 0; i < strlen(str)+1; i++)
    {
        ret[i] = str[i];
    }

    return ret;
}

#include <codecvt>
#include <locale>
#include <sstream>

std::string char16_to_string(const char16_t* s16, size_t len)
{
    // https://stackoverflow.com/a/7235204/865719
    //((char16_t*)(s16))[len-1] = 0;
    if (len == 0)
    {
        for (; s16[len] != 0; len++) {}
        //std::cout << "subbed len for " << len << std::endl;
    }
    char16_t *clone = (char16_t*)malloc((len+1)*2);
    clone[len] = 0;
    memcpy(clone, s16, len*2);
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::string ret = convert.to_bytes(clone).substr(0, len);
    //std::cout << "converted: " << ret << std::endl;
    free(clone);

    return ret;
}

static const char8_t *EASTLFixedString8Wrapper_GetCharacters(EA::WebKit::EASTLFixedString8Wrapper str)
{
    return str.GetCharacters();//reinterpret_cast<const char8_t *>(str.GetImpl());
}

static const char16_t *EASTLFixedString16Wrapper_GetCharacters(EA::WebKit::EASTLFixedString16Wrapper str)
{
    return str.GetCharacters();//reinterpret_cast<const char16_t *>(str.GetImpl());
}

#include "json.hpp"
using json = nlohmann::json;

static void sendHeaders(EA::WebKit::TransportInfo *pTInfo, size_t dataSize, const char *mimeType)
{
    pTInfo->mpTransportServer->SetMimeType(pTInfo, mimeType);
    pTInfo->mpTransportServer->SetExpectedLength(pTInfo, dataSize);
    pTInfo->mResultCode = 200;
    //pTInfo->mpTransportServer->HeadersReceived(pTInfo);
}

static void transferSendData(EA::WebKit::TransportInfo *pTInfo, const char *data, size_t dataSize)
{
    pTInfo->mpTransportServer->DataReceived(pTInfo, data, dataSize);
    //pTInfo->mpTransportServer->DataDone(pTInfo, true);
}

class GameTransportHandler : public EA::WebKit::TransportHandler {
    
    enum RequestType {
        kRequestTypeNone,
        kRequestTypeFile,
        kRequestTypeGameEvent,
        kRequestTypeGameData,
    };

    enum RequestState {
        kRequestStateHeaders,
        kRequestStateData,
        kRequestStateDone
    };

    struct GameData {
        RequestType requestType;

        // kRequestTypeFile data
        EA::WebKit::FileSystem::FileObject mFileObject;
        void *mBuffer;
        int64_t mFileSize;

        RequestState mRequestState;
        std::string mResponse;

        // kRequestTypeGameEvent
        bool mSentOfflineNoUpdates;
    };
    
    const unsigned kFileBufferSize = 65536;

    bool InitJob(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
    {
        std::string sPath = char16_to_string(EASTLFixedString16Wrapper_GetCharacters(pTInfo->mURI), 0);
        const char *path = strdup(sPath.c_str() + strlen("game://")); // we strdup because sPath will be deleted once we leave InitJob
        pTInfo->mPath.SetCharacters(path);
        GameData *data = new GameData;

        data->mRequestState = kRequestStateHeaders;

        if (!strncmp(path, "/dbpf", 5))
        {
            data->requestType = kRequestTypeFile;
            EA::WebKit::FileSystem *pFS = EA::WebKit::GetFileSystem();
            data->mFileObject = pFS->CreateFileObject();
            data->mBuffer = malloc(kFileBufferSize);
            data->mFileSize = -1;
        }
        else if (!strncmp(path, "/gameevents", strlen("/gameevents")))
        {
            data->requestType = kRequestTypeGameEvent;
            data->mSentOfflineNoUpdates = false;
        }
        else if (!strncmp(path, "/gamedata", strlen("/gamedata")))
        {
            data->requestType = kRequestTypeGameData;
        }
        else
        {
            data->requestType = kRequestTypeNone;
            std::cout << "game:// TODO " << path << std::endl;
        }

        pTInfo->mTransportHandlerData = (uintptr_t)data;

        bStateComplete = true;
        return true;
    }

    bool Connect(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
    {
        GameData *data = (GameData*)pTInfo->mTransportHandlerData;

        if (data->requestType == kRequestTypeFile)
        {
            EA::WebKit::FileSystem *pFS = EA::WebKit::GetFileSystem();

            const char8_t *pathChars = EASTLFixedString8Wrapper_GetCharacters(pTInfo->mPath);
            pFS->OpenFile(data->mFileObject, pathChars+5, EA::WebKit::FileSystem::kRead);
            const char *ext = GetFileExtension(pathChars);
            const char *mimeType;
            if (!strcmp(ext, ".html"))
            {
                mimeType = "text/html";
            }
            else if (!strcmp(ext, ".css"))
            {
                mimeType = "text/css";
            }
            else if (!strcmp(ext, ".js"))
            {
                mimeType = "text/javascript";
            }
            else if (!strcmp(ext, ".json"))
            {
                mimeType = "application/json";
            }
            else if (!strcmp(ext, ".png"))
            {
                mimeType = "image/png";
            }
            else if (!strcmp(ext, ".gif"))
            {
                mimeType = "image/gif";
            }
            else if (!strcmp(ext, ".jpg"))
            {
                mimeType = "image/jpeg";
            }
            else
            {
                TRACELOG(LOG_WARNING, "GAME: No mime type for extension %s", ext);
                mimeType = NULL;
            }

            if (mimeType)
                pTInfo->mpTransportServer->SetMimeType(pTInfo, mimeType);
        }

        bStateComplete = true;
        return true;
    }

    bool Transfer(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
    {
        GameData *data = (GameData*)pTInfo->mTransportHandlerData;

        if (data->requestType == kRequestTypeFile)
        {
            EA::WebKit::FileSystem *pFS = EA::WebKit::GetFileSystem();

            if (data->mFileSize < 0)
            {
                data->mFileSize = pFS->GetFileSize(data->mFileObject);
                pTInfo->mpTransportServer->SetExpectedLength(pTInfo, data->mFileSize);
            }

            int64_t read = pFS->ReadFile(data->mFileObject, data->mBuffer, kFileBufferSize);

            if (read > 0)
            {
                pTInfo->mpTransportServer->DataReceived(pTInfo, data->mBuffer, read);
            }
            else
            {
                bStateComplete = true;
                pTInfo->mResultCode = 200;
                pTInfo->mpTransportServer->DataDone(pTInfo, true);
            }
        }
        else if (data->mRequestState == kRequestStateData)
        {
            std::cout << "data send " << data->mResponse << std::endl;
            transferSendData(pTInfo, data->mResponse.c_str(), data->mResponse.length());
            data->mRequestState = kRequestStateDone;
        }
        else if (data->mRequestState == kRequestStateDone)
        {
            std::cout << "data done" << std::endl;
            pTInfo->mpTransportServer->DataDone(pTInfo, true);
            bStateComplete = true;
        }
        else if (data->requestType == kRequestTypeGameEvent)
        {
            const char *method = EASTLFixedString8Wrapper_GetCharacters(pTInfo->mPath) + strlen("/gameevents/");

            if (!strcmp(method, "attach"))
            {
                std::cout << "TODO gameevents/attach: done" << std::endl;
                // this is a get request, so return constant data.
                const char *response = "{\"gameEventToken\": \"OpenSC5\"}";
                sendHeaders(pTInfo, sizeof(response), "application/json");
                data->mRequestState = kRequestStateData;
                data->mResponse = response;

                //bStateComplete = true;
            }
            else if (!strcmp(method, ""))
            {
                json response = json::array();
                int next = 0;

                //if (!data->mSentOfflineNoUpdates)
                {
                    json event;
                    event["eventID"] = (unsigned)278776844; // kGameMessageOfflineNoUpdateAvailable
                    response[next++] = event;
                    data->mSentOfflineNoUpdates = true;
                }

                json jResp;
                jResp["events"] = response;
                jResp["length"] = next;
                std::string resp = jResp.dump();
                sendHeaders(pTInfo, resp.length(), "application/json");
                data->mResponse = resp;
                data->mRequestState = kRequestStateData;
            }
            else
            {
                std::cout << "TODO gameevents/" << method << std::endl;
            }
        }
        else if (data->requestType == kRequestTypeGameData)
        {
            const char *method = EASTLFixedString8Wrapper_GetCharacters(pTInfo->mPath) + strlen("/gamedata/");

            if (!strcmp(method, "batch/"))
            {
                std::cout << "TODO gamedata/batch/ " << pTInfo->mPostSize << " " << pTInfo->mHttpRequestType << std::endl;

                char *pRequest = (char*)malloc(pTInfo->mPostSize+1);
                pTInfo->mpTransportServer->ReadData(pTInfo, pRequest, pTInfo->mPostSize);
                //std::string request(pRequest);
                pRequest[pTInfo->mPostSize] = 0;
                //std::cout << "gamedataRaw: " << pRequest << "\n" << std::endl;
                std::istringstream sRequest;
                sRequest.str(pRequest);

                json response = json::array();
                int next = 0;

                for (std::string req; std::getline(sRequest, req);)
                {
                    std::cout << "gamedata: " << req << std::endl;
                    if (req == "OnlineGameState")
                    {
                        response[next] = 1; // offline
                    }
                    
                    next++;
                }

                free(pRequest);

                json jResp;
                jResp["results"] = response;
                jResp["length"] = next;

                std::string resp = jResp.dump();

                std::cout << "response: " << resp << std::endl;

                sendHeaders(pTInfo, resp.length(), "application/json");
                data->mResponse = resp;
                data->mRequestState = kRequestStateData;

                //bStateComplete = true;

                //return true;
            }
            else
            {
                std::cout << "TODO gamedata/" << method << std::endl;
            }
        }

        return true;
    }

    bool Disconnect(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
    {
        GameData *data = (GameData*)pTInfo->mTransportHandlerData;

        if (data->requestType == kRequestTypeFile)
        {
            EA::WebKit::FileSystem *pFS = EA::WebKit::GetFileSystem();

            pFS->CloseFile(data->mFileObject);
        }

        bStateComplete = true;
        return true;
    }
    
    bool ShutdownJob(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
    {
        GameData *data = (GameData*)pTInfo->mTransportHandlerData;

        if (data->requestType == kRequestTypeFile)
        {
            EA::WebKit::FileSystem *pFS = EA::WebKit::GetFileSystem();
            pFS->DestroyFileObject(data->mFileObject);
            free(data->mBuffer);
            delete data;
        }

        bStateComplete = true;
        return true;
    }
};

extern "C" bool initWebkit()
{
   PF_CreateEAWebkitInstance create_Webkit_instance = get_CreateEAWebKitInstance();

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
   // init winsock manually, this is required
   WSADATA wsadata = {};
   WSAStartup(MAKEWORD(2, 0), &wsadata);

#endif

   if (!create_Webkit_instance)
   {
      std::cout << "EAWebkit.dll missing" << std::endl;
      return false;
   }

   wk = create_Webkit_instance();

   //check that dll is same version as our headers
   const char* verStr = wk->GetVersion();
   if (strcmp(verStr, EAWEBKIT_VERSION_S) != 0)
   {
      std::cout << "Error!  Mismatched versions of EA Webkit" << std::endl;
      return false;
   }

   EA::WebKit::GetAllocator(); // Ensure the allocator is initialized
   
   //initialize the system
   wk->Init(&callbacks, &systems);

   EA::WebKit::Parameters& params = wk->GetParameters();
   //params.mEAWebkitLogLevel = 4;
   //params.mHttpManagerLogLevel = 4;
   //params.mRemoteWebInspectorPort = 1234;
   params.mReportJSExceptionCallstacks = true;
   params.mVerifySSLCert = true;
   params.mJavaScriptStackSize = 1024 * 1024; //1MB of stack space
   params.mJavaScriptDebugOutputEnabled = true;
   
   wk->SetParameters(params);

   //wk->AddTransportHandler(wk->GetTransportHandler(EA_CHAR16("file")), tochar16(""));

   wk->AddTransportHandler(new GameTransportHandler, EA_CHAR16("game"));

   //NetConnStartup("-servicename=rlWebKit");

   // Symbol is not present in EAWebKit.dll.
#ifndef GLWEBKIT_PLATFORM_WINDOWS
   EA::Text::Init();
#endif

   //should be pulling these from the OS by their family type
   //times new roman is the default fallback if a font isn't found, so we need 
   //to at least load this (should probably be built in)
   int ret = add_ttf_font(wk, "times.ttf");
   if (ret == 0)
   {
      std::cout << "Error adding times.ttf font. " << std::endl;
   }

   return true;
}

// TODO what do we do about this?
class JSClient : public EA::WebKit::IJSBoundObject {

public:
    JSClient(EA::WebKit::View *pView)
        : mView(pView),
          mSentOfflineNoUpdateAvailable(false),
          mErroredNetworkServicesOffline(false),
          mSendAuthToken(false),
          mStartShardFlow(false)
    {}

    bool hasMethod(const char *name)
    {
        if (!strcmp(name, "RequestUpdaterData"))
        {
            return true;
        }
        else if (!strcmp(name, "hasOwnProperty"))
        {
            return true;
        }
        else if (!strcmp(name, "RequestGameData"))
        {
            return true;
        }
        else if (!strcmp(name, "DebugPrint") || !strcmp(name, "Assert"))
        {
            return true;
        }
        else if (!strcmp(name, "ProfBegin") || !strcmp(name, "ProfEnd") || !strcmp(name, "RequestGameEvents") || !strcmp(name, "PostGameCommand") || !strcmp(name, "Set3DMode") || !strcmp(name, "Set3DCoords") || !strcmp(name, "Set3DViewWorldSize") || !strcmp(name, "Set3DViewSize"))
        {
            return true;
        }
        
        std::cout << "Attempt to access scrui.gClient." << name << std::endl;
        return false;
    }

    bool invokeMethod(const char *name, EA::WebKit::JavascriptValue *args, unsigned argCount, EA::WebKit::JavascriptValue *resultOut)
    {
        //std::cout << "JSINVOKE scrui.gClient." << name << " " << argCount << " args" << std::endl;

        if (!strcmp(name, "RequestUpdaterData"))
        {
            unsigned int command = args[0].GetNumberValue();

            switch (command)
            {
                case 278506655: // ORIGIN_IS_ONLINE
                {
                    resultOut->SetBooleanValue(false); // is origin there?
                } break;
                case 285775037: // kHaveValidLineState
                {
                    resultOut->SetBooleanValue(true);
                } break;
                case 286464908: // kGameMessageUpdaterReady
                {
                    std::cout << "updater ready." << std::endl;
                    // This is the updater telling us something we don't need
                } break;
                case 244423987: // GET_SHOW_RETRY_BTN
                {
                    // This only pops up if the updater tells the engine to update, which we don't want

                    resultOut->SetBooleanValue(false);
                    //std::cout << "!!! The updater tried to tell the engine to update." << std::endl;
                    return true;
                } break;
                case 287607681: // UPDATE_IN_PROGRESS
                {
                    resultOut->SetBooleanValue(false);
                } break;
                case 252943297: // GET_UPDATER_STATE
                {
                    resultOut->SetNumberValue((int)0); // normal
                } break;
                case 254316777: // GET_RESTART_PENDING
                {
                    resultOut->SetBooleanValue(false);
                } break;
                case 243655923: // DOWNLOAD_FINSIHED (sic)
                {
                    resultOut->SetBooleanValue(true);
                } break;
                case 243610865: // DOWNLOAD_PROGRESS
                {
                    resultOut->SetNumberValue(0.5);
                } break;
                case 243527656: // STATUS_CODE
                {
                    // Entry in UpdateManagerErrorCodes.json
                    // Having a result puts the updater back into updating state.
                    //resultOut->SetStringValue(EA_CHAR16("0x80000001"));
                } break;
                case 255285637: // kGameMessageStartShardFlow
                {
                    mStartShardFlow = true;
                } break;
                default:
                {
                    std::cout << "scrui.gClient.RequestUpdaterData " << command << std::endl;
                } break;
            }
            return true;
        }
        else if (!strcmp(name, "hasOwnProperty"))
        {
            size_t len = 0;
            const char16_t *x = args[0].GetStringValue(&len);
            std::string has = char16_to_string(x, len);
            std::cout << "scrui.gClient.hasOwnProperty " << has << std::endl;

            const std::vector<std::string> meths = {
                "RequestGameData",
                "ProfBegin",
                "ProfEnd",
                "DebugPrint",
                "RequestGameEvents",
                "PostGameCommand",
                "Set3DMode",
                "Set3DCoords",
                "Set3DViewWorldSize",
                "Set3DViewSize"
            };

            for (int i = 0; i < meths.size(); i++)
            {
                if (has == meths[i])
                {
                    resultOut->SetBooleanValue(true);
                    return true;
                }
            }

            std::cout << "scrui.gClient." << has << " is expected to exist" << std::endl;

            return true;
        }
        else if (!strcmp(name, "RequestGameData"))
        {
            size_t len = 0;
            const char16_t *x = args[0].GetStringValue(&len);
            std::string req = char16_to_string(x, len);

            if (req == "origin/isLocaleEntitled")
            {
                resultOut->SetBooleanValue(true);
            }
            else if (req == "urlproperty/150330524") // kPropEcoNetRESTAPI
            {
                resultOut->SetStringValue(EA_CHAR16("http://simcity.elysiumorpheus.com"));
            }
            else if (req == "GetTutorialPlayed")
            {
                resultOut->SetBooleanValue(false);
            }
            else if (req == "OnlineGameState")
            {
                resultOut->SetNumberValue(1); // we are offline. 
            }
            else if (req == "appProperties/255207476") // mSCWorldConnect
            {
                resultOut->SetBooleanValue(true); // we are connected to the simcity worlds?
            }
            else if (req == "origin/IsFreeTrial")
            {
                resultOut->SetBooleanValue(false); // free trial
            }
            else if (req == "origin/FreeTrialTime")
            {
                resultOut->SetNumberValue(100);
            }
            else if (req == "origin/FreeTrialExpiryDate")
            {
                resultOut->SetNumberValue(10000000);
            }
            else if (req == "error")
            {
                EA::WebKit::JavascriptValue *errors = wk->CreateJavascriptValue(mView);
                errors->SetArrayType();

                if (!mErroredNetworkServicesOffline)
                {
                    EA::WebKit::JavascriptValue *error = wk->CreateJavascriptValue(mView);
                    mView->EvaluateJavaScript("{messageType = \"errordata\", code = 10010}", error);
                    errors->PushArrayValue(*error);
                    mErroredNetworkServicesOffline = true;
                }

                resultOut->SetArrayType();
                resultOut->PushArrayValue(*errors);
            }
            else if (req == "appproperties/231936915") // kPropEnableOriginLogin
            {
                resultOut->SetBooleanValue(true);
            }
            else if (req == "origin/3043198785") // isIntegration
            {
                resultOut->SetBooleanValue(false); // production
            }
            else if (req == "appproperties/242912640") // kShowStore
            {
                resultOut->SetBooleanValue(false);
            }
            else if (req == "appproperties/242821410") // kPropAdWebServer
            {
                resultOut->SetStringValue(EA_CHAR16("http://simcity.elysiumorpheus.com/ad"));
            }
            else if (req == "appproperties/4220525218") // kSkipIntroMovie
            {
                resultOut->SetBooleanValue(false);
            }
            else if (req == "appproperties/2002965982") // kBetaFlow
            {
                resultOut->SetBooleanValue(true);
            }
            else if (req == "appproperties/4263713262") // kPropOriginAddOnStore
            {
                // nothing
            }
            else if (req == "demo")
            {
                resultOut->SetBooleanValue(false);
            }
            else if (req == "hasEntitlement/2") // do we have cities of tomorrow expansion pack
            {
                resultOut->SetBooleanValue(false);
            }
            else if (req == "urlproperty/292003341") // kPropBuyNowLink
            {
                resultOut->SetStringValue(EA_CHAR16("http://simcity.elysiumorpheus.com/buynow"));
            }
            else if (req == "appproperties/292005258") // kPropPromoURL
            {
                // nothing
            }
            else if (req == "origin/authToken")
            {
                resultOut->SetStringValue(EA_CHAR16("DUMMY_AUTH_TOKEN"));
                mSendAuthToken = true;
            }
            else
            {
                std::cout << "scrui.gClient.RequestGameData " << req << std::endl;
            }

            return true;
        }
        else if (!strcmp(name, "PostGameCommand"))
        {
            size_t len = 0;
            const char16_t *x = args[0].GetStringValue(&len);
            std::string req = char16_to_string(x, len);

            std::cout << "scrui.gClient.PostGameCommand " << req << std::endl;

            if (req == "requestOnlineState")
            {
                resultOut->SetBooleanValue(false);
            }
            else
            {
                resultOut->SetBooleanValue(true);
            }

            return true;
        }
        else if (!strcmp(name, "Assert"))
        {
            std::cout << "args[0], args[1] = " << args[0].Type() << ", " << args[1].Type() << std::endl;

            bool assertVal = args[1].GetBooleanValue();
        
            if (!assertVal)
            {
                size_t len = 0;
                const char16_t *x = args[0].GetStringValue(&len);
                std::string str = char16_to_string(x, len);

                std::cout << "ASSERT FAILED: " << str << std::endl;
                return true;
            }
        }
        else if (!strcmp(name, "DebugPrint"))
        {
            size_t len = 0;
            const char16_t *x = args[0].GetStringValue(&len);
            std::string str = char16_to_string(x, len);

            std::cout << "scrui.DebugPrint: " << str << std::endl;

            return true;
        }
        else if (!strcmp(name, "RequestGameEvents"))
        {
            resultOut->SetObjectType();
            int length = 0;

            EA::WebKit::JavascriptValue *events = wk->CreateJavascriptValue(mView);
            events->SetArrayType();
            /*if (!mSentOfflineNoUpdateAvailable)
            {
                length++;
                EA::WebKit::JavascriptValue *event = wk->CreateJavascriptValue(mView);

                mView->EvaluateJavaScript("{eventID = 278776844}", event); // kGameMessageOfflineNoUpdateAvailable
                mSentOfflineNoUpdateAvailable = true;

                events->PushArrayValue(*event);
            }*/

            if (mSendAuthToken)
            {
                length++;
                EA::WebKit::JavascriptValue *event = wk->CreateJavascriptValue(mView);
                mView->EvaluateJavaScript("{eventID = 231431032, eventData = {authToken = \"DUMMY_AUTH_TOKEN\"}}", event);
                events->PushArrayValue(*event);
                mSendAuthToken = false;
            }

            if (mStartShardFlow)
            {
                length++;
                EA::WebKit::JavascriptValue *event = wk->CreateJavascriptValue(mView);
                mView->EvaluateJavaScript(R"(JSON.parse('{ "eventID": 3438476797, "eventData": { "config": { "hosts": [ { "Desc": "Antarctica", "name": "0x0f3b425d", "url": "p21.api.awsprod.simcity.com", "game": "p21.api.awsprod.simcity.com", "websocket": "p21.api.awsprod.simcity.com:2001", "telemetry": "http://telemetry.simcity.com", "news": "p21.api.awsprod.simcity.com", "statuses": [ { "status": "available" } ], "id": 999143101, "sort": 4 }, { "Desc": "Antarctica2", "name": "0x0f3b4256", "url": "p21.api.awsprod.simcity.com", "game": "p21.api.awsprod.simcity.com", "websocket": "p21.api.awsprod.simcity.com:2001", "telemetry": "http://telemetry.simcity.com", "news": "p21.api.awsprod.simcity.com", "statuses": [ { "status": "available" } ], "id": 999143101, "sort": 4 } ] } } }'))", event);
                events->PushArrayValue(*event);
                mStartShardFlow = false;
            }

            EA::WebKit::JavascriptValue *vLength = wk->CreateJavascriptValue(mView);
            vLength->SetNumberValue(length);

            resultOut->SetProperty(EA_CHAR16("events"), *events);
            resultOut->SetProperty(EA_CHAR16("length"), *vLength);

            return true;
        }
        else if (!strcmp(name, "ProfBegin"))
        {
            size_t len = 0;
            const char16_t *x = args[0].GetStringValue(&len);
            std::string has = char16_to_string(x, len);
            //std::cout << "scrui.gClient.ProfBegin " << has << std::endl;

            return true;
        }
        else if (!strcmp(name, "ProfEnd"))
        {
            return true;
        }

        std::cout << "Attempt to invoke scrui.gClient." << name << std::endl;
        return false;
    }

    bool hasProperty(const char *propName)
    {
        if (hasMethod(propName)) return false;
        std::cout << "gClient has prop " << propName << "?" << std::endl;
        return false;
    }

    private:
    EA::WebKit::View *mView;

    bool mSentOfflineNoUpdateAvailable;
    bool mErroredNetworkServicesOffline;
    bool mSendAuthToken;
    bool mStartShardFlow;
};

static std::vector<EA::WebKit::View *> gViews;

EA::WebKit::View* createView(int x, int y)
{
   EA::WebKit::View* v = 0;

   v = wk->CreateView();
   EA::WebKit::ViewParameters vp;
   vp.mHardwareRenderer = nullptr; // use default renderer
   vp.mDisplaySurface = nullptr; // use default surface
   vp.mWidth = x;
   vp.mHeight = y;
   vp.mBackgroundColor = 0xffffffff; //clear  0xffffffff; //white
   vp.mTileSize = 256;
   vp.mUseTiledBackingStore = false;
   vp.mpUserData = v;
   v->InitView(vp);
   v->SetSize(EA::WebKit::IntSize(vp.mWidth, vp.mHeight));

   //v->ShowInspector(true);
   v->SetDrawDebugVisuals(true);

   JSClient *cl = new JSClient(v);
   v->BindJavaScriptObject("Client", cl);

   gViews.push_back(v);

   return v;
}

bool shutdownWebKit()
{
   wk->Shutdown();

   return true;
}

void updateWebkit(EA::WebKit::View *v)
{
   if (!v) 
       return;
    
    for (int i = 0; i < gViews.size(); i++)
    {
        if (gViews[i] != v)
        {
            gViews[i]->Tick();
        }
    }

   //v->EvaluateJavaScript("console.log(\"tick\");");
   //v->EvaluateJavaScript("if (scrui) {scrui.DEBUG = !0; scrui.ALLOW_EDITOR = !0; scrui.gUIManager.mRequestManager.mUseGameEventQueue = !0;} console.log(\"tick \" + scrui + scrui.DEBUG);"); // set no debug in scrui
   //v->EvaluateJavaScript("if (scrui && !window.didDebugEnable) {window.didDebugEnable = true; scrui.DEBUG = !0; window.ClientHooks.ShowDebugConsole(!0); } "); 
   v->EvaluateJavaScript("window.ClientHooks.Update(10);");
   v->Tick();
    
}

void setViewUrl(EA::WebKit::View* v, const char* url)
{
   v->SetURI(url);
}

void updateView(EA::WebKit::View* v)
{
    //v->Paint();
    //v->ShowInspector(true);
}

void resize(EA::WebKit::View* v, int width, int height)
{
    if (!v) 
       return;

    v->SetSize(EA::WebKit::IntSize(width, height));
}

void mousemove(EA::WebKit::View* v, int x, int y)
{
    if (!v) 
       return;

    EA::WebKit::MouseMoveEvent e = {};
    e.mX = x;
    e.mY = y;
    v->OnMouseMoveEvent(e);
}

void mousebutton(EA::WebKit::View* v, int x, int y, int btn, bool depressed)
{
    if (!v) 
       return;

    EA::WebKit::MouseButtonEvent e = {};
    e.mId = btn;
    e.mX = x;
    e.mY = y;
    e.mbDepressed = depressed;
    v->OnMouseButtonEvent(e);
}

void mousewheel(EA::WebKit::View* v, int x, int y, int keys, int delta)
{
    if (!v) 
       return;

    EA::WebKit::MouseWheelEvent e = {};
    e.mX = x;
    e.mY = y;
    e.mZDelta = delta;

#if defined(GLWEBKIT_PLATFORM_WINDOWS)
    UINT scrollLines = 1;
    SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);
    e.mNumLines = ((delta * (int32_t)scrollLines) /*/ (int32_t)WHEEL_DELTA*/);
#elif defined(GLWEBKIT_PLATFORM_LINUX)
    e.mNumLines = delta;
#endif

    v->OnMouseWheelEvent(e);
}

void keyboard(EA::WebKit::View* v, int id, bool ischar, bool depressed)
{
    if (!v) 
       return;

    EA::WebKit::KeyboardEvent e = {};
    e.mId = id;
    e.mbChar = ischar;
    e.mbDepressed = depressed;
    v->OnKeyboardEvent(e);
}

void reload(EA::WebKit::View* v)
{
    if (!v)
       return;

    v->Refresh();
}

void destroyView(EA::WebKit::View* v)
{
   if(!wk || !v)
      return;

   wk->DestroyView(v);
}
