#include <unabto/unabto_stream.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_common_main.h>
#include <modules/timers/auto_update/unabto_time_auto_update.h>
#include <modules/network/select/unabto_network_select_api.h>
#include <modules/tcp_fallback/tcp_fallback_select.h>
#include <unabto/unabto_util.h>

#include "unabto_tunnel_select.h"
#include "unabto_tunnel.h"

#if defined(WIN32) || defined(WINCE)
// use winsock
#define WINSOCK 1
#include <modules/network/winsock/unabto_winsock.h>
#else
// use a bsd api
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <stdio.h>

void tunnel_loop_select() {
    if (!unabto_init()) {
        NABTO_LOG_FATAL(("Failed to initialize unabto"));
    }

    if (!init_tunnel_module()) {
        NABTO_LOG_FATAL(("Cannot initialize tunnel module"));
        return;
    }

    unabto_time_auto_update(false);
    // time is updated here and after the select since that's the only blocking point.
    unabto_time_update_stamp();
    while(true) {
        nabto_stamp_t nextEvent;
        nabto_stamp_t now;
        int timeout;
        fd_set read_fds;
        fd_set write_fds;
        int max_read_fd = 0;
        int max_write_fd = 0;
        struct timeval timeout_val;
        int nfds;
        
        unabto_next_event(&nextEvent);
        now = nabtoGetStamp();
        timeout = nabtoStampDiff2ms(nabtoStampDiff(&nextEvent, &now));

        if (timeout < 0) {
            timeout = 1;
        }

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        unabto_network_select_add_to_read_fd_set(&read_fds, &max_read_fd);

#if NABTO_ENABLE_TCP_FALLBACK
        unabto_tcp_fallback_select_add_to_read_fd_set(&read_fds, &max_read_fd);
        unabto_tcp_fallback_select_add_to_write_fd_set(&write_fds, &max_write_fd);
#endif

        unabto_tunnel_select_add_to_fd_set(&read_fds, &max_read_fd, &write_fds, &max_write_fd);

        timeout_val.tv_sec = (timeout/1000);
        timeout_val.tv_usec = ((timeout)%1000)*1000;

        fflush(stdout);
        //NABTO_LOG_TRACE(("MAX(max_read_fd+1, max_write_fd+1)=%i",MAX(max_read_fd+1, max_write_fd+1)));
        
        nfds = select(MAX(max_read_fd+1, max_write_fd+1), &read_fds, &write_fds, NULL, &timeout_val);

        if (nfds < 0) {
            
            int err = errno;
            if (err == EINTR) {
                // ok
            } else {
                NABTO_LOG_ERROR(("Select returned error %s, %i", strerror(err), err));
                return;
            }
            
        }

        unabto_time_update_stamp();

        if (nfds > 0) {
#if NABTO_ENABLE_TCP_FALLBACK
            unabto_tcp_fallback_select_write_sockets(&write_fds);
            unabto_tcp_fallback_select_read_sockets(&read_fds);
#endif
            unabto_network_select_read_sockets(&read_fds);

            unabto_tunnel_select_handle(&read_fds, &write_fds);
        }
        unabto_time_event();
    }
    deinit_tunnel_module();
    unabto_close();
}

