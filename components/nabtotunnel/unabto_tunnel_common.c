#include <unabto/unabto_app.h>
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <errno.h>
#include <fcntl.h>

#include <modules/tunnel/unabto_tunnel_common.h>
#include <modules/tunnel/unabto_tunnel_tcp.h>
#include <modules/tunnel/unabto_tunnel_uart.h>
#include <modules/tunnel/unabto_tunnel_echo.h>



#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels = 0;
NABTO_THREAD_LOCAL_STORAGE tunnel_static_memory* tunnels_static_memory = 0;
NABTO_THREAD_LOCAL_STORAGE static int tunnelCounter = 0;


static const char* defaultUartDevice = 0;

void uart_tunnel_set_default_device(const char* device) {
    defaultUartDevice = device;
}
const char* uart_tunnel_get_default_device() {
    return defaultUartDevice;
}

bool unabto_tunnel_init_tunnels()
{
    int i;
    tunnels = (tunnel*)malloc(sizeof(struct tunnel) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    if (tunnels == NULL) {
        return false;
    }

    tunnels_static_memory = (tunnel_static_memory*)malloc(sizeof(struct tunnel_static_memory) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    if (tunnels_static_memory == NULL) {
        return false;
    }

    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
        unabto_tunnel_reset_tunnel_struct(&tunnels[i]);
    }

    return true;
}

void unabto_tunnel_deinit_tunnels()
{
    free(tunnels); tunnels = 0;
    free(tunnels_static_memory); tunnels_static_memory = 0;
}

void unabto_tunnel_stream_accept(unabto_stream* stream) {
    bool allowTunnel = false;
    tunnel* t;
    nabto_connect* con = unabto_stream_connection(stream);
    if (con && unabto_tunnel_allow_client_access(con)) {
        allowTunnel = true;
    }

    t = &tunnels[unabto_stream_index(stream)];
    NABTO_LOG_TRACE(("Accempting stream and assigning it to tunnel %i", (int)t));
    UNABTO_ASSERT(t->state == TS_IDLE);
    unabto_tunnel_reset_tunnel_struct(t);

    t->stream = stream;
    if (allowTunnel) {
        t->state = TS_READ_COMMAND;
    } else {
        t->state = TS_CLOSING;
    }
    t->tunnelId = tunnelCounter++;
}

tunnel* unabto_tunnel_get_tunnel(unabto_stream* stream)
{
    tunnel* t;
    t = &tunnels[unabto_stream_index(stream)];
    return t;
}

void unabto_tunnel_event(tunnel* tunnel, tunnel_event_source event_source)
{
    NABTO_LOG_TRACE(("Tunnel event on tunnel %i, source %i", tunnel->tunnelId, event_source));
    if (tunnel->state == TS_IDLE) {
        unabto_tunnel_idle(tunnel, event_source);
        return;
    }

    if (tunnel->state == TS_READ_COMMAND) {
        unabto_tunnel_read_command(tunnel, event_source);
    }

    if (tunnel->state == TS_PARSE_COMMAND) {
        unabto_tunnel_parse_command(tunnel, event_source);
    }

    if (tunnel->state == TS_FAILED_COMMAND) {
        unabto_tunnel_closing(tunnel, event_source);
    }
    
    if (tunnel->state >= TS_PARSE_COMMAND) {
        unabto_tunnel_event_dispatch(tunnel, event_source);
    }
}

void unabto_tunnel_read_command(tunnel* tunnel, tunnel_event_source event_source)
{
    const uint8_t* buf;
    unabto_stream_hint hint;
    size_t readen;
    if (tunnel->state != TS_READ_COMMAND) {
        return;
    }
    readen = unabto_stream_read(tunnel->stream, &buf, &hint);
    if (hint != UNABTO_STREAM_HINT_OK) {
        tunnel->state = TS_CLOSING;
    } else {
        if (readen > 0) {
            size_t i;
            for (i = 0; i < readen; i++) {
                if (buf[i] == '\n') {
                    tunnel->state = TS_PARSE_COMMAND;
                } else {
                    tunnel->staticMemory->command[tunnel->commandLength] = buf[i];
                    tunnel->commandLength++;
                }
                
                if (tunnel->commandLength > MAX_COMMAND_LENGTH) {
                    NABTO_LOG_ERROR(("Tunnel command too long"));
                    tunnel->state = TS_CLOSING;
                }
            }
            
            unabto_stream_ack(tunnel->stream, buf, i, &hint);
            
            if (hint != UNABTO_STREAM_HINT_OK) {
                NABTO_LOG_ERROR(("Failed to ack on stream."));
                tunnel->state = TS_CLOSING;
            }
        }
    }
}

void unabto_tunnel_reset_tunnel_struct(tunnel* t)
{
    ptrdiff_t offset = t - tunnels;
    memset(t, 0, sizeof(struct tunnel));
    t->staticMemory = &tunnels_static_memory[offset];
    memset(t->staticMemory, 0, sizeof(tunnel_static_memory));
}

bool unabto_tunnel_has_uart(){
#if NABTO_ENABLE_TUNNEL_UART
    return true;
#else
    return false;
#endif
}

#define TUNNEL_TXT "tunnel"
#define UART_TXT "uart"
#define ECHO_TXT "echo"

void unabto_tunnel_parse_command(tunnel* tunnel, tunnel_event_source tunnel_event)
{
#if NABTO_ENABLE_TUNNEL_UART
    if (uart_tunnel_get_default_device() != 0){ 
       if (strncmp((const char*)tunnel->staticMemory->command, UART_TXT, strlen(UART_TXT)) == 0) {
           unabto_tunnel_uart_parse_command(tunnel, tunnel_event, tunnels, NABTO_MEMORY_STREAM_MAX_STREAMS);
           return;
       }
    }
#endif

#if NABTO_ENABLE_TUNNEL_TCP
    if (strncmp((const char*)tunnel->staticMemory->command, TUNNEL_TXT, strlen(TUNNEL_TXT)) == 0) {
        unabto_tunnel_tcp_parse_command(tunnel, tunnel_event);
        return;
    }
#endif

#if NABTO_ENABLE_TUNNEL_ECHO
    if (strncmp((const char*)tunnel->staticMemory->command, ECHO_TXT, strlen(ECHO_TXT)) == 0) {
        unabto_tunnel_echo_parse_command(tunnel, tunnel_event);
        return;
    }
#endif
    // if we rend here command parsing failed.
    NABTO_LOG_INFO(("Failed to parse command: %s",tunnel->staticMemory->command));
    tunnel->state = TS_FAILED_COMMAND;
}


void unabto_tunnel_closing(tunnel* tunnel, tunnel_event_source tunnel_event)
{
    if (tunnel->state == TS_CLOSING || tunnel->state == TS_FAILED_COMMAND) {
        const uint8_t* buf;
        unabto_stream_hint hint;
        size_t readen;
        
        do {
            readen = unabto_stream_read(tunnel->stream, &buf, &hint);
            if (readen > 0) {
                unabto_stream_ack(tunnel->stream, buf, readen, &hint);
            }
        } while (readen > 0);
        
        if (unabto_stream_close(tunnel->stream)) {
            unabto_stream_stats info;
            unabto_stream_get_stats(tunnel->stream, &info);
            
            NABTO_LOG_TRACE(("Closed tunnel successfully"));
            NABTO_LOG_INFO(("Tunnel(%i) closed, " UNABTO_STREAM_STATS_PRI, tunnel->tunnelId, UNABTO_STREAM_STATS_MAKE_PRINTABLE(info)));
            
            unabto_stream_release(tunnel->stream);
            unabto_tunnel_reset_tunnel_struct(tunnel);
        }
    }
}


void unabto_tunnel_idle(tunnel* tunnel, tunnel_event_source event_source)
{
    NABTO_LOG_ERROR(("Tunnel(%i), Event on tunnel which should not be in IDLE state. source %i, unabtoReadState %i, stream index %i, Tunnel type unrecognized.", tunnel->tunnelId, event_source,tunnel->unabtoReadState, unabto_stream_index(tunnel->stream)));
}


void unabto_tunnel_event_dispatch(tunnel* tunnel, tunnel_event_source tunnel_event)
{
#if NABTO_ENABLE_TUNNEL_UART
    if (tunnel->tunnelType == TUNNEL_TYPE_UART) {
        unabto_tunnel_uart_event(tunnel, tunnel_event);
        return;
    }
#endif

#if NABTO_ENABLE_TUNNEL_TCP
    if (tunnel->tunnelType == TUNNEL_TYPE_TCP) {
        unabto_tunnel_tcp_event(tunnel, tunnel_event);
        return;
    }
#endif

#if NABTO_ENABLE_TUNNEL_ECHO
    if (tunnel->tunnelType == TUNNEL_TYPE_ECHO) {
        unabto_tunnel_echo_event(tunnel, tunnel_event);
        return;
    }
#endif

    // else unknown type or closing.
    if (tunnel->tunnelType == TUNNEL_TYPE_NONE) {
        unabto_tunnel_closing(tunnel, tunnel_event);
        return;
    }
}

void unabto_tunnel_select_add_to_fd_set(fd_set* readFds, int* maxReadFd, fd_set* writeFds, int* maxWriteFd)
{
    int i;
    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
        if (tunnels[i].state != TS_IDLE) {
#if NABTO_ENABLE_TUNNEL_UART
            if (tunnels[i].tunnelType == TUNNEL_TYPE_UART){
                if (tunnels[i].state == TS_FORWARD && tunnels[i].extReadState == FS_READ && tunnels[i].tunnel_type_vars.uart.fd != -1) {
                    
                    FD_SET(tunnels[i].tunnel_type_vars.uart.fd, readFds);
                    *maxReadFd = MAX(*maxReadFd, tunnels[i].tunnel_type_vars.uart.fd);
                }
                if ((tunnels[i].state == TS_FORWARD && tunnels[i].unabtoReadState == FS_WRITE && tunnels[i].tunnel_type_vars.uart.fd != -1)) {
                    FD_SET(tunnels[i].tunnel_type_vars.uart.fd, writeFds);
                    *maxWriteFd = MAX(*maxWriteFd, tunnels[i].tunnel_type_vars.uart.fd);
                }
            }
#endif

#if NABTO_ENABLE_TUNNEL_TCP
            if (tunnels[i].tunnelType == TUNNEL_TYPE_TCP){
                if (tunnels[i].state == TS_FORWARD && tunnels[i].extReadState == FS_READ) {
                    FD_SET(tunnels[i].tunnel_type_vars.tcp.sock.socket, readFds);
                    *maxReadFd = MAX(*maxReadFd, (int)(tunnels[i].tunnel_type_vars.tcp.sock.socket));
                }
                if ((tunnels[i].state == TS_FORWARD && tunnels[i].unabtoReadState == FS_WRITE) ||
                    tunnels[i].state == TS_OPENING_SOCKET) {
                    FD_SET(tunnels[i].tunnel_type_vars.tcp.sock.socket, writeFds);
                    *maxWriteFd = MAX(*maxWriteFd, (int)(tunnels[i].tunnel_type_vars.tcp.sock.socket));
                }                    
            }
#endif
        }
    }
}

