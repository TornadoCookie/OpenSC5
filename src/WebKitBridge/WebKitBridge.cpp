
// OpenSC5 EA WebKit bridge
// There are many functions missing from the SimCity EAWebKit.
// However, this method proves to be easier than making our own win32 port of EAWebKit.
// Here, USE_WEBKIT_BRIDGE includes the definitions for the missing functions.

#if USE_WEBKIT_BRIDGE == 1
#include "eastl/fixed_pool.cpp"
#include "eastl/red_black_tree.cpp"

#include "eaio/EAFileStream.cpp"

#include "FileSystemDefault.cpp" // EA:WebKit::FileSystemDefault
#include "EAWebKitSTLWrapper.cpp" // EA::WebKit::EASTLFixedString*Wrapper
#include "EAWebKitAllocator.cpp" // EA:WebKit::GetAllocator
#include "EAWebKitNewDelete.cpp"

// Our own stuff

namespace eastl {
    EASTL_API EmptyString gEmptyString;
}

namespace EA {

    namespace WebKit {


        FileSystem *spFileSystem = NULL;

        FileSystem *GetFileSystem()
        {
            if (!spFileSystem)
            {
                static FileSystemDefault defaultFileSystem;
                spFileSystem = &defaultFileSystem;
            }

            return spFileSystem;
        }

    }
}

#endif
