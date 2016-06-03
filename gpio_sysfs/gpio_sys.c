#include "gpio_sys.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int GPIOExport(int pin)
{
	char buff[BUFFMAX];

	int fd;
	if((fd=open(SYSFS_GPIO_EXPORT, O_WRONLY)) == -1)
	{
		ERR("Failed to open export for writing!\n");
		return -1;
	}

	int len = snprintf(buff, sizeof(buff), "%d", pin);
	if(write(fd, buff, len) == -1)
	{
		ERR("Failed to export gpio!\n");
		return -1;
	}

	if(close(fd) == -1)
	{
		ERR("Failed to close export!\n");
		return -1;
	}
	return 0;
}

int GPIOUnexport(int pin)
{
	char buff[BUFFMAX];
	int fd;
	if((fd=open(SYSFS_GPIO_UNEXPORT, O_WRONLY)) == -1)
	{
		ERR("Failed to open unexport for writing!\n");
		return -1;		
	}

	int len = snprintf(buff, sizeof(buff), "%d", pin);
	if(write(fd, buff, len) == -1)
	{
		ERR("Failed to unexport gpio!\n");
		return -1;
	}

	if(close(fd) == -1)
	{
		ERR("Failed to close unexport!\n");
		return -1;
	}
	return 0;
}

int GPIODirection(int pin, int dir)
{
	char dirCh[][5] =  {"in", "out"};
	char path[64];

	int fd;
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
	printf(path);
	if((fd = open(path, O_WRONLY)) == -1)
	{
		ERR("Failed to open direction for writing!\n");
		return -1;	
	}

	if(write(fd, dirCh[dir], strlen(dirCh[dir])) == -1)
	{
		ERR("Failed to set direction!\n");
		return -1;		
	}

	if(close(fd) == -1)
	{
		ERR("Failed to close direction!\n");
		return -1;
	}
	return 0;
}

int GPIORead(int pin)
{
	char path[64];
	char buff[BUFFMAX];

	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);

	int fd;
	if((fd == open(path, O_RDONLY)) == -1)
	{
		ERR("Failed to open value for reading!\n");
		return -1;	
	}

	if(read(fd, buff, sizeof(buff)) == -1)
	{
		ERR("Failed to read value!\n");
		return -1;
	}

	if(close(fd) == -1)
	{
		ERR("Failed to close value!\n");
		return -1;
	}

	return atoi(buff);
}

int GPIOWrite(int pin, int value)
{
	char path[64];
	char valuestr[][2] = {"0", "1"};
	
	if(value != 0 && value != 1)
	{
		fprintf(stderr, "value = %d\n", value);
		ERR("Writing erro value!\n");
		return -1;
	}
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
	int fd;
	if((fd = open(path, O_WRONLY)) == -1)
	{
		ERR("Failed to open value for writing!\n");
		return -1;	
	}

	if(write(fd, valuestr[value], 1) == -1)
	{
		ERR("Failed to write value!\n");
		return -1;
	}

	if(close(fd) == -1)
	{
		ERR("Failed to close value!\n");
		return -1;
	}
	return 0;
}