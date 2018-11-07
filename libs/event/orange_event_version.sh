#!/bin/sh
BUILD_NUM_FILE="build-number.txt"
BUILD_NUM=0
if [ -f ${BUILD_NUM_FILE} ] ; then
	BUILD_NUM=`cat ${BUILD_NUM_FILE}`
fi
echo -n "BuildNum: "
let BUILD_NUM=${BUILD_NUM}+1
echo ${BUILD_NUM} > ${BUILD_NUM_FILE}
echo "#include \"orange_event_version.h\"" > orange_event_version.c
echo "int orange_event_build_num = ${BUILD_NUM};" >> orange_event_version.c
echo "char orange_event_build_date[] = \"`date`\";" >> orange_event_version.c

