#ifndef _UNABTO_PLATFORM_TYPES_H_
#define _UNABTO_PLATFORM_TYPES_H_


#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include <inttypes.h>



/** The timestamp definition */
typedef portTickType nabto_stamp_t;
typedef nabto_stamp_t nabto_stamp_diff_t;

#define nabtoMsec2Stamp(msec) (msec / portTICK_PERIOD_MS)


//typedef int nabto_socket_t;


typedef struct nabto_socket_t nabto_socket_t;


//typedef int nabto_socket_t;
struct nabto_socket_t {
    int sock;
};


#endif
