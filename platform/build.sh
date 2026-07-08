#!/bin/bash

CUR_PATH=`pwd`
RELEASE_PATH=$CUR_PATH/../release
KVM_PLATFORM='kvm'
SONIC_PLATFORM="otn-$KVM_PLATFORM"
SONIC_DEVICE='OLS-V'


BUILD_DIRS=(./HalPlatformApiServer ./OtnVirtualHal )
BUILD_TARS=('HalPlatformApiServer HalPlatformApiTest' 'otn-vhal')


####  check execute result
function return_val()
{
    val=`echo $?`
    if test $val -ne 0;then
        echo "#########  `pwd` is ERROR..."
        exit -1
    fi
}

function cmake_build()
{
    rm -fr build; return_val
    mkdir -p build; return_val
    cd build; return_val
    cmake ..; return_val
    make -j4; return_val
}

if [ $# -eq 2 ]; then
    SONIC_PLATFORM=$1
    SONIC_DEVICE=$2
elif [ $# -eq 1 ]; then
    SONIC_PLATFORM=$1
fi

echo "platform = $SONIC_PLATFORM"
echo "device   = $SONIC_DEVICE"

export BUILD_MODEL=$SONIC_PLATFORM


echo "building..."

#build servers
for ((i=0;i<${#BUILD_DIRS[*]};i++)); do
    cd ${BUILD_DIRS[i]}; return_val
    cmake_build

    cd $CUR_PATH; return_val
done

#package hal and platform server
cd HalPackage; return_val
#copy some dependent files to cache for package
CACHE_DIR=${CUR_PATH}/HalPackage/cache
rm -rf $CACHE_DIR; return_val
mkdir $CACHE_DIR; return_val

# ensure release path exists
mkdir -p "$RELEASE_PATH"; return_val

./pack.sh $SONIC_PLATFORM; return_val
cp build/*.deb $RELEASE_PATH; return_val
cd $CUR_PATH; return_val

#package hal platform client
cd HalPlatformApiClient; return_val
./pack.sh; return_val
cp *.deb $RELEASE_PATH; return_val
cd $CUR_PATH; return_val


echo "completed"
