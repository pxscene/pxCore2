/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

//
//  pxClipboardNative.cpp
//  pxScene
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <setjmp.h>
#include <signal.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "pxClipboardNative.h"

pxClipboardNative *pxClipboardNative::s_instance;

static Display           *display;
static Window             window;
static Window             root;

 // NOTE:  Lots of code inspired by ..  "xsel.c"
 //
 // https://raw.githubusercontent.com/kfish/xsel/master/xsel.c


//############################################################################################################
//############################################################################################################

#if 1

/* The name we were invoked as (argv[0]) */
//static char * progname;

/* Maxmimum request size supported by this X server */
// static long max_req;

/* Our timestamp for all operations */
static Time timestamp;

// static Atom timestamp_atom; /* The TIMESTAMP atom */
// static Atom multiple_atom; /* The MULTIPLE atom */
// static Atom targets_atom; /* The TARGETS atom */
static Atom delete_atom; /* The DELETE atom */
static Atom incr_atom; /* The INCR atom */
static Atom null_atom; /* The NULL atom */
static Atom text_atom; /* The TEXT atom */
static Atom utf8_atom; /* The UTF8 atom */
static Atom compound_text_atom; /* The COMPOUND_TEXT atom */

static long timeout = 0;
static struct itimerval timer;

#define USEC_PER_SEC 1000000


#define D_FATAL 0
#define D_WARN  1
#define D_INFO  2
#define D_OBSC  3
#define D_TRACE 4


/* Default debug level (ship at 0) */
#define DEBUG_LEVEL 0

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

#define empty_string(s) (s==NULL||s[0]=='\0')
#define free_string(s) { free(s); s=NULL; }

/* Maximum line length for error messages */
#define MAXLINE 4096

/* Maximum filename length */
#define MAXFNAME 1024

/* Maximum incremental selection size. (Ripped from Xt) */
#define MAX_SELECTION_INCR(dpy) (((65536 < XMaxRequestSize(dpy)) ? \
        (65536 << 2)  : (XMaxRequestSize(dpy) << 2))-100)



#define MAX_NUM_TARGETS 9
//static int NUM_TARGETS;

//static Atom supported_targets[MAX_NUM_TARGETS];

/*
 * print_debug (level, fmt)
 *
 * Print a formatted debugging message of level 'level' to stderr
 */
//#define print_debug(x,y...) {if (x <= debug_level) print_err (y);}
//#define print_debug printf


/*
 * get_atom_name (atom)
 *
 * Returns a string with a printable name for the Atom 'atom'.
 */
// static char *
// get_atom_name (Atom atom)
// {
//   char *ret;
//   static char atom_name[MAXLINE+1];

//   if (atom == None) return (char *) "None";
//   if (atom == XA_STRING) return (char *) "STRING";
//   if (atom == XA_PRIMARY) return (char *) "PRIMARY";
//   if (atom == XA_SECONDARY) return (char *) "SECONDARY";
//   if (atom == timestamp_atom) return (char *) "TIMESTAMP";
//   if (atom == multiple_atom) return (char *) "MULTIPLE";
//   if (atom == targets_atom) return (char *) "TARGETS";
//   if (atom == delete_atom) return (char *) "DELETE";
//   if (atom == incr_atom) return (char *) "INCR";
//   if (atom == null_atom) return (char *) "NULL";
//   if (atom == text_atom) return (char *) "TEXT";
//   if (atom == utf8_atom) return (char *) "UTF8_STRING";

//   ret = XGetAtomName (display, atom);
//   strncpy (atom_name, ret, sizeof (atom_name));
//   if (atom_name[MAXLINE] != '\0')
//     {
//       atom_name[MAXLINE-3] = '.';
//       atom_name[MAXLINE-2] = '.';
//       atom_name[MAXLINE-1] = '.';
//       atom_name[MAXLINE] = '\0';
//     }
//   XFree (ret);

