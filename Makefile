#
# Top makefile
#

TOP_DIR			:= ${shell pwd}
LIBS_DIR		:= $(TOP_DIR)/libs
APPS_DIR		:= $(TOP_DIR)/apps

TARGET_CC := gcc
TARGET_CXX := g++

export TOP_DIR
export TARGET_CC
export TARGET_CXX
export LIBS_DIR
export APPS_DIR

nothing:
	@echo "Nothing to do"

include build/build.mk
