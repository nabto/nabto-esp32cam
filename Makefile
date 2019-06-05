#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := esp32_nabto

EXTRA_CFLAGS := -Wno-maybe-uninitialized

include $(IDF_PATH)/make/project.mk

