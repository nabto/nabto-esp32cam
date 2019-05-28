/*
@file fp_acl_esp32_nvs.c
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

//#include "app_common_defs.h"
//#include "nabto_cred_config.h"
//#include "esp_http_client.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "fp_acl_esp32_nvs.h"

/* Macros */

/* Static variables */
static const char *NVS_NABTOPERSIST_TAG  = "nabtopersist";
static const char *NABTO_PERSIST_WHATSTR = "Nabto persistence store";
#define NVS_STORAGE_NAMESPACE "nabtocreds"



/* Implementation */


/* Implementation */

// Retrieve specific config string (key or modbus cfg) from NVS.
esp_err_t _read_blob_from_nvs(char *blob, const char *nvstag, size_t sz, const char *what)
{
    esp_err_t   status = ESP_OK;
    nvs_handle  nabto_nvs_handle;

    NABTO_LOG_TRACE(("open NVS storage"));
    status = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READONLY, &nabto_nvs_handle);
    if (status != ESP_OK)
        return status;

    NABTO_LOG_TRACE(("namespace exists, trying to read %s from NVS", what));
    status = nvs_get_blob(nabto_nvs_handle, nvstag, blob, &sz);

    if (status != ESP_OK)
    {
        NABTO_LOG_ERROR(("No valid %s in NVS", what));
        status = ESP_ERR_NOT_FOUND;
    } else {
        NABTO_LOG_TRACE(("%s: %s read from NVS", what, blob));
    }
    nvs_close(nabto_nvs_handle);

    return status;
}

// write specific config string (key or modbus cfg) to NVS.
esp_err_t _write_blob_to_nvs(char *blob, const char *nvstag, size_t sz, const char *what)
{
    esp_err_t   status = ESP_OK;
    nvs_handle  nabto_nvs_handle;

    NABTO_LOG_INFO(("open NVS storage for write"));
    status = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &nabto_nvs_handle);
    if (status != ESP_OK)
        return status;

    NABTO_LOG_INFO(("Writing %s to NVS", what));

    status = nvs_set_blob(nabto_nvs_handle, nvstag, blob, sz);
    if (status != ESP_OK) 
    {
        nvs_close(nabto_nvs_handle);
        return status;
    }

    status = nvs_commit(nabto_nvs_handle);
    nvs_close(nabto_nvs_handle);
    NABTO_LOG_INFO(("End of write status:%i", status));
    
    return status;   
}

esp_err_t _erase_tag_from_nvs(const char *nvstag, const char *what)
{
    esp_err_t   status = ESP_OK;
    nvs_handle  nabto_nvs_handle;

    NABTO_LOG_INFO(("open NVS storage for erase"));
    status = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &nabto_nvs_handle);
    if (status != ESP_OK)
        return status;

    NABTO_LOG_INFO(("Erasing %s from NVS", what));
    status = nvs_erase_key(nabto_nvs_handle, nvstag);
    nvs_close(nabto_nvs_handle);
    return status;   
}


fp_acl_db_status fp_acl_nvs_load(struct fp_mem_state* acl)
{
    esp_err_t status = _read_blob_from_nvs((void *)acl, NVS_NABTOPERSIST_TAG, sizeof(struct fp_mem_state), NABTO_PERSIST_WHATSTR);

    // The status is OK even if the NVS tag did not exist - bootstrap scenario (acl comes initialized with default values already).
    return (status == ESP_OK || status == ESP_ERR_NVS_NOT_FOUND) ? FP_ACL_DB_OK : FP_ACL_DB_LOAD_FAILED;
}

fp_acl_db_status fp_acl_nvs_save(struct fp_mem_state* acl)
{
    esp_err_t status = _write_blob_to_nvs((void *)acl, NVS_NABTOPERSIST_TAG, sizeof(struct fp_mem_state), NABTO_PERSIST_WHATSTR);

    return (status == ESP_OK) ? FP_ACL_DB_OK : FP_ACL_DB_SAVE_FAILED;
}

fp_acl_db_status fp_acl_nvs_reset()
{
    esp_err_t status = _erase_tag_from_nvs(NVS_NABTOPERSIST_TAG, NABTO_PERSIST_WHATSTR);

    return (status == ESP_OK) ? FP_ACL_DB_OK : FP_ACL_DB_SAVE_FAILED;
}

fp_acl_db_status fp_acl_nvs_init(struct fp_mem_persistence* per)
{
    per->load = &fp_acl_nvs_load;
    per->save = &fp_acl_nvs_save;
    return FP_ACL_DB_OK;
}


