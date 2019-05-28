#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>
#include <modules/tunnel/unabto_tunnel_common.h>
#include <modules/tunnel/unabto_tunnel_uart.h>
#include <modules/tunnel/unabto_tunnel_tcp.h>
#include <modules/tunnel/unabto_tunnel_echo.h>

#if defined(WIN32) || defined(WINCE)
// use winsock
#define WINSOCK 1
#endif

bool init_tunnel_module();
void deinit_tunnel_module();

void tunnel_event(tunnel* state, tunnel_event_source event_source);

#endif // _TUNNEL_H_
