obj-m += eth_r8169.o
KBUILD_CFLAGS+="-std=gnu99"

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean 
