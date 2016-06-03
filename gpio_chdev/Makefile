ARCH		:= arm
CROSS_COMPILE	:= arm-linux-gnueabi-

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

obj-m := gpio_chdev.o

KERNELDIR := /home/jack/Documents/course/EmbededSystem/RaspberrySource/modules/lib/modules/4.4.11-v7/build
PWD = $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -f *.o