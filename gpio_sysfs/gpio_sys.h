#ifndef _GPIO_SYS_H_
#define _GPIO_SYS_H_

#include <stdio.h>
#include <stdlib.h>

#define BUFFMAX 3
#define SYSFS_GPIO_EXPORT 	"/sys/class/gpio/export"
#define SYSFS_GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define SYSFS_GPIO_DIR_IN	0
#define SYSFS_GPIO_DIR_OUT	1
#define SYSFS_GPIO_VAL_HIGH 1
#define SYSFS_GPIO_VAL_LOW	0

#define ERR(args...) fprintf(stderr, "%s\n", args);

int GPIOExport(int pin);
int GPIOUnexport(int pin);
int GPIODirection(int pin, int dir);
int GPIORead(int pin);
int GPIOWrite(int pin, int value);

#endif