/*
  TRURLib
  
  $Id$
 */

#ifndef __TRURL_HASH_H
#define __TRURL_HASH_H

#include <stddef.h>           /* For size_t     */

typedef struct trurl_hash_table tn_hash;

/*
** This is used to construct the table.  If it doesn't succeed,
** returns NULL
*/

tn_hash *n_hash_new(size_t size, void (*freefn)(void*));


/*
** Inserts a pointer to 'data' in the table, with a copy of 'key' as its
** key.  Note that this makes a copy of the key, but NOT of the
** associated data.
*/

tn_hash *n_hash_insert(tn_hash *ht, const char *key, const void *data);

tn_hash *n_hash_replace(tn_hash *ht, const char *key, const void *data);


/*
** Returns a pointer to the data associated with a key.  If the key has
** not been inserted in the table, returns NULL.
*/

void *n_hash_get(const tn_hash *ht, const char *key);


/*
** Deletes an entry from the table.  Returns a pointer to the data that
** was associated with the key so the calling code can dispose of it
** properly.
*/

void *n_hash_remove(tn_hash *table, const char *key);


/*
** Goes through a hash table and calls the function passed to it
** for each node that has been inserted.  The function is passed
** a pointer to the key, and a pointer to the data associated
** with it.
*/

int n_hash_map(const tn_hash *ht, void (*map_fn)(const char *, void *));

int n_hash_map_arg(const tn_hash *ht,
                   void (*map_fn)(const char *, void *, void *), void *arg);


/*
** Frees a hash table.
*/
void n_hash_free(tn_hash *ht);


#endif /* __TRURL_HASH_H */
