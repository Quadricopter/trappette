#ifndef __READER_H__
#define __READER_H__

/*
 *
 */

typedef struct  {

    int (*read_cb)(char *dst, int size, void *data);
    void    *data;
    char    *read_buffer;   // ASYNC callback read buffer
    int     buffer_size;
    int     read_size;      // Total bytes read by ASYNC callback
    int     idx;
}   reader_t;

/*
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

int reader_init(reader_t *ctx, int (*read_cb)(char *dst, int size, void *data), int size, void *data);
int reader_getNextLine(reader_t *ctx, char *dst, int size, char eol);
int reader_getNBytes(reader_t *ctx, char *dst, int size);
int reader_release(reader_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /*__READER_H__*/
