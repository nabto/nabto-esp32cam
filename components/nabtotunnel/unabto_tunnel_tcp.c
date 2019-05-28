 
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_external_environment.h>
#include <errno.h>
#include <fcntl.h>
#include <modules/network/epoll/unabto_epoll.h>

#include <modules/tunnel/unabto_tunnel_common.h>
#include <modules/tunnel/unabto_tunnel_tcp.h>

#ifdef WIN32
#define _SCL_SECURE_NO_WARNINGS
#endif

#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

static const char* tunnel_host = UNABTO_TUNNEL_TCP_DEFAULT_HOST;
static uint16_t tunnel_port = UNABTO_TUNNEL_TCP_DEFAULT_PORT;


void unabto_tunnel_tcp_close_stream_reader(tunnel* tunnel);
void unabto_tunnel_tcp_close_tcp_reader(tunnel* tunnel);
void unabto_tunnel_tcp_closing(tunnel* tunnel, tunnel_event_source event_source);

#if NABTO_ENABLE_TUNNEL_STATUS_CALLBACKS
#define INVOKE_STATUS_CALLBACK(event, tunnel) do { unabto_tunnel_status_callback(event, tunnel); } while(0)
#else
#define INVOKE_STATUS_CALLBACK(event, tunnel)
#endif


void unabto_tunnel_tcp_init(tunnel* tunnel)
{
    tunnel->tunnel_type_vars.tcp.sock.socket = INVALID_SOCKET;
}

void unabto_tunnel_tcp_event(tunnel* tunnel, tunnel_event_source event_source)
{
    if (tunnel->state == TS_OPENING_SOCKET && event_source == TUNNEL_EVENT_SOURCE_TCP_WRITE) {
        if (!opening_socket(tunnel)) {
            tunnel->state = TS_CLOSING;
        }
    }
    
    if (tunnel->state == TS_OPEN_SOCKET) {
        if (open_socket(tunnel)) {
            INVOKE_STATUS_CALLBACK(NABTO_TCP_TUNNEL_OPENED, tunnel);
        } else {
            tunnel->state = TS_CLOSING;
        }
    }
    if (tunnel->state == TS_FORWARD) {
        tcp_forward(tunnel);
        unabto_forward_tcp(tunnel);
    }
    
    if (tunnel->extReadState == FS_CLOSING && tunnel->unabtoReadState == FS_CLOSING) {
        tunnel->state = TS_CLOSING;
    }
    
    if (tunnel->state == TS_CLOSING) {
        unabto_tunnel_tcp_closing(tunnel, event_source);
    }
}

void unabto_tunnel_tcp_closing(tunnel* tunnel, tunnel_event_source event_source)
{
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
        INVOKE_STATUS_CALLBACK(NABTO_TCP_TUNNEL_CLOSED, tunnel);
        
        unabto_tcp_close(&tunnel->tunnel_type_vars.tcp.sock);
        unabto_stream_release(tunnel->stream);
        unabto_tunnel_reset_tunnel_struct(tunnel);
    }
}

/**
 * read from tcp, write to unabto
 */ 