//   return atom_name;
// }

#if 0
/*
 * The set of terminal signals we block while handling SelectionRequests.
 *
 * If we exit in the middle of handling a SelectionRequest, we might leave the
 * requesting client hanging, so we try to be nice and finish handling
 * requests before terminating.  Hence we block SIG{ALRM,INT,TERM} while
 * handling requests and unblock them only while waiting in XNextEvent().
 */
static sigset_t exit_sigs;

static void block_exit_sigs(void)
{
  sigprocmask (SIG_BLOCK, &exit_sigs, NULL);
}

static void unblock_exit_sigs(void)
{
  sigprocmask (SIG_UNBLOCK, &exit_sigs, NULL);
}
#endif //00


/* The jmp_buf to longjmp out of the signal handler */
static sigjmp_buf env_alrm;

/*
 * alarm_handler (sig)
 *
 * Signal handler for catching SIGALRM.
 */
static void
alarm_handler(int sig)
{
  siglongjmp(env_alrm, 1);
}



/*
 * debug_property (level, requestor, property, target, length)
 *
 * Print debugging information (at level 'level') about a property received.
 */
// static void
// debug_property (int level, Window requestor, Atom property, Atom target,
//                 unsigned long length)
// {
//   printf("Got window property: requestor 0x%x, property 0x%x, target 0x%x %s, length %ld bytes", requestor, property, target, get_atom_name (target), length);
// }

/*
 * exit_err (fmt)
 *
 * Print a formatted error message and errno information to stderr,
 * then exit with return code 1.
 */
 static void
 exit_err (const char * fmt, ...)
 {
   va_list ap;
   int errno_save;
   char buf[MAXLINE];
   int n = 0;

   errno_save = errno;

   va_start (ap, fmt);

// snprintf (buf, MAXLINE, "%s: ", progname);
// n = strlen (buf);

   vsnprintf (buf+n, MAXLINE-n, fmt, ap);
   n = strlen (buf);

   snprintf (buf+n, MAXLINE-n, ": %s\n", strerror (errno_save));

   fflush (stdout); /* in case stdout and stderr are the same */
   fputs (buf, stderr);
   fflush (NULL);

   va_end (ap);
   exit (1);
 }

#ifdef __GNUC__
__attribute__((noreturn)) static void
exit_err (const char * fmt, ...)
          __attribute__((nonnull(1)))
          __attribute__ ((__format__ (printf, 1, 2)));
#endif

/*
 * set_timer_timeout ()
 *
 * Set timer parameters according to specified timeout.
 */
static void
set_timer_timeout (void)
{
  timer.it_interval.tv_sec  = timeout / USEC_PER_SEC;
  timer.it_interval.tv_usec = timeout % USEC_PER_SEC;
  timer.it_value.tv_sec     = timeout / USEC_PER_SEC;
  timer.it_value.tv_usec    = timeout % USEC_PER_SEC;
}

/*
 * set_daemon_timeout ()
 *
 * Set up a timer to cause the daemon to exit after the desired
 * amount of time.
 */
// static void
// set_daemon_timeout (void)
// {
//   if (signal (SIGALRM, alarm_handler) == SIG_ERR)
//   {
//     exit_err ("error setting timeout handler");
//   }

//   set_timer_timeout();

//   if (sigsetjmp (env_alrm, 0) == 0)
//   {
//     setitimer (ITIMER_REAL, &timer, (struct itimerval *)0);
//   }
//   else
//   {
//     printf("daemon exiting after %d ms", timeout / 1000);
//     exit (0);
//   }
// }

/*
 * xs_malloc (size)
 *
 * Malloc wrapper. Always returns a successful allocation. Exits if the
 * allocation didn't succeed.
 */
static void *
xs_malloc (size_t size)
{
  void * ret;

  if (size == 0) size = 1;
  if ((ret = malloc (size)) == NULL)
  {
    exit_err ("malloc error");
  }

  return ret;
}

