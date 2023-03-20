//
// Created by Neven Boric on 3/16/23.
//

#include <stdlib.h>
#include "util.h"

struct ll_node* ll_node_create()
{
	struct ll_node* node = malloc(sizeof(struct ll_node));
	node->prev = node->next = node->value = NULL;
	return node;
}

struct ll_node* ll_create()
{
	return ll_node_create();
}

struct ll_node* ll_copy(struct ll_node* head, void* (* val_copy_func)(void* value))
{
	struct ll_node* new_head = ll_node_create();
	struct ll_node* n = head->next;
	while(n != NULL)
	{
		void* val_copy = val_copy_func(n->value);
		// using ll_add will reverse the order
		ll_add(new_head, val_copy);
		n = n->next;
	}
	return new_head;
}

struct ll_node* ll_find(struct ll_node* head, void* value)
{
	for (struct ll_node* n = head->next; n != NULL; n = n->next)
	{
		if (n->value == value)
		{
			return n;
		}
	}
	return NULL;
}

void ll_add(struct ll_node* head, void* value)
{
	struct ll_node* tmp = head->next;
	struct ll_node* new = ll_node_create();
	new->value = value;
	head->next = new;
	new->prev = head;
	new->next = tmp;
	if (tmp != NULL)
	{
		tmp->prev = new;
	}
}

void ll_remove(struct ll_node* node)
{
	struct ll_node* prev = node->prev;
	struct ll_node* next = node->next;
	prev->next = next;
	if (next != NULL)
	{
		next->prev = prev;
	}
    free(node);
}

void ll_free(struct ll_node* head, void (* val_free_func)(void*))
{
	struct ll_node* n = head->next;
	while (n != NULL)
	{
		struct ll_node* next = n->next;
		val_free_func(n->value);
		free(n);
		n = next;
	}
    free(head);
}

void ll_apply(struct ll_node* head, void (* val_apply_func)(void*, void*), void *data)
{
	for (struct ll_node* n = head->next; n != NULL; n = n->next)
	{
		val_apply_func(n->value, data);
	}
}