#!/bin/bash

if [ ! -d certificate ];then
    tar xvf portal_client.tar.bz2
fi

if [ ! -d /home/wipap/portal ];then
    mkdir -p /home/wipap/portal
fi

rm -fr /home/wipap/portal/*

cp -v lib*.* /lib
cp -v openssl /usr/bin
cp -a app /home/wipap/portal/
cp -a certificate /home/wipap/portal/

if [ ! -d /usr/mnt/userdata/PortalClient ]; then
    mkdir -p /usr/mnt/userdata/PortalClient
fi

rm -fr /usr/mnt/userdata/PortalClient/*
cp -a certificate/* /usr/mnt/userdata/PortalClient/

#rm -fr lib*.*
#rm -fr openssl
#rm -fr app
#rm -fr certificate 

exit 0

