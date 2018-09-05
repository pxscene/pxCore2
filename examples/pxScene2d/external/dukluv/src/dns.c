#include "dns.h"

static int duv_af_string_to_num(const char* string) {
  if (!string) return AF_UNSPEC;
  #ifdef AF_UNIX
    if (strcmp(string, "unix") == 0) return AF_UNIX;
  #endif
  #ifdef AF_INET
    if (strcmp(string, "inet") == 0) return AF_INET;
  #endif
  #ifdef AF_INET6
    if (strcmp(string, "inet6") == 0) return AF_INET6;
  #endif
  #ifdef AF_IPX
    if (strcmp(string, "ipx") == 0) return AF_IPX;
  #endif
  #ifdef AF_NETLINK
    if (strcmp(string, "netlink") == 0) return AF_NETLINK;
  #endif
  #ifdef AF_X25
    if (strcmp(string, "x25") == 0) return AF_X25;
  #endif
  #ifdef AF_AX25
    if (strcmp(string, "ax25") == 0) return AF_AX25;
  #endif
  #ifdef AF_ATMPVC
    if (strcmp(string, "atmpvc") == 0) return AF_ATMPVC;
  #endif
  #ifdef AF_APPLETALK
    if (strcmp(string, "appletalk") == 0) return AF_APPLETALK;
  #endif
  #ifdef AF_PACKET
    if (strcmp(string, "packet") == 0) return AF_PACKET;
  #endif
  return 0;
}

static const char* duv_af_num_to_string(const int num) {
  switch (num) {
    #ifdef AF_UNIX
      case AF_UNIX: return "unix";
    #endif
    #ifdef AF_INET
      case AF_INET: return "inet";
    #endif
    #ifdef AF_INET6
      case AF_INET6: return "inet6";
    #endif
    #ifdef AF_IPX
      case AF_IPX: return "ipx";
    #endif
    #ifdef AF_NETLINK
      case AF_NETLINK: return "netlink";
    #endif
    #ifdef AF_X25
      case AF_X25: return "x25";
    #endif
    #ifdef AF_AX25
      case AF_AX25: return "ax25";
    #endif
    #ifdef AF_ATMPVC
      case AF_ATMPVC: return "atmpvc";
    #endif
    #ifdef AF_APPLETALK
      case AF_APPLETALK: return "appletalk";
    #endif
    #ifdef AF_PACKET
      case AF_PACKET: return "packet";
    #endif
  }
  return NULL;
}

static int duv_sock_string_to_num(const char* string) {
  if (!string) return 0;
  #ifdef SOCK_STREAM
    if (strcmp(string, "stream") == 0) return SOCK_STREAM;
  #endif
  #ifdef SOCK_DGRAM
    if (strcmp(string, "dgram") == 0) return SOCK_DGRAM;
  #endif
  #ifdef SOCK_SEQPACKET
    if (strcmp(string, "seqpacket") == 0) return SOCK_SEQPACKET;
  #endif
  #ifdef SOCK_RAW
    if (strcmp(string, "raw") == 0) return SOCK_RAW;
  #endif
  #ifdef SOCK_RDM
    if (strcmp(string, "rdm") == 0) return SOCK_RDM;
  #endif
  return 0;
}

static const char* duv_sock_num_to_string(const int num) {
  switch (num) {
    #ifdef SOCK_STREAM
      case SOCK_STREAM: return "stream";
    #endif
    #ifdef SOCK_DGRAM
      case SOCK_DGRAM: return "dgram";
    #endif
    #ifdef SOCK_SEQPACKET
      case SOCK_SEQPACKET: return "seqpacket";
    #endif
    #ifdef SOCK_RAW
      case SOCK_RAW: return "raw";
    #endif
    #ifdef SOCK_RDM
      case SOCK_RDM: return "rdm";
    #endif
  }
  return NULL;
}


void duv_pushaddrinfo(duk_context *ctx, struct addrinfo* res) {
  char ip[INET6_ADDRSTRLEN];
  int port, i = 0;
  const char *addr;
  struct addrinfo* curr = res;
  duk_push_array(ctx);
  for (curr = res; curr; curr = curr->ai_next) {
    if (curr->ai_family == AF_INET || curr->ai_family == AF_INET6) {
      duk_push_object(ctx);
      if (curr->ai_family == AF_INET) {
        addr = (char*) &((struct sockaddr_in*) curr->ai_addr)->sin_addr;
        port = ((struct sockaddr_in*) curr->ai_addr)->sin_port;
      } else {
        addr = (char*) &((struct sockaddr_in6*) curr->ai_addr)->sin6_addr;
        port = ((struct sockaddr_in6*) curr->ai_addr)->sin6_port;
      }
      duk_push_string(ctx, duv_af_num_to_string(curr->ai_family));
      duk_put_prop_string(ctx, -2, "family");
      uv_inet_ntop(curr->ai_family, addr, ip, INET6_ADDRSTRLEN);
      duk_push_string(ctx, ip);
      duk_put_prop_string(ctx, -2, "addr");
      if (ntohs(port)) {
        duk_push_int(ctx, ntohs(port));
        duk_put_prop_string(ctx, -2, "port");
      }
      duk_push_string(ctx, duv_sock_num_to_string(curr->ai_socktype));
      duk_put_prop_string(ctx, -2, "socktype");
      duk_push_string(ctx, duv_af_num_to_string(curr->ai_protocol));
      duk_put_prop_string(ctx, -2, "protocol");
      if (curr->ai_canonname) {
        duk_push_string(ctx, curr->ai_canonname);
        duk_put_prop_string(ctx, -2, "canonname");
      }
      duk_put_prop_index(ctx, -2, i++);
    }
  }
}

