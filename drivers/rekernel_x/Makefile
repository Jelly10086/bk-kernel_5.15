rekernel_x-y := rkx.o rkx_genl.o rkx_netuid.o rkx_free_async.o rkx_frozen.o rkx_binder.o rkx_signal.o rkx_netfilter.o rkx_binder_kp.o rkx_binder_alloc.o

ifneq ($(KBUILD_EXTMOD),)
obj-m += rekernel_x.o
else
obj-$(CONFIG_REKERNEL_X) += rekernel_x.o
endif

RKX_VERSION ?= v1.6
ccflags-y += -DRKX_VERSION=\"$(RKX_VERSION)\"
