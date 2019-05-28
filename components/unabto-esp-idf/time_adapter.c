/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "unabto_platform_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


nabto_stamp_t nabtoGetStamp() {
    return xTaskGetTickCount();
}

#define MAX_STAMP_DIFF 0x7fffffff;

bool nabtoIsStampPassed(nabto_stamp_t *stamp) {
    return *(uint32_t*)stamp - (uint32_t)nabtoGetStamp() > MAX_STAMP_DIFF;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    return (*newest - *oldest);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return (int) diff*portTICK_PERIOD_MS;
}

void unabto_time_auto_update(bool enabled) {
}

void unabto_time_update_stamp() {
}
