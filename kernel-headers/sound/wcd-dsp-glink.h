/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _WCD_DSP_GLINK_H
#define _WCD_DSP_GLINK_H
#include <linux/types.h>
#define WDSP_CH_NAME_MAX_LEN 50
enum {
  WDSP_REG_PKT = 1,
  WDSP_CMD_PKT,
  WDSP_READY_PKT,
};
#define WDSP_READY_PKT WDSP_READY_PKT
struct wdsp_reg_pkt {
  __u8 no_of_channels;
  __u8 payload[0];
};
struct wdsp_cmd_pkt {
  char ch_name[WDSP_CH_NAME_MAX_LEN];
  __u32 payload_size;
  __u8 payload[0];
};
struct wdsp_write_pkt {
  __u8 pkt_type;
  __u8 payload[0];
};
struct wdsp_glink_ch_cfg {
  char name[WDSP_CH_NAME_MAX_LEN];
  __u32 latency_in_us;
  __u32 no_of_intents;
  __u32 intents_size[0];
};
#endif
