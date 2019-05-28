#ifndef _UNABTO_TUNNEL_COMMON_H_
#define _UNABTO_TUNNEL_COMMON_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>
#include <modules/network/select/unabto_select.h>
#include <modules/network/tcp/unabto_tcp.h>

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif


#define MAX_COMMAND_LENGTH 512
#define MAX_DEVICE_NAME_LENGTH 128
#define MAX_HOST_LENGTH 128

typedef enum {
    TS_IDLE,
    TS_READ_COMMAND,
    TS_PARSE_COMMAND,
    TS_FAILED_COMMAND, // failed to read/parse command, close connect attempt.
    TS_OPEN_SOCKET,
    TS_OPENING_SOCKET,
    TS_FORWARD,
    TS_CLOSING
} tunnelStates;

typedef enum {
    FS_READ,
    FS_WRITE,
    FS_CLOSING
} forwardState;

typedef enum {
    TUNNEL_EVENT_SOURCE_UART_WRITE,
    TUNNEL_EVENT_SOURCE_UART_READ,
    TUNNEL_EVENT_SOURCE_TCP_WRITE,
    TUNNEL_EVENT_SOURCE_TCP_READ,
    TUNNEL_EVENT_SOURCE_UNABTO
} tunnel_event_source;

typedef enum {
    TUNNEL_TYPE_NONE,
    TUNNEL_TYPE_TCP,
    TUNNEL_TYPE_UART,
    TUNNEL_TYPE_ECHO
} tunnel_type;

typedef struct uart_tunnel_static_memory {
    char deviceName[MAX_DEVICE_NAME_LENGTH];
} uart_tunnel_static_memory;

typedef struct tcp_tunnel_static_memory {
    char host[MAX_HOST_LENGTH];
    //~ uint8_t tcpReadBuffer[NABTO_MEMORY_STREAM_SEGMENT_SIZE];
} tcp_tunnel_static_memory;

union tunnel_static_memory_union{
    struct tcp_tunnel_static_memory tcp_sm;
    struct uart_tunnel_static_memory uart_sm;
};

typedef struct tunnel_static_memory{
    uint8_t command[MAX_COMMAND_LENGTH];
    union tunnel_static_memory_union stmu;
} tunnel_static_memory;

typedef struct uart_vars{
    int fd;
} uart_vars;

typedef struct tcp_vars{
    int port;
    struct unabto_tcp_socket sock;
} tcp_vars;



typedef struct tunnel {
#if NABTO_ENABLE_EPOLL
    // it's important that this is the first member
    int epollEventType;
#endif
    unabto_stream* stream;
    tunnelStates state;
    int commandLength;
    forwardState extReadState;
    forwardState unabtoReadState;
    int tunnelId;
    tunnel_static_memory* staticMemory;

    tunnel_type tunnelType;
    union {
        uart_vars uart;
        tcp_vars tcp;
    } tunnel_type_vars;
} tunnel;

void unabto_tunnel_reset_tunnel_struct(tunnel* t);
bool unabto_tunnel_init_tunnels();
void unabto_tunnel_deinit_tunnels();
void unabto_tunnel_stream_accept(unabto_stream* stream);
tunnel* unabto_tunnel_get_tunnel(unabto_stream* stream);

void unabto_tunnel_event(tunnel* tunnel, tunnel_event_source event_source);
void unabto_tunnel_idle(tunnel* tunnel, tunnel_event_source tunnel_event);
void unabto_tunnel_read_command(tunnel* tunnel, tunnel_event_source tunnel_event);
void unabto_tunnel_parse_command(tunnel* tunnel, tunnel_event_source tunnel_event);
void unabto_tunnel_closing(tunnel* tunnel, tunnel_event_source tunnel_event);

void unabto_tunnel_event_dispatch(tunnel* tunnel, tunnel_event_source event_source);

void unabto_tunnel_select_add_to_fd_set(fd_set* readFds, int* maxReadFd, fd_set* writeFds, int* maxWriteFd);
void unabto_tunnel_select_handle(fd_set* readFds, fd_set* writeFds);
bool tunnel_send_init_message(tunnel* tunnel, const char* msg);

bool unabto_tunnel_has_uart();

#if NABTO_ENABLE_EPOLL
void unabto_tunnel_epoll_event(struct epoll_event* event);
#endif

/**
 * Query at tunnel open request whether a client is allowed access (check ACL, optional functionality).
 * @param connection  the connection being established
 * @return            true if access to the devices is allowed
 */
bool unabto_tunnel_allow_client_access(nabto_connect* connection);

#if NABTO_ENABLE_TUNNEL_STATUS_CALLBACKS

typedef enum {
    NABTO_TCP_TUNNEL_OPENED,
    NABTO_TCP_TUNNEL_CLOSED 
} tunnel_status_event;

/**
 * Inform the application of a tunnel event
 */
void unabto_tunnel_status_callback(tunnel_status_event event, tunnel* tunnel);

typedef struct tunnel_status_tcp_details {
    char* host;
    int port;
    unsigned sentBytes;
    unsigned receivedBytes;
} tunnel_status_tcp_details;

/**
 * Access tcp tunnel information
 */
void unabto_tunnel_status_get_tcp_info(tunnel* tunnel, tunnel_status_tcp_details* info);


#endif



#endif // _UNABTO_TUNNEL_COMMON_H_
