#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include "duktape.h"
#include "duk_debugger_interface.h"
#include <pthread.h>

#define DUK_DEBUG_PORT 9091

static int server_sock = -1;
static int client_sock = -1;
static int debug_port = DUK_DEBUG_PORT;


/*
 *  Duktape callbacks
 */

/* Duktape debug transport callback: (possibly partial) read. */
duk_size_t duk_trans_socket_read_cb(void *udata, char *buffer, duk_size_t length) {
	ssize_t ret;

	(void) udata;

#if defined(DEBUG_PRINTS)
	fprintf(stderr, "%s: udata=%p, buffer=%p, length=%ld\n",
	        __func__, (void *) udata, (void *) buffer, (long) length);
	fflush(stderr);
#endif

	if (client_sock < 0) {
		return 0;
	}

	if (length == 0) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: read request length == 0, closing connection\n",
		        __FILE__);
		fflush(stderr);
		goto fail;
	}

	if (buffer == NULL) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: read request buffer == NULL, closing connection\n",
		        __FILE__);
		fflush(stderr);
		goto fail;
	}

	/* In a production quality implementation there would be a sanity
	 * timeout here to recover from "black hole" disconnects.
	 */

	ret = read(client_sock, (void *) buffer, (size_t) length);
	if (ret < 0) {
		fprintf(stderr, "%s: debug read failed, closing connection: %s\n",
		        __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	} else if (ret == 0) {
		fprintf(stderr, "%s: debug read failed, ret == 0 (EOF), closing connection\n",
		        __FILE__);
		fflush(stderr);
		goto fail;
	} else if (ret > (ssize_t) length) {
		fprintf(stderr, "%s: debug read failed, ret too large (%ld > %ld), closing connection\n",
		        __FILE__, (long) ret, (long) length);
		fflush(stderr);
		goto fail;
	}

	return (duk_size_t) ret;

 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
	return 0;
}

/* Duktape debug transport callback: (possibly partial) write. */
duk_size_t duk_trans_socket_write_cb(void *udata, const char *buffer, duk_size_t length) {
	ssize_t ret;

	(void) udata;

#if defined(DEBUG_PRINTS)
	fprintf(stderr, "%s: udata=%p, buffer=%p, length=%ld\n",
	        __func__, (void *) udata, (const void *) buffer, (long) length);
	fflush(stderr);
#endif

	if (client_sock < 0) {
		return 0;
	}

	if (length == 0) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: write request length == 0, closing connection\n",
		        __FILE__);
		fflush(stderr);
		goto fail;
	}

	if (buffer == NULL) {
		/* This shouldn't happen. */
		fprintf(stderr, "%s: write request buffer == NULL, closing connection\n",
		        __FILE__);
		fflush(stderr);
		goto fail;
	}

	/* In a production quality implementation there would be a sanity
	 * timeout here to recover from "black hole" disconnects.
	 */

	ret = write(client_sock, (const void *) buffer, (size_t) length);
	if (ret <= 0 || ret > (ssize_t) length) {
		fprintf(stderr, "%s: debug write failed, closing connection: %s\n",
		        __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	return (duk_size_t) ret;

 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
	return 0;
}

duk_size_t duk_trans_socket_peek_cb(void *udata) {
	struct pollfd fds[1];
	int poll_rc;

	(void) udata;  /* not needed by the example */

#if defined(DEBUG_PRINTS)
	fprintf(stderr, "%s: udata=%p\n", __func__, (void *) udata);
	fflush(stderr);
#endif

	if (client_sock < 0) {
		return 0;
	}
	fds[0].fd = client_sock;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	poll_rc = poll(fds, 1, 0);
	if (poll_rc < 0) {
		fprintf(stderr, "%s: poll returned < 0, closing connection: %s\n",
		        __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;  /* also returns 0, which is correct */
	} else if (poll_rc > 1) {
		fprintf(stderr, "%s: poll returned > 1, treating like 1\n",
		        __FILE__);
		fflush(stderr);
		return 1;  /* should never happen */
	} else if (poll_rc == 0) {
		return 0;  /* nothing to read */
	} else {
		return 1;  /* something to read */
	}
 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
	return 0;
}

void duk_trans_socket_waitconn(void* context) {
	struct sockaddr_in addr;
	socklen_t sz;

	if (server_sock < 0) {
		fprintf(stderr, "%s: no server socket, skip waiting for connection\n",
		        __FILE__);
		fflush(stderr);
		return NULL;
	}
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}

	fprintf(stderr, "Waiting for debug connection on port %d\n", (int) debug_port);
	fflush(stderr);

	sz = (socklen_t) sizeof(addr);

	client_sock = accept(server_sock, (struct sockaddr *) &addr, &sz);
	if (client_sock < 0) {
		fprintf(stderr, "%s: accept() failed, skip waiting for connection: %s\n",
		        __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	fprintf(stderr, "Debug connection established\n");
	fflush(stderr);

	/* XXX: For now, close the listen socket because we won't accept new
	 * connections anyway.  A better implementation would allow multiple
	 * debug attaches.
	 */

	duk_debugger_attach((duk_context*)context,
                        duk_trans_socket_read_cb,
                        duk_trans_socket_write_cb,
                        duk_trans_socket_peek_cb,
                          NULL,NULL,NULL,NULL, NULL);

	duk_debugger_pause((duk_context*)context);

	if (server_sock >= 0) {
		(void) close(server_sock);
		server_sock = -1;
	}

	return NULL;

 fail:
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
  return NULL;
}

void *duk_trans_socket_connection_handler(void *context)
{
        struct sockaddr_in addr;
        socklen_t sz;

        if (server_sock < 0) {
                fprintf(stderr, "%s: no server socket, skip waiting for connection\n",
                        __FILE__);
                fflush(stderr);
                return NULL;
        }
        if (client_sock >= 0) {
                (void) close(client_sock);
                client_sock = -1;
        }

        fprintf(stderr, "Waiting for debug connection on port %d\n", (int) debug_port);
        fflush(stderr);

        sz = (socklen_t) sizeof(addr);

        client_sock = accept(server_sock, (struct sockaddr *) &addr, &sz);
        if (client_sock < 0) {
                fprintf(stderr, "%s: accept() failed, skip waiting for connection: %s\n",
                        __FILE__, strerror(errno));
                fflush(stderr);
                goto fail;
        }

        fprintf(stderr, "Debug connection established\n");
        fflush(stderr);

        /* XXX: For now, close the listen socket because we won't accept new
         * connections anyway.  A better implementation would allow multiple
         * debug attaches.
         */

	duk_debugger_attach((duk_context*)context,
                        duk_trans_socket_read_cb,
                        duk_trans_socket_write_cb,
                        duk_trans_socket_peek_cb,
                          NULL,NULL,NULL,NULL, NULL);

	duk_debugger_pause((duk_context*)context);

        if (server_sock >= 0) {
                (void) close(server_sock);
                server_sock = -1;
        }

        return NULL;

 fail:
        if (client_sock >= 0) {
                (void) close(client_sock);
                client_sock = -1;
        }
		return NULL;
}

void duk_debugger_init(void *ctx, duk_bool_t pauseOnStart) {

        //finalize port
        char const *port = getenv("DUKTAPE_DEBUGGER_PORT");
        if (port)
        {
          debug_port = atoi(port);
        }

	struct sockaddr_in addr;
	int on;

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0) {
		fprintf(stderr, "%s: failed to create server socket: %s\n",
		        __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	on = 1;
	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on)) < 0) {
		fprintf(stderr, "%s: failed to set SO_REUSEADDR for server socket: %s\n",
		        __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	memset((void *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(debug_port);

	if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		fprintf(stderr, "%s: failed to bind server socket: %s\n",
		        __FILE__, strerror(errno));
		fflush(stderr);
		goto fail;
	}

	listen(server_sock, 1 /*backlog*/);

        if  (1 == pauseOnStart)
        {
          duk_trans_socket_waitconn(ctx);
        }
        else
        {
          pthread_t accept_thread_id;
          if (pthread_create( &accept_thread_id , NULL ,  duk_trans_socket_connection_handler , ctx) < 0)
            goto fail;
        }
	return;

 fail:
	if (server_sock >= 0) {
		(void) close(server_sock);
		server_sock = -1;
	}
}

void duk_debugger_finish(void* context) {
	if (client_sock >= 0) {
		(void) close(client_sock);
		client_sock = -1;
	}
	if (server_sock >= 0) {
		(void) close(server_sock);
		server_sock = -1;
	}
        duk_debugger_detach((duk_context*)context);
}
