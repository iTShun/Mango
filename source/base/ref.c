#define IMPLEMENTATION

#include "ref.h"

/*
 ---------------------
 Object implementation
 ---------------------
 */
void
constMethodDecl(print)
{
    debug_printf("name:\t%s\tref_count:\t%d\n", this->m.name, this->m.ref_count);
}

BASEOBJECT_IMPLEMENTATION

methodName(print)

ENDOF_IMPLEMENTATION

/*
 --------------------
 Class implementation
 --------------------
 */
initClassDecl() /* required */
{
    debug_printf("ref initClassDecl");
}

dtorDecl() /* required */
{
    safe_free(this->m.name);
    this->m.ref_count = 0;
    
    debug_printf("ref dtorDecl");
}

t_ref
classMethodDecl_(*const create) char const name[] __
{
    t_ref *const this = ref.alloc();
    if (this) ref.init(this, name);
    return this;
}

void
methodDecl_(init) char const name[] __
{
    this->m.name = strdup(name);
    this->m.ref_count = 1;
}

void
methodDecl_(copy) t_ref const*const per __
{
    ref._ref(this);
    ref.init(this, per->m.name);
}

int
methodDecl(getRefCount)
{
    return this->m.ref_count;
}

void
methodDecl(retain)
{
    assert(this->m.ref_count > 0);
    ++this->m.ref_count;
}

bool
methodDecl(release)
{
    assert(this->m.ref_count > 0);
    --this->m.ref_count;
    
    if (this->m.ref_count == 0)
    {
        delete(this);
        return true;
    }
    
    return false;
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(init),
methodName(copy),
methodName(getRefCount),
methodName(retain),
methodName(release)

ENDOF_IMPLEMENTATION
