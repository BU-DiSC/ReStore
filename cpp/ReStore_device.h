// ReStore_device.h - Raw block-device I/O for the storage tiers (implemented in device_driver/ReStore_device.c)
#ifndef RESTORE_DEVICE_H
#define RESTORE_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

const char *tierDevicePath(int tier_num);  // "" if the tier is simulated
int openFastDevice();
int openMiddleDevice();
int openSlowDevice();
int device_read(int fd, unsigned long byte_offset, unsigned int nSize);
int device_write(int fd, unsigned long byte_offset, unsigned int nSize);
void closeDevice(int fd);

#ifdef __cplusplus
}
#endif

#endif // RESTORE_DEVICE_H
