
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_arm_v1 simd_test_arm_v2

strip:
	arm-linux-gnueabi-strip simd_test.arm_v*

clean:
	rm simd_test.arm_v*


simd_test_arm_v1:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_v1

simd_test_arm_v2:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM=2 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_v2


build_n900: simd_test_arm_n900

strip_n900:
	arm-linux-gnueabi-strip simd_test.arm_n900

clean_n900:
	rm simd_test.arm_n900


simd_test_arm_n900:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_n900


build_rpiX: simd_test_arm_rpi2 simd_test_arm_rpi3

strip_rpiX:
	arm-linux-gnueabihf-strip simd_test.arm_rpi*

clean_rpiX:
	rm simd_test.arm_rpi*


simd_test_arm_rpi2:
	arm-linux-gnueabihf-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM=2 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_rpi2

simd_test_arm_rpi3:
	arm-linux-gnueabihf-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM=2 -DRT_128=4 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_rpi3


# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for ARMv7 is installed and in the PATH variable.
# sudo apt-get install g++-arm-linux-gnueabi
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_arm.mk
# qemu-arm -cpu cortex-a8  simd_test.arm_v1
# qemu-arm -cpu cortex-a15 simd_test.arm_v2

# 0) Build flags above are intended for default "vanilla" ARMv7 target, while
# settings suitable for specific hardware platforms are given below (replace).
# 1) Nokia N900, Maemo 5 scratchbox: "vanilla"
# 2) Raspberry Pi 2, Raspbian: arm-linux-gnueabihf-g++ -DRT_ARM=2 -DRT_128=2
# 3) Raspberry Pi 3, Raspbian: arm-linux-gnueabihf-g++ -DRT_ARM=2 -DRT_128=4
