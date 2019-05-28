#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := . ../unabto/src ../unabto-esp-idf
COMPONENT_SRCDIRS :=  ../unabto/src/modules/fingerprint_acl

COMPONENT_OBJS := ../unabto/src/modules/fingerprint_acl/fp_acl.o \
                  ../unabto/src/modules/fingerprint_acl/fp_acl_ae.o \
                  ../unabto/src/modules/fingerprint_acl/fp_acl_memory.o





