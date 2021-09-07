#ifndef LOG_UTILS_H
#define LOG_UTILS_H

#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <string.h>

#include "EnclaveVerifier_t.h"  /* print_string */

void printf(const char *fmt, ...);
#endif