// ReStore_device.c - Raw block-device I/O for storage tiers (fast, middle, slow).
// Built as the ReStore_device library and used by the Tier class in ReStore_driver_IO.hpp.
//
// Adapted from the driver of Sibyl (https://github.com/CMU-SAFARI/Sibyl), under the MIT License.

#define _GNU_SOURCE // to access O_DIRECT
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

// Device path of each tier. An empty path means the tier is simulated (see ReStore_driver_IO.hpp).
// Compile-time defaults (CMake passes -D..._DEVICE_PATH) can be overridden at run time with the
// environment variables RESTORE_FAST_DEVICE, RESTORE_MIDDLE_DEVICE, and RESTORE_SLOW_DEVICE
// (set by `restore run -fast_device ... -middle_device ... -slow_device ...`).
#ifndef FAST_DEVICE_PATH
#define FAST_DEVICE_PATH ""
#endif

#ifndef MIDDLE_DEVICE_PATH
#define MIDDLE_DEVICE_PATH ""
#endif

#ifndef SLOW_DEVICE_PATH
#define SLOW_DEVICE_PATH ""
#endif

const char *tierDevicePath(int tier_num)
{
	const char *env = NULL, *compiled = "";
	switch (tier_num) {
		case 1: env = getenv("RESTORE_FAST_DEVICE");   compiled = FAST_DEVICE_PATH;   break;
		case 2: env = getenv("RESTORE_MIDDLE_DEVICE"); compiled = MIDDLE_DEVICE_PATH; break;
		case 3: env = getenv("RESTORE_SLOW_DEVICE");   compiled = SLOW_DEVICE_PATH;   break;
	}
	return env ? env : compiled;
}

// WARNING: the tier devices are written to directly (raw O_DIRECT writes starting at offset 0),
// which destroys any partition table or filesystem on them.
// Safety guard: O_EXCL on a block device makes open() fail with EBUSY when the device, or any
// partition on it, is mounted or otherwise in use (e.g. the system disk, LVM, swap).
// Set RESTORE_ALLOW_BUSY_DEVICE=1 to skip this check, e.g. for scratch volumes that are mounted.
static int openTierDevice(const char *path, const char *label)
{
	int flags = O_RDWR|O_SYNC|O_DIRECT;
	const char *allow_busy = getenv("RESTORE_ALLOW_BUSY_DEVICE");
	if (!(allow_busy && strcmp(allow_busy, "1") == 0))
		flags |= O_EXCL;

	int fp = open(path, flags);
	printf("Opening %s device: %s, FP=%d\n", label, path, fp);
	if(fp < 0) {
		if (errno == EBUSY)
			fprintf(stderr, "Refusing to use %s device %s: it (or a partition on it) is mounted or in use.\n"
			                "Use an unused device, or set RESTORE_ALLOW_BUSY_DEVICE=1 if you are sure.\n", label, path);
		else
			fprintf(stderr, "Error opening %s device %s: %s\n", label, path, strerror(errno));
		return(-1);
	}
	return fp;
}

int openFastDevice()
{
	return openTierDevice(tierDevicePath(1), "fast");
}

int openMiddleDevice()
{
	return openTierDevice(tierDevicePath(2), "middle");
}

int openSlowDevice()
{
	return openTierDevice(tierDevicePath(3), "slow");
}

int device_read(int fd, unsigned long byte_offset, unsigned int nSize)
{
	char readBuf[nSize] __attribute__ ((__aligned__ (4096)));
	memset(readBuf, 0x00, sizeof(char) * nSize);
	off_t readOffset = lseek(fd, byte_offset, SEEK_SET);
	ssize_t len = read(fd, readBuf, sizeof(char) * nSize);
	return len;
	// print the return value to check
}

int device_write(int fd, unsigned long byte_offset, unsigned int nSize)
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
