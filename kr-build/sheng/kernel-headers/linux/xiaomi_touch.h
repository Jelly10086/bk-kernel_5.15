/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef __UAPI__XIAOMI__TOUCH_H
#define __UAPI__XIAOMI__TOUCH_H
#include <linux/ioctl.h>
#include <linux/types.h>
enum touch_mode {
  TOUCH_MODE_SINGLETAP_GESTURE,
  TOUCH_MODE_DOUBLETAP_GESTURE,
  TOUCH_MODE_FOD_PRESS_GESTURE,
  TOUCH_MODE_FOD_FINGER_STATE,
  TOUCH_MODE_NONUI_MODE,
  TOUCH_MODE_REPORT_RATE,
  TOUCH_MODE_FOLD_STATUS,
  TOUCH_MODE_NUM,
};
enum touch_mode_cmd {
  TOUCH_MODE_SET,
  TOUCH_MODE_GET,
};
struct touch_mode_request {
  enum touch_mode mode;
  int value;
};
enum touch_fold_status {
  TOUCH_FOLD_STATUS_UNFOLDED,
  TOUCH_FOLD_STATUS_FOLDED,
  TOUCH_FOLD_STATUS_NUM,
};
#define TOUCH_IOC_SET_CUR_VALUE _IOW('T', TOUCH_MODE_SET, struct touch_mode_request)
#define TOUCH_IOC_GET_CUR_VALUE _IOR('T', TOUCH_MODE_GET, struct touch_mode_request)
#endif
