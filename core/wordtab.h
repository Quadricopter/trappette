#ifndef __WORDTAB_H__
#define __WORDTAB_H__

typedef char*   wordtab_t;

#ifdef __cplusplus
extern "C" {
#endif

wordtab_t   *StrToWordTab(const char *szString, char sep);
int         GetWTWordNumber(wordtab_t *wt);
int         DestroyWordTab(wordtab_t *wt);

#ifdef __cplusplus
}
#endif


#endif /* __WORDTAB_H__ */
