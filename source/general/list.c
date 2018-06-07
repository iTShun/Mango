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
    t_list *const this = list.alloc();
    if (this)
    {
        this->m.head.first = NULL;
        this->m.head.last = &this->m.head.first;
    }
    else
        delete(this);
    
    return this;
}

bool
methodDecl(clear)
{
    return false;
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
        
    }
    
    ++this->m.count;
    
    return true;
}

bool
methodDecl_(del)
    void *ptr __
{
    return false;
}

bool
methodDecl_(delWithIndex)
    size_t idx, void *ptr __
{
    return false;
}

void
methodDecl_(*const get)
    size_t idx __
{
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
    return NULL;
}
void
methodDecl_(*const next)
    void **it __
{
    return NULL;
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(clear),
methodName(add),
methodName(insert),
methodName(del),
methodName(delWithIndex),
methodName(get),
methodName(count),
methodName(first),
methodName(next)

ENDOF_IMPLEMENTATION