/*
 * xs_strdup (s)
 *
 * strdup wrapper for unsigned char *
 */
#define xs_strdup(s) ((unsigned char *) _xs_strdup ((const char *)s))
static char * _xs_strdup (const char * s)
{
  char * ret;

  if (s == NULL) return NULL;
  if ((ret = strdup(s)) == NULL)
  {
    exit_err ("strdup error");
  }

  return ret;
}

/*
 * xs_strlen (s)
 *
 * strlen wrapper for unsigned char *
 */
#define xs_strlen(s) (strlen ((const char *) s))

/*
 * xs_strncpy (s)
 *
 * strncpy wrapper for unsigned char *
 */
#define xs_strncpy(dest,s,n) (_xs_strncpy ((char *)dest, (const char *)s, n))
static char *
_xs_strncpy (char * dest, const char * src, size_t n)
{
  if (n > 0)
  {
    strncpy (dest, src, n);
    dest[n-1] = '\0';
  }
  return dest;
}


/*
 * get_timestamp ()
 *
 * Get the current X server time.
 *
 * This is done by doing a zero-length append to a random property of the
 * window, and checking the time on the subsequent PropertyNotify event.
 *
 * PRECONDITION: the window must have PropertyChangeMask set.
 */
static Time
get_timestamp (void)
{
  XEvent event;

  XChangeProperty (display, window, XA_WM_NAME, XA_STRING, 8,
                   PropModeAppend, NULL, 0);
  while (1)
  {
    XNextEvent (display, &event);

    if (event.type == PropertyNotify)
      return event.xproperty.time;
  }
}


/*
 * SELECTION RETRIEVAL
 * ===================
 *
 * The following functions implement retrieval of an X selection,
 * optionally within a user-specified timeout.
 *
 *
 * Selection timeout handling.
 * ---------------------------
 *
 * The selection retrieval can time out if no response is received within
 * a user-specified time limit. In order to ensure we time the entire
 * selection retrieval, we use an interval timer and catch SIGALRM.
 * [Calling select() on the XConnectionNumber would only provide a timeout
 * to the first XEvent.]
 */

/*
 * get_append_property ()
 *
 * Get a window property and append its data to a buffer at a given offset
 * pointed to by *offset. 'offset' is modified by this routine to point to
 * the end of the data.
 *
 * Returns True if more data is available for receipt.
 *
 * If an error is encountered, the buffer is free'd.
 */

static Bool
get_append_property (XSelectionEvent * xsl, unsigned char ** buffer,
                     unsigned long * offset, unsigned long * alloc)
{
  unsigned char * ptr;
  Atom target;
  int format;
  unsigned long bytesafter, length;
  unsigned char * value;

  XGetWindowProperty (xsl->display, xsl->requestor, xsl->property,
                      0L, 1000000, True, (Atom)AnyPropertyType,
                      &target, &format, &length, &bytesafter, &value);

 // debug_property (D_TRACE, xsl->requestor, xsl->property, target, length);

  if (target != XA_STRING && target != utf8_atom)
  {
    // printf("target %s not XA_STRING nor UTF8_STRING in get_append_property()",
    //              get_atom_name (target));
    free (*buffer);
    *buffer = NULL;
    return False;
  }
  else if (length == 0)
  {
    /* A length of 0 indicates the end of the transfer */
    printf("Got zero length property; end of INCR transfer");
    return False;
  } else if (format == 8)
  {
    if (*offset + length > *alloc)
    {
      *alloc = *offset + length;
      if ((*buffer = (unsigned char *) realloc (*buffer, *alloc)) == NULL)
      {
        exit_err ("realloc error");
      }
    }
    ptr = *buffer + *offset;
    xs_strncpy (ptr, value, length);
    *offset += length;
//    printf("Appended %ld bytes to buffer\n", length);
  }
  else
  {
//    printf("Retrieved non-8-bit data\n");
  }

  return True;
}

