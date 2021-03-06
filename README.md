# simplehash #

## Description ##

Super simple hashing library.  Maps from strings to void pointers:

    hash_set(treasure_map, "X_marks_the_spot", treasure_ptr);`

    ...

    my_treasure = hash_get(treasure_map, "X_marks_the_spot");

The itch that this scratches is:  I just want a very simple way to record
memory locations based on strings.


## Details ##

Why not just use the X/Open functions `hcreate()`, `hsearch()`, and `hdestroy()`?:

  * The API is needlessly gross.
  * The library shouldn't require you to create the hash
  * The naming scheme/terminology is confusing and non-standard
  * POSIX version non-reentrant, only one hash at a time
  * I prefered the system to be smaller/tighter
  * It's easier to use.


Important things to note:

 * simplehash will allocate or free all the memory it uses, _including keeping track
    of its keys_.  That is, it will malloc space and copy your string key so that
    there's no confusion about who's responsible for that memory.  Managing memory
    for the data your value pointers refer to is up to you, though.

 * `hash_get()` will return NULL if it fails to find a key.  This is only a problem if
    you expect to be storing NULLs as the values to keys.  If it's an issue, I
    suggest storing a pointer to a sentinel as the value in this situation.

 * `hash_set(myhash, "foo", NULL)` doesn't clear the key/value pair from the hash, you
    should use `hash_clear(myhash, "foo")` for that instead.  The difference is that
    `hash_clear()` will cascade delete empty hash tables whereas `hash_set()` does not.
    This is because setting a key/value pair where the value is NULL is semantially
    different than clearing a key/value pair.

 * The hash algorithm is pretty good, but not cryptographically sound.  This means
    that it is vulnerable to memory exhaustion where an attacker crafts large keys
    that conflict, causing nearly empty tables (ie. a "vertical" table arrangement).
    This is not unique to this library of course, and the solution would defeat
    the goal of simplicity and speed in this library.  The likelihood of this kind
    of attack is very, very low, but full disclosure...

 * The algorithm doesn't rebalance the hash tree on hash_clear(), so a heavily
    churned hash will not be optimally organized.  This is almost never a problem,
    but if you require that your datastructures be perfectly formed in memory at
    all times at the expense of the occasional rebalancing, this library will not
    suffice. 

 * Unless you set `HASH_KEYS_PER_TABLE` very low, the hash tends to be very sparse,
    usually 90%+.  That's usually ok, the tables don't take up THAT much space.


## How to Use ##

Declare your hash pointer, _making sure to initialize it to NULL_:

    struct hash_entry * myhash = NULL;

Then you can add key/value pairs to the hash...:

    hash_set(myhash, "my_key_foo", value_bar_ptr);
    // don't free the memory that your values point to!  we
    // only make a copy of the key string, not the value data

...retrieve the values from the keys...:

    tmpval = hash_get(myhash, "my_key_baz");

...and clear the values:

    tmpval = hash_clear(myhash, "my_key_quux");
    // "tmp_val" points to the value the key "my_key_quux" used to 
    // refer to, and "my_key_quux" is no longer a key in the hash

If necessary, you can also dump the hash to stdout:

    hash_dump(myhash);

There's also a set of functions that will provide stats on a hash:

    i = j = k = 0;  max_ptr = NULL;
    hash_stats(myhash, &i, &j, &k, &max_ptr);
    printf("tables\t: %d\nentries\t: %d\nnulls\t: %d\nmax pointer\t:%p\n", i, j, k, max_ptr);

    printf("depth:\t: %d\n", hash_depth(myhash));

    printf("sparseness\t:%f\n", hash_sparseness(myhash));

Finally, if you need to, there are some constants in hash.h that can
be altered to your needs:

  `HASH_KEYS_PER_TABLE` - This is a trade-off between sparseness of the
        hashtable and depth for a given entry.

  `HASH_MAX_KEYSIZE` - This is to prevent runaway strcmp()'s, increase
        as necessary if you find that your keys aren't unique within
        this many characters.


## Installation ##

All you really need is hash.h and hash.c.  I've included my test program
("hash_test.c") and a quick program that shows how to use these functions
("hash_example.c").  Also included is a quick Makefile.  If you want to see it
come to life just get all the files and:

    # make
    # ./hash_test
    # ./hash_example


## Todo ##

Audit:

  * is my code cache-friendly?  (for example, good: hash_stats always compares against hash_next_magic, which keeps it in L1)
  * rigorously test mem performance
  * rigorously test hash_clear and hash_get... make big test cases...
  * graphs showing:
    * statistical randomness of hashing algo with random keys/random rounds
    * effect of hash entries on sparseness, mem utilization, and speed
    * test suite should automatically test millions of variable-length random strings, churning millions of times
    * time of inserts/lookups/deletes

New features:

  * `hash_map()`: takes a function and runs it against all the elements in the hash (depth first, breadth first too?)
    * see `l_mapt` from <http://www.pasteit4me.com/46003>,<http://www.pasteit4me.com/46002>,<http://www.pasteit4me.com/45003>
    * this could provide hash_copy()
  * `hash_value()`: given a void *, find the first string that is the key for it (requires a full search)
  * `hash_map_key()`, `hash_map_value()`: combination of the two above ideas: depth first search for a value that causes the map fcn to return true
  * `hash_copy()`: this would be a simple solution to the "sparseness" problem.
  * Advanced feature: vararg `hash_set()` and `hash_get()`: `hash_set_varg(myhash, &user, "New York", "New York City", "Grantham", "Bart", ...)`

Outstanding questions:

  * Would an sdbm-derived hash be better?


## Bugs, etc. ##

Please let me know if you find any, or if you have license-friendly
enhancements to add.


## License ##

MIT License.  See ./LICENSE or
<http://www.opensource.org/licenses/mit-license.php>

Few things are more enjoyable than the knowledge that you've helped another
person. If you do use these functions for anything, I'd love to hear about it:

<bart@bartgrantham.com>


Enjoy!

-Bart

