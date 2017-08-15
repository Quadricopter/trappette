#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reader.h"

//#define TRACE

int reader_init(reader_t *ctx, int (*read_cb)(char *dst, int size, void *data), int size, void *data)
{
    memset((void*)ctx, 0, sizeof(reader_t));
    ctx->read_cb = read_cb;
    ctx->data = data;
    ctx->buffer_size = size;
    ctx->read_size = -1;
    ctx->read_buffer = malloc((ctx->buffer_size+1)*sizeof(char));
    ctx->idx = -1;

    return 0;
}

int reader_release(reader_t *ctx)
{
    if (ctx && ctx->read_buffer) {

        free(ctx->read_buffer);  
        memset(ctx, 0, sizeof(reader_t));
    }

    return 0;
}

#ifdef TRACE

void    dump_string(char *str, int size)
{
    int n;
    
    for (n = 0; n < size; n++)
        printf("%c", isprint(str[n])?str[n]:'.');
}

void    dump_read_buffer(reader_t *ctx)
{
    int n;
    printf("READ[");
    dump_string(ctx->read_buffer, ctx->buffer_size);
    printf("]\n     ");
    for (n = 0; n < ctx->idx; n++)
        printf(" ");
    printf("^\n");
}

#endif

static int find_next_index(int idx, char *str, int size, char eol)
{
    int n;

    for (n = idx; n < size; n++) {
        if (str[n] == eol)
            return n+1;
    }
    return -1;
}

int reader_getNextLine(reader_t *ctx, char *dst, int size, char eol)
{
    int next, len;

#ifdef TRACE
    printf("%s(%p,%p,%d,%02X)\n", __FUNCTION__, ctx, dst, size, eol);
#endif

    /* Check parameters */
    if (!ctx || !ctx->read_cb) {

#ifdef TRACE
        printf("No ctx!\n");
#endif
        return -1;
    }

    /* First read */
    if (ctx->idx < 0) {

        ctx->read_size = ctx->read_cb(ctx->read_buffer, ctx->buffer_size, ctx->data);
        if (ctx->read_size <= 0) {
            dst[0] = 0;
            return ctx->read_size;
        }
        ctx->idx = find_next_index(0, ctx->read_buffer, ctx->read_size, eol);
        ctx->idx = 0;
    }
#ifdef TRACE
    dump_read_buffer(ctx);
#endif

    /* Find next 'eol' char */
    next = find_next_index(ctx->idx, ctx->read_buffer, ctx->read_size, eol);
    if (next < 0) {

        len = ctx->read_size - ctx->idx;
#ifdef TRACE
        printf("not found... len:%d\n", len);
#endif
        memcpy(dst, &ctx->read_buffer[ctx->idx], len);
        dst[len] = '\0';
        ctx->idx = -1;
        return reader_getNextLine(ctx, &dst[len], size-len, eol);
    }
    len = next - ctx->idx;
    if (len >= size) {
#ifdef TRACE
        printf("to small! %d >= %d\n", len , size);
#endif
        dst[0] = '!';
        dst[1] = '\0';
        return -1;
    }
    memcpy(dst, &ctx->read_buffer[ctx->idx], len);
    dst[len] = '\0';
    ctx->idx = next;

    return len;
}

/*
 * --------------------------------------------------------------------------
 */

int reader_getNBytes(reader_t *ctx, char *dst, int size)
{
    int len;

#ifdef TRACE
    printf("%s(%p,%p,%d)\n", __FUNCTION__, ctx, dst, size);
#endif

    /* Check parameters */
    if (!ctx || !ctx->read_cb) {

#ifdef TRACE
        printf("No ctx!\n");
#endif
        return -1;
    }

    /* First read */
    if (ctx->idx < 0) {

        ctx->read_size = ctx->read_cb(ctx->read_buffer, ctx->buffer_size, ctx->data);
        if (ctx->read_size <= 0) {
            dst[0] = 0;
            return ctx->read_size;
        }
        ctx->idx = 0;
    }
#ifdef TRACE
    dump_read_buffer(ctx);
#endif

    len = ctx->read_size - ctx->idx;

    /*
     * Case A: enough byte in ctx->read_buffer
     * Simply copy "size" bytes to dst and update ctx->idx
     */
    if (len > size) {

        memcpy(dst, &ctx->read_buffer[ctx->idx], size);
        ctx->idx += size;
        return size;
    }

    /*
     * Case B: not enough bytes in in ctx->read_buffer
     * - Copy left bytes in ctx->read_buffer
     * - Async fill ctx->read_buffer
     * - Copy missing bytes, if not enough, recurvive?
     */
    else {

        memcpy(dst, &ctx->read_buffer[ctx->idx], len);
        ctx->idx = -1;
        return reader_getNBytes(ctx, &dst[len], size-len)+len;
    }

    return 0;
}
