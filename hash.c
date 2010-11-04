/*  Licensed under the MIT License by Bart Grantham, 2010.  See ./LICENSE or
 *  http://www.opensource.org/licenses/mit-license.php
 */

#include "hash.h"

char hash_next_magic[] = HASH_NEXT_MAGIC;

// NOTE: "(hash_entry **)&((*h)[i].value)" should be simplified if I can.  It means:
// "the address of the 'value' member of the i'th hash_entry of the array pointed to by h"
// It is used in three places in this code.


// similiar to "djb2", but I continue iterating through the string _round_ number of characters
inline int hash(const char * key, int round)
{
    unsigned long hash_key = 5381;
    int i = 0, j = 0;

    // quick short circuit
    if ( key[0] == 0 ) {  return hash_key;  }

    while(key[i++] != 0 )
    {
        hash_key = hash_key * 33 ^ (int)key[i];
    }

    while(j < round)
    {
       hash_key = hash_key * 33 ^ (int)key[j%i];
       j++;
    }

    return abs(hash_key);
}


void * hash_get_depth(hash_entry h[], const char * key, int depth)
{
    int keyhash, i;
    keyhash = hash(key, depth);
    i = keyhash % HASH_KEYS_PER_TABLE;

    // quick short circuit
    if ( h[i].key == NULL ) {  return HASH_SELECT_FAILED;  }

    // another hash to look in...
    if ( h[i].key == hash_next_magic )
    {
        return hash_get_depth(h[i].value, key, depth+1);
    }

    // found the key
    if ( strncmp(h[i].key, key, HASH_MAX_KEYSIZE) == 0 )
    {
        return h[i].value;
    }

    return HASH_SELECT_FAILED;
}


void * hash_clear_depth(hash_entry * h[], const char * key, int depth)
{
    int keyhash, i;
    hash_entry * old_hash, * first_entry;
    void * deleted_val;

    keyhash = hash(key, depth);
    i = keyhash % HASH_KEYS_PER_TABLE;

    // quick short circuit
    if ( (*h)[i].key == NULL ) {  return NULL;  }

    // the value is another hash: call again with this hash
    if ( (*h)[i].key == hash_next_magic )
    {
        deleted_val = hash_clear_depth((hash_entry **)&((*h)[i].value), key, depth+1);
        // subtle: if the hash that we passed in was destroyed because it no longer
        // had any entries, then we need to make sure to set the key to null
        // !!! don't attempt to free the string though, it points to hash_next_magic !!!
        if ( (*h)[i].value ==  NULL ) {  (*h)[i].key = NULL;  }
        if ( hash_entries((*h)) == 0 ) {  free(*h);  *h = NULL;  }
        return deleted_val;
    }

    // key found: free the key I stored, set the key and value pointers to NULL, return the deleted value pointer
    if ( strncmp((*h)[i].key, key, HASH_MAX_KEYSIZE) == 0 )
    {
        deleted_val = (*h)[i].value;
        free((*h)[i].key);
        (*h)[i].key = NULL;  (*h)[i].value = NULL;
        // if after deleting this key/value pair the hash has no other entries,
        // then we free the memory and set it's value to NULL
        if ( hash_entries((*h)) == 0 ) {  free(*h);  *h = NULL;  }
        return deleted_val;
    }

    return HASH_DELETE_FAILED;
}