/*
 * wait_incr_selection (selection)
 *
 * Retrieve a property of target type INCR. Perform incremental retrieval
 * and return the resulting data.
 */
static unsigned char *
wait_incr_selection (Atom selection, XSelectionEvent * xsl, int init_alloc)
{
  XEvent event;
  //unsigned char *incr_base = NULL, *incr_ptr = NULL;
  unsigned char *incr_base = NULL;
  unsigned long incr_alloc = 0, incr_xfer = 0;
  Bool wait_prop = True;

 // printf("Initialising incremental retrieval of at least %d bytes\n", init_alloc);

  /* Take an interest in the requestor */
  XSelectInput (xsl->display, xsl->requestor, PropertyChangeMask);

  incr_alloc = init_alloc;
  incr_base = (unsigned char *) xs_malloc (incr_alloc);
  //incr_ptr = incr_base;

//  printf("Deleting property that informed of INCR transfer");
  XDeleteProperty (xsl->display, xsl->requestor, xsl->property);

//  printf("Waiting on PropertyNotify events");
  while (wait_prop)
  {
    XNextEvent (xsl->display, &event);

    switch (event.type)
    {
    case PropertyNotify:
      if (event.xproperty.state != PropertyNewValue) break;

      wait_prop = get_append_property (xsl, &incr_base, &incr_xfer,
                                       &incr_alloc);
      break;
    default:
      break;
    }
  }

  /* when zero length found, finish up & delete last */
  XDeleteProperty (xsl->display, xsl->requestor, xsl->property);

//  printf("Finished INCR retrieval");

  return incr_base;
}

/*
 * wait_selection (selection, request_target)
 *
 * Block until we receive a SelectionNotify event, and return its
 * contents; or NULL in the case of a deletion or error. This assumes we
 * have already called XConvertSelection, requesting a string (explicitly
 * XA_STRING) or deletion (delete_atom).
 */
static unsigned char *
wait_selection (Atom selection, Atom request_target)
{
  XEvent event;
  Atom target;
  int format;
  unsigned long bytesafter, length;
  unsigned char *value, *retval = NULL;
  Bool keep_waiting = True;

  while (keep_waiting)
  {
    XNextEvent (display, &event);

    switch (event.type)
    {
    case SelectionNotify:
      if (event.xselection.selection != selection) break;

      if (event.xselection.property == None)
      {
//        printf("Conversion refused");
        value = NULL;
        keep_waiting = False;
      }
      else if (event.xselection.property == null_atom &&
                 request_target == delete_atom)
      {
      }
      else
      {
        XGetWindowProperty (event.xselection.display,
                    event.xselection.requestor,
                    event.xselection.property, 0L, 1000000,
                    False, (Atom)AnyPropertyType, &target,
                    &format, &length, &bytesafter, &value);

        // debug_property (D_TRACE, event.xselection.requestor,
        //                 event.xselection.property, target, length);

        if (request_target == delete_atom && value == NULL)
        {
          keep_waiting = False;
        }
        else if (target == incr_atom)
        {
          /* Handle INCR transfers */
          retval = wait_incr_selection (selection, &event.xselection,
                                        *(int *)value);
          keep_waiting = False;
        }
        else if (target != utf8_atom && target != XA_STRING &&
                 target != compound_text_atom &&
                 request_target != delete_atom)
        {
          /* Report non-TEXT atoms */
        //   printf("Selection (type %s) is not a string.",
        //                get_atom_name (target));
          free (retval);
          retval = NULL;
          keep_waiting = False;
        }
        else
        {
          retval = xs_strdup (value);
          XFree (value);
          keep_waiting = False;
        }

        XDeleteProperty (event.xselection.display,
                         event.xselection.requestor,
                         event.xselection.property);
      }
      break;
    default:
      break;
    }
  }

  /* Now that we've received the SelectionNotify event, clear any
   * remaining timeout. */
  if (timeout > 0)
  {
    setitimer (ITIMER_REAL, (struct itimerval *)0, (struct itimerval *)0);
  }

  return retval;
}

