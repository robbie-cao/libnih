/* libnih
 *
 * Copyright © 2007 Scott James Remnant <scott@netsplit.com>.
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

#ifndef NIH_LIST_H
#define NIH_LIST_H

#include <nih/macros.h>


/**
 * NihList:
 * @prev: previous entry in the list,
 * @next: next entry in the list.
 *
 * This structure can be used both to refer to a linked list and can be
 * placed in your own structures to use them as list entries.
 *
 * The list is circular so the @next pointer of the last entry points to
 * the first, and the @prev pointer of the first entry points to the last.
 * An empty list simply has the @prev and @next pointers pointing to itself.
 **/
typedef struct nih_list {
	struct nih_list *prev, *next;
} NihList;

/**
 * NihListEntry:
 * @entry: list header,
 * @data: data pointer,
 * @str: string pointer,
 * @int_data: integer value.
 *
 * This structure can be used as a generic NihList node that contains
 * a pointer to generic data, a string or contains an integer value.
 *
 * You should take care of setting the data yourself.
 **/
typedef struct nih_list_entry {
	NihList entry;
	union {
		void *data;
		char *str;
		int   int_data;
	};
} NihListEntry;


/**
 * NIH_LIST_EMPTY:
 * @list: entry in the list to check.
 *
 * Checks whether the given list is empty by comparing the next and
 * previous pointers for equality.
 *
 * Returns: TRUE if empty, FALSE otherwise.
 **/
#define NIH_LIST_EMPTY(list) (((list)->prev == (list)) \
			      && ((list)->next) == (list))

/**
 * NIH_LIST_FOREACH:
 * @list: entry in the list to iterate,
 * @iter: name of iterator variable.
 *
 * Expands to a for statement that iterates over each entry in @list
 * except @list itself, setting @iter to each entry for the block within
 * the loop.
 *
 * If you wish to modify the list, e.g. remove entries, use
 * NIH_LIST_FOREACH_SAFE() instead.
 **/
#define NIH_LIST_FOREACH(list, iter) \
	for (NihList *iter = (list)->next; iter != (list); iter = iter->next)

/**
 * NIH_LIST_FOREACH_SAFE:
 * @list: entry in the list to iterate,
 * @iter: name of iterator variable.
 *
 * Expands to a for statement that iterates over each entry in @list
 * except @list itself, setting @iter to each entry for the block within
 * the loop.
 *
 * The iteration is performed safely by placing a cursor node after @iter;
 * this means that any node including @iter can be removed from the list,
 * added to a different list, or entries added before or after it.
 *
 * Note that if you add an entry directly after @iter and wish it to be
 * visited, you would need to use NIH_LIST_FOREACH() instead, as this
 * would be placed before the cursor and thus skipped.
 **/
#define NIH_LIST_FOREACH_SAFE(list, iter)				\
	for (NihList  _##iter##_cursor = { &_##iter##_cursor, &_##iter##_cursor }, \
		     *_##iter = &_##iter##_cursor;			\
	     _##iter;							\
	     nih_list_destroy (_##iter), _##iter = NULL)		\
		for (NihList *iter = nih_list_add_after ((list)->next, _##iter)->prev; \
		     iter != (list) && iter != _##iter;		\
		     iter = nih_list_add_after (_##iter->next, _##iter)->prev)


NIH_BEGIN_EXTERN

void          nih_list_init      (NihList *entry);
NihList *     nih_list_new       (const void *parent)
	__attribute__ ((warn_unused_result, malloc));

NihListEntry *nih_list_entry_new (const void *parent)
	__attribute__ ((warn_unused_result, malloc));


NihList *     nih_list_add       (NihList *list, NihList *entry);
NihList *     nih_list_add_after (NihList *list, NihList *entry);

NihList *     nih_list_remove    (NihList *entry);
int           nih_list_destroy   (NihList *entry);

NIH_END_EXTERN

#endif /* NIH_LIST_H */
