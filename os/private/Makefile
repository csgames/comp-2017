obj-m += startup_simulator.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -O3 -o ngelctl ngelctl.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	cp ngelctl /usr/sbin/ngelctl
	chown root:root /usr/sbin/ngelctl
	chmod 4755 /usr/sbin/ngelctl
	
	chmod 0555 startup_simulator.ko
	chown root:root startup_simulator.ko
	echo startup_simulator >> /etc/modules
	cp startup_simulator.ko /lib/modules/$(shell uname -r)/kernel/drivers/char/
	cp startup_simulator.rules /lib/udev/rules.d
	depmod -a
	echo "reboot to finish install"

