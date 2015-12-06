obj-m += timer.o

PWD := $(shell pwd)

KVER ?= $(shell uname -r)

KDIR := /lib/modules/$(KVER)/build

all:

	make -C $(KDIR) M=$(PWD) modules

clean:

	rm -rf *.mod.c *.mod.o *.o *.tmp_versions *.order *symvers
