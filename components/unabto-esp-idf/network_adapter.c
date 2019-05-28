
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_logging.h>

#include <unabto/unabto_common_main.h>

#include <modules/network/poll/unabto_network_poll_api.h>
#include <modules/list/utlist.h>

#define MAX(a,b) (((a)>(b))?(a):(b))


#include "unabto_platform.h"


typedef struct socketListElement {
    nabto_socket_t socket;
    struct socketListElement *prev;
    struct socketListElement *next;
} socketListElement;

static NABTO_THREAD_LOCAL_STORAGE struct socketListElement* socketList = 0;




void nabto_socket_set_invalid(nabto_socket_t* s) {
    s->sock = -1;
}

bool nabto_socket_is_equal(const nabto_socket_t* s1, const nabto_socket_t* s2) {
    return s1->sock == s2->sock;
}

void nabto_resolve_ipv4(uint32_t ipv4, struct nabto_ip_address* ip) {
    ip->type = NABTO_IP_V4;
    ip->addr.ipv4 = ipv4;
}


/**
* Close a socket. 
* Close can be called on already closed sockets. And should tolerate this behavior.
*
* @param socket the socket to be closed
*/
void nabto_socket_close(nabto_socket_t* sock)
{

    if (sock && sock->sock != NABTO_INVALID_SOCKET) {
        socketListElement* se;
        socketListElement* found = 0;
        DL_FOREACH(socketList,se) {
            if (se->socket.sock == sock->sock) {
                found = se;
                break;
            }
        }
        if (!found) {
            NABTO_LOG_ERROR(("Socket %i Not found in socket list", sock->sock));
        } else {
            DL_DELETE(socketList, se);
            free(se);
        }

        close(sock->sock);
        sock->sock = NABTO_INVALID_SOCKET;

    }    
    
}


bool nabto_socket_init(uint16_t* localPort, nabto_socket_t* sock) {

    socketListElement* se;
    int sd;
    
    NABTO_LOG_TRACE(("Open socket: port=%u", (int)*localPort));
    
    sd = socket(AF_INET, SOCK_DGRAM, 0);

    NABTO_LOG_TRACE(("Created socket: %u", (int)sd));
    
    if (sd == -1) {
        NABTO_LOG_ERROR(("Unable to create socket: (%i) '%s'.", errno, strerror(errno)));
        return false;
    }

    {
        struct sockaddr_in sa;
        int status;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(*localPort);
        
        status = bind(sd, (struct sockaddr*)&sa, sizeof(sa));
        
        if (status < 0) {
            NABTO_LOG_ERROR(("Unable to bind socket: (%i)  '%s'.", errno, strerror(errno)));
            close(sd);
            return false;
        }
        int flags = fcntl(sd, F_GETFL, 0);
        if (flags == -1) flags = 0;
        fcntl(sd, F_SETFL, flags | O_NONBLOCK);

        se = (socketListElement*)malloc(sizeof(socketListElement));
        if (!se) {
            NABTO_LOG_FATAL(("Malloc of a single small list element should not fail!"));
            close(sd);
            return false;
        }
        se->socket.sock = sd;
        DL_APPEND(socketList, se);
        sock->sock = sd;
    }
    {
        struct sockaddr_in sao;
        socklen_t len = sizeof(sao);
        if ( getsockname(sd, (struct sockaddr*)&sao, &len) != -1) {
            *localPort = htons(sao.sin_port);
        } else {
            NABTO_LOG_ERROR(("Unable to get local port of socket: (%i) '%s'.", errno, strerror(errno)));
        }
    }
    
    NABTO_LOG_INFO(("Socket opened: port=%u", (int)*localPort));
    
    return true;
}

void nabto_close_socket(nabto_socket_t* s) {
    if (s && s->sock != NABTO_INVALID_SOCKET) {
        close(s->sock);
        s->sock = NABTO_INVALID_SOCKET;
    }
}

ssize_t nabto_read(nabto_socket_t s,
                   uint8_t*       buf,
                   size_t         len,
                   struct nabto_ip_address*  addr,
                   uint16_t*      port)
{
    int res;
    struct sockaddr_in sa;
    size_t salen = sizeof(sa);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    res = recvfrom(s.sock, (char*) buf, (int)len, 0, (struct sockaddr*)&sa, &salen);
    
    if (res >= 0) {
        addr->type = NABTO_IP_V4;
        addr->addr.ipv4 = ntohl(sa.sin_addr.s_addr);
        *port = ntohs(sa.sin_port);
        NABTO_LOG_TRACE(("Socket read: ip=" PRIip ", port=%u", MAKE_IP_PRINTABLE(addr->addr.ipv4), (int)*port));
    }
    return res;
}

ssize_t nabto_write(nabto_socket_t s,
                 const uint8_t* buf,
                 size_t         len,
                 const struct nabto_ip_address*  addr,
                 uint16_t       port)
{
    int res;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl((*addr).addr.ipv4);
    sa.sin_port = htons(port);
    res = sendto(s.sock, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));
    NABTO_LOG_TRACE(("Socket write: ip=" PRIip ", port=%u socket:%i", MAKE_IP_PRINTABLE(addr->addr.ipv4), (int)port, s.sock));
    if (res < 0) {
        NABTO_LOG_ERROR(("ERROR: %i in nabto_write() '%s'", (int) errno,strerror(errno)));
    }
    return res;
}


void unabto_network_select_add_to_read_fd_set(fd_set* readFds, int* maxReadFd) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        //NABTO_LOG_TRACE(("Adding Socket:%i to read_fd_set", se->socket.sock));
        FD_SET(se->socket.sock, readFds);
        *maxReadFd = MAX(*maxReadFd, se->socket.sock);
    }
}

void unabto_network_select_read_sockets(fd_set* readFds) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        if (FD_ISSET(se->socket.sock, readFds)) {
            unabto_read_socket(se->socket);
        }
    }
}