void tcp_forward(tunnel* tunnel) {
    while(true) {
        unabto_stream_hint hint;
        size_t canWriteToStreamBytes;
        
        if (tunnel->extReadState == FS_CLOSING) {
            break;
        }

        canWriteToStreamBytes = unabto_stream_can_write(tunnel->stream, &hint);
        if (hint != UNABTO_STREAM_HINT_OK) {
            unabto_tunnel_tcp_close_tcp_reader(tunnel);
            break;
        }
        if (canWriteToStreamBytes == 0) {
            tunnel->extReadState = FS_WRITE;
            break;
        } else {
            tunnel->extReadState = FS_READ;
        }
        {
            unabto_tcp_status status;
            size_t readen;
            uint8_t readBuffer[NABTO_MEMORY_STREAM_SEGMENT_SIZE];
            size_t maxRead = MIN(NABTO_MEMORY_STREAM_SEGMENT_SIZE, canWriteToStreamBytes);

            status = unabto_tcp_read(&tunnel->tunnel_type_vars.tcp.sock, readBuffer, maxRead, &readen);
            if (status == UTS_EOF) {
                unabto_tunnel_tcp_close_tcp_reader(tunnel);
                break;
            } else if (status == UTS_OK) {
                unabto_stream_hint hint;
                size_t written = unabto_stream_write(tunnel->stream, readBuffer, readen, &hint);
                if (hint != UNABTO_STREAM_HINT_OK) {
                    NABTO_LOG_TRACE(("Can't write to stream"));
                    unabto_tunnel_tcp_close_tcp_reader(tunnel);
                    break;
                }
                
                if (written != readen) {
                    // Invalid state
                    NABTO_LOG_ERROR(("Impossible state! wanted to write %i, wrote %i, unabto_said it could write %i bytes", readen, written, canWriteToStreamBytes));
                }
            } else if (status == UTS_WOULD_BLOCK) {
                break;
            } else { // UTS_FAILED or undefined behaviour
                unabto_tunnel_tcp_close_tcp_reader(tunnel);
                break;
            }
        }
    }
}

bool opening_socket(tunnel* tunnel) {
    unabto_tcp_status status;

    status = unabto_tcp_connect_poll(&tunnel->tunnel_type_vars.tcp.sock);
    if (status == UTS_OK) {
        tunnel->state = TS_FORWARD;
    } else { // UTS_CONNECTING OR UTS_FAILED
        NABTO_LOG_ERROR(("Error opening tcp tunnelsocket"));
        return false;
    }
    return true;
}




bool open_socket(tunnel* tunnel) {
    unabto_tcp_status status;


    status = unabto_tcp_open(&tunnel->tunnel_type_vars.tcp.sock, NABTO_IP_V4, (void *)tunnel);
    if(status == UTS_OK) {
        unabto_tcp_status conStat;
        nabto_endpoint ep;
        ep.addr.type = NABTO_IP_V4;
        ep.addr.addr.ipv4 = ntohl(inet_addr(tunnel->staticMemory->stmu.tcp_sm.host));
        ep.port = tunnel->tunnel_type_vars.tcp.port;
        conStat = unabto_tcp_connect(&tunnel->tunnel_type_vars.tcp.sock, &ep);
        if(conStat == UTS_CONNECTING) {
            tunnel->state = TS_OPENING_SOCKET;
        } else if (conStat == UTS_OK) {
            tunnel->state = TS_FORWARD;
        } else { // UTS_FAILED
            unabto_tcp_close(&tunnel->tunnel_type_vars.tcp.sock);
            NABTO_LOG_ERROR(("connect failed"));
            return false;
        }
    } else {
        NABTO_LOG_ERROR(("Open socket failed"));
        return false;
    }
    return true;
}

/**
 * read from unabto, write to tcp
 */
void unabto_forward_tcp(tunnel* tunnel) {

    if (tunnel->unabtoReadState == FS_WRITE) {
        tunnel->unabtoReadState = FS_READ;
    }
    while(true) {
        if (tunnel->unabtoReadState == FS_READ) {
            const uint8_t* buf;
            unabto_stream_hint hint;
            size_t readen = unabto_stream_read(tunnel->stream, &buf, &hint);
            if (hint != UNABTO_STREAM_HINT_OK) {
                unabto_tunnel_tcp_close_stream_reader(tunnel);
                break;
            } else {
                if (readen == 0) {
                    break;
                } else {
                    unabto_tcp_status status;
                    size_t written;
                    NABTO_LOG_TRACE(("Write to tcp stream %i", readen));

                    status = unabto_tcp_write(&tunnel->tunnel_type_vars.tcp.sock, buf, readen, &written);

                    if(status == UTS_OK) {
                        if (written > 0) {
                            NABTO_LOG_TRACE(("Wrote to tcp stream %i", written));
                            unabto_stream_ack(tunnel->stream, buf, written, &hint);
                            if (hint != UNABTO_STREAM_HINT_OK) {
                                NABTO_LOG_TRACE(("Closing stream_reader1"));
                                unabto_tunnel_tcp_close_stream_reader(tunnel);
                                break;
                            }
                        } else if (written == 0) {
                            tunnel->unabtoReadState = FS_WRITE;
                            break;
                        }
                    } else if (status == UTS_WOULD_BLOCK) {
                        tunnel->unabtoReadState = FS_WRITE;
                        break;

                    } else { // UTS_FAILED
                        NABTO_LOG_TRACE(("Closing stream_reader2"));
                        unabto_tunnel_tcp_close_stream_reader(tunnel);
                        break;
                    }
                }
            }
        }

        if (tunnel->unabtoReadState == FS_WRITE) {
            break;
        }

        if (tunnel->unabtoReadState == FS_CLOSING) {
            break;
        }
    }
}

#define PORT_KW_TXT "port="
#define HOST_KW_TXT "host="

void unabto_tunnel_tcp_parse_command(tunnel* tunnel, tunnel_event_source event_source)
{
    char* s;

    if (NULL != (s = strstr((const char*)tunnel->staticMemory->command, PORT_KW_TXT)))
    {
        s += strlen(PORT_KW_TXT);
        if (1 != sscanf(s, "%d", &tunnel->tunnel_type_vars.tcp.port)) {
            NABTO_LOG_ERROR(("failed to read port number"));
            tunnel->state = TS_FAILED_COMMAND;
            return;
        }
    } else {
        tunnel->tunnel_type_vars.tcp.port = unabto_tunnel_tcp_get_default_port();
    }
    
    if (NULL != (s = strstr((const char*)tunnel->staticMemory->command, HOST_KW_TXT)))
    {
        char *sp;
        int length;
        s += strlen(HOST_KW_TXT);
        sp = strchr(s, ' ');
        
        if (sp != NULL) {
            length = sp-s;
        } else {
            length = strlen(s);
        }
        
        strncpy(tunnel->staticMemory->stmu.tcp_sm.host, s, MIN(length, MAX_COMMAND_LENGTH-1));
    } else {
        strncpy(tunnel->staticMemory->stmu.tcp_sm.host, unabto_tunnel_tcp_get_default_host(), MAX_HOST_LENGTH);
    }
    tunnel->tunnelType = TUNNEL_TYPE_TCP;

    if (tunnel_allow_connection(tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port)) {
        tunnel->state = TS_OPEN_SOCKET;
        NABTO_LOG_INFO(("Tunnel(%i) connecting to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
    } else {
        NABTO_LOG_ERROR(("Tunnel(%i) not allowed to connect to %s:%i", tunnel->tunnelId, tunnel->staticMemory->stmu.tcp_sm.host, tunnel->tunnel_type_vars.tcp.port));
        tunnel->state = TS_FAILED_COMMAND;
    }
}

void unabto_tunnel_tcp_set_default_host(const char* host) {
    tunnel_host = host;
}

void unabto_tunnel_tcp_set_default_port(uint16_t port) {
    tunnel_port = port;
}

const char* unabto_tunnel_tcp_get_default_host() {
    return tunnel_host;
}

uint16_t unabto_tunnel_tcp_get_default_port() {
    return tunnel_port;
}

void unabto_tunnel_tcp_close_stream_reader(tunnel* tunnel) {
    NABTO_LOG_INFO(("closing stream_reader socket %i", tunnel->tunnel_type_vars.tcp.sock.socket));
    tunnel->unabtoReadState = FS_CLOSING;
    unabto_tcp_shutdown(&tunnel->tunnel_type_vars.tcp.sock);
}

// no more data will come from the tcp connection to the stream
void unabto_tunnel_tcp_close_tcp_reader(tunnel* tunnel) {
    NABTO_LOG_INFO(("closing tcp_reader socket %i", tunnel->tunnel_type_vars.tcp.sock.socket));
    unabto_stream_close(tunnel->stream);
    tunnel->extReadState = FS_CLOSING;
}
