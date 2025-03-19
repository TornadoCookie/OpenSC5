#include "wwriff.h"

#define __RPCNDR_H__ // Doesn't like wwriff's definition of byte
  typedef unsigned char boolean;

#ifndef _HYPER_DEFINED
#define _HYPER_DEFINED
#define hyper /* __MINGW_EXTENSION */ __int64
#define MIDL_uhyper /* __MINGW_EXTENSION */ unsigned __int64
#endif

#define MIDL_INTERFACE(x) struct
#ifdef __cplusplus
#define EXTERN_GUID(itf,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8) EXTERN_C const IID DECLSPEC_SELECTANY itf = {l1,s1,s2,{c1,c2,c3,c4,c5,c6,c7,c8}}
#else
#define EXTERN_GUID(itf,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8) const IID DECLSPEC_SELECTANY itf = {l1,s1,s2,{c1,c2,c3,c4,c5,c6,c7,c8}}
#endif

#include <cpl_raylib.h>
#include <sstream>

static void ConvWWRiffToOGG(std::istream &in, std::ostream &out)
{
    const static std::string packed_codebooks("packed_codebooks_aoTuV_603.bin");

    Wwise_RIFF_Vorbis *wwrv;

    try {
        wwrv = new Wwise_RIFF_Vorbis(in,
            packed_codebooks, /* codebooks_filename */
            false, /* inline_codebooks */
            false, /* full_setup */
            kNoForcePacketFormat /* force_packet_format */
    );
    
        wwrv->generate_ogg(out);
    }
#define err_handle(T) catch (T e) \
                      { \
                          std::cerr << e << '\n'; \
                      }//throw e; \
                      //}
    err_handle(Argument_error)
    err_handle(File_open_error)
    err_handle(Size_mismatch)
    err_handle(Invalid_id)
    err_handle(Parse_error_str)
    err_handle(Parse_error)
}

extern "C" void ExportWWRiffToFile(unsigned char *data, int dataSize, const char *filename)
{
    std::string cpp_data(reinterpret_cast<const char *>(data), dataSize);
    std::istringstream in(cpp_data);
    std::ostringstream out;

    ConvWWRiffToOGG(in, out);

    SaveFileData(filename, const_cast<char *>(out.str().c_str()), out.str().size());
}

extern "C" Music LoadWWRiffMusic(unsigned char *data, int dataSize)
{
    std::string cpp_data(reinterpret_cast<const char *>(data), dataSize);
    std::istringstream in(cpp_data);
    std::ostringstream out;

    ConvWWRiffToOGG(in, out);

    Music music = LoadMusicStreamFromMemory(".ogg", reinterpret_cast<const unsigned char *>(out.str().c_str()), out.str().size());

    // TODO: can we free ogg?

    return music;

}

extern "C" Wave LoadWWRiffWave(unsigned char *data, int dataSize)
{
    std::string cpp_data(reinterpret_cast<const char *>(data), dataSize);
    std::istringstream in(cpp_data);
    std::ostringstream out;

    ConvWWRiffToOGG(in, out);

    Wave wave = LoadWaveFromMemory(".ogg", reinterpret_cast<const unsigned char *>(out.str().c_str()), out.str().size());

    return wave;
}