/*
 * get_selection (selection, request_target)
 *
 * Retrieves the specified selection and returns its value.
 *
 * If a non-zero timeout is specified then set a virtual interval
 * timer. Return NULL and print an error message if the timeout
 * expires before the selection has been retrieved.
 */
static unsigned char *
get_selection(Atom selection, Atom request_target)
{
  Atom prop;
  unsigned char * retval;

  prop = XInternAtom(display, "XSEL_DATA", False);
  XConvertSelection(display, selection, request_target, prop, window,
                     timestamp);
  XSync (display, False);

  if (timeout > 0)
  {
    if (signal (SIGALRM, alarm_handler) == SIG_ERR)
    {
      printf ("error setting timeout handler");
      exit(-1);
    }

    set_timer_timeout();

    if (sigsetjmp (env_alrm, 0) == 0)
    {
      setitimer(ITIMER_REAL, &timer, (struct itimerval *)0);
      retval = wait_selection(selection, request_target);
    }
    else
    {
      printf("selection timed out");
      retval = NULL;
    }
  }
  else
  {
    retval = wait_selection(selection, request_target);
  }

  return retval;
}

/*
 * get_selection_text (Atom selection)
 *
 * Retrieve a text selection. First attempt to retrieve it as UTF_STRING,
 * and if that fails attempt to retrieve it as a plain XA_STRING.
 *
 * NB. Before implementing this, an attempt was made to query TARGETS and
 * request UTF8_STRING only if listed there, as described in:
 * http://www.pps.jussieu.fr/~jch/software/UTF8_STRING/UTF8_STRING.text
 * However, that did not seem to work reliably when tested against various
 * applications (eg. Mozilla Firefox). This method is of course more
 * reliable.
 */
static unsigned char *
get_selection_text (Atom selection)
{
  unsigned char * retval;

  if ((retval = get_selection(selection, utf8_atom)) == NULL)
  {
    retval = get_selection(selection, XA_STRING);
  }

  return retval;
}


#if 0

/*
 * change_property (display, requestor, property, target, format, mode,
 *                  data, nelements)
 *
 * Wrapper to XChangeProperty that performs INCR transfer if required and
 * returns status of entire transfer.
 */
static HandleResult
change_property (Display * display, Window requestor, Atom property,
                 Atom target, int format, int mode,
                 unsigned char * data, int nelements,
                 Atom selection, Time time, MultTrack * mparent)
{
  XSelectionEvent ev;
  int nr_bytes;
  //IncrTrack * it;

 // printf("change_property ()");

 printf("\n\n####  change_property() - sel = %s   [len=%d] \n\n", data,nelements);

  nr_bytes = nelements * format / 8;

//   if (nr_bytes <= max_req)
//   {
//     printf("data within maximum request size");

//     printf("\n\n####  change_property() - XChangeProperty()\n\n");

//     XChangeProperty(display, requestor, property, target, format, mode,
//                      data, nelements);

//     return HANDLE_OK;
//   }

  /* else */
  printf("large data transfer");

printf("\n\n####  change_property() - Send a SelectionNotify\n\n");

  /* Send a SelectionNotify event of type INCR */
  ev.type      = SelectionNotify;
  ev.display   = display;
  ev.requestor = requestor;
  ev.selection = selection;
  ev.time      = time;
  ev.target    = incr_atom; /* INCR */
  ev.property  = property;

  XSelectInput (ev.display, ev.requestor, PropertyChangeMask);

  XChangeProperty (ev.display, ev.requestor, ev.property, incr_atom, 32,
                   PropModeReplace, (unsigned char *)&nr_bytes, 1);

  XSendEvent (display, requestor, False,
              (unsigned long)NULL, (XEvent *)&ev);

#if 0
  /* Set up the IncrTrack to track this */
  it = fresh_incrtrack();

  it->mparent   = mparent;
  it->state     = S_INCR_1;
  it->display   = display;
  it->requestor = requestor;
  it->property  = property;
  it->selection = selection;
  it->time      = time;
  it->target    = target;
  it->format    = format;
  it->data      = data;
  it->nelements = nelements;
  it->offset    = 0;

  /* Maximum nr. of elements that can be transferred in one go */
  it->max_elements = max_req * 8 / format;

  /* Nr. of elements to transfer in this instance */
  it->chunk = MIN (it->max_elements, it->nelements - it->offset);

  /* Wait for that property to get deleted */
  printf("Waiting on intial property deletion (%s)",
               get_atom_name (it->property));
#endif
  return HANDLE_INCOMPLETE;
}


