#if !defined(DUK_DEBUGGER_INTERFACE_H_INCLUDED)
#define DUK_DEBUGGER_INTERFACE_H_INCLUDED

#include "duktape.h"

void duk_debugger_init(void*, duk_bool_t);
void duk_debugger_finish(void*);

#endif  /* DUK_TRANS_SOCKET_H_INCLUDED */
