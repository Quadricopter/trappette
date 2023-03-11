#include "trappette_sdk.h"
#include "m10.h"

trappette_lib_t lib_ctx = {

    .szLibName         = "M10",
    .szLibInfo         = "Meteomodem M10 decoder (http://www.meteomodem.com/)",

    .init              = M10_init,
    .release           = M10_release,
    .setTsipCallback   = M10_setTsipCallback,
    .setStreamCallback = M10_setStreamCallback,
    .process16bit48k   = M10_process16bit48k,
    .setVerboseLevel   = M10_setVerboseLevel,
    .enableFilter      = M10_enableFilter
};

trappette_lib_t *get_lib_info(void)
{
    return &lib_ctx;
}
