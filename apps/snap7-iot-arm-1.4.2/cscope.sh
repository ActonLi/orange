#!/bin/bash

if [ "$1" = "linux" ] ; then

    # linux kernel architecture
    #arch   COPYING  crypto         drivers  include  ipc     Kconfig  lib          MAINTAINERS  mm   README      samples  security  tools  virt
    #block  CREDITS  Documentation  fs   init     Kbuild  kernel   localversion-rt  Makefile net  REPORTING-BUGS  scripts  sound usr

    linux_src="$2"
    [ -z "${linux_src}" ] && linux_src="drivers ipc lib mm block init kernel net fs init usr"

    [ ! -d "`pwd`/include" ] && echo "The target dir is not exist." && exit 0

    find `pwd`/include -name "*.[chSs]" > cscope.files
    find `pwd`/arch/x86 -name "*.[chSs]" >> cscope.files
    for d in ${linux_src}; do
        find `pwd`/$d -name "*.[chSs]" >> cscope.files
    done
    cscope -kbq -i cscope.files
    #cat cscope.files | xargs -J % exctags %
    exit 0
fi

find `pwd` -name "*.[chSs]" > cscope.files
find `pwd` -name "*.cc" >> cscope.files

cscope -kbq -i cscope.files

current_libs_dir=`pwd | awk -F "/" '{print $3}'`

phoenix_libs_dir=`sed -n '/^PHOENIX_LIB_SRC=/'p ~/.bashrc | sed 's/PHOENIX_LIB_SRC=//' | awk -F "/" '{print $3}'`

if [ "$phoenix_libs_dir" == "" ]; then
    echo "PHOENIX_LIB_SRC=~/$current_libs_dir/libs" >> ~/.bashrc
    echo "export PHOENIX_LIB_SRC" >> ~/.bashrc
    echo "PHOENIX_OS=Linux" >> ~/.bashrc
    echo "export PHOENIX_OS" >> ~/.bashrc

    echo "PHOENIX_CFLAGS=\"$PHOENIX_CFFLAGS -O2 -pipe -fPIC -fno-strict-aliasing -fno-common -Wall -Werror -Wno-pointer-sign -Wno-unused-parameter \
        -Wredundant-decls -Wnested-externs -Wstrict-prototypes \
        -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual -Wundef -g -std=c99 -fstack-protector\"" >> ~/.bashrc

    echo "PHOENIX_CXXFLAGS=\"-O2 -pipe -fPIC -fno-strict-aliasing -fno-common -Wall -Werror -Wno-unused-parameter \
        -Wredundant-decls -Wpointer-arith -Winline -Wcast-qual -Wundef\"" >> ~/.bashrc

    echo "export PHOENIX_CFLAGS" >> ~/.bashrc
    echo "export PHOENIX_CXXFLAGS" >> ~/.bashrc
fi

if [ "$current_libs_dir" != "$phoenix_libs_dir" ] && [ "$phoenix_libs_dir" != "" ]; then
    sed -i "s/\(^PHOENIX_LIB_SRC=\)\/root\/$phoenix_libs_dir\/libs/\1\/root\/$current_libs_dir\/libs/" ~/.bashrc
fi

ORANGE_SRC_DIR=`pwd`
export ORANGE_SRC_DIR

chmod a+x ~/.bashrc
source ~/.bashrc

#cat cscope.files | xargs -J % exctags %
exit 0

