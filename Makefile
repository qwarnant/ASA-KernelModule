# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := kprobes.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /gfs/warnant/linux-3.19.1/
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) O=/gfs/warnant/build/kvm32/ M=$(PWD) modules
endif
