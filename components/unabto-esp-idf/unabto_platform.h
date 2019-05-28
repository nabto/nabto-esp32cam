#include "unabto_platform_types.h"

#include <platforms/unabto_common_types.h>
#include <modules/log/unabto_log_header.h>
#include <stdio.h>



#define NABTO_INVALID_SOCKET -1


//#ifndef NABTO_LOG_SEVERITY_FILTER
//#define NABTO_LOG_SEVERITY_FILTER   NABTO_LOG_SEVERITY_LEVEL_TRACE 
//#endif


//#define NABTO_LOG_BASIC_PRINT(severity, message)

#ifndef NABTO_LOG_BASIC_PRINT
/** Print debugging info.
 * @param loglevel  logging level
 * @param cmsg      the message
 */
#define NABTO_LOG_BASIC_PRINT(loglevel, cmsg) do {    \
    unabto_log_header(__FILE__, __LINE__);             \
    printf cmsg;                                      \
    printf("\n");                                     \
    fflush(stdout);                                   \
} while(0)
#endif

