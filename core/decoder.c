#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <dlfcn.h>
#include "decoder.h"
#include "trappette_sdk.h"

/*
 * Private
 */

typedef struct {

    void            *handle;
    trappette_lib_t *pLibInfo;
    void            *pDecoder;
}   lib_t;

int decoder_add(decoder_t *ctx, void *handle, trappette_lib_t *pLibInfo)
{
    lib_t   *lib = NULL;

    lib = malloc(sizeof(lib_t));
    memset(lib, 0, sizeof(lib_t));
    lib->handle = handle;
    lib->pLibInfo = pLibInfo;
    if (pLibInfo->init)
        lib->pDecoder = pLibInfo->init();

    ctx->libs = realloc(ctx->libs, sizeof(lib_t*) * (ctx->count+1));
    ctx->libs[ctx->count] = lib;
    ctx->count++;

    return 0;
}

/*
 * Public
 */

int decoder_init(decoder_t *ctx, const char *szPath)
{
    DIR            *d;
    struct dirent  *dir;

    memset(ctx, 0, sizeof(decoder_t));

    d = opendir(szPath);
    if (d) {

        // List all files in dir
        while ((dir = readdir(d)) != NULL) {

            // Check if file is a library
            if ((dir->d_type == DT_REG || dir->d_type == DT_LNK) &&
                (strlen(dir->d_name) > strlen(DEFAULT_DECODER_EXTENSION)) &&
                (!strcmp(&dir->d_name[strlen(dir->d_name)-strlen(DEFAULT_DECODER_EXTENSION)], DEFAULT_DECODER_EXTENSION))) {

                void *handle = NULL;
                char *pszLongFilename = malloc(strlen(szPath)+strlen(dir->d_name)+2);

                pszLongFilename[0] = '\0';
                sprintf(pszLongFilename, "%s%s%s", szPath, "/", dir->d_name);          

                // Load library
                handle = dlopen(pszLongFilename, RTLD_LAZY);
                if (!handle) {
                    fprintf(stderr, "[DECODER] Error: Can't open %s\n", pszLongFilename);
                }
                else {

                    trappette_lib_t *(*get_lib_info)(void) = NULL;
                    trappette_lib_t *pLibInfo = NULL;

                    // Search library info
                    *(void**) (&get_lib_info) = dlsym(handle, "get_lib_info");
                    if (!get_lib_info) {
                        fprintf(stderr, "[DECODER] Error: %s: get_lib_info() not found\n", pszLongFilename);
                        dlclose(handle);
                    }
                    else {
                        pLibInfo = get_lib_info();
                        if (!pLibInfo) {
                            fprintf(stderr, "[DECODER] Error: %s: get_lib_info() returns NULL\n", pszLongFilename);
                            dlclose(handle);
                        }
                        else {
                            fprintf(stderr, "[DECODER] %s Loaded: %s\n", pLibInfo->szLibName,
                                                                         pLibInfo->szLibInfo);

                            // Check library min requirement 
                            if (!pLibInfo->process16bit48k) {
                                fprintf(stderr, "[DECODER] Error: %s trappette_lib_t doesn't have process16bit48k()\n", pszLongFilename);
                                dlclose(handle);
                            }
                            else {
                                decoder_add(ctx, handle, pLibInfo);
                            }
                        }
                    }
                }

                free(pszLongFilename);
            }
        }
        closedir(d);
    } 

    return ctx->count;
}

void decoder_release(decoder_t *ctx)
{
    if (ctx->libs) {

        for (int n = 0; n < ctx->count; n++) {

            lib_t *lib = ctx->libs[n];

            if (lib->pLibInfo->release)
                lib->pLibInfo->release(lib->pDecoder);

            dlclose(lib->handle);
            free(lib);
            ctx->libs[n] = NULL;
        }

        free(ctx->libs);
        ctx->libs = NULL;
    }
}

void    decoder_setDecodedCallback(decoder_t *ctx, int (*decoded_cb)(const decoded_position_t *position, void *data), void *data)
{
        for (int n = 0; n < ctx->count; n++) {

            lib_t *lib = ctx->libs[n];

            if (lib->pLibInfo->setDecodedCallback)
                lib->pLibInfo->setDecodedCallback(lib->pDecoder, decoded_cb, data);
        }
}

void    decoder_setStreamCallback(decoder_t *ctx, int (*stream_cb)(const uint8_t *stream, uint16_t size, void *data), void *data)
{
        for (int n = 0; n < ctx->count; n++) {

            lib_t *lib = ctx->libs[n];

            if (lib->pLibInfo->setStreamCallback)
                lib->pLibInfo->setStreamCallback(lib->pDecoder, stream_cb, data);
        }
}

void    decoder_process16bit48k(decoder_t *ctx, int16_t *samples, uint16_t count)
{
    for (int n = 0; n < ctx->count; n++) {

        lib_t *lib = ctx->libs[n];

        if (lib->pLibInfo->process16bit48k)
            lib->pLibInfo->process16bit48k(lib->pDecoder, samples, count);
    }
}

void    decoder_setVerboseLevel(decoder_t *ctx, uint8_t level)
{
    for (int n = 0; n < ctx->count; n++) {

        lib_t *lib = ctx->libs[n];

        if (lib->pLibInfo->setVerboseLevel)
            lib->pLibInfo->setVerboseLevel(lib->pDecoder, level);
    }
}

void    decoder_enableFilter(decoder_t *ctx, bool bEnableFilter)
{
    for (int n = 0; n < ctx->count; n++) {

        lib_t *lib = ctx->libs[n];

        if (lib->pLibInfo->enableFilter)
            lib->pLibInfo->enableFilter(lib->pDecoder, bEnableFilter);
    }
}