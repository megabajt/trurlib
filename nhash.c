/*
  TRURLib, hash table
  Copyright (C) 1999 Pawel Gajda (mis@k2.net.pl)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  
Module is based on:
**
** public domain code by Jerry Coffin.
**
** Tested with Visual C 1.0 and Borland C 3.1.
** Compiles without warnings, and seems like it should be pretty
** portable.
*/

#include <string.h>
#include <stdlib.h>

#ifdef USE_N_ASSERT
#include "nassert.h"
#else
#include <assert.h>
#define n_assert(expr) assert(expr)
#endif


#ifdef USE_XMALLOCS
#include "xmalloc.h"
#endif

#include "nhash.h"

#define USE_HASHSTRING 1

extern void trurl_die(const char *fmt,...);



/*
   ** A hash table consists of an array of these buckets.  Each bucket
   ** holds a copy of the key, a pointer to the data associated with the
   ** key, and a pointer to the next bucket that collided with this one,
   ** if there was one.
 */

struct hash_bucket {
    char *key;
    void *data;
    struct hash_bucket *next;
};


/*
   ** This is what you actually declare an instance of to create a table.
   ** You then call 'construct_table' with the address of this structure,
   ** and a guess at the size of the table.  Note that more nodes than this
   ** can be inserted in the table, but performance degrades as this
   ** happens.  Performance should still be quite adequate until 2 or 3
   ** times as many nodes have been inserted as the table was created with.
 */

struct trurl_hash_table {
    size_t size;
    struct hash_bucket **table;
    size_t items;

    void (*free_fn) (void *);
};

/* Initialize the hash_table to the size asked for.  Allocates space
   ** for the correct number of pointers and sets them to NULL.  If it
   ** can't allocate sufficient memory, signals error by setting the size
   ** of the table to 0.

   ** RET : new hash table or NULL
 */
tn_hash *n_hash_new(size_t size, void (*freefn) (void *))
{
    tn_hash *ht = NULL;

    ht = calloc(1, sizeof(*ht));

    if (ht == NULL)
	return NULL;

    ht->table = calloc(size, sizeof(*ht->table));

    if (ht->table == NULL) {
	free(ht);
	return NULL;
    }
    ht->size = size;
    ht->items = 0;
    ht->free_fn = freefn;

    return ht;
}


#if USE_HASHSTRING
#include "hash-string.h"
#else

/*
** Hashes a string to produce an unsigned short, which should be
** sufficient for most purposes.
*/
static unsigned hash_string(const char *string)
{
    unsigned ret_val = 0;
    int i;

    while (*string) {
	i = (int) (*(string++));
	ret_val ^= i;
	ret_val <<= 1;
    }
    return ret_val;
}
#endif


/*
 * Insert 'key' into hash table.
 */
static
tn_hash *n_hash_put(tn_hash *ht, const char *key, const void *data, int replace)
{
    struct hash_bucket *ptr;
    unsigned val = hash_string(key) % ht->size;


    ptr = ht->table[val];

    /*
       ** NULL means this bucket hasn't been used yet.  We'll simply
       ** allocate space for our new bucket and put our data there, with
       ** the table pointing at it.
     */

    if (ptr == NULL) {
	if ((ptr = malloc(sizeof(*ptr))) == NULL)
	    return NULL;

	ptr->key  = strdup(key);
	ptr->next = NULL;
	ptr->data = (void *) data;

	ht->items++;
	ht->table[val] = ptr;

	return ht;
    }

    /*
    ** This spot in the table is already in use.  See if the current string
    ** has already been inserted, and if so, increment its count.
    */

    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {

	if (strcmp(key, ptr->key) == 0) {

	    if (!replace) {
		trurl_die("n_hash_insert: key '%s' already in table\n", key);
		return NULL;
	    }
	    if (ht->free_fn == NULL) {
		trurl_die("n_hash_replace: free function is NULL\n");
		return NULL;
	    }
	    ht->free_fn(ptr->data);
	    ptr->data = (void *) data;
	    return ht;
	}
    }

    /*
    ** This key must not be in the table yet.  We'll add it to the head of
    ** the list at this spot in the hash table.  Speed would be
    ** slightly improved if the list was kept sorted instead.  In this case,
    ** this code would be moved into the loop above, and the insertion would
    ** take place as soon as it was determined that the present key in the
    ** list was larger than this one.
    */

    if ((ptr = malloc(sizeof(*ptr))) == NULL)
	return NULL;

    ptr->key = strdup(key);
    ptr->data = (void *) data;
    ptr->next = ht->table[val];
    ht->table[val] = ptr;

    ht->items++;
    return ht;

}


