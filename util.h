//
// Created by Neven Boric on 3/16/23.
//

#ifndef _UTIL_H_
#define _UTIL_H_

struct ll_node
{
	void* value;
	struct ll_node* prev;
	struct ll_node* next;
};

struct ll_node* ll_create();
struct ll_node* ll_copy(struct ll_node* head, void* (* val_copy_func)(void* value));
void ll_add(struct ll_node* head, void* value);
struct ll_node* ll_find(struct ll_node* head, void* value);
void ll_remove(struct ll_node* node);
void ll_free(struct ll_node* head, void (* val_free_func)(void*));
void ll_apply(struct ll_node* head, void (* val_apply_func)(void*, void*), void *data);

#endif //_UTIL_H_
