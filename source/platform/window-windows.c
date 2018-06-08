#define IMPLEMENTATION

#include "window-windows.h"

#if BX_PLATFORM_WINDOWS

/*
---------------------
Object implementation
---------------------
*/

void
constMethodOvldDecl(print, ref)
{
	sendCMsg(this, ref, print);
	//printf("\tXX:\t%s\n", sub_cast(this, ref)->m.XX);
}

OBJECT_IMPLEMENTATION

SUPERCLASS(ref)

ENDOF_IMPLEMENTATION

/*
--------------------
Class implementation
--------------------
*/

initClassDecl() /* required */
{
	/* initialize super class */
	initSuper(ref);

	/* overload super class methods */
	overload(ref.print) = methodOvldName(print, ref);
}

dtorDecl() /* required */
{
	ref._ref(super(this, ref));
}

t_window
classMethodDecl(*const create)
{
	t_window * this = window.alloc();
	if (this)
	{

	}
	else
		BX_SAFECLASS(this);

	return this;
}

CLASS_IMPLEMENTATION

methodName(create)

ENDOF_IMPLEMENTATION

#endif /* BX_PLATFORM_WINDOWS */