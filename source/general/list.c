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
    
}

t_list
classMethodDecl(*const create)
{
    
    return NULL;
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
    return false;
}

bool
methodDecl_(insert)
    void *ptr __
{
    return false;
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
