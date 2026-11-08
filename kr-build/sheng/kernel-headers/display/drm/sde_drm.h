/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef _SDE_DRM_H_
#define _SDE_DRM_H_
#include <drm/drm.h>
#ifdef __cplusplus
extern "C" {
#endif
#define SDE_MAX_PLANES 4
#define SDE_MAX_DE_CURVES 3
#define FILTER_EDGE_DIRECTED_2D 0x0
#define FILTER_CIRCULAR_2D 0x1
#define FILTER_SEPARABLE_1D 0x2
#define FILTER_BILINEAR 0x3
#define FILTER_ALPHA_DROP_REPEAT 0x0
#define FILTER_ALPHA_BILINEAR 0x1
#define FILTER_ALPHA_2D 0x3
#define FILTER_BLEND_CIRCULAR_2D 0x0
#define FILTER_BLEND_SEPARABLE_1D 0x1
#define SCALER_LUT_SWAP 0x1
#define SCALER_LUT_DIR_WR 0x2
#define SCALER_LUT_Y_CIR_WR 0x4
#define SCALER_LUT_UV_CIR_WR 0x8
#define SCALER_LUT_Y_SEP_WR 0x10
#define SCALER_LUT_UV_SEP_WR 0x20
#define DRM_FORMAT_MOD_QCOM_DX fourcc_mod_code(QCOM, 0x2)
#define DRM_FORMAT_MOD_QCOM_TIGHT fourcc_mod_code(QCOM, 0x4)
#define DRM_FORMAT_MOD_QCOM_TILE fourcc_mod_code(QCOM, 0x8)
#define DRM_FORMAT_MOD_QCOM_ALPHA_SWAP fourcc_mod_code(QCOM, 0x10)
#define SDE_DRM_BLEND_OP_NOT_DEFINED 0
#define SDE_DRM_BLEND_OP_OPAQUE 1
#define SDE_DRM_BLEND_OP_PREMULTIPLIED 2
#define SDE_DRM_BLEND_OP_COVERAGE 3
#define SDE_DRM_BLEND_OP_MAX 4
#define SDE_DRM_BLEND_OP_SKIP 5
#define SDE_DRM_DEINTERLACE 0
#define SDE_DRM_BITMASK_COUNT 64
#define SDE_DRM_FB_NON_SEC 0
#define SDE_DRM_FB_SEC 1
#define SDE_DRM_FB_NON_SEC_DIR_TRANS 2
#define SDE_DRM_FB_SEC_DIR_TRANS 3
#define SDE_DRM_SEC_NON_SEC 0
#define SDE_DRM_SEC_ONLY 1
struct sde_drm_pix_ext_v1 {
  __s32 num_ext_pxls_lr[SDE_MAX_PLANES];
  __s32 num_ext_pxls_tb[SDE_MAX_PLANES];
  __s32 left_ftch[SDE_MAX_PLANES];
  __s32 right_ftch[SDE_MAX_PLANES];
  __s32 top_ftch[SDE_MAX_PLANES];
  __s32 btm_ftch[SDE_MAX_PLANES];
  __s32 left_rpt[SDE_MAX_PLANES];
  __s32 right_rpt[SDE_MAX_PLANES];
  __s32 top_rpt[SDE_MAX_PLANES];
  __s32 btm_rpt[SDE_MAX_PLANES];
};
struct sde_drm_scaler_v1 {
  struct sde_drm_pix_ext_v1 pe;
  __s32 init_phase_x[SDE_MAX_PLANES];
  __s32 phase_step_x[SDE_MAX_PLANES];
  __s32 init_phase_y[SDE_MAX_PLANES];
  __s32 phase_step_y[SDE_MAX_PLANES];
  __u32 horz_filter[SDE_MAX_PLANES];
  __u32 vert_filter[SDE_MAX_PLANES];
};
struct sde_drm_de_v1 {
  __u32 enable;
  __s16 sharpen_level1;
  __s16 sharpen_level2;
  __u16 clip;
  __u16 limit;
  __u16 thr_quiet;
  __u16 thr_dieout;
  __u16 thr_low;
  __u16 thr_high;
  __u16 prec_shift;
  __s16 adjust_a[SDE_MAX_DE_CURVES];
  __s16 adjust_b[SDE_MAX_DE_CURVES];
  __s16 adjust_c[SDE_MAX_DE_CURVES];
};
#define SDE_DYN_EXP_DISABLE 0x1
#define SDE_DE_LPF_BLEND_FILT
#define SDE_DE_LPF_BLEND_FLAG_EN (1 << 0)
#define SDE_DRM_QSEED3LITE
#define SDE_DRM_QSEED4
#define SDE_DRM_INLINE_PREDOWNSCALE
struct sde_drm_scaler_v2 {
  __u32 enable;
  __u32 dir_en;
  struct sde_drm_pix_ext_v1 pe;
  __u32 horz_decimate;
  __u32 vert_decimate;
  __s32 init_phase_x[SDE_MAX_PLANES];
  __s32 phase_step_x[SDE_MAX_PLANES];
  __s32 init_phase_y[SDE_MAX_PLANES];
  __s32 phase_step_y[SDE_MAX_PLANES];
  __u32 preload_x[SDE_MAX_PLANES];
  __u32 preload_y[SDE_MAX_PLANES];
  __u32 src_width[SDE_MAX_PLANES];
  __u32 src_height[SDE_MAX_PLANES];
  __u32 dst_width;
  __u32 dst_height;
  __u32 y_rgb_filter_cfg;
  __u32 uv_filter_cfg;
  __u32 alpha_filter_cfg;
  __u32 blend_cfg;
  __u32 lut_flag;
  __u32 dir_lut_idx;
  __u32 y_rgb_cir_lut_idx;
  __u32 uv_cir_lut_idx;
  __u32 y_rgb_sep_lut_idx;
  __u32 uv_sep_lut_idx;
  struct sde_drm_de_v1 de;
  __u32 dir_weight;
  __u32 unsharp_mask_blend;
  __u32 de_blend;
  __u32 flags;
  __u32 pre_downscale_x_0;
  __u32 pre_downscale_x_1;
  __u32 pre_downscale_y_0;
  __u32 pre_downscale_y_1;
  __u32 de_lpf_flags;
  __u32 de_lpf_h;
  __u32 de_lpf_l;
  __u32 de_lpf_m;
};
#define SDE_MAX_DS_COUNT 4
#define SDE_DRM_DESTSCALER_ENABLE 0x1
#define SDE_DRM_DESTSCALER_SCALE_UPDATE 0x2
#define SDE_DRM_DESTSCALER_ENHANCER_UPDATE 0x4
#define SDE_DRM_DESTSCALER_PU_ENABLE 0x8
struct sde_drm_dest_scaler_cfg {
  __u32 flags;
  __u32 index;
  __u32 lm_width;
  __u32 lm_height;
  __u64 scaler_cfg;
};
struct sde_drm_dest_scaler_data {
  __u32 num_dest_scaler;
  struct sde_drm_dest_scaler_cfg ds_cfg[SDE_MAX_DS_COUNT];
};
#define SDE_CSC_MATRIX_COEFF_SIZE 9
#define SDE_CSC_CLAMP_SIZE 6
#define SDE_CSC_BIAS_SIZE 3
struct sde_drm_csc_v1 {
  __s64 ctm_coeff[SDE_CSC_MATRIX_COEFF_SIZE];
  __u32 pre_bias[SDE_CSC_BIAS_SIZE];
  __u32 post_bias[SDE_CSC_BIAS_SIZE];
  __u32 pre_clamp[SDE_CSC_CLAMP_SIZE];
  __u32 post_clamp[SDE_CSC_CLAMP_SIZE];
};
struct sde_drm_color {
  __u32 color_0;
  __u32 color_1;
  __u32 color_2;
  __u32 color_3;
};
#define SDE_MAX_DIM_LAYERS 7
#define SDE_DRM_DIM_LAYER_INCLUSIVE 0x1
#define SDE_DRM_DIM_LAYER_EXCLUSIVE 0x2
#define SDE_DP_DSC_RESERVATION_SWITCH (1 << 0)
struct sde_drm_dim_layer_cfg {
  __u32 flags;
  __u32 stage;
  struct sde_drm_color color_fill;
  struct drm_clip_rect rect;
};
struct sde_drm_dim_layer_v1 {
  __u32 num_layers;
  struct sde_drm_dim_layer_cfg layer_cfg[SDE_MAX_DIM_LAYERS];
};
#define SDE_DRM_WB_CFG 0x1
#define SDE_DRM_WB_CFG_FLAGS_CONNECTED (1 << 0)
struct sde_drm_wb_cfg {
  __u32 flags;
  __u32 connector_id;
  __u32 count_modes;
  __u64 modes;
};
#define SDE_MAX_ROI_V1 4
struct sde_drm_roi_v1 {
  __u32 num_rects;
  struct drm_clip_rect roi[SDE_MAX_ROI_V1];
};
#define SDE_MODE_DPMS_ON 0
#define SDE_MODE_DPMS_LP1 1
#define SDE_MODE_DPMS_LP2 2
#define SDE_MODE_DPMS_STANDBY 3
#define SDE_MODE_DPMS_SUSPEND 4
#define SDE_MODE_DPMS_OFF 5
#define SDE_RECOVERY_SUCCESS 0
#define SDE_RECOVERY_CAPTURE 1
#define SDE_RECOVERY_HARD_RESET 2
#define UBWC_STATS_MAX_ROI 0x3
struct sde_drm_ubwc_stats_roi {
  __u16 y_coord0;
  __u16 y_coord1;
};
struct sde_drm_ubwc_stats_data {
  struct sde_drm_ubwc_stats_roi roi;
  __u16 worst_bw[UBWC_STATS_MAX_ROI];
  __u16 worst_bw_y_coord[UBWC_STATS_MAX_ROI];
  __u32 total_bw[UBWC_STATS_MAX_ROI];
  __u32 error;
  __u32 meta_error;
};
#define SDE_FRAME_DATA_BUFFER_MAX 0x3
#define SDE_FRAME_DATA_GUARD_BYTES 0xFF
#define SDE_FRAME_DATA_MAX_PLANES 0x14
struct sde_drm_frame_data_buffers_ctrl {
  __u32 num_buffers;
  __u32 fds[SDE_FRAME_DATA_BUFFER_MAX];
};
struct sde_drm_frame_data_buf {
  __u32 fd;
  __u32 offset;
  __u32 status;
};
struct sde_drm_plane_frame_data {
  __u32 plane_id;
  struct sde_drm_ubwc_stats_data ubwc_stats;
};
struct sde_drm_frame_data_packet {
  __u32 frame_count;
  __u64 commit_count;
  struct sde_drm_plane_frame_data plane_frame_data[SDE_FRAME_DATA_MAX_PLANES];
};
#define DRM_EDID_CLRMETRY_xvYCC_601 (1 << 0)
#define DRM_EDID_CLRMETRY_xvYCC_709 (1 << 1)
#define DRM_EDID_CLRMETRY_sYCC_601 (1 << 2)
#define DRM_EDID_CLRMETRY_ADOBE_YCC_601 (1 << 3)
#define DRM_EDID_CLRMETRY_ADOBE_RGB (1 << 4)
#define DRM_EDID_CLRMETRY_BT2020_CYCC (1 << 5)
#define DRM_EDID_CLRMETRY_BT2020_YCC (1 << 6)
#define DRM_EDID_CLRMETRY_BT2020_RGB (1 << 7)
#define DRM_EDID_CLRMETRY_DCI_P3 (1 << 15)
#define HDR_PRIMARIES_COUNT 3
#define HDR_EOTF_SDR_LUM_RANGE 0x0
#define HDR_EOTF_HDR_LUM_RANGE 0x1
#define HDR_EOTF_SMTPE_ST2084 0x2
#define HDR_EOTF_HLG 0x3
#define DRM_MSM_EXT_HDR_METADATA
#define DRM_MSM_EXT_HDR_PLUS_METADATA
struct drm_msm_ext_hdr_metadata {
  __u32 hdr_state;
  __u32 eotf;
  __u32 hdr_supported;
  __u32 display_primaries_x[HDR_PRIMARIES_COUNT];
  __u32 display_primaries_y[HDR_PRIMARIES_COUNT];
  __u32 white_point_x;
  __u32 white_point_y;
  __u32 max_luminance;
  __u32 min_luminance;
  __u32 max_content_light_level;
  __u32 max_average_light_level;
  __u64 hdr_plus_payload;
  __u32 hdr_plus_payload_size;
};
#define DRM_MSM_EXT_HDR_PROPERTIES
#define DRM_MSM_EXT_HDR_PLUS_PROPERTIES
struct drm_msm_ext_hdr_properties {
  __u8 hdr_metadata_type_one;
  __u32 hdr_supported;
  __u32 hdr_eotf;
  __u32 hdr_max_luminance;
  __u32 hdr_avg_luminance;
  __u32 hdr_min_luminance;
  __u32 hdr_plus_supported;
};
#define DISPLAY_PRIMARIES_WX 0
#define DISPLAY_PRIMARIES_WY 1
#define DISPLAY_PRIMARIES_RX 2
#define DISPLAY_PRIMARIES_RY 3
#define DISPLAY_PRIMARIES_GX 4
#define DISPLAY_PRIMARIES_GY 5
#define DISPLAY_PRIMARIES_BX 6
#define DISPLAY_PRIMARIES_BY 7
#define DISPLAY_PRIMARIES_MAX 8
struct drm_panel_hdr_properties {
  __u32 hdr_enabled;
  __u32 display_primaries[DISPLAY_PRIMARIES_MAX];
  __u32 peak_brightness;
  __u32 blackness_level;
};
struct drm_msm_event_req {
  __u32 object_id;
  __u32 object_type;
  __u32 event;
  __u64 client_context;
  __u32 index;
};
struct drm_msm_event_resp {
  struct drm_event base;
  struct drm_msm_event_req info;
  __u8 data[];
};
struct drm_msm_power_ctrl {
  __u32 enable;
  __u32 flags;
};
struct drm_msm_early_wakeup {
  __u32 wakeup_hint;
  __u32 connector_id;
};
struct drm_msm_display_hint {
  __u64 data;
  __u32 hint_flags;
};
#define DRM_NOISE_LAYER_CFG
#define DRM_NOISE_TEMPORAL_FLAG (1 << 0)
#define DRM_NOISE_ATTN_MAX 255
#define DRM_NOISE_STREN_MAX 6
struct drm_msm_noise_layer_cfg {
  __u64 flags;
  __u32 zposn;
  __u32 zposattn;
  __u32 attn_factor;
  __u32 strength;
  __u32 alpha_noise;
};
#define FEATURE_DNSC_BLUR
#define DNSC_BLUR_COEF_NUM 64
#define DNSC_BLUR_EN (1 << 0)
#define DNSC_BLUR_RND_8B_EN (1 << 1)
#define DNSC_BLUR_DITHER_EN (1 << 2)
#define DNSC_BLUR_MIRROR_BLK_CFG (1 << 16)
#define DNSC_BLUR_INDEPENDENT_BLK_CFG (1 << 17)
#define DNSC_BLUR_GAUS_FILTER (1 << 0)
#define DNSC_BLUR_PCMN_FILTER (1 << 1)
#define DNSC_BLUR_DITHER_MATRIX_SZ 16
#define DNSC_BLUR_DITHER_LUMA_MODE (1 << 0)
struct sde_drm_dnsc_blur_cfg {
  __u64 flags;
  __u32 num_blocks;
  __u32 src_width;
  __u32 src_height;
  __u32 dst_width;
  __u32 dst_height;
  __u32 flags_h;
  __u32 flags_v;
  __u32 phase_init_h;
  __u32 phase_step_h;
  __u32 phase_init_v;
  __u32 phase_step_v;
  __u32 norm_h;
  __u32 ratio_h;
  __u32 norm_v;
  __u32 ratio_v;
  __u32 coef_hori[DNSC_BLUR_COEF_NUM];
  __u32 coef_vert[DNSC_BLUR_COEF_NUM];
  __u64 dither_flags;
  __u32 temporal_en;
  __u32 c0_bitdepth;
  __u32 c1_bitdepth;
  __u32 c2_bitdepth;
  __u32 c3_bitdepth;
  __u32 dither_matrix[DNSC_BLUR_DITHER_MATRIX_SZ];
};
#define DRM_SDE_WB_CONFIG 0x40
#define DRM_MSM_REGISTER_EVENT 0x41
#define DRM_MSM_DEREGISTER_EVENT 0x42
#define DRM_MSM_RMFB2 0x43
#define DRM_MSM_POWER_CTRL 0x44
#define DRM_MSM_DISPLAY_HINT 0x45
#define DRM_EVENT_HISTOGRAM 0x80000000
#define DRM_EVENT_AD_BACKLIGHT 0x80000001
#define DRM_EVENT_CRTC_POWER 0x80000002
#define DRM_EVENT_SYS_BACKLIGHT 0x80000003
#define DRM_EVENT_SDE_POWER 0x80000004
#define DRM_EVENT_IDLE_NOTIFY 0x80000005
#define DRM_EVENT_PANEL_DEAD 0x80000006
#define DRM_EVENT_SDE_HW_RECOVERY 0X80000007
#define DRM_EVENT_LTM_HIST 0X80000008
#define DRM_EVENT_LTM_WB_PB 0X80000009
#define DRM_EVENT_LTM_OFF 0X8000000A
#define DRM_EVENT_MMRM_CB 0X8000000B
#define DRM_EVENT_FRAME_DATA 0x8000000C
#define DRM_EVENT_DIMMING_BL 0X8000000D
#define DRM_EVENT_VM_RELEASE 0X8000000E
#define DRM_EVENT_OPR_VALUE 0X8000000F
#define DRM_EVENT_MISR_SIGN 0X80000010
#ifndef DRM_MODE_FLAG_VID_MODE_PANEL
#define DRM_MODE_FLAG_VID_MODE_PANEL 0x01
#endif
#ifndef DRM_MODE_FLAG_CMD_MODE_PANEL
#define DRM_MODE_FLAG_CMD_MODE_PANEL 0x02
#endif
#define DRM_MSM_DISPLAY_EARLY_WAKEUP_HINT 0x01
#define DRM_MSM_DISPLAY_POWER_COLLAPSE_HINT 0x02
#define DRM_MSM_DISPLAY_IDLE_TIMEOUT_HINT 0x04
#define DRM_MSM_DISPLAY_MODE_CHANGE_HINT 0x08
#define DRM_MSM_WAKE_UP_ALL_DISPLAYS 0xFFFFFFFF
#define DRM_IOCTL_SDE_WB_CONFIG DRM_IOW((DRM_COMMAND_BASE + DRM_SDE_WB_CONFIG), struct sde_drm_wb_cfg)
#define DRM_IOCTL_MSM_REGISTER_EVENT DRM_IOW((DRM_COMMAND_BASE + DRM_MSM_REGISTER_EVENT), struct drm_msm_event_req)
#define DRM_IOCTL_MSM_DEREGISTER_EVENT DRM_IOW((DRM_COMMAND_BASE + DRM_MSM_DEREGISTER_EVENT), struct drm_msm_event_req)
#define DRM_IOCTL_MSM_RMFB2 DRM_IOW((DRM_COMMAND_BASE + DRM_MSM_RMFB2), unsigned int)
#define DRM_IOCTL_MSM_POWER_CTRL DRM_IOW((DRM_COMMAND_BASE + DRM_MSM_POWER_CTRL), struct drm_msm_power_ctrl)
#define DRM_IOCTL_MSM_DISPLAY_HINT DRM_IOW((DRM_COMMAND_BASE + DRM_MSM_DISPLAY_HINT), struct drm_msm_display_hint)
#ifdef __cplusplus
}
#endif
#endif