static void duv_on_addrinfo(uv_getaddrinfo_t* req, int status, struct addrinfo* res) {
  duk_context *ctx = req->loop->data;//req->data;
  duv_push_status(ctx, status);
  if (status < 0) {
    //duv_resolve((uv_req_t*)req, 1);
    duv_fulfill_req(ctx, (uv_req_t*)req, 1);
  }
  else {
    duv_pushaddrinfo(ctx, res);
    //duv_resolve((uv_req_t*)req, 2);
    duv_fulfill_req(ctx, (uv_req_t*)req, 2);
  }
  req->data = duv_cleanup_req(ctx, req->data);
}

static void duv_on_nameinfo(uv_getnameinfo_t* req, int status, const char* hostname, const char* service) {
  duk_context *ctx = req->loop->data;//req->data;
  duv_push_status(ctx, status);
  if (status < 0) {
    //duv_resolve((uv_req_t*)req, 1);
    duv_fulfill_req(ctx, (uv_req_t*)req, 1);
  }
  else {
    duk_push_string(ctx, hostname);
    duk_push_string(ctx, service);
    //duk_dump_context_stdout(ctx);
    //duv_resolve((uv_req_t*)req, 3);
    duv_fulfill_req(ctx, (uv_req_t*)req, 3);
  }
  req->data = duv_cleanup_req(ctx, req->data);
}

duk_ret_t duv_getaddrinfo(duk_context *ctx) {
  dschema_check(ctx, (const duv_schema_entry[]) {
    {"options", duk_is_object},
    {"callback", dschema_is_continuation},
    {0,0}
  });

  uv_getaddrinfo_t* req;
  const char* node;
  const char* service;
  struct addrinfo hints_s;
  struct addrinfo* hints = &hints_s;

  duk_get_prop_string(ctx, 0, "node");
  if (!duk_is_null_or_undefined(ctx, -1)) {
    node = duk_get_string(ctx, -1);
  }
  else {
    node = NULL;
  }
  duk_pop(ctx);

  duk_get_prop_string(ctx, 0, "service");
  if (!duk_is_null_or_undefined(ctx, -1)) {
    service = duk_get_string(ctx, -1);
  }
  else {
    service = NULL;
  }
  duk_pop(ctx);

  // Initialize the hints
  memset(hints, 0, sizeof(*hints));

  // Process the `family` hint.
  duk_get_prop_string(ctx, 0, "family");
  if (duk_is_number(ctx, -1)) {
    hints->ai_family = duk_get_int(ctx, -1);
  }
  else if (duk_is_string(ctx, -1)) {
    hints->ai_family = duv_af_string_to_num(duk_get_string(ctx, -1));
  }
  else if (duk_is_null_or_undefined(ctx, -1)) {
    hints->ai_family = AF_UNSPEC;
  }
  else {
    duk_error(ctx, DUK_ERR_TYPE_ERROR, "family hint must be string if set");
  }
  duk_pop(ctx);

  // Process the `socktype` hint.
  duk_get_prop_string(ctx, 0, "socktype");
  if (duk_is_number(ctx, -1)) {
    hints->ai_socktype = duk_get_int(ctx, -1);
  }
  else if (duk_is_string(ctx, -1)) {
    hints->ai_socktype = duv_sock_string_to_num(duk_get_string(ctx, -1));
  }
  else if (!duk_is_null_or_undefined(ctx, -1)) {
    duk_error(ctx, DUK_ERR_TYPE_ERROR, "socktype hint must be string if set");
  }
  duk_pop(ctx);

  // Process the `protocol` hint
  duk_get_prop_string(ctx, 0, "protocol");
  if (duk_is_number(ctx, -1)) {
    hints->ai_protocol = duk_get_int(ctx, -1);
  }
  else if (duk_is_string(ctx, -1)) {
    int protocol = duv_af_string_to_num(duk_get_string(ctx, -1));
    if (protocol) {
      hints->ai_protocol = protocol;
    }
    else {
      duk_error(ctx, DUK_ERR_TYPE_ERROR, "Invalid protocol hint");
    }
  }
  else if (!duk_is_null_or_undefined(ctx, -1)) {
    duk_error(ctx, DUK_ERR_TYPE_ERROR, "protocol hint must be string if set");
  }
  duk_pop(ctx);

  duk_get_prop_string(ctx, 0, "addrconfig");
  if (duk_get_boolean(ctx, -1)) hints->ai_flags |=  AI_ADDRCONFIG;
  duk_pop(ctx);

  #ifdef AI_V4MAPPED
    duk_get_prop_string(ctx, 0, "v4mapped");
    if (duk_get_boolean(ctx, -1)) hints->ai_flags |=  AI_V4MAPPED;
    duk_pop(ctx);
  #endif

  #ifdef AI_ALL
    duk_get_prop_string(ctx, 0, "all");
    if (duk_get_boolean(ctx, -1)) hints->ai_flags |=  AI_ALL;
    duk_pop(ctx);
  #endif

  duk_get_prop_string(ctx, 0, "numerichost");
  if (duk_get_boolean(ctx, -1)) hints->ai_flags |=  AI_NUMERICHOST;
  duk_pop(ctx);

  duk_get_prop_string(ctx, 0, "passive");
  if (duk_get_boolean(ctx, -1)) hints->ai_flags |=  AI_PASSIVE;
  duk_pop(ctx);

  duk_get_prop_string(ctx, 0, "numericserv");
  if (duk_get_boolean(ctx, -1)) {
    hints->ai_flags |=  AI_NUMERICSERV;
    /* On OS X upto at least OSX 10.9, getaddrinfo crashes
     * if AI_NUMERICSERV is set and the servname is NULL or "0".
     * This workaround avoids a segfault in libsystem.
     */
    if (NULL == service) service = "00";
  }
  duk_pop(ctx);

  duk_get_prop_string(ctx, 0, "canonname");
  if (duk_get_boolean(ctx, -1)) hints->ai_flags |=  AI_CANONNAME;
  duk_pop(ctx);

  req = duk_push_fixed_buffer(ctx, sizeof(*req));
  //duv_setup_request(ctx, (uv_req_t*)req, 1);

 duv_check(ctx,
    uv_getaddrinfo(duv_loop(ctx), req, duv_on_addrinfo, node, service, hints)
  );

  req->data = duv_setup_req(ctx,1);

  return 1;
}

