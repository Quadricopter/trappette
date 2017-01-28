#include <stdlib.h>
#include <string.h>
#include "wordtab.h"

wordtab_t *StrToWordTab(const char *szString, char sep)
{
    wordtab_t   *wordtab;
    char        *p, *szStringCpy;
    int         nWordCount = 1;
    int         n = 0;

    /*
     * Calcule la taille du tableau
     */

    szStringCpy = strdup(szString);
    for (p = szStringCpy; *p; p++) {

        if ( *p == sep ) {

            *p = '\0';
            nWordCount++;
    	}
    }

    /*
     * Copie des chaines dans le tableau de ptr
     */

    wordtab = (wordtab_t*) malloc((nWordCount+2) * sizeof(char*));
    for (n = 0, p = szStringCpy; n < nWordCount; n++) {

        if (*p) {

            wordtab[n] = strdup(p);
            p += strlen(p);
        }
        else
            wordtab[n] = strdup("");
        p++;
    }
    wordtab[n] = NULL;
    free(szStringCpy);

    return (wordtab);
}

int     GetWTWordNumber(wordtab_t *wt)
{
    int n;

    for (n = 0; wt[n]; n++) { }

    return (n);
}

int     DestroyWordTab(wordtab_t *wt)
{
    int n;

    for (n = 0; wt[n]; n++)
        free(wt[n]);
    free(wt);

    return (0);
}
