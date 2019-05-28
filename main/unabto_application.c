/**
 *  uNabto application logic implementation
 */
#include "demo_application.h"
#include <string.h>
#include "unabto/unabto_app.h"
#include <modules/fingerprint_acl/fp_acl_ae.h>
#include <modules/fingerprint_acl/fp_acl_memory.h>
#include "fp_acl_esp32_nvs.h"

typedef enum { HPM_COOL = 0,
               HPM_HEAT = 1,
               HPM_CIRCULATE = 2,
               HPM_DEHUMIDIFY = 3} heatpump_mode_t;

#define AMP_DEVICE_NAME_DEFAULT "Tunnel"
#define MAX_DEVICE_NAME_LENGTH 50
static char device_name[MAX_DEVICE_NAME_LENGTH];
static const char* device_product = "uNabto Video";
static const char* device_icon = "video.png";
static const char* device_interface_id_ = "8eee78e7-8f22-4019-8cee-4dcbc1c8186c";
static uint16_t device_interface_version_major_ = 1;
static uint16_t device_interface_version_minor_ = 0;


static struct fp_acl_db db_;
struct fp_mem_persistence fp_file_;

#define REQUIRES_GUEST FP_ACL_PERMISSION_NONE
#define REQUIRES_OWNER FP_ACL_PERMISSION_ADMIN




void debug_dump_acl() {
    void* it = db_.first();
    NABTO_LOG_INFO(("Access control list dump:"));
    while (it != NULL) {
        struct fp_acl_user user;
        fp_acl_db_status res = db_.load(it, &user);
        if (res != FP_ACL_DB_OK) {
            NABTO_LOG_WARN(("ACL error %d\n", res));
            return;
        }
        NABTO_LOG_INFO(("%s [%02x:%02x:%02x:%02x:...]: %04x",
                        user.name,
                        user.fp.value.data[0],
                        user.fp.value.data[1],
                        user.fp.value.data[2],
                        user.fp.value.data[3],
                        user.permissions));
        it = db_.next(it);
    }
}

void demo_init() {


  NABTO_LOG_INFO(("In demo_init"));

  struct fp_acl_settings default_settings;

  default_settings.systemPermissions =
    FP_ACL_SYSTEM_PERMISSION_PAIRING |
    FP_ACL_SYSTEM_PERMISSION_LOCAL_ACCESS |
    FP_ACL_PERMISSION_REMOTE_ACCESS;
  default_settings.defaultUserPermissions =
    FP_ACL_PERMISSION_LOCAL_ACCESS|
    FP_ACL_PERMISSION_REMOTE_ACCESS;
  default_settings.firstUserPermissions =
    FP_ACL_PERMISSION_ADMIN |
    FP_ACL_PERMISSION_LOCAL_ACCESS |
    FP_ACL_PERMISSION_REMOTE_ACCESS;


  (void)fp_acl_nvs_init(&fp_file_);        // persistence via ESP32 Non Volatile Store (NVS)
  
  NABTO_LOG_INFO(("Before fp_mem_init"));
  fp_mem_init(&db_, &default_settings, &fp_file_);

  NABTO_LOG_INFO(("Before acl_ae_init"));
  fp_acl_ae_init(&db_);

  //snprintf(device_name_, sizeof(device_name_), DEVICE_NAME_DEFAULT);
  //updateLed();
}


void demo_application_tick() {

}

int copy_buffer(unabto_query_request* read_buffer, uint8_t* dest, uint16_t bufSize, uint16_t* len) {
    uint8_t* buffer;
    if (!(unabto_query_read_uint8_list(read_buffer, &buffer, len))) {
        return AER_REQ_TOO_SMALL;
    }
    if (*len > bufSize) {
        return AER_REQ_TOO_LARGE;
    }
    memcpy(dest, buffer, *len);
    return AER_REQ_RESPONSE_READY;
}

int copy_string(unabto_query_request* read_buffer, char* dest, uint16_t destSize) {
    uint16_t len;
    int res = copy_buffer(read_buffer, (uint8_t*)dest, destSize-1, &len);
    if (res != AER_REQ_RESPONSE_READY) {
        return res;
    }
    dest[len] = 0;
    return AER_REQ_RESPONSE_READY;
}

int write_string(unabto_query_response* write_buffer, const char* string) {
    return unabto_query_write_uint8_list(write_buffer, (uint8_t *)string, strlen(string));
}

bool allow_client_access(nabto_connect* connection) {
    return true;
    bool local = connection->isLocal;
    bool allow = fp_acl_is_connection_allowed(connection) || local;
    NABTO_LOG_INFO(("Allowing %s connect request: %s", (local ? "local" : "remote"), (allow ? "yes" : "no")));
    debug_dump_acl();
    return allow;    
}

application_event_result application_event(application_request* request,
                                           unabto_query_request* query_request,
                                           unabto_query_response* query_response) {

    NABTO_LOG_TRACE(("Nabto application_event: %u", request->queryId));
    debug_dump_acl();

    // handle requests as defined in interface definition shared with
    // client - for the default demo, see
    // https://github.com/nabto/ionic-starter-nabto/blob/master/www/nabto/unabto_queries.xml

    //application_event_result res;

    NABTO_LOG_INFO(("Nabto application_event: %u", request->queryId));
    
    if (request->queryId == 0) {
        // AMP get_interface_info.json
        if (!write_string(query_response, device_interface_id_)) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint16(query_response, device_interface_version_major_)) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint16(query_response, device_interface_version_minor_)) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;
        
    } else if (request->queryId == 10000) {
        // AMP get_public_device_info.json
        if (!write_string(query_response, device_name)) return AER_REQ_RSP_TOO_LARGE;
        if (!write_string(query_response, device_product)) return AER_REQ_RSP_TOO_LARGE;
        if (!write_string(query_response, device_icon)) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint8(query_response, fp_acl_is_pair_allowed(request))) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint8(query_response, fp_acl_is_user_paired(request))) return AER_REQ_RSP_TOO_LARGE; 
        if (!unabto_query_write_uint8(query_response, fp_acl_is_user_owner(request))) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;
        
    } else if (request->queryId == 10010) {
        int res;
        // AMP set_device_info.json
        if (!fp_acl_is_request_allowed(request, REQUIRES_OWNER)) return AER_REQ_NO_ACCESS;
        res = copy_string(query_request, device_name, sizeof(device_name));
        if (res != AER_REQ_RESPONSE_READY) return res;
        if (!write_string(query_response, device_name)) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;
        
    } else if (request->queryId >= 11000 && request->queryId < 12000) {
        // PPKA access control
        return fp_acl_ae_dispatch(11000, request, query_request, query_response);
        
    } else {
        NABTO_LOG_WARN(("Unhandled query id: %u", request->queryId));
        return AER_REQ_INV_QUERY_ID;
    }

}



bool application_poll_query(application_request** applicationRequest) {
    return false;
}

application_event_result application_poll(application_request* applicationRequest, unabto_query_response* writeBuffer) {
    return AER_REQ_SYSTEM_ERROR;
}

void application_poll_drop(application_request* applicationRequest) {
}
