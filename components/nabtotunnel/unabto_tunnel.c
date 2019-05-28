#include <unabto/unabto_stream.h>
#include "unabto_tunnel.h"

bool init_tunnel_module()
{
    return unabto_tunnel_init_tunnels();
}

void deinit_tunnel_module()
{
    unabto_tunnel_deinit_tunnels();
}

void unabto_stream_accept(unabto_stream* stream) {
    unabto_tunnel_stream_accept(stream);
}

void unabto_stream_event(unabto_stream* stream, unabto_stream_event_type event) {
    tunnel* t = unabto_tunnel_get_tunnel(stream);
    NABTO_LOG_TRACE(("Stream %i, %i", unabto_stream_index(stream), event));
    unabto_tunnel_event(t, TUNNEL_EVENT_SOURCE_UNABTO);
}



/*
bool allow_client_access(nabto_connect* connection) {
    return true;
}
*/

bool tunnel_allow_connection(const char* host, int port) {

    int c = strncmp(host, "127.0.0.1", 10);

    if( c==0 && port == 8081) {
        NABTO_LOG_INFO(("Allow connection host:%s port:%i", host, port));
        return true;
    }

    NABTO_LOG_INFO(("Not allowing connection host:%s port:%i", host, port));
    return false;
}



bool unabto_tunnel_allow_client_access(nabto_connect* connection) {
    return true;
}

#if NABTO_ENABLE_TUNNEL_STATUS_CALLBACKS
void unabto_tunnel_status_callback(tunnel_status_event event, tunnel* tunnel) {
    tunnel_status_tcp_details info;
    unabto_tunnel_status_get_tcp_info(tunnel, &info);
    NABTO_LOG_INFO(("Tunnel event [%d] on tunnel [%d] to host [%s] on port [%d], bytes sent: [%d]", event, tunnel->tunnelId,
                    info.host, info.port, info.sentBytes));
}
#endif

