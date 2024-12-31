NAME = kernel_stack
obj-m = ${NAME}.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo insmod ${NAME}.ko dyndbg

uninstall:
	sudo rmmod ${NAME}.ko

reinstall: uninstall install 
