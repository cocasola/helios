#include <soul/debug.h>

bool_t(*debug_host_handler)(severity_t severity, const char *message) = 0;

void __debug_log(const char *file,
                 const char *function,
                 severity_t severity,
                 const char *format,
                 ...)
{
    char buffer_a[4096];
    buffer_a[0] = 0;

    if (severity > SEVERITY_VERBOSE)
    {
        const char *severity_string = (severity == SEVERITY_ERROR) ? "ERROR" : "WARNING";

        sprintf(buffer_a, "%s at %s in %s:\n\t", severity_string, function, file);
    }

    char buffer_b[4096];
    buffer_b[0] = 0;
 
    va_list argptr;
    va_start(argptr, format);

    vsprintf(buffer_b, format, argptr);

    va_end(argptr);

    char buffer_c[4096];
    buffer_c[0] = 0;

    sprintf(buffer_c, "%s%s", buffer_a, buffer_b);

    if (debug_host_handler)
        debug_host_handler(severity, buffer_c);
    else
        printf(buffer_c);
}