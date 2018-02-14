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

static void handleError(char* message)
{
    fprintf(stderr, "%s\n",message);
    fflush(stderr);
    if (client_sock >= 0) {
      (void) close(client_sock);
      client_sock = -1;
    }
}

static void handleInitError(char* message)
{
    fprintf(stderr, "%s\n",message);
    fflush(stderr);
    if (server_sock >= 0) {
      (void) close(server_sock);
      server_sock = -1;
    }

}

static duk_size_t duk_trans_socket_read_cb(void *udata, char *buffer, duk_size_t length) {
	ssize_t ret;

	(void) udata;

#if defined(DUK_DEBUG_PRINTS)
	fprintf(stderr, "%s: udata=%p, buffer=%p, length=%ld\n",
	        __func__, (void *) udata, (void *) buffer, (long) length);
	fflush(stderr);
#endif

	if (client_sock < 0) {
		return 0;
	}

	if (length == 0) {
                handleError("read request length == 0, closing connection");
		return 0;
	}

	if (buffer == NULL) {
		/* This shouldn't happen. */
		handleError("read request buffer == NULL, closing connection\n");
		return 0;
	}

	ret = read(client_sock, (void *) buffer, (size_t) length);
	if (ret < 0) {
		handleError("debug read failed, ret < 0");
		return 0;
	} else if (ret == 0) {
		handleError("debug read failed, ret == 0");
		return 0;
	} else if (ret > (ssize_t) length) {
		handleError("debug read failed, ret > buffer length");
		return 0;
	}

	return (duk_size_t) ret;
}

static duk_size_t duk_trans_socket_write_cb(void *udata, const char *buffer, duk_size_t length) {
	ssize_t ret;

	(void) udata;

#if defined(DUK_DEBUG_PRINTS)
	fprintf(stderr, "%s: udata=%p, buffer=%p, length=%ld\n",
	        __func__, (void *) udata, (const void *) buffer, (long) length);
	fflush(stderr);
#endif

	if (client_sock < 0) {
		return 0;
	}

	if (length == 0) {
		/* This shouldn't happen. */
		handleError("write request length == 0, closing connection\n");
		return 0;
	}

	if (buffer == NULL) {
		/* This shouldn't happen. */
		handleError("write request buffer == NULL, closing connection\n");
		return 0;
	}

	ret = write(client_sock, (const void *) buffer, (size_t) length);
	if (ret <= 0 || ret > (ssize_t) length) {
		handleError("write failed, closing connection\n");
		return 0;
	}

	return (duk_size_t) ret;
}

static duk_size_t duk_trans_socket_peek_cb(void *udata) {
	struct pollfd fds[1];
	int poll_rc;

	(void) udata;  /* not needed by the example */

#if defined(DUK_DEBUG_PRINTS)
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
                handleError("poll returned < 0, closing connection");
		return 0;
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
}

void* duk_trans_socket_waitconn(void* context) {
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
		handleError("accept() failed, skip waiting for connection");
		return NULL;
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
          handleInitError("failed to create server socket");
	  return;
	}

	on = 1;
	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on)) < 0) {
                handleInitError("failed to set SO_REUSEADDR for server socket");
                return;
	}

	memset((void *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(debug_port);

	if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                handleInitError("failed to bind server socket");
                return;
	}

	listen(server_sock, 1 /*backlog*/);

        if  (1 == pauseOnStart)
        {
          duk_trans_socket_waitconn(ctx);
        }
        else
        {
          pthread_t accept_thread_id;
          if (pthread_create( &accept_thread_id, NULL, duk_trans_socket_waitconn, ctx) < 0)
          {
            handleInitError("Thread creation error");
          }
        }
	return;
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
