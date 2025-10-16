#define _GNU_SOURCE // to access O_DIRECT
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

// Default device paths - can be overridden at compile time
#ifndef FAST_DEVICE_PATH
//#define FAST_DEVICE_PATH "/dev/nvme0n1"
#define FAST_DEVICE_PATH "/dev/vdb"
#endif

#ifndef MIDDLE_DEVICE_PATH
//#define MIDDLE_DEVICE_PATH "/dev/nvme1n1"
#define MIDDLE_DEVICE_PATH "/dev/vdc"
#endif

#ifndef SLOW_DEVICE_PATH
//#define SLOW_DEVICE_PATH "/dev/nvme2n1"
#define SLOW_DEVICE_PATH "/dev/vdd"
#endif

int openFastDevice()
{
	int fp = open(FAST_DEVICE_PATH, O_RDWR|O_SYNC|O_DIRECT);
    printf("Opening fast device: %s, FP=%d\n", FAST_DEVICE_PATH, fp);
    if(fp <= 0) {
        perror("Error opening fast device");
        return(-1);
    }
	return fp;
}

int openMiddleDevice()
{
	int fp = open(MIDDLE_DEVICE_PATH, O_RDWR|O_SYNC|O_DIRECT);
    printf("Opening middle device: %s, FP=%d\n", MIDDLE_DEVICE_PATH, fp);
    if(fp <= 0) {
        perror("Error opening middle device");
        return(-1);
    }
	return fp;
}

int openSlowDevice()
{
	int fp = open(SLOW_DEVICE_PATH, O_RDWR|O_SYNC|O_DIRECT);
    printf("Opening slow device: %s, FP=%d\n", SLOW_DEVICE_PATH, fp);
    if(fp <= 0) {
        perror("Error opening slow device");
        return(-1);
    }
	return fp;
}

int sibyl_read(int fd, unsigned long byte_offset, unsigned int nSize)
{
	char readBuf[nSize] __attribute__ ((__aligned__ (4096)));
	memset(readBuf, 0x00, sizeof(char) * nSize);
	off_t readOffset = lseek(fd, byte_offset, SEEK_SET);
	ssize_t len = read(fd, readBuf, sizeof(char) * nSize);
	return len;
	// print the return value to check
}

int sibyl_write(int fd, unsigned long byte_offset, unsigned int nSize)
{
	char writeBuf[nSize] __attribute__ ((__aligned__ (4096)));
	memset(writeBuf, 0xA5, sizeof(char) * nSize);
	off_t writeOffset = lseek(fd, byte_offset, SEEK_SET);
	ssize_t len = write(fd, writeBuf, sizeof(char) * nSize);
	return len;
	// print the return value to check
}

void closeDevice(int fd)
{
	close(fd);
}