/*
 * handle_utf8_string (display, requestor, property, sel)
 *
 * Handle a UTF8_STRING request; setting 'sel' as the data
 */
static HandleResult
handle_utf8_string (Display * display, Window requestor, Atom property,
                    unsigned char * sel, Atom selection, Time time,
                    MultTrack * mparent)
{
  return
    change_property (display, requestor, property, utf8_atom, 8,
                     PropModeReplace, sel, xs_strlen(sel),
                     selection, time, mparent);
}

/*
 * handle_delete (display, requestor, property)
 *
 * Handle a DELETE request.
 */
static HandleResult
handle_delete (Display * display, Window requestor, Atom property)
{
  XChangeProperty (display, requestor, property, null_atom, 0,
                   PropModeReplace, NULL, 0);

  return DID_DELETE;
}


/*
 * handle_selection_request (event, sel)
 *
 * Processes a SelectionRequest event 'event' and replies to its
 * sender appropriately, eg. with the contents of the string 'sel'.
 * Returns False if a DELETE request is processed, indicating to
 * the calling function to delete the corresponding selection.
 * Returns True otherwise.
 */

static Bool
handle_selection_request (XEvent event, unsigned char * sel)
{
  XSelectionRequestEvent * xsr = &event.xselectionrequest;
  XSelectionEvent ev;
  HandleResult hr = HANDLE_OK;
  Bool retval = True;

  print_debug (D_TRACE, "handle_selection_request, property=0x%x (%s), target=0x%x (%s)",
               xsr->property, get_atom_name (xsr->property),
               xsr->target, get_atom_name (xsr->target));

  /* Prepare a SelectionNotify event to send, either as confirmation of
   * placing the selection in the requested property, or as notification
   * that this could not be performed. */
  ev.type      = SelectionNotify;
  ev.display   = xsr->display;
  ev.requestor = xsr->requestor;
  ev.selection = xsr->selection;
  ev.time      = xsr->time;
  ev.target    = xsr->target;

  if (xsr->property == None && ev.target != multiple_atom) {
      /* Obsolete requestor */
      xsr->property = xsr->target;
  }

  if (ev.time != CurrentTime && ev.time < timestamp) {
    /* If the time is outside the period we have owned the selection,
     * which is any time later than timestamp, or if the requested target
     * is not a string, then refuse the SelectionRequest. NB. Some broken
     * clients don't set a valid timestamp, so we have to check against
     * CurrentTime here. */
    ev.property = None;
  } else if (ev.target == timestamp_atom) {
    /* Return timestamp used to acquire ownership if target is TIMESTAMP */
    ev.property = xsr->property;
    hr = handle_timestamp (ev.display, ev.requestor, ev.property,
                           ev.selection, ev.time, NULL);
  } else if (ev.target == targets_atom) {
    /* Return a list of supported targets (TARGETS)*/
    ev.property = xsr->property;
    hr = handle_targets (ev.display, ev.requestor, ev.property,
                         ev.selection, ev.time, NULL);
  } else if (ev.target == multiple_atom) {
    if (xsr->property == None) { /* Invalid MULTIPLE request */
      ev.property = None;
    } else {
      /* Handle MULTIPLE request */
      hr = handle_multiple (ev.display, ev.requestor, ev.property, sel,
                            ev.selection, ev.time, NULL);
    }
  } else if (ev.target == XA_STRING || ev.target == text_atom)
  {
    /* Received STRING or TEXT request */
    ev.property = xsr->property;
    hr = handle_string (ev.display, ev.requestor, ev.property, sel,
                        ev.selection, ev.time, NULL);
  } else if (ev.target == utf8_atom)
  {
    /* Received UTF8_STRING request */
    ev.property = xsr->property;
    hr = handle_utf8_string (ev.display, ev.requestor, ev.property, sel,
                             ev.selection, ev.time, NULL);
  } else if (ev.target == delete_atom) {
    /* Received DELETE request */
    ev.property = xsr->property;
    hr = handle_delete (ev.display, ev.requestor, ev.property);
    retval = False;
  } else {
    /* Cannot convert to requested target. This includes most non-string
     * datatypes, and INSERT_SELECTION, INSERT_PROPERTY */
    ev.property = None;
  }

  /* Return False if a DELETE was processed */
  retval = (hr & DID_DELETE) ? False : True;

  /* If there was an error in the transfer, it should be refused */
  if (hr & HANDLE_ERR) {
    print_debug (D_TRACE, "Error in transfer");
    ev.property = None;
  }

  if ((hr & HANDLE_INCOMPLETE) == 0) {
    if (ev.property == None) {print_debug (D_TRACE, "Refusing conversion");}
    else { print_debug (D_TRACE, "Confirming conversion");}

    XSendEvent (display, ev.requestor, False,
                (unsigned long)NULL, (XEvent *)&ev);

    /* If we return False here, we may quit immediately, so sync out the
     * X queue. */
    if (!retval) XSync (display, False);
  }

  return retval;
}