tn_hash *n_hash_insert(tn_hash *ht, const char *key, const void *data)
{
    return n_hash_put(ht, key, data, 0);
}


tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data)
{
    return n_hash_put(ht, key, data, 1);
}


/*
 * Look up a key and return the associated data.  Returns NULL if
 * the key is not in the table.
 */
void *n_hash_get(const tn_hash *ht, const char *key)
{
    unsigned val = hash_string(key) % ht->size;
    struct hash_bucket *ptr;

    if (NULL == ht->table[val])
	return NULL;

    for (ptr = ht->table[val]; NULL != ptr; ptr = ptr->next) {
	if (0 == strcmp(key, ptr->key))
	    return ptr->data;
    }

    return NULL;
}


/*
 * Delete a key from the hash table and return associated
 * data, or NULL if not present.
 */
void *n_hash_remove(tn_hash *ht, const char *key)
{
    unsigned val = hash_string(key) % ht->size;
    void *data;
    struct hash_bucket *ptr, *last = NULL;

    if (ht->table[val] == NULL)
	return 0;

    /*
    ** Traverse the list, keeping track of the previous node in the list.
    ** When we find the node to delete, we set the previous node's next
    ** pointer to point to the node after ourself instead.  We then delete
    ** the key from the present node, and return a pointer to the data it
    ** contains.
    */

    for (last = NULL, ptr = ht->table[val]; ptr != NULL;
	 last = ptr, ptr = ptr->next) {

	if (strcmp(key, ptr->key) == 0) {

	    if (last != NULL) {
		data = ptr->data;

		last->next = ptr->next;

		free(ptr->key);
		free(ptr);
		ht->items--;

		return data;
	    }
	    /* If 'last' still equals NULL, it means that we need to
            ** delete the first node in the list. This simply consists
            ** of putting our own 'next' pointer in the array holding
            ** the head of the list.  We then dispose of the current
            ** node as above.
            */
	    else {
		ht->table[val] = ptr->next;
		data = ptr->data;

		free(ptr->key);
		free(ptr);
		ht->items--;

		return data;
	    }
	}
    }

    /*
    ** If we get here, it means we didn't find the item in the table.
    ** Signal this by returning NULL.
    */
    return NULL;
}


/*
 * Frees a complete table by iterating over it and freeing each node.
 */
void n_hash_free(tn_hash *ht)
{
    size_t i;

    for (i = 0; i < ht->size; i++) {

	while (ht->table[i] != NULL) {
	    void *d = n_hash_remove(ht, ht->table[i]->key);
	    if (d != NULL && ht->free_fn != NULL)
		ht->free_fn(d);
	}
    }

    free(ht->table);

    ht->table = NULL;
    ht->size = 0;
    ht->items = 0;

    free(ht);
}


/*
 * Simply invokes the function given as the second parameter for each
 * node in the table, passing it the key and the associated data.
 */
int n_hash_map(const tn_hash *ht, void (*map_fn) (const char *, void *))
{
    register size_t i, n = 0;
    register struct hash_bucket *tmp;

    for (i = 0; i < ht->size; i++) {

	if (ht->table[i] == NULL)
            continue;
        
        for (tmp = ht->table[i]; tmp != NULL; tmp = tmp->next) {
            map_fn(tmp->key, tmp->data);
            n++;
        }
    }

    return n;
}

/*
 * Simply invokes the function given as the second parameter for each
 * node in the table, passing it the key, the associated data and given arg.
 */
int n_hash_map_arg(const tn_hash *ht,
		   void (*map_fn) (const char *, void *, void *),
		   void *arg)
{
    register size_t i, n = 0;
    register struct hash_bucket *tmp;

    for (i = 0; i < ht->size; i++) {

	if (ht->table[i] == NULL)
            continue;
        
        for (tmp = ht->table[i]; tmp != NULL; tmp = tmp->next) {
            map_fn(tmp->key, tmp->data, arg);
            n++;
        }
    }

    return n;
}
