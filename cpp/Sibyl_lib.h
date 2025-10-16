#ifndef SIBYL_LIB_H
#define SIBYL_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

int openFastDevice();
int openMiddleDevice();
int openSlowDevice();
int sibyl_read(int fd, unsigned long byte_offset, unsigned int nSize);
int sibyl_write(int fd, unsigned long byte_offset, unsigned int nSize);
void closeDevice(int fd);

#ifdef __cplusplus
}
#endif

#endif // SIBYL_LIB_H 