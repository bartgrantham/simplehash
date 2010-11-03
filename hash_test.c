#include "hash.h"
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#ifdef __linux
#define TIMESTART clock_gettime(CLOCK_REALTIME, &tp_start);
#define TIMEEND   clock_gettime(CLOCK_REALTIME, &tp_end); \
    elapsed = tp_end.tv_sec - tp_start.tv_sec; \
    elapsed += (tp_end.tv_nsec - tp_start.tv_nsec)/1000000000.0;
#else
#define TIMESTART start = clock();
#define TIMEEND   end = clock(); \
    elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
#endif



int main(int argc, char * argv[])
{
    int i, j, k, l, max_len;
    float r, s, t, elapsed;
    char * dictionary = "/usr/share/dict/words";
    char * test_string = "this_is_a_test";
    FILE * words;
    void * tmp_ptr;
    char buffer[1024];
    int freq[HASH_KEYS_PER_TABLE];
    struct hash_entry * words_hash = NULL;
#ifdef __linux
    struct timespec tp_start, tp_end;
#else
    clock_t start, end;
#endif

    printf("Testing hash():\n");

    /**** Does hash() not fail with a zero round? ****/
    printf("    Testing hash() with a zero round...");
    fflush(stdout);
    i = hash("test", 0);
    printf("result is %d (didn't crash)\n", i);


    /**** Given a zero-length string, does it produce a reasonable hash? ****/
    printf("    Testing result of zero-length string...");
    fflush(stdout);
    i = hash("", 1000);
    printf("result is %d (didn't crash)\n", i);


    /**** Given many strings does it produce a numerically even result? ****/
    printf("    Reading strings from %s, testing for hash results...\n", dictionary);
    bzero(freq, sizeof(freq));
    bzero(buffer, sizeof(buffer));
    j = k = l = 0;

    words = fopen(dictionary, "r");
    while ( fgets(buffer, sizeof(buffer)-1, words) )
    {
        i = strlen(buffer);
        buffer[i-1] = '\0';  // strip the newline
        freq[hash(buffer, 100) % HASH_KEYS_PER_TABLE]++;
        j++;
    }
    fclose(words);

    for(i=0; i< HASH_KEYS_PER_TABLE; i++)
    {
        if ( freq[i]/(float)j > freq[k]/(float)j ) {  k = i;  }
        if ( freq[i]/(float)j < freq[l]/(float)j ) {  l = i;  }
    }
    printf("        most hit == %d (%.3f%%), least hit == %d (%.3f%%), ideal == %.3f%%\n", \
        k, (freq[k]/(float)j)*100, l, (freq[l]/(float)j)*100, 100.0/HASH_KEYS_PER_TABLE);



    /**** Predictable, non-static runtime result? ****/
    printf("    Predictability (non-static execution, string is \"%s\")...\n", test_string);
    for(i=1; i<=12; i++)
    {
        l = (int)powl(2,i);
        printf("\t\tTesting %d rounds\t : ", l);
        fflush(stdout);
        j = hash(test_string, l);
        k = hash(test_string, l);
        if ( j == k ) {  printf("Yes (%d==%d)\n", j, k);  }
                 else {  printf("NO! (%d==%d)\n", j, k);  exit -1;  }
    }


    /**** Performance ****/
    k = 1024*1024*1;
    l = strlen(test_string);
    printf("    Performance (hashing \"%s\" %d times with %d rounds, this may take a bit)...", test_string, k, l);
    fflush(stdout);
    j = HASH_INITIAL_SALT;
    TIMESTART;
    for(i=0; i<k; i++)
    {
        j = hash(test_string, l);
    }
    TIMEEND;
    printf("Done %.3fs (%.3f/sec)\n", elapsed, k/elapsed);



    /******** Testing hash_*() ********/
    printf("Testing hash_*()...\n");

    /**** Load the up a hash with the dictionary ****/
    printf("    Loading new hash with contents of %s [hash_set()]...", dictionary);
    fflush(stdout);
    words = fopen(dictionary, "r");
    j=0;
    TIMESTART;
    while ( fgets(buffer, sizeof(buffer)-1, words) )
    {
        i = strlen(buffer);
        buffer[i-1] = '\0';  // strip the newline
        hash_set(words_hash, buffer, test_string);  // key == dict word, value is test_string
        j++;
    }
    fclose(words);
    TIMEEND;
    printf("Done.  %d entries loaded in %.3fs (%.3f/sec)\n", j, elapsed, j/elapsed);

    /**** Hash lookup ****/
    printf("    Looking up all entries of the hash [hash_get()]...");
    fflush(stdout);
    words = fopen(dictionary, "r");
    j=0;
    TIMESTART;
    while ( fgets(buffer, sizeof(buffer)-1, words) )
    {
        i = strlen(buffer);
        buffer[i-1] = '\0';  // strip the newline
        tmp_ptr = hash_get(words_hash, buffer);
        if ( tmp_ptr != test_string )
        {  printf("\n        ERROR retreiving value for key %s : expected %p, got %p\n", buffer, test_string, tmp_ptr);  exit -1;  }
        j++;
    }
    fclose(words);
    TIMEEND;
    printf("Done.  %d entries found in %.3fs (%.3f/sec)\n", j, elapsed, j/elapsed);
    l = j;  // number of words... we'll need this later

    /**** Hash stats ****/
    printf("    Running stats on the hash [hash_stats(), hash_depth(), hash_sparseness()]...\n");
    i = j = k = 0;
    tmp_ptr = NULL;
    hash_stats(words_hash, &i, &j, &k, &tmp_ptr);
    printf("        Depth = %d, Tables = %d, Entries = %d, Nulls : %d, Sparseness = %f, Max pointer = %p\n", \
        hash_depth(words_hash), i, j, k, hash_sparseness(words_hash), tmp_ptr);


    /**** Hash clearing ****/
    printf("    Clearing all but one entries of the hash [hash_clear()]...");
    fflush(stdout);
    words = fopen(dictionary, "r");
    j=0;
    TIMESTART;
    while ( fgets(buffer, sizeof(buffer)-1, words) )
    {
        i = strlen(buffer);
        buffer[i-1] = '\0';  // strip the newline
        tmp_ptr = hash_clear(words_hash, buffer);
        if ( tmp_ptr != test_string )
        {  printf("\n        ERROR clear value for key %s : expected %p, got %p\n", buffer, test_string, tmp_ptr);  exit -1;  }
        j++;
        if ( j == l-1 ) {  break;  }
    }
    fclose(words);
    TIMEEND;
    printf("Done.  %d entries cleared in %.3fs (%.3f/sec)\n", j, elapsed, j/elapsed);

    /**** Hash stats again ****/
    printf("    Running stats on the now almost empty hash [hash_stats(), hash_depth(), hash_sparseness()]...\n");
    i = j = k = 0;
    tmp_ptr = NULL;
    hash_stats(words_hash, &i, &j, &k, &tmp_ptr);
    printf("        Depth = %d, Tables = %d, Entries = %d, Nulls : %d, Sparseness = %f, Max pointer = %p\n", \
        hash_depth(words_hash), i, j, k, hash_sparseness(words_hash), tmp_ptr);




//    hash_dump(myhash);

    // TODO: fuzzing of the hash_get/set/etc. functions
}