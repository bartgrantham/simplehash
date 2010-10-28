#include "hash.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>


int main(int argc, char * argv[])
{
    int i, j, k, l;
    float r, s, t;
    char * dictionary = "/usr/share/dict/words";
    FILE * words;
    struct timespec tp_start, tp_end;
    char buffer[1024];
    int freq[HASH_KEYS_PER_TABLE];

/*  Tests for int hash(const char * key, int salt);
 *  - Given a zero-length string, does it produce a reasonable hash?
 *  - Given a series of possible strings does it produce a numerically even result?
 *  - Predictable, non-static runtime result?
 *  - Performance
 */

    // Zero-length string
    printf("Testing hash()\n");
    printf("    Testing result of zero-length string...", strlen(""));
    i = hash("", HASH_INITIAL_SALT);
    printf("result is 0x%X (didn't crash)\n", i);


    // Numerically even
    printf("    Reading strings from %s, testing for hash results...\n", dictionary);
    bzero(freq, sizeof(freq));
    bzero(buffer, sizeof(buffer));
    j = k = l = 0;
    words = fopen(dictionary, "r");
    while ( fgets(buffer, sizeof(buffer)-1, words) )
    {
        j = strlen(buffer);  buffer[j-1] = '\0';  // strip the newline
        freq[hash(buffer, HASH_INITIAL_SALT) % HASH_KEYS_PER_TABLE]++;
        j++;
    }
    for(i=0; i< HASH_KEYS_PER_TABLE; i++)
    {
        if ( freq[i]/(float)j > freq[k]/(float)j ) {  k = i;  }
        if ( freq[i]/(float)j < freq[l]/(float)j ) {  l = i;  }
    }
    printf("        most hit == %d (%f), least hit == %d (%f)\n", k, freq[k]/(float)j, l, freq[l]/(float)j);


    // Predictable result
    printf("    Predictability (non-static execution)...");
    i = hash("this_is_a_test", HASH_INITIAL_SALT);
    j = hash("this_is_a_test", hash("this_is_a_test", hash("this_is_a_test", hash("this_is_a_test", HASH_INITIAL_SALT))));
    k = hash("this_is_a_test", HASH_INITIAL_SALT);
    l = hash("this_is_a_test", hash("this_is_a_test", hash("this_is_a_test", hash("this_is_a_test", HASH_INITIAL_SALT))));
    if ( i == k && j == l ) {  printf("Yes (%d==%d , %d==%d)\n", i, k, j, l);  }
                       else {  printf("NO! (%d==%d , %d==%d)\n", i, k, j, l);  exit;  }


    // Performance
    k = 1024*1024;
    printf("    Performance (hashing \"this_is_a_test\" %d times)...", k);
    j = HASH_INITIAL_SALT;
    clock_gettime(CLOCK_PROF, &tp_start);
    for(i=0; i<k; i++)
    {
        j = hash("this_is_a_test", j);
    }
    clock_gettime(CLOCK_PROF, &tp_end);
    r = tp_end.tv_sec - tp_start.tv_sec;  r += (tp_end.tv_nsec - tp_start.tv_nsec)/1000000000.0;
    printf("Done %.3fs (%.3f/sec)\n", r, k/r);


/*
 *
 *
 */




/*
    i = j = k = 0;
    max_pointer = NULL;
    hash_stats(myhash, &i, &j, &k, &max_pointer);
    printf("depth:\t: %d\n", hash_depth(myhash));
    printf("tables\t: %d\nentries\t: %d\nnulls\t: %d\n", i, j, k);
    printf("sparseness\t:%f\n", hash_sparseness(myhash));
    printf("max pointer\t:%p\n", max_pointer);
*/
}
