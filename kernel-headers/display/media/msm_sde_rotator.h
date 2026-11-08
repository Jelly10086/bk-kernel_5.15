/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef __UAPI_MSM_SDE_ROTATOR_H__
#define __UAPI_MSM_SDE_ROTATOR_H__
#include <linux/videodev2.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#define SDE_PIX_FMT_XRGB_8888 v4l2_fourcc('X', 'R', '2', '4')
#define SDE_PIX_FMT_ARGB_8888 v4l2_fourcc('A', 'R', '2', '4')
#define SDE_PIX_FMT_ABGR_8888 v4l2_fourcc('R', 'A', '2', '4')
#define SDE_PIX_FMT_RGBA_8888 v4l2_fourcc('A', 'B', '2', '4')
#define SDE_PIX_FMT_BGRA_8888 v4l2_fourcc('B', 'A', '2', '4')
#define SDE_PIX_FMT_RGBX_8888 v4l2_fourcc('X', 'B', '2', '4')
#define SDE_PIX_FMT_BGRX_8888 v4l2_fourcc('B', 'X', '2', '4')
#define SDE_PIX_FMT_XBGR_8888 v4l2_fourcc('R', 'X', '2', '4')
#define SDE_PIX_FMT_RGBA_5551 v4l2_fourcc('R', 'A', '1', '5')
#define SDE_PIX_FMT_ARGB_1555 v4l2_fourcc('A', 'R', '1', '5')
#define SDE_PIX_FMT_ABGR_1555 v4l2_fourcc('A', 'B', '1', '5')
#define SDE_PIX_FMT_BGRA_5551 v4l2_fourcc('B', 'A', '1', '5')
#define SDE_PIX_FMT_BGRX_5551 v4l2_fourcc('B', 'X', '1', '5')
#define SDE_PIX_FMT_RGBX_5551 v4l2_fourcc('R', 'X', '1', '5')
#define SDE_PIX_FMT_XBGR_1555 v4l2_fourcc('X', 'B', '1', '5')
#define SDE_PIX_FMT_XRGB_1555 v4l2_fourcc('X', 'R', '1', '5')
#define SDE_PIX_FMT_ARGB_4444 v4l2_fourcc('A', 'R', '1', '2')
#define SDE_PIX_FMT_RGBA_4444 v4l2_fourcc('R', 'A', '1', '2')
#define SDE_PIX_FMT_BGRA_4444 v4l2_fourcc('b', 'A', '1', '2')
#define SDE_PIX_FMT_ABGR_4444 v4l2_fourcc('A', 'B', '1', '2')
#define SDE_PIX_FMT_RGBX_4444 v4l2_fourcc('R', 'X', '1', '2')
#define SDE_PIX_FMT_XRGB_4444 v4l2_fourcc('X', 'R', '1', '2')
#define SDE_PIX_FMT_BGRX_4444 v4l2_fourcc('B', 'X', '1', '2')
#define SDE_PIX_FMT_XBGR_4444 v4l2_fourcc('X', 'B', '1', '2')
#define SDE_PIX_FMT_RGB_888 v4l2_fourcc('R', 'G', 'B', '3')
#define SDE_PIX_FMT_BGR_888 v4l2_fourcc('B', 'G', 'R', '3')
#define SDE_PIX_FMT_RGB_565 v4l2_fourcc('R', 'G', 'B', 'P')
#define SDE_PIX_FMT_BGR_565 v4l2_fourcc('B', 'G', '1', '6')
#define SDE_PIX_FMT_Y_CB_CR_H2V2 v4l2_fourcc('Y', 'U', '1', '2')
#define SDE_PIX_FMT_Y_CR_CB_H2V2 v4l2_fourcc('Y', 'V', '1', '2')
#define SDE_PIX_FMT_Y_CR_CB_GH2V2 v4l2_fourcc('Y', 'U', '4', '2')
#define SDE_PIX_FMT_Y_CBCR_H2V2 v4l2_fourcc('N', 'V', '1', '2')
#define SDE_PIX_FMT_Y_CRCB_H2V2 v4l2_fourcc('N', 'V', '2', '1')
#define SDE_PIX_FMT_Y_CBCR_H1V2 v4l2_fourcc('N', 'H', '1', '6')
#define SDE_PIX_FMT_Y_CRCB_H1V2 v4l2_fourcc('N', 'H', '6', '1')
#define SDE_PIX_FMT_Y_CBCR_H2V1 v4l2_fourcc('N', 'V', '1', '6')
#define SDE_PIX_FMT_Y_CRCB_H2V1 v4l2_fourcc('N', 'V', '6', '1')
#define SDE_PIX_FMT_YCBYCR_H2V1 v4l2_fourcc('Y', 'U', 'Y', 'V')
#define SDE_PIX_FMT_Y_CBCR_H2V2_VENUS v4l2_fourcc('Q', 'N', 'V', '2')
#define SDE_PIX_FMT_Y_CRCB_H2V2_VENUS v4l2_fourcc('Q', 'N', 'V', '1')
#define SDE_PIX_FMT_RGBA_8888_UBWC v4l2_fourcc('Q', 'R', 'G', 'B')
#define SDE_PIX_FMT_RGBX_8888_UBWC v4l2_fourcc('Q', 'X', 'B', '4')
#define SDE_PIX_FMT_RGB_565_UBWC v4l2_fourcc('Q', 'R', 'G', '6')
#define SDE_PIX_FMT_Y_CBCR_H2V2_UBWC v4l2_fourcc('Q', '1', '2', '8')
#define SDE_PIX_FMT_RGBA_1010102 v4l2_fourcc('A', 'B', '3', '0')
#define SDE_PIX_FMT_RGBX_1010102 v4l2_fourcc('X', 'B', '3', '0')
#define SDE_PIX_FMT_ARGB_2101010 v4l2_fourcc('A', 'R', '3', '0')
#define SDE_PIX_FMT_XRGB_2101010 v4l2_fourcc('X', 'R', '3', '0')
#define SDE_PIX_FMT_BGRA_1010102 v4l2_fourcc('B', 'A', '3', '0')
#define SDE_PIX_FMT_BGRX_1010102 v4l2_fourcc('B', 'X', '3', '0')
#define SDE_PIX_FMT_ABGR_2101010 v4l2_fourcc('R', 'A', '3', '0')
#define SDE_PIX_FMT_XBGR_2101010 v4l2_fourcc('R', 'X', '3', '0')
#define SDE_PIX_FMT_RGBA_1010102_UBWC v4l2_fourcc('Q', 'R', 'B', 'A')
#define SDE_PIX_FMT_RGBX_1010102_UBWC v4l2_fourcc('Q', 'X', 'B', 'A')
#define SDE_PIX_FMT_Y_CBCR_H2V2_P010 v4l2_fourcc('P', '0', '1', '0')
#define SDE_PIX_FMT_Y_CBCR_H2V2_P010_VENUS v4l2_fourcc('Q', 'P', '1', '0')
#define SDE_PIX_FMT_Y_CBCR_H2V2_TP10 v4l2_fourcc('T', 'P', '1', '0')
#define SDE_PIX_FMT_Y_CBCR_H2V2_TP10_UBWC v4l2_fourcc('Q', '1', '2', 'A')
#define SDE_PIX_FMT_Y_CBCR_H2V2_P010_UBWC v4l2_fourcc('Q', '1', '2', 'B')
struct msm_sde_rotator_fence {
  __u32 index;
  __u32 type;
  __s32 fd;
  __u32 reserved[5];
};
struct msm_sde_rotator_comp_ratio {
  __u32 index;
  __u32 type;
  __u32 numer;
  __u32 denom;
  __u32 reserved[4];
};
#define VIDIOC_G_SDE_ROTATOR_FENCE _IOWR('V', BASE_VIDIOC_PRIVATE + 10, struct msm_sde_rotator_fence)
#define VIDIOC_S_SDE_ROTATOR_FENCE _IOWR('V', BASE_VIDIOC_PRIVATE + 11, struct msm_sde_rotator_fence)
#define VIDIOC_G_SDE_ROTATOR_COMP_RATIO _IOWR('V', BASE_VIDIOC_PRIVATE + 12, struct msm_sde_rotator_comp_ratio)
#define VIDIOC_S_SDE_ROTATOR_COMP_RATIO _IOWR('V', BASE_VIDIOC_PRIVATE + 13, struct msm_sde_rotator_comp_ratio)
#define V4L2_CID_SDE_ROTATOR_SECURE (V4L2_CID_USER_BASE + 0x1000)
#define V4L2_CID_SDE_ROTATOR_SECURE_CAMERA (V4L2_CID_USER_BASE + 0x2000)
#endif
