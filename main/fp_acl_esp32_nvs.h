/*
@file fp_acl_esp32_nvs.h
@brief Handles persistent storage of Nabto ACL into ESP32 NVS.

Copyright (c) 2019 Connictro GmbH / Michael Paar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

/* Includes */

#ifndef _FP_ACL_ESP32_NVS_H_
#define _FP_ACL_ESP32_NVS_H_

//#include "app_common_defs.h"
//#include "nabto_cred_config.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <modules/fingerprint_acl/fp_acl_ae.h>
#include <modules/fingerprint_acl/fp_acl_memory.h>

/* Save / Load Nabto persistency information to/from NVS. */
esp_err_t nabto_set_persistency_file(void *persist_str, size_t sz);
esp_err_t nabto_get_persistency_file(void *persist_str, size_t sz);

fp_acl_db_status fp_acl_nvs_save(struct fp_mem_state* acl);
fp_acl_db_status fp_acl_nvs_load(struct fp_mem_state* acl);
fp_acl_db_status fp_acl_nvs_reset();
fp_acl_db_status fp_acl_nvs_init(struct fp_mem_persistence* per);


#endif /* _FP_ACL_ESP32_NVS_H_ */
