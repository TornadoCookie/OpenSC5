#include "SPClient.hpp"
#include "tracelog.h"
#include <raylib.h>

#include "char16helpers.h"
#include <EAText/internal/StdC.h>

#define STARTSWITH(x,y) !memcmp((void*)x, (void*)y, sizeof(y))
static bool shouldForwardToFileTransport(const char16_t *uri)
{
    if (STARTSWITH(uri, u"file:///layoutbatch"))
    {
        return false;
    }

    if (STARTSWITH(uri, u"file:///gameevents"))
    {
        return false;
    }

    if (STARTSWITH(uri, u"file:///gamecommand"))
    {
        return false;
    }

    if (STARTSWITH(uri, u"file:///gamedata"))
    {
        return false;
    }

    return true;
}

#define FFIN(f) if (shouldForwardToFileTransport(pTInfo->mURI.GetCharacters())) { \
    return EA::WebKit::GetTransportHandler(EA_CHAR16("file"))->f(pTInfo, bStateComplete); \
}

bool ClientHandler::InitJob(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
{
    TRACELOG(LOG_WARNING, "CLIENT: init job %s", char16tochar8(pTInfo->mURI.GetCharacters()));

    pTInfo->mPath = strdup(char16tochar8(pTInfo->mURI.GetCharacters())+7);

    FFIN(InitJob);
    
    bStateComplete = false;
    return false;
}

bool ClientHandler::Connect(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
{
    TRACELOG(LOG_WARNING, "CLIENT: connect %s", char16tochar8(pTInfo->mURI.GetCharacters()));

    FFIN(Connect);
    
    bStateComplete = false;
    return false;
}


bool ClientHandler::Transfer(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
{
    TRACELOG(LOG_WARNING, "CLIENT: transfer %s", char16tochar8(pTInfo->mURI.GetCharacters()));

    FFIN(Transfer);
    
    bStateComplete = false;
    return false;
}


bool ClientHandler::Disconnect(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
{
    TRACELOG(LOG_WARNING, "CLIENT: disconnect %s", char16tochar8(pTInfo->mURI.GetCharacters()));

    FFIN(Disconnect);
    
    bStateComplete = false;
    return false;
}


bool ClientHandler::ShutdownJob(EA::WebKit::TransportInfo *pTInfo, bool &bStateComplete)
{
    TRACELOG(LOG_WARNING, "CLIENT: shutdown job %s", char16tochar8(pTInfo->mURI.GetCharacters()));

    FFIN(ShutdownJob);

    bStateComplete = false;
    return false;
}