void unabto_tunnel_select_handle(fd_set* readFds, fd_set* writeFds)
{
    int i;
    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
        if (tunnels[i].state != TS_IDLE) {
#if NABTO_ENABLE_TUNNEL_UART
            if (tunnels[i].tunnelType == TUNNEL_TYPE_UART){
                if (tunnels[i].tunnel_type_vars.uart.fd != -1 && FD_ISSET(tunnels[i].tunnel_type_vars.uart.fd, readFds)) {
                    unabto_tunnel_uart_event(&tunnels[i], TUNNEL_EVENT_SOURCE_UART_READ);
                }
                if (tunnels[i].tunnel_type_vars.uart.fd != -1 && FD_ISSET(tunnels[i].tunnel_type_vars.uart.fd, writeFds)) {
                    unabto_tunnel_uart_event(&tunnels[i], TUNNEL_EVENT_SOURCE_UART_WRITE);
                }
            }
#endif

#if NABTO_ENABLE_TUNNEL_TCP
            if (tunnels[i].tunnelType == TUNNEL_TYPE_TCP){
                if (tunnels[i].tunnel_type_vars.tcp.sock.socket != INVALID_SOCKET && FD_ISSET(tunnels[i].tunnel_type_vars.tcp.sock.socket, readFds)) {
                    unabto_tunnel_tcp_event(&tunnels[i], TUNNEL_EVENT_SOURCE_TCP_READ);
                }
                if (tunnels[i].tunnel_type_vars.tcp.sock.socket != INVALID_SOCKET && FD_ISSET(tunnels[i].tunnel_type_vars.tcp.sock.socket, writeFds)) {
                    unabto_tunnel_tcp_event(&tunnels[i], TUNNEL_EVENT_SOURCE_TCP_WRITE);
                }
            }
#endif
        }
    }
}

