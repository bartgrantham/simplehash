#include "hash.h"
#include <stdio.h>
#include <fcntl.h>

#define KEYS_IN_FLIGHT 100

int main(int argc, char * argv[])
{
    struct hash_entry * myhash = NULL;
    FILE * words;
    int i=0, j=0, k=0, l=0, m=0;
    char buf[1024];
    char * testwords[KEYS_IN_FLIGHT];
    void * max_pointer;

    bzero(testwords, KEYS_IN_FLIGHT * sizeof(char*));
    words = fopen("/usr/share/dict/words", "r");

    // rotate words in and out of the hash tables...
    while ( fgets(buf, sizeof(buf)-1, words) )
    {
        j = strlen(buf);
        buf[j-1] = '\0';
        if ( testwords[i%KEYS_IN_FLIGHT] != NULL )
        {
            hash_clear(myhash, testwords[i%KEYS_IN_FLIGHT]);
            free(testwords[i%KEYS_IN_FLIGHT]);
        }

        testwords[i%KEYS_IN_FLIGHT] = malloc(j);
        strncpy(testwords[i%KEYS_IN_FLIGHT], buf, j);
        hash_set(myhash, testwords[i%KEYS_IN_FLIGHT], NULL);
        i++;
    }
    hash_dump(myhash);
/*
    // clear the remaining words...
    for(i=0; i<KEYS_IN_FLIGHT; i++)
    {
        hash_clear(myhash, testwords[i%KEYS_IN_FLIGHT]);
        free(testwords[i%KEYS_IN_FLIGHT]);
        hash_dump(myhash);
    }
*/
    i = j = k = 0;
    max_pointer = NULL;
    hash_stats(myhash, &i, &j, &k, &max_pointer);
    printf("depth:\t: %d\n", hash_depth(myhash));
    printf("tables\t: %d\nentries\t: %d\nnulls\t: %d\n", i, j, k);
    printf("sparseness\t:%f\n", hash_sparseness(myhash));
    printf("max pointer\t:%p\n", max_pointer);

    return 0;
}