duk_ret_t duv_getnameinfo(duk_context *ctx) {
  dschema_check(ctx, (const duv_schema_entry[]) {
    {"options", duk_is_object},
    {"callback", dschema_is_continuation},
    {0,0}
  });

  struct sockaddr_storage addr;
  const char* ip = NULL;
  int flags = 0;
  int port = 0;

  memset(&addr, 0, sizeof(addr));

  duk_get_prop_string(ctx, 0, "ip");
  if (duk_is_string(ctx, -1)) {
    ip = duk_get_string(ctx, -1);
  }
  else if (!duk_is_null_or_undefined(ctx, -1)) {
    duk_error(ctx, DUK_ERR_TYPE_ERROR, "ip property must be string if set");
  }
  duk_pop(ctx);

  duk_get_prop_string(ctx, 0, "port");
  if (duk_is_number(ctx, -1)) {
    port = duk_get_int(ctx, -1);
  }
  else if (!duk_is_null_or_undefined(ctx, -1)) {
    duk_error(ctx, DUK_ERR_TYPE_ERROR, "port property must be integer if set");
  }
  duk_pop(ctx);

  if (ip || port) {
    if (!ip) ip = "0.0.0.0";
    if (!uv_ip4_addr(ip, port, (struct sockaddr_in*)&addr)) {
      addr.ss_family = AF_INET;
    }
    else if (!uv_ip6_addr(ip, port, (struct sockaddr_in6*)&addr)) {
      addr.ss_family = AF_INET6;
    }
    else {
      duk_error(ctx, DUK_ERR_TYPE_ERROR, "Invalid ip address or port");
    }
  }

  duk_get_prop_string(ctx, 0, "family");
  if (duk_is_number(ctx, -1)) {
    addr.ss_family = duk_get_int(ctx, -1);
  }
  else if (duk_is_string(ctx, -1)) {
    addr.ss_family = duv_af_string_to_num(duk_get_string(ctx, -1));
  }
  else if (!duk_is_null_or_undefined(ctx, -1)) {
    duk_error(ctx, DUK_ERR_TYPE_ERROR, "family must be string if set");
  }
  duk_pop(ctx);

  uv_getnameinfo_t* req = duk_push_fixed_buffer(ctx, sizeof(*req));
  //duv_setup_request(ctx, (uv_req_t*)req, 1);

  duv_check(ctx,
    uv_getnameinfo(duv_loop(ctx), req, duv_on_nameinfo, (const struct sockaddr *)&addr, flags)
  );

  req->data = duv_setup_req(ctx,1);
  return 1;
}
