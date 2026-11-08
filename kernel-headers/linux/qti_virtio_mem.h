/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_QTI_VIRTIO_MEM_H
#define _UAPI_LINUX_QTI_VIRTIO_MEM_H
#include <linux/ioctl.h>
#include <linux/types.h>
#define QTI_VIRTIO_MEM_IOC_MAGIC 'M'
#define QTI_VIRTIO_MEM_IOC_MAX_NAME_LEN 128
struct qti_virtio_mem_ioc_hint_create_arg {
  char name[QTI_VIRTIO_MEM_IOC_MAX_NAME_LEN];
  __s64 size;
  __u32 fd;
  __u32 reserved0;
  __u64 reserved1;
};
#define QTI_VIRTIO_MEM_IOC_HINT_CREATE _IOWR(QTI_VIRTIO_MEM_IOC_MAGIC, 0, struct qti_virtio_mem_ioc_hint_create_arg)
#endif
