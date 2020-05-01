idf_component_register(SRCS ./unabto_tunnel.c ./unabto_tunnel_common.c ./unabto_tunnel_select.c ./unabto_tunnel_tcp.c ./esp32tcp/unabto_tcp.c
                       INCLUDE_DIRS . ../unabto-esp-idf ../../unabto/src ../../unabto/src/modules/network/tcp/
                       REQUIRES )