/*
 * set_selection (selection, sel)
 *
 * Takes ownership of the selection 'selection', then loops waiting for
 * its SelectionClear or SelectionRequest events.
 *
 * Handles SelectionRequest events, first checking for additional
 * input if the user has specified 'follow' mode. Returns when a
 * SelectionClear event is received for the specified selection.
 */
static void
set_selection(Atom selection, unsigned char *sel)
{
  XEvent event;
 // IncrTrack * it;

 // if (own_selection (selection) == False) return;

 // for (;;)
  {
    /* Flush before unblocking signals so we send replies before exiting */
    // XFlush (display);
    // unblock_exit_sigs();
    // XNextEvent(display, &event);
    // block_exit_sigs();

//event.type = SelectionRequest;

    switch (event.type)
    {
      case SelectionClear:
        if (event.xselectionclear.selection == selection) return;
        break;

      case SelectionRequest:
        if (event.xselectionrequest.selection != selection) break;

       // if (do_follow)
       //   sel = (unsigned char *) "My New Selection"; // read_input (sel, True);

       // if (!handle_selection_request (event, sel)) return;

        break;
    case PropertyNotify:

        printf("\nTODO:  skip PropertyNotify !!!");

    //   if (event.xproperty.state != PropertyDelete) break;

    //   it = find_incrtrack (event.xproperty.atom);

    //   if (it != NULL)
    //   {
    //     continue_incr (it);
    //   }

      break;
    default:
      break;
    }
  }
}

/*
 * set_selection__daemon (selection, sel)
 *
 * Creates a daemon process to handle selection requests for the
 * specified selection 'selection', to respond with selection text 'sel'.
 * If 'sel' is an empty string (NULL or "") then no daemon process is
 * created and the specified selection is cleared instead.
 */
static void
set_selection__daemon (Atom selection, unsigned char * sel)
{
//   if (empty_string (sel) && !do_follow)
//   {
//     clear_selection (selection);
//     return;
//   }

  //become_daemon ();

  set_daemon_timeout();

  set_selection(selection, sel);
}
#endif //0

