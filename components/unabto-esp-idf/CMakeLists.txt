
set(esp-idf-nabto-src	"./dns_adapter.c"
			"./network_adapter.c"
			"./random_adapter.c"
			"./time_adapter.c"
			"./unabto_aes_cbc_test.c"
			"./unabto_basename.c"
			"./unabto_log_header_unix.c"
			"./crypto/unabto_aes.c"
			"./crypto/unabto_aes_cbc.c"
			"./crypto/unabto_hmac_sha256.c"
			"./crypto/unabto_sha256.c")

idf_component_register(SRCS ${esp-idf-nabto-src} ${unabto_core_src}
                       INCLUDE_DIRS "." "../../../unabto/src"
                       REQUIRES mbedtls)

