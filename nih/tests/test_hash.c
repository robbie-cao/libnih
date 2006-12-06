/* libnih
 *
 * test_hash.c - test suite for nih/hash.c
 *
 * Copyright © 2006 Scott James Remnant <scott@netsplit.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <nih/test.h>

#include <assert.h>

#include <nih/macros.h>
#include <nih/alloc.h>
#include <nih/list.h>
#include <nih/hash.h>


typedef struct hash_entry {
	NihList     list;
	const char *key;
} HashEntry;

static NihList *
new_entry (void       *parent,
	   const char *key)
{
	HashEntry *entry;

	assert (key != NULL);

	entry = nih_new (parent, HashEntry);
	assert (entry != NULL);

	nih_list_init (&entry->list);
	entry->key = key;

	return (NihList *)entry;
}

static const char *
key_function (NihList *entry)
{
	assert (entry != NULL);

	return ((HashEntry *)entry)->key;
}


void
test_new (void)
{
	NihHash *hash;
	size_t   i;

	TEST_FUNCTION ("nih_hash_new");

	/* Check that we can create a small hash table; a small prime number
	 * should be selected for the actual size, and that number of empty
	 * bins should be allocated as a child of the hash table.
	 */
	TEST_FEATURE ("with zero size");
	hash = nih_hash_new (NULL, 0, key_function);

	TEST_ALLOC_SIZE (hash, sizeof(NihHash));
	TEST_EQ_P (hash->key_function, key_function);

	TEST_EQ (hash->size, 17);
	TEST_NE_P (hash->bins, NULL);
	TEST_ALLOC_PARENT (hash->bins, hash);

	for (i = 0; i < hash->size; i++)
		TEST_LIST_EMPTY (&hash->bins[i]);

	nih_free (hash);


	/* Check again with a medium size, which should pick a medium prime
	 * number.
	 */
	TEST_FEATURE ("with medium size");
	hash = nih_hash_new (NULL, 650, key_function);

	TEST_EQ (hash->size, 331);
	TEST_NE_P (hash->bins, NULL);
	TEST_ALLOC_PARENT (hash->bins, hash);

	for (i = 0; i < hash->size; i++)
		TEST_LIST_EMPTY (&hash->bins[i]);

	nih_free (hash);


	/* Check with a much larger size, which should pick the largest prime
	 * that we know of.
	 */
	TEST_FEATURE ("with large size");
	hash = nih_hash_new (NULL, 40000000, key_function);

	TEST_EQ (hash->size, 10250323);
	TEST_NE_P (hash->bins, NULL);
	TEST_ALLOC_PARENT (hash->bins, hash);

	for (i = 0; i < hash->size; i++)
		TEST_LIST_EMPTY (&hash->bins[i]);

	nih_free (hash);
}

void
test_add (void)
{
	NihHash *hash;
	NihList *entry1, *entry2, *entry3, *entry4, *ptr;

	TEST_FUNCTION ("nih_hash_add");
	hash = nih_hash_new (NULL, 0, key_function);
	entry1 = new_entry (hash, "entry 1");
	entry2 = new_entry (hash, "entry 2");
	entry3 = new_entry (hash, "entry 1");
	entry4 = new_entry (hash, "entry 4");

	/* Check that we can add an entry to an empty hash table; it should
	 * be returned and turn up in the appropriate bin.
	 */
	TEST_FEATURE ("with empty hash");
	ptr = nih_hash_add (hash, entry1);

	TEST_EQ_P (ptr, entry1);

	TEST_EQ_P (hash->bins[15].next, entry1);
	TEST_EQ_P (entry1->next, &hash->bins[15]);
	TEST_EQ_P (hash->bins[15].prev, entry1);
	TEST_EQ_P (entry1->prev, &hash->bins[15]);


	/* Check that we can add an entry to a populated hash table. */
	TEST_FEATURE ("with non-empty hash");
	nih_hash_add (hash, entry2);

	TEST_EQ_P (hash->bins[14].next, entry2);
	TEST_EQ_P (entry2->next, &hash->bins[14]);
	TEST_EQ_P (hash->bins[14].prev, entry2);
	TEST_EQ_P (entry2->prev, &hash->bins[14]);


	/* Check that we can add an entry with a duplicate key, and it is
	 * added to the end of the same bin as the previous entry with that
	 * key.
	 */
	TEST_FEATURE ("with duplicate key");
	nih_hash_add (hash, entry3);

	TEST_EQ_P (hash->bins[15].next, entry1);
	TEST_EQ_P (entry1->next, entry3);
	TEST_EQ_P (entry3->next, &hash->bins[15]);
	TEST_EQ_P (hash->bins[15].prev, entry3);
	TEST_EQ_P (entry3->prev, entry1);
	TEST_EQ_P (entry1->prev, &hash->bins[15]);


	/* Check that nih_hash_add can rip an entry out of an existing list
	 * and place it in the hash table.
	 */
	TEST_FEATURE ("with entry already in a list");
	ptr = nih_list_new (NULL);
	nih_list_add (ptr, entry4);
	nih_hash_add (hash, entry4);

	TEST_EQ_P (ptr->next, ptr);
	TEST_EQ_P (ptr->prev, ptr);

	TEST_EQ_P (hash->bins[3].next, entry4);
	TEST_EQ_P (entry4->next, &hash->bins[3]);
	TEST_EQ_P (hash->bins[3].prev, entry4);
	TEST_EQ_P (entry4->prev, &hash->bins[3]);

	nih_free (hash);
	nih_free (ptr);
}

