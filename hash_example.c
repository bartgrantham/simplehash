/*  Licensed under the MIT License by Bart Grantham, 2010.  See ./LICENSE or
 *  http://www.opensource.org/licenses/mit-license.php
 */
#include "hash.h"
#include <fcntl.h>

#define KEYS_IN_FLIGHT 1000

int main(int argc, char * argv[])
{
    hash_entry * myhash = NULL;
    FILE * words;
    int i=0, j=0, k=0, l=0, m=0;
    char buf[1024];
    char * dictionary = "/usr/share/dict/words";
    char * value_sentinel = "value_sentinel";
    char * testwords[KEYS_IN_FLIGHT];
    void * max_ptr;

    printf("Churning through %s, %d keys \"in flight\" at any given moment\n", dictionary, KEYS_IN_FLIGHT);

    bzero(testwords, KEYS_IN_FLIGHT * sizeof(char*));
    words = fopen(dictionary, "r");

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
        hash_set(myhash, testwords[i%KEYS_IN_FLIGHT], value_sentinel);
        i++;
    }

/*
    // clear the remaining words, commented so that hash_stats(), etc. below
    // have something to operate on
    for(i=0; i<KEYS_IN_FLIGHT; i++)
    {
        hash_clear(myhash, testwords[i%KEYS_IN_FLIGHT]);
        free(testwords[i%KEYS_IN_FLIGHT]);
    }
*/

    hash_dump(myhash);

    i = j = k = 0;
    max_ptr = NULL;
    hash_stats(myhash, &i, &j, &k, &max_ptr);
    printf("depth:\t: %d\n", hash_depth(myhash));
    printf("tables\t: %d\nentries\t: %d\nnulls\t: %d\n", i, j, k);
    printf("sparseness\t:%f\n", hash_sparseness(myhash));
    printf("max pointer\t:%p\n", max_ptr);

    return 0;
}