/**
 * utility function for either sending +ok\n or -error message\n in
 * case of either success or failure.  see spec in
 * https://www.rfc-editor.org/rfc/rfc1078.txt instead of CRLF only LF
 * is used.
 */
bool tunnel_send_init_message(tunnel* tunnel, const char* msg)
{
    unabto_stream_hint hint;
    size_t written;
    size_t writeLength = strlen(msg);
    written = unabto_stream_write(tunnel->stream, (uint8_t*)msg, writeLength, &hint);
    if (written != writeLength) {
        NABTO_LOG_ERROR(("we should at a minimum be able to send this simple message, we will probably just goive up..."));
        return false;
    }
    return true;
}


#if NABTO_ENABLE_TUNNEL_TCP && NABTO_ENABLE_TUNNEL_STATUS_CALLBACKS

void unabto_tunnel_status_get_tcp_info(tunnel* tunnel, tunnel_status_tcp_details* info) {
    unabto_stream_stats stats;
    unabto_stream_get_stats(tunnel->stream, &stats);
    info->host = tunnel->staticMemory->stmu.tcp_sm.host;
    info->port = tunnel->tunnel_type_vars.tcp.port;
    info->sentBytes = stats.sentBytes;
    info->receivedBytes = stats.receivedBytes;
}

#endif
