#ifndef DNS_H
#define DNS_H

#include "duv.h"

void duv_pushaddrinfo(duk_context *ctx, struct addrinfo* res);
duk_ret_t duv_getaddrinfo(duk_context *ctx);
duk_ret_t duv_getnameinfo(duk_context *ctx);

#endif