#define IMPLEMENTATION

#include "list.h"

/*
 ---------------------
 Object implementation
 ---------------------
 */

BASEOBJECT_IMPLEMENTATION

ENDOF_IMPLEMENTATION

/*
 --------------------
 Class implementation
 --------------------
 */

initClassDecl() /* required */
{
    
}

dtorDecl() /* required */
{
    list.clear(this);
}

t_list
classMethodDecl(*const create)
{
    t_list *this = list.alloc();
    if (this)
    {
        this->m.head.first = NULL;
        this->m.head.last = &this->m.head.first;
		this->m.count = 0;
    }
    else
		BX_SAFECLASS(this);
    
    return this;
}

bool
methodDecl(clear)
{
	assert(this != NULL);

	list_item *item = NULL;

	while ((item = this->m.head.first) != NULL )
	{
		if (item->next != NULL)
			item->next->prev = item->prev;
		else
			this->m.head.last = item->prev;
		*item->prev = item->next;

		--this->m.count;
		BX_SAFEFREE(getDefaultAllocator(), item);
	}

    return (this->m.count == 0);
}

bool
methodDecl_(add)
    void *ptr __
{
    return list.insert(this, this->m.count, ptr);
}

bool
methodDecl_(insert)
    size_t idx, void *ptr __
{
    list_item *prev, *item = NULL;
    assert(this != NULL);
    
    item = BX_ALLOC(getDefaultAllocator(), sizeof(list_item));
    assert(item != NULL);
    
    item->ptr = ptr;
    
    if (idx == 0)
    {
        if ((item->next = this->m.head.first) != NULL)
            item->next->prev = &item->next;
        else
        {
            this->m.head.last = &item->next;
            this->m.head.first = item;
            item->prev = &this->m.head.first;
        }
    }
    else if (idx == this->m.count)
    {
        item->next = NULL;
        item->prev = this->m.head.last;
        *this->m.head.last = item;
        this->m.head.last = &item->next;
    }
    else
    {
		for (prev = this->m.head.first; prev != NULL; prev = prev->next)
		{
			if (idx-- == 0)
				break;
		}

		item->prev = prev->prev;
		item->next = prev;
		*prev->prev = item;
		prev->prev = &item->next;
    }
    
    ++this->m.count;
    
    return true;
}

bool
methodDecl_(remove)
    void *ptr __
{
	assert(this != NULL);

	for (list_item *item = this->m.head.first; item != NULL; item = item->next)
	{
		if (item->ptr == ptr)
		{
			if (item->next != NULL)
				item->next->prev = item->prev;
			else
				this->m.head.last = item->prev;
			*item->prev = item->next;

			--this->m.count;
			BX_SAFEFREE(getDefaultAllocator(), item);
			return true;
		}
	}

    return false;
}

bool
methodDecl_(removeWithIndex)
    size_t idx, void **pptr __
{
	assert(this != NULL);
	assert(idx < this->m.count);

	list_item *item = NULL;

	for (item = this->m.head.first; item != NULL; item = item->next)
	{
		if (idx-- == 0)
			break;
	}

	if (item == NULL) return false;

	if (pptr)
		*pptr = item->ptr;

	if (item->next != NULL)
		item->next->prev = item->prev;
	else
		this->m.head.last = item->prev;
	*item->prev = item->next;

	--this->m.count;
	BX_SAFEFREE(getDefaultAllocator(), item);

    return true;
}

void
methodDecl_(*const get)
    size_t idx __
{
	assert(this != NULL);
	assert(idx < this->m.count);

	for (list_item *item = this->m.head.first; item != NULL; item = item->next)
	{
		if (idx-- == 0)
			return item->ptr;
	}

    return NULL;
}

size_t
methodDecl(count)
{
    return this->m.count;
}

void
methodDecl_(*const first)
    void **it __
{
	assert(this != NULL);

	list_item *item = this->m.head.first;

	if (it && item)
		*it = item->next;
	else if (it)
		*it = NULL;

	if (item)
		return item->ptr;

    return NULL;
}

void
methodDecl_(*const next)
    void **it __
{
	assert(this != NULL);
	assert(it != NULL);
	assert(*it != NULL);

	list_item *item = *it;

	if (it && item)
		*it = item->next;
	else if (it)
		*it = NULL;

	if (item)
		return item->ptr;

    return NULL;
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(clear),
methodName(add),
methodName(insert),
methodName(remove),
methodName(removeWithIndex),
methodName(get),
methodName(count),
methodName(first),
methodName(next)

ENDOF_IMPLEMENTATION
