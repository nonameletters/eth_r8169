obj-m += nn_r8169.o

nn_r8169-objs := eth_r8169.o eth_tool.o

KBUILD_CFLAGS+="-std=gnu99"

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean 
