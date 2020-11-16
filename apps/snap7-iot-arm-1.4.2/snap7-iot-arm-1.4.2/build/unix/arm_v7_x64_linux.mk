##
## arm V7 64 Linux based (Tested on Ubuntu Server Raspberry Pi 4) Makefile
## Use make
##
TargetCPU  :=arm_v7_x64
OS         :=linux
CXXFLAGS   := -O3 -fPIC -pedantic

# Standard part

include common.mk

