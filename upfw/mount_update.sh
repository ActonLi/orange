#!/bin/bash

ret_mount="no"  

#mount update partion
if [ -e /dev/mtdblock10 ];then  #有升级分区
    mkdir -p /update
    umount /update 2>>/dev/null
    ubidetach /dev/ubi_ctrl -m 10  2>>/dev/null
    ubiattach /dev/ubi_ctrl -m 10 -d 1 &&
        mount -t ubifs ubi1:update /update -o sync &&
        ret_mount="yes" 

    if [ $ret_mount = "no" ] ;   #第一次mfgtool升级启动，没有格式化成ubi过
    then
        umount /update 2>>/dev/null
        ubidetach /dev/ubi_ctrl -m 10  2>>/dev/null
        ret_mount="no" &&
            flash_erase /dev/mtd10 0 0 &&
            ubiformat /dev/mtd10 &&
            ubiattach /dev/ubi_ctrl -m 10 -d 1 &&
            ubimkvol /dev/ubi1  -N update -m &&
            mount -t ubifs ubi1:update /update -o sync &&
            ret_mount="yes" 

        #        if [ $ret_mount = "no" ] ; 
        #        then
        #            #reboot
        #        else
        if [ $ret_mount = "yes" ];then
            rm -r -f /update/*
            sync
        fi
        #        fi
    else
        rm -r -f /update/* 
        sync
    fi
fi

exit 0

