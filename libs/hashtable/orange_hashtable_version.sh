#!/bin/sh

BUILD_NUM_FILE="build-number.txt"
BUILD_NUM=0
if [ -f ${BUILD_NUM_FILE} ] ; then
	BUILD_NUM=`cat ${BUILD_NUM_FILE}`
fi
echo -n "BuildNum: "
BUILD_NUM=`expr ${BUILD_NUM} + 1`
echo ${BUILD_NUM} > ${BUILD_NUM_FILE}
echo "#include \"orange_hashtable_version.h\"" > orange_hashtable_version.c
echo "int orange_hashtable_build_num = ${BUILD_NUM};" >> orange_hashtable_version.c
echo "char orange_hashtable_build_date[] = \"`date`\";" >> orange_hashtable_version.c

