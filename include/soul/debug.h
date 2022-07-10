#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>

#include "typedefs.h"

typedef int severity_t;
#define SEVERITY_VERBOSE    0
#define SEVERITY_WARNING    1
#define SEVERITY_ERROR      2

extern bool_t(*debug_host_handler)(severity_t severity, const char *message);

void __debug_log(const char *file,
                 const char *function,
                 severity_t severity,
                 const char *format,
                 ...);

#define debug_log(severity, format, ...) __debug_log(__FILE__, __func__, severity, format, ##__VA_ARGS__)

#endif // DEBUG_H