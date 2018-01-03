obj-m+=charDeviceDriver.o 
charDeviceDriver-objs= charDeviceDriver_.o linkedList.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
