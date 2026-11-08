/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef __QCOM_ADSPSLEEPMON_H__
#define __QCOM_ADSPSLEEPMON_H__
#include <linux/types.h>
#define ADSPSLEEPMON_DEVICE_NAME "/dev/msm_adsp_sleepmon"
#define ADSPSLEEPMON_IOCTL_AUDIO_ACTIVITY _IOWR('R', 1, struct adspsleepmon_ioctl_audio)
#define ADSPSLEEPMON_IOCTL_CONFIGURE_PANIC _IOWR('R', 2, struct adspsleepmon_ioctl_panic)
#define ADSPSLEEPMON_IOCTL_AUDIO_VER_1 1
#define ADSPSLEEPMON_IOCTL_CONFIG_PANIC_VER_1 1
#define ADSPSLEEPMON_IOCTL_AUDIO_NUM_RES 3
enum adspsleepmon_ioctl_audio_cmd {
  ADSPSLEEPMON_AUDIO_ACTIVITY_START = 1,
  ADSPSLEEPMON_AUDIO_ACTIVITY_STOP,
  ADSPSLEEPMON_AUDIO_ACTIVITY_LPI_START,
  ADSPSLEEPMON_AUDIO_ACTIVITY_LPI_STOP,
  ADSPSLEEPMON_AUDIO_ACTIVITY_RESET,
  ADSPSLEEPMON_AUDIO_ACTIVITY_MAX,
};
enum adspsleepmon_ioctl_panic_cmd {
  ADSPSLEEPMON_DISABLE_PANIC_LPM = 1,
  ADSPSLEEPMON_DISABLE_PANIC_LPI,
  ADSPSLEEPMON_RESET_PANIC_LPM,
  ADSPSLEEPMON_RESET_PANIC_LPI,
  ADSPSLEEPMON_RESET_PANIC_MAX,
};
struct adspsleepmon_ioctl_audio {
  __u32 version;
  __u32 command;
  __u32 reserved[ADSPSLEEPMON_IOCTL_AUDIO_NUM_RES];
};
struct adspsleepmon_ioctl_panic {
  __u32 version;
  __u32 command;
};
#endif