#endif

//############################################################################################################
//############################################################################################################

pxClipboardNative::pxClipboardNative()
{
}


std::string pxClipboardNative::getString(std::string type)
{
  char *display_name = NULL;

  display = XOpenDisplay(display_name);
  if (display==NULL)
  {
    printf("\n ERROR: Can't open display: %s\n", display_name);
    exit(-1);
  }

  root = XDefaultRootWindow(display);

  /* Create an unmapped window for receiving events */
  int black = BlackPixel(display, DefaultScreen (display));

  window = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, black, black);

  /* Get a timestamp */
  XSelectInput(display, window, PropertyChangeMask);
  timestamp = get_timestamp();

  Atom selection = XInternAtom(display, "CLIPBOARD", False);

  /* Get the UTF8_STRING atom */
  utf8_atom = XInternAtom (display, "UTF8_STRING", True);
  if(utf8_atom == None)
  {
    utf8_atom = XA_STRING;
  }

  unsigned char *sel = get_selection_text(selection);

  // Tidy up
  XDestroyWindow(display, window);
  XCloseDisplay(display);

  if (NULL == sel)
  {
    return std::string("");
  }
  return std::string((const char*) sel);
}

void pxClipboardNative::setString(std::string type, std::string clip)
{
  printf("pxClipboardNative::setString() - ENTER\n");

  char *display_name = NULL;
  long timeout_ms = 0L;
  timeout = timeout_ms * 1000;

  display = XOpenDisplay(display_name);
  if (display==NULL)
  {
    printf("Can't open display: %s\n", display_name);
    exit(-1);
  }

  root = XDefaultRootWindow(display);

  /* Create an unmapped window for receiving events */
  int black = BlackPixel(display, DefaultScreen (display));

  window = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, black, black);

  /* Get a timestamp */
  XSelectInput(display, window, PropertyChangeMask);
  timestamp = get_timestamp();

  /* Get the TEXT atom */
  text_atom = XInternAtom (display, "TEXT", False);

  /* Get the UTF8_STRING atom */
  utf8_atom = XInternAtom (display, "UTF8_STRING", True);
  if(utf8_atom == None)
  {
    utf8_atom = XA_STRING;
  }

//    Atom XA_UTF8      = XInternAtom(display, "UTF8",      0);
//    Atom XA_UNICODE   = XInternAtom(display, "UNICODE",   0);
   Atom XA_CLIPBOARD = XInternAtom(display, "CLIPBOARD", 0);

  printf("\n\n####  pxClipboardNative::setString() - sel = %s  \n\n", clip.c_str());

 // set_selection(selection, (unsigned char*) clip.c_str());

// change_property (display, window, /*property*/XA_ATOM, utf8_atom, 8,
//                      PropModeReplace,
//                      (unsigned char*) clip.c_str(),
//                      xs_strlen(clip.c_str()),
//                      selection, get_timestamp(), NULL);

// XChangeProperty(display, RootWindow(display, 0), XA_CLIPBOARD, XA_UTF8, 8,
//                 PropModeReplace,
//                 (const unsigned char*) clip.c_str(),
//                 xs_strlen(clip.c_str()) );

    XStoreBytes(display,
    (const char*) clip.c_str(),
        xs_strlen( clip.c_str() ) );


   XChangeProperty(display,
     DefaultRootWindow(display),
     XA_CLIPBOARD,
     XA_STRING, 8, PropModeReplace,
     (unsigned const char *) clip.c_str(),
        xs_strlen( clip.c_str() ) );

    if (XGetSelectionOwner(display, XA_PRIMARY) != window)
    {
      XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
    }

    //property = XInternAtom(dpy, "PASTE", 0);
//    XSetSelectionOwner(display, selection, window, 0);

//  set_selection__daemon(selection, (unsigned char*) clip.c_str());
}
