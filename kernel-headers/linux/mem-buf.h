/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _UAPI_LINUX_MEM_BUF_H
#define _UAPI_LINUX_MEM_BUF_H
#include <linux/ioctl.h>
#include <linux/types.h>
#define MEM_BUF_IOC_MAGIC 'M'
enum mem_buf_mem_type {
  MEM_BUF_ION_MEM_TYPE,
  MEM_BUF_MAX_MEM_TYPE,
};
#define MEM_BUF_DMAHEAP_MEM_TYPE (MEM_BUF_ION_MEM_TYPE + 1)
#define MEM_BUF_VMID_PRIMARY_VM 0
#define MEM_BUF_VMID_TRUSTED_VM 1
#define MEM_BUF_PERM_FLAG_READ (1U << 0)
#define MEM_BUF_PERM_FLAG_WRITE (1U << 1)
#define MEM_BUF_PERM_FLAG_EXEC (1U << 2)
#define MEM_BUF_PERM_VALID_FLAGS (MEM_BUF_PERM_FLAG_READ | MEM_BUF_PERM_FLAG_WRITE | MEM_BUF_PERM_FLAG_EXEC)
#define MEM_BUF_MAX_NR_ACL_ENTS 16
struct acl_entry {
  __u32 vmid;
  __u32 perms;
};
struct mem_buf_ion_data {
  __u32 heap_id;
};
#define MEM_BUF_MAX_DMAHEAP_NAME_LEN 128
struct mem_buf_dmaheap_data {
  __u64 heap_name;
};
struct mem_buf_alloc_ioctl_arg {
  __u64 size;
  __u64 acl_list;
  __u32 nr_acl_entries;
  __u32 src_mem_type;
  __u64 src_data;
  __u32 dst_mem_type;
  __u32 mem_buf_fd;
  __u64 dst_data;
  __u64 reserved0;
  __u64 reserved1;
  __u64 reserved2;
};
#define MEM_BUF_IOC_ALLOC _IOWR(MEM_BUF_IOC_MAGIC, 0, struct mem_buf_alloc_ioctl_arg)
struct mem_buf_lend_ioctl_arg {
  __u32 dma_buf_fd;
  __u32 nr_acl_entries;
  __u64 acl_list;
  __u64 memparcel_hdl;
  __u64 reserved0;
  __u64 reserved1;
  __u64 reserved2;
};
#define MEM_BUF_IOC_LEND _IOWR(MEM_BUF_IOC_MAGIC, 3, struct mem_buf_lend_ioctl_arg)
#define MEM_BUF_VALID_FD_FLAGS (O_CLOEXEC | O_ACCMODE)
struct mem_buf_retrieve_ioctl_arg {
  __u32 sender_vm_fd;
  __u32 nr_acl_entries;
  __u64 acl_list;
  __u64 memparcel_hdl;
  __u32 dma_buf_import_fd;
  __u32 fd_flags;
  __u64 reserved0;
  __u64 reserved1;
  __u64 reserved2;
};
#define MEM_BUF_IOC_RETRIEVE _IOWR(MEM_BUF_IOC_MAGIC, 4, struct mem_buf_retrieve_ioctl_arg)
struct mem_buf_reclaim_ioctl_arg {
  __u64 memparcel_hdl;
  __u32 dma_buf_fd;
  __u32 reserved0;
  __u64 reserved1;
  __u64 reserved2;
};
#define MEM_BUF_IOC_RECLAIM _IOWR(MEM_BUF_IOC_MAGIC, 3, struct mem_buf_reclaim_ioctl_arg)
struct mem_buf_share_ioctl_arg {
  __u32 dma_buf_fd;
  __u32 nr_acl_entries;
  __u64 acl_list;
  __u64 memparcel_hdl;
  __u64 reserved0;
  __u64 reserved1;
  __u64 reserved2;
};
#define MEM_BUF_IOC_SHARE _IOWR(MEM_BUF_IOC_MAGIC, 6, struct mem_buf_share_ioctl_arg)
struct mem_buf_exclusive_owner_ioctl_arg {
  __u32 dma_buf_fd;
  __u32 is_exclusive_owner;
};
#define MEM_BUF_IOC_EXCLUSIVE_OWNER _IOWR(MEM_BUF_IOC_MAGIC, 2, struct mem_buf_exclusive_owner_ioctl_arg)
#endif
