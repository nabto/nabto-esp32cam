#ifndef _TUNNEL_TCP_H_
#define _TUNNEL_TCP_H_
#include <modules/tunnel/unabto_tunnel_common.h>

/**
 * return false to disallow connections to the specified host:port
 */
bool tunnel_allow_connection(const char* host, int port);

void tcp_forward(tunnel* tunnel);
void unabto_forward_tcp(tunnel* tunnel);
bool opening_socket(tunnel* tunnel);
bool open_socket(tunnel* tunnel);

void unabto_tunnel_tcp_init(tunnel* tunnel);

void unabto_tunnel_tcp_parse_command(tunnel* tunnel, tunnel_event_source event_source);

void unabto_tunnel_tcp_event(tunnel* tunnel, tunnel_event_source event_source);

void unabto_tunnel_tcp_set_default_host(const char* host);
void unabto_tunnel_tcp_set_default_port(uint16_t port);

const char* unabto_tunnel_tcp_get_default_host();
uint16_t    unabto_tunnel_tcp_get_default_port();

#ifndef UNABTO_TUNNEL_TCP_DEFAULT_PORT
#define UNABTO_TUNNEL_TCP_DEFAULT_PORT 22
#endif

#ifndef UNABTO_TUNNEL_TCP_DEFAULT_HOST
#define UNABTO_TUNNEL_TCP_DEFAULT_HOST "127.0.0.1"
#endif

#endif // _TUNNEL_TCP_H_