void
test_add_unique (void)
{
	NihHash *hash;
	NihList *entry1, *entry2, *entry3, *entry4, *ptr;

	TEST_FUNCTION ("nih_hash_add_unique");
	hash = nih_hash_new (NULL, 0, key_function);
	entry1 = new_entry (hash, "entry 1");
	entry2 = new_entry (hash, "entry 2");
	entry3 = new_entry (hash, "entry 1");
	entry4 = new_entry (hash, "entry 4");

	/* Check that we can add an entry to an empty hash table; it should
	 * be returned and turn up in the appropriate bin.
	 */
	TEST_FEATURE ("with empty hash");
	ptr = nih_hash_add_unique (hash, entry1);

	TEST_EQ_P (ptr, entry1);

	TEST_EQ_P (hash->bins[15].next, entry1);
	TEST_EQ_P (entry1->next, &hash->bins[15]);
	TEST_EQ_P (hash->bins[15].prev, entry1);
	TEST_EQ_P (entry1->prev, &hash->bins[15]);


	/* Check that we can add an entry to a populated hash table. */
	TEST_FEATURE ("with non-empty hash");
	nih_hash_add_unique (hash, entry2);

	TEST_EQ_P (hash->bins[14].next, entry2);
	TEST_EQ_P (entry2->next, &hash->bins[14]);
	TEST_EQ_P (hash->bins[14].prev, entry2);
	TEST_EQ_P (entry2->prev, &hash->bins[14]);


	/* Check that we get NULL if we try and add an entry with a
	 * duplicate key, and that the hash table is not altered.
	 */
	TEST_FEATURE ("with duplicate key");
	ptr = nih_hash_add_unique (hash, entry3);

	TEST_EQ_P (ptr, NULL);

	TEST_EQ_P (hash->bins[15].next, entry1);
	TEST_EQ_P (entry1->next, &hash->bins[15]);
	TEST_EQ_P (hash->bins[15].prev, entry1);
	TEST_EQ_P (entry1->prev, &hash->bins[15]);


	/* Check that nih_hash_add can rip an entry out of an existing list
	 * and place it in the hash table.
	 */
	TEST_FEATURE ("with entry already in a list");
	ptr = nih_list_new (NULL);
	nih_list_add (ptr, entry4);
	nih_hash_add_unique (hash, entry4);

	TEST_EQ_P (ptr->next, ptr);
	TEST_EQ_P (ptr->prev, ptr);

	TEST_EQ_P (hash->bins[3].next, entry4);
	TEST_EQ_P (entry4->next, &hash->bins[3]);
	TEST_EQ_P (hash->bins[3].prev, entry4);
	TEST_EQ_P (entry4->prev, &hash->bins[3]);

	nih_free (hash);
	nih_free (ptr);
}

