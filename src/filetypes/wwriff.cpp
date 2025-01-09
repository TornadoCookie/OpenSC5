#include "wwriff.h"
#include <cpl_raylib.h>
#include <sstream>

static void ConvWWRiffToOGG(std::istream &in, std::ostream &out)
{
    const static std::string packed_codebooks("packed_codebooks_aoTuV_603.bin");

    Wwise_RIFF_Vorbis *wwrv = new Wwise_RIFF_Vorbis(&in,
            packed_codebooks, /* codebooks_filename */
            false, /* inline_codebooks */
            false, /* full_setup */
            kNoForcePacketFormat /* force_packet_format */
    );
    
    try {
        wwrv->generate_ogg(out);
    }
#define err_handle(T) catch (T e) \
                      { \
                          std::cerr << e << '\n'; \
                          throw e; \
                      }
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