int hash_set_depth(hash_entry * h[], const char * key, const void * value, int depth)
{
    int keyhash, i;
    char * tmp_key;
    void * tmp_val;

    keyhash = hash(key, depth);
    i = keyhash % HASH_KEYS_PER_TABLE;

    // if we have a null hash, then we must create the new hash
    if ( *h == NULL )
    {
        *h = malloc(HASH_KEYS_PER_TABLE * sizeof(hash_entry));
        bzero(*h, HASH_KEYS_PER_TABLE * sizeof(hash_entry));
    }

    // the key for this hash entry is null: copy key, set value
    if ( (*h)[i].key == NULL )
    {
        if ( (*h)[i].key = malloc(strlen(key)+1) )
        {
            strcpy((*h)[i].key, key);
            (*h)[i].value = (void *)value;
            return HASH_INSERT;
        }
        else {  return HASH_INSERT_FAILED;  }
    }

    // the key for this hash entry is pointing to another hash: insert the value into this sub-hash
    if ( (*h)[i].key == hash_next_magic )
    {
        return hash_set_depth((hash_entry **)&((*h)[i].value), key, value, depth+1);
    }

    // the key for this hash entry is non-null and is equal to the key parameter: update value
    if ( strncmp((*h)[i].key, key, HASH_MAX_KEYSIZE) == 0 )
    {
        (*h)[i].value = (void *)value;  return HASH_UPDATE;
    }

    // the key for this hash entry is non-null and is not equal to hash_next_magic:
    // copy the magic keyword to the key, create a new hash, and add the previous
    // key/val and the new key/value pairs to the new hash
    tmp_key = (*h)[i].key;  tmp_val = (*h)[i].value;
    (*h)[i].key = hash_next_magic;
    (*h)[i].value = NULL;
    hash_set_depth((hash_entry **)&((*h)[i].value), tmp_key, tmp_val, depth+1);
    return hash_set_depth((hash_entry **)&((*h)[i].value), key, value, depth+1);
}



inline int hash_entries(hash_entry h[])
{
    int i = 0, entries = 0;
    for(i=0; i<HASH_KEYS_PER_TABLE; i++)
    {
        if ( h[i].key != NULL ) {  entries++;  }
    }
    return entries;
}


inline hash_entry * hash_first_entry(hash_entry h[])
{
    int i = 0;
    for(i=0; i<HASH_KEYS_PER_TABLE; i++)
    {
        if ( h[i].key != NULL ) {  return &(h[i]);  }
    }
    return NULL;
}


int hash_depth(hash_entry h[])
{
    int i = 0, temp = 0, max = 0;
    for(i=0; i<HASH_KEYS_PER_TABLE; i++)
    {
        if ( h[i].key != NULL && h[i].key == hash_next_magic ) {  temp = hash_depth(h[i].value);  if (temp > max) max = temp;  }
    }
    return max + 1;
}


void hash_dump_depth(hash_entry h[], int depth)
{
    int i;
    if ( h == NULL ) {  return;  }
    printf("Hashtable %p:\n", h );
    for(i=0; i<HASH_KEYS_PER_TABLE; i++)
    {
        if ( h[i].key == NULL ) {  continue;  }
        if ( h[i].key == hash_next_magic ) {  printf("%*d ", 4 + (depth*4), i);  hash_dump_depth(h[i].value, depth+4);  }
        else {  printf("%*d %s\t%p\n", 4 + (depth*4), i, h[i].key, h[i].value);  }
    }
}


void hash_stats(hash_entry h[], int * tables, int * entries, int * nulls, void ** max_ptr)
{
    int i;
    for(i=0; i<HASH_KEYS_PER_TABLE; i++)
    {
        if ( h[i].key == NULL ) {  (*nulls)++;  }
        else if ( h[i].key == hash_next_magic )
        {
            (*tables)++;
            if ( h[i].value > *max_ptr ) {  *max_ptr = h[i].value;  }
            hash_stats(h[i].value, tables, entries, nulls, max_ptr);
        }
        else
        {
            (*entries)++;
            if ( (void *)h[i].key > *max_ptr ) {  *max_ptr = (void *)h[i].key;  }
        }
    }
}


double hash_sparseness(hash_entry h[])
{
    int i=0, j=0, k=0;
    void * max_ptr = NULL;
    hash_stats(h, &i, &j, &k, &max_ptr);
    // the sum of ( tables + non-table entries + nulls ) is the number of key/value pairs
    // the hash has.  The number of nulls divided by this nulls gives 'sparseness', which
    // can be as little as zero and in the most pathological case should never be more than
    // (HASH_KEYS_PER_TABLE-1)/HASH_KEYS_PER_TABLE
    return (double)k / (double)(i + j + k);
}
