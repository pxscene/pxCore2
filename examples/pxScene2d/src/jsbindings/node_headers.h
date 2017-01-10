#ifndef NODE_HEADERS_H
#define NODE_HEADERS_H

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <v8-util.h>
#include <uv.h>
#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

#ifdef WIN32
#define USE_STD_THREADS
#include <mutex>
#include <condition_variable>
#endif

#endif