void
test_replace (void)
{
	NihHash *hash;
	NihList *entry1, *entry2, *entry3, *entry4, *ptr;

	TEST_FUNCTION ("nih_hash_replace");
	hash = nih_hash_new (NULL, 0, key_function);
	entry1 = new_entry (hash, "entry 1");
	entry2 = new_entry (hash, "entry 2");
	entry3 = new_entry (hash, "entry 1");
	entry4 = new_entry (hash, "entry 4");

	/* Check that we can add an entry to an empty hash table; NULL should
	 * be returned (nothing replaced) and the entry should turn up in the
	 * appropriate bin.
	 */
	TEST_FEATURE ("with empty hash");
	ptr = nih_hash_replace (hash, entry1);

	TEST_EQ_P (ptr, NULL);

	TEST_EQ_P (hash->bins[15].next, entry1);
	TEST_EQ_P (entry1->next, &hash->bins[15]);
	TEST_EQ_P (hash->bins[15].prev, entry1);
	TEST_EQ_P (entry1->prev, &hash->bins[15]);


	/* Check that we can add an entry to a populated hash table. */
	TEST_FEATURE ("with non-empty hash");
	nih_hash_replace (hash, entry2);

	TEST_EQ_P (hash->bins[14].next, entry2);
	TEST_EQ_P (entry2->next, &hash->bins[14]);
	TEST_EQ_P (hash->bins[14].prev, entry2);
	TEST_EQ_P (entry2->prev, &hash->bins[14]);


	/* Check that we can add an entry with a duplicate key, replacing
	 * the existing one in the hash.  The replaced entry should be
	 * returned, and removed from the bin.
	 */
	TEST_FEATURE ("with duplicate key");
	ptr = nih_hash_replace (hash, entry3);

	TEST_EQ_P (ptr, entry1);

	TEST_EQ_P (entry1->next, entry1);
	TEST_EQ_P (entry1->prev, entry1);

	TEST_EQ_P (hash->bins[15].next, entry3);
	TEST_EQ_P (entry3->next, &hash->bins[15]);
	TEST_EQ_P (hash->bins[15].prev, entry3);
	TEST_EQ_P (entry3->prev, &hash->bins[15]);


	/* Check that nih_hash_add can rip an entry out of an existing list
	 * and place it in the hash table.
	 */
	TEST_FEATURE ("with entry already in a list");
	ptr = nih_list_new (NULL);
	nih_list_add (ptr, entry4);
	nih_hash_replace (hash, entry4);

	TEST_EQ_P (ptr->next, ptr);
	TEST_EQ_P (ptr->prev, ptr);

	TEST_EQ_P (hash->bins[3].next, entry4);
	TEST_EQ_P (entry4->next, &hash->bins[3]);
	TEST_EQ_P (hash->bins[3].prev, entry4);
	TEST_EQ_P (entry4->prev, &hash->bins[3]);

	nih_free (hash);
	nih_free (ptr);
}

void
test_search (void)
{
	NihHash *hash;
	NihList *entry1, *entry2, *entry3, *ptr;

	TEST_FUNCTION ("nih_hash_search");
	hash = nih_hash_new (NULL, 0, key_function);
	entry1 = nih_hash_add (hash, new_entry (hash, "entry 1"));
	entry2 = nih_hash_add (hash, new_entry (hash, "entry 2"));
	entry3 = nih_hash_add (hash, new_entry (hash, "entry 2"));

	/* Check that we find the sole matching entry. */
	ptr = nih_hash_search (hash, "entry 1", NULL);

	TEST_EQ_P (ptr, entry1);

	/* Searching again should find nothing */
	ptr = nih_hash_search (hash, "entry 1", ptr);

	TEST_EQ_P (ptr, NULL);


	/* Check that where there's multiple matches, we find the first one. */
	TEST_FEATURE ("with multiple matches");
	ptr = nih_hash_search (hash, "entry 2", NULL);

	TEST_EQ_P (ptr, entry2);

	/* And that searching again finds the second one. */
	ptr = nih_hash_search (hash, "entry 2", ptr);

	TEST_EQ_P (ptr, entry3);

	/* And again finds nothing. */
	ptr = nih_hash_search (hash, "entry 2", ptr);

	TEST_EQ_P (ptr, NULL);


	/* Check that we get NULL if there are no matches. */
	TEST_FEATURE ("with no matches");
	ptr = nih_hash_search (hash, "entry 3", NULL);

	TEST_EQ_P (ptr, NULL);

	nih_free (hash);
}

void
test_lookup (void)
{
	NihHash *hash;
	NihList *entry1, *entry2, *entry3, *ptr;

	TEST_FUNCTION ("nih_hash_lookup");
	hash = nih_hash_new (NULL, 0, key_function);
	entry1 = nih_hash_add (hash, new_entry (hash, "entry 1"));
	entry2 = nih_hash_add (hash, new_entry (hash, "entry 2"));
	entry3 = nih_hash_add (hash, new_entry (hash, "entry 2"));

	/* Check that we find a single matching entry. */
	TEST_FEATURE ("with single match");
	ptr = nih_hash_lookup (hash, "entry 1");

	TEST_EQ_P (ptr, entry1);


	/* Check that we find the first matching entry. */
	TEST_FEATURE ("with multiple matches");
	ptr = nih_hash_lookup (hash, "entry 2");

	TEST_EQ_P (ptr, entry2);


	/* Check that we get NULL when there are no matching entries. */
	TEST_FEATURE ("with no matches");
	ptr = nih_hash_lookup (hash, "entry 3");

	TEST_EQ_P (ptr, NULL);

	nih_free (hash);
}


int
main (int   argc,
      char *argv[])
{
	test_new ();
	test_add ();
	test_add_unique ();
	test_replace ();
	test_search ();
	test_lookup ();

	return 0;
}
