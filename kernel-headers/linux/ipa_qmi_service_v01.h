/*
 * This file is auto-generated. Modifications will be lost.
 *
 * See https://android.googlesource.com/platform/bionic/+/master/libc/kernel/
 * for more information.
 */
#ifndef IPA_QMI_SERVICE_V01_H
#define IPA_QMI_SERVICE_V01_H
#include <linux/types.h>
#define QMI_IPA_REMOTE_MHI_CHANNELS_NUM_MAX_V01 6
#define QMI_IPA_MAX_FILTERS_EX_V01 128
#define QMI_IPA_MAX_FILTERS_EX2_V01 256
#define QMI_IPA_IPFLTR_NUM_IHL_RANGE_16_EQNS_V01 2
#define QMI_IPA_MAX_FILTERS_V01 64
#define QMI_IPA_IPFLTR_NUM_MEQ_128_EQNS_V01 2
#define QMI_IPA_MAX_IPV4_ADD_LEN_V01 34
#define QMI_IPA_MAX_IPV6_ADD_LEN_V01 35
#define QMI_IPA_IPV6_WORD_ADDR_LEN_V01 4
#define QMI_IPA_MAX_ETH_HDR_SIZE_V01 64
#define QMI_IPA_ENDP_DESC_NUM_MAX_V01 31
#define QMI_IPA_MAX_APN_V01 8
#define QMI_IPA_MAX_CLIENT_DST_PIPES_V01 8
#define QMI_IPA_IPFLTR_NUM_IHL_MEQ_32_EQNS_V01 2
#define QMI_IPA_MAX_UL_FIREWALL_RULES_V01 64
#define QMI_IPA_REMOTE_MHI_MEMORY_MAPPING_NUM_MAX_V01 6
#define QMI_IPA_IPFLTR_NUM_MEQ_32_EQNS_V01 2
#define QMI_IPA_MAX_PIPES_V01 20
#define QMI_IPA_MAX_PER_CLIENTS_V01 64
#define IPA_QMI_SUPPORTS_STATS
#define IPA_QMI_SUPPORT_MHI_DEFAULT
#define IPA_INT_MAX ((int) (~0U >> 1))
#define IPA_INT_MIN (- IPA_INT_MAX - 1)
enum ipa_qmi_result_type_v01 {
  IPA_QMI_RESULT_TYPE_MIN_ENUM_VAL_V01 = IPA_INT_MIN,
  IPA_QMI_RESULT_SUCCESS_V01 = 0,
  IPA_QMI_RESULT_FAILURE_V01 = 1,
  IPA_QMI_RESULT_TYPE_MAX_ENUM_VAL_V01 = IPA_INT_MAX,
};
enum ipa_qmi_error_type_v01 {
  IPA_QMI_ERROR_TYPE_MIN_ENUM_VAL_V01 = IPA_INT_MIN,
  IPA_QMI_ERR_NONE_V01 = 0x0000,
  IPA_QMI_ERR_MALFORMED_MSG_V01 = 0x0001,
  IPA_QMI_ERR_NO_MEMORY_V01 = 0x0002,
  IPA_QMI_ERR_INTERNAL_V01 = 0x0003,
  IPA_QMI_ERR_CLIENT_IDS_EXHAUSTED_V01 = 0x0005,
  IPA_QMI_ERR_INVALID_ID_V01 = 0x0029,
  IPA_QMI_ERR_ENCODING_V01 = 0x003A,
  IPA_QMI_ERR_INCOMPATIBLE_STATE_V01 = 0x005A,
  IPA_QMI_ERR_NOT_SUPPORTED_V01 = 0x005E,
  IPA_QMI_ERROR_TYPE_MAX_ENUM_VAL_V01 = IPA_INT_MAX,
};
struct ipa_qmi_response_type_v01 {
  __u16 result;
  __u16 error;
};
enum ipa_platform_type_enum_v01 {
  IPA_PLATFORM_TYPE_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_PLATFORM_TYPE_INVALID_V01 = 0,
  QMI_IPA_PLATFORM_TYPE_TN_V01 = 1,
  QMI_IPA_PLATFORM_TYPE_LE_V01 = 2,
  QMI_IPA_PLATFORM_TYPE_MSM_ANDROID_V01 = 3,
  QMI_IPA_PLATFORM_TYPE_MSM_WINDOWS_V01 = 4,
  QMI_IPA_PLATFORM_TYPE_MSM_QNX_V01 = 5,
  QMI_IPA_PLATFORM_TYPE_LE_MHI_V01 = 6,
  IPA_PLATFORM_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
#define QMI_IPA_PLATFORM_TYPE_LE_MHI_V01 QMI_IPA_PLATFORM_TYPE_LE_MHI_V01
struct ipa_hdr_tbl_info_type_v01 {
  __u32 modem_offset_start;
  __u32 modem_offset_end;
};
struct ipa_route_tbl_info_type_v01 {
  __u32 route_tbl_start_addr;
  __u32 num_indices;
};
#define IPA_RQOS_FILTER_STATS_INFO
struct ipa_filter_stats_info_type_v01 {
  __u32 hw_filter_stats_start_addr;
  __u32 hw_filter_stats_size;
  __u8 hw_filter_stats_start_index;
  __u8 hw_filter_stats_end_index;
};
struct ipa_modem_mem_info_type_v01 {
  __u32 block_start_addr;
  __u32 size;
};
struct ipa_hdr_proc_ctx_tbl_info_type_v01 {
  __u32 modem_offset_start;
  __u32 modem_offset_end;
};
struct ipa_zip_tbl_info_type_v01 {
  __u32 modem_offset_start;
  __u32 modem_offset_end;
};
struct ipa_init_modem_driver_req_msg_v01 {
  __u8 platform_type_valid;
  enum ipa_platform_type_enum_v01 platform_type;
  __u8 hdr_tbl_info_valid;
  struct ipa_hdr_tbl_info_type_v01 hdr_tbl_info;
  __u8 v4_route_tbl_info_valid;
  struct ipa_route_tbl_info_type_v01 v4_route_tbl_info;
  __u8 v6_route_tbl_info_valid;
  struct ipa_route_tbl_info_type_v01 v6_route_tbl_info;
  __u8 v4_filter_tbl_start_addr_valid;
  __u32 v4_filter_tbl_start_addr;
  __u8 v6_filter_tbl_start_addr_valid;
  __u32 v6_filter_tbl_start_addr;
  __u8 modem_mem_info_valid;
  struct ipa_modem_mem_info_type_v01 modem_mem_info;
  __u8 ctrl_comm_dest_end_pt_valid;
  __u32 ctrl_comm_dest_end_pt;
  __u8 is_ssr_bootup_valid;
  __u8 is_ssr_bootup;
  __u8 hdr_proc_ctx_tbl_info_valid;
  struct ipa_hdr_proc_ctx_tbl_info_type_v01 hdr_proc_ctx_tbl_info;
  __u8 zip_tbl_info_valid;
  struct ipa_zip_tbl_info_type_v01 zip_tbl_info;
  __u8 v4_hash_route_tbl_info_valid;
  struct ipa_route_tbl_info_type_v01 v4_hash_route_tbl_info;
  __u8 v6_hash_route_tbl_info_valid;
  struct ipa_route_tbl_info_type_v01 v6_hash_route_tbl_info;
  __u8 v4_hash_filter_tbl_start_addr_valid;
  __u32 v4_hash_filter_tbl_start_addr;
  __u8 v6_hash_filter_tbl_start_addr_valid;
  __u32 v6_hash_filter_tbl_start_addr;
  __u8 hw_stats_quota_base_addr_valid;
  __u32 hw_stats_quota_base_addr;
  __u8 hw_stats_quota_size_valid;
  __u32 hw_stats_quota_size;
  __u8 hw_drop_stats_base_addr_valid;
  __u32 hw_drop_stats_base_addr;
  __u8 hw_drop_stats_table_size_valid;
  __u32 hw_drop_stats_table_size;
  __u8 hw_fiter_stats_info_valid;
  struct ipa_filter_stats_info_type_v01 hw_filter_stats_info;
  __u8 smem_info_valid;
  struct ipa_modem_mem_info_type_v01 smem_info;
  __u8 per_stats_smem_info_valid;
  struct ipa_modem_mem_info_type_v01 per_stats_smem_info;
};
struct ipa_init_modem_driver_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 ctrl_comm_dest_end_pt_valid;
  __u32 ctrl_comm_dest_end_pt;
  __u8 default_end_pt_valid;
  __u32 default_end_pt;
  __u8 modem_driver_init_pending_valid;
  __u8 modem_driver_init_pending;
};
struct ipa_init_modem_driver_cmplt_req_msg_v01 {
  __u8 status;
};
struct ipa_init_modem_driver_cmplt_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_indication_reg_req_msg_v01 {
  __u8 master_driver_init_complete_valid;
  __u8 master_driver_init_complete;
  __u8 data_usage_quota_reached_valid;
  __u8 data_usage_quota_reached;
  __u8 ipa_mhi_ready_ind_valid;
  __u8 ipa_mhi_ready_ind;
  __u8 endpoint_desc_ind_valid;
  __u8 endpoint_desc_ind;
  __u8 bw_change_ind_valid;
  __u8 bw_change_ind;
};
struct ipa_indication_reg_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_master_driver_init_complt_ind_msg_v01 {
  struct ipa_qmi_response_type_v01 master_driver_init_status;
};
struct ipa_ipfltr_range_eq_16_type_v01 {
  __u8 offset;
  __u16 range_low;
  __u16 range_high;
};
struct ipa_ipfltr_mask_eq_32_type_v01 {
  __u8 offset;
  __u32 mask;
  __u32 value;
};
struct ipa_ipfltr_eq_16_type_v01 {
  __u8 offset;
  __u16 value;
};
struct ipa_ipfltr_eq_32_type_v01 {
  __u8 offset;
  __u32 value;
};
struct ipa_ipfltr_mask_eq_128_type_v01 {
  __u8 offset;
  __u8 mask[16];
  __u8 value[16];
};
struct ipa_filter_rule_type_v01 {
  __u16 rule_eq_bitmap;
  __u8 tos_eq_present;
  __u8 tos_eq;
  __u8 protocol_eq_present;
  __u8 protocol_eq;
  __u8 num_ihl_offset_range_16;
  struct ipa_ipfltr_range_eq_16_type_v01 ihl_offset_range_16[QMI_IPA_IPFLTR_NUM_IHL_RANGE_16_EQNS_V01];
  __u8 num_offset_meq_32;
  struct ipa_ipfltr_mask_eq_32_type_v01 offset_meq_32[QMI_IPA_IPFLTR_NUM_MEQ_32_EQNS_V01];
  __u8 tc_eq_present;
  __u8 tc_eq;
  __u8 flow_eq_present;
  __u32 flow_eq;
  __u8 ihl_offset_eq_16_present;
  struct ipa_ipfltr_eq_16_type_v01 ihl_offset_eq_16;
  __u8 ihl_offset_eq_32_present;
  struct ipa_ipfltr_eq_32_type_v01 ihl_offset_eq_32;
  __u8 num_ihl_offset_meq_32;
  struct ipa_ipfltr_mask_eq_32_type_v01 ihl_offset_meq_32[QMI_IPA_IPFLTR_NUM_IHL_MEQ_32_EQNS_V01];
  __u8 num_offset_meq_128;
  struct ipa_ipfltr_mask_eq_128_type_v01 offset_meq_128[QMI_IPA_IPFLTR_NUM_MEQ_128_EQNS_V01];
  __u8 metadata_meq32_present;
  struct ipa_ipfltr_mask_eq_32_type_v01 metadata_meq32;
  __u8 ipv4_frag_eq_present;
};
struct ipa_filter_rule_req2_type_v01 {
  __u16 rule_eq_bitmap;
  __u8 pure_ack_eq_present;
  __u8 pure_ack_eq;
  __u8 protocol_eq_present;
  __u8 protocol_eq;
  __u8 num_ihl_offset_range_16;
  struct ipa_ipfltr_range_eq_16_type_v01 ihl_offset_range_16[QMI_IPA_IPFLTR_NUM_IHL_RANGE_16_EQNS_V01];
  __u8 num_offset_meq_32;
  struct ipa_ipfltr_mask_eq_32_type_v01 offset_meq_32[QMI_IPA_IPFLTR_NUM_MEQ_32_EQNS_V01];
  __u8 tc_eq_present;
  __u8 tc_eq;
  __u8 flow_eq_present;
  __u32 flow_eq;
  __u8 ihl_offset_eq_16_present;
  struct ipa_ipfltr_eq_16_type_v01 ihl_offset_eq_16;
  __u8 ihl_offset_eq_32_present;
  struct ipa_ipfltr_eq_32_type_v01 ihl_offset_eq_32;
  __u8 num_ihl_offset_meq_32;
  struct ipa_ipfltr_mask_eq_32_type_v01 ihl_offset_meq_32[QMI_IPA_IPFLTR_NUM_IHL_MEQ_32_EQNS_V01];
  __u8 num_offset_meq_128;
  struct ipa_ipfltr_mask_eq_128_type_v01 offset_meq_128[QMI_IPA_IPFLTR_NUM_MEQ_128_EQNS_V01];
  __u8 metadata_meq32_present;
  struct ipa_ipfltr_mask_eq_32_type_v01 metadata_meq32;
  __u8 ipv4_frag_eq_present;
};
enum ipa_ip_type_enum_v01 {
  IPA_IP_TYPE_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_IP_TYPE_INVALID_V01 = 0,
  QMI_IPA_IP_TYPE_V4_V01 = 1,
  QMI_IPA_IP_TYPE_V6_V01 = 2,
  QMI_IPA_IP_TYPE_V4V6_V01 = 3,
  IPA_IP_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
enum ipa_filter_action_enum_v01 {
  IPA_FILTER_ACTION_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_FILTER_ACTION_INVALID_V01 = 0,
  QMI_IPA_FILTER_ACTION_SRC_NAT_V01 = 1,
  QMI_IPA_FILTER_ACTION_DST_NAT_V01 = 2,
  QMI_IPA_FILTER_ACTION_ROUTING_V01 = 3,
  QMI_IPA_FILTER_ACTION_EXCEPTION_V01 = 4,
  IPA_FILTER_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
struct ipa_filter_spec_type_v01 {
  __u32 filter_spec_identifier;
  enum ipa_ip_type_enum_v01 ip_type;
  struct ipa_filter_rule_type_v01 filter_rule;
  enum ipa_filter_action_enum_v01 filter_action;
  __u8 is_routing_table_index_valid;
  __u32 route_table_index;
  __u8 is_mux_id_valid;
  __u32 mux_id;
};
struct ipa_filter_spec_ex_type_v01 {
  enum ipa_ip_type_enum_v01 ip_type;
  struct ipa_filter_rule_type_v01 filter_rule;
  enum ipa_filter_action_enum_v01 filter_action;
  __u8 is_routing_table_index_valid;
  __u32 route_table_index;
  __u8 is_mux_id_valid;
  __u32 mux_id;
  __u32 rule_id;
  __u8 is_rule_hashable;
};
struct ipa_filter_spec_ex2_type_v01 {
  enum ipa_ip_type_enum_v01 ip_type;
  struct ipa_filter_rule_req2_type_v01 filter_rule;
  enum ipa_filter_action_enum_v01 filter_action;
  __u8 is_routing_table_index_valid;
  __u32 route_table_index;
  __u8 is_mux_id_valid;
  __u32 mux_id;
  __u32 rule_id;
  __u8 is_rule_hashable;
};
struct ipa_install_fltr_rule_req_msg_v01 {
  __u8 filter_spec_list_valid;
  __u32 filter_spec_list_len;
  struct ipa_filter_spec_type_v01 filter_spec_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 source_pipe_index_valid;
  __u32 source_pipe_index;
  __u8 num_ipv4_filters_valid;
  __u32 num_ipv4_filters;
  __u8 num_ipv6_filters_valid;
  __u32 num_ipv6_filters;
  __u8 xlat_filter_indices_list_valid;
  __u32 xlat_filter_indices_list_len;
  __u32 xlat_filter_indices_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 filter_spec_ex_list_valid;
  __u32 filter_spec_ex_list_len;
  struct ipa_filter_spec_ex_type_v01 filter_spec_ex_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 filter_spec_ex2_list_valid;
  __u32 filter_spec_ex2_list_len;
  struct ipa_filter_spec_ex2_type_v01 filter_spec_ex2_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 ul_firewall_indices_list_valid;
  __u32 ul_firewall_indices_list_len;
  __u32 ul_firewall_indices_list[QMI_IPA_MAX_FILTERS_V01];
};
struct ipa_filter_rule_identifier_to_handle_map_v01 {
  __u32 filter_spec_identifier;
  __u32 filter_handle;
};
struct ipa_install_fltr_rule_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 filter_handle_list_valid;
  __u32 filter_handle_list_len;
  struct ipa_filter_rule_identifier_to_handle_map_v01 filter_handle_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 rule_id_valid;
  __u32 rule_id_len;
  __u32 rule_id[QMI_IPA_MAX_FILTERS_V01];
};
struct ipa_filter_handle_to_index_map_v01 {
  __u32 filter_handle;
  __u32 filter_index;
};
struct ipa_fltr_installed_notif_req_msg_v01 {
  __u32 source_pipe_index;
  enum ipa_qmi_result_type_v01 install_status;
  __u32 filter_index_list_len;
  struct ipa_filter_handle_to_index_map_v01 filter_index_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 embedded_pipe_index_valid;
  __u32 embedded_pipe_index;
  __u8 retain_header_valid;
  __u8 retain_header;
  __u8 embedded_call_mux_id_valid;
  __u32 embedded_call_mux_id;
  __u8 num_ipv4_filters_valid;
  __u32 num_ipv4_filters;
  __u8 num_ipv6_filters_valid;
  __u32 num_ipv6_filters;
  __u8 start_ipv4_filter_idx_valid;
  __u32 start_ipv4_filter_idx;
  __u8 start_ipv6_filter_idx_valid;
  __u32 start_ipv6_filter_idx;
  __u8 rule_id_valid;
  __u32 rule_id_len;
  __u32 rule_id[QMI_IPA_MAX_FILTERS_V01];
  __u8 dst_pipe_id_valid;
  __u32 dst_pipe_id_len;
  __u32 dst_pipe_id[QMI_IPA_MAX_CLIENT_DST_PIPES_V01];
  __u8 rule_id_ex_valid;
  __u32 rule_id_ex_len;
  __u32 rule_id_ex[QMI_IPA_MAX_FILTERS_EX2_V01];
};
struct ipa_fltr_installed_notif_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_enable_force_clear_datapath_req_msg_v01 {
  __u32 source_pipe_bitmask;
  __u32 request_id;
  __u8 throttle_source_valid;
  __u8 throttle_source;
  __u8 source_pipe_bitmask_ext_valid;
  __u32 source_pipe_bitmask_ext[4];
};
struct ipa_enable_force_clear_datapath_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_disable_force_clear_datapath_req_msg_v01 {
  __u32 request_id;
};
struct ipa_disable_force_clear_datapath_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
enum ipa_peripheral_speed_enum_v01 {
  IPA_PERIPHERAL_SPEED_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_PER_USB_FS_V01 = 1,
  QMI_IPA_PER_USB_HS_V01 = 2,
  QMI_IPA_PER_USB_SS_V01 = 3,
  QMI_IPA_PER_WLAN_V01 = 4,
  IPA_PERIPHERAL_SPEED_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
enum ipa_pipe_mode_enum_v01 {
  IPA_PIPE_MODE_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_PIPE_MODE_HW_V01 = 1,
  QMI_IPA_PIPE_MODE_SW_V01 = 2,
  IPA_PIPE_MODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
enum ipa_peripheral_type_enum_v01 {
  IPA_PERIPHERAL_TYPE_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_PERIPHERAL_USB_V01 = 1,
  QMI_IPA_PERIPHERAL_HSIC_V01 = 2,
  QMI_IPA_PERIPHERAL_PCIE_V01 = 3,
  IPA_PERIPHERAL_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
struct ipa_config_req_msg_v01 {
  __u8 peripheral_type_valid;
  enum ipa_peripheral_type_enum_v01 peripheral_type;
  __u8 hw_deaggr_supported_valid;
  __u8 hw_deaggr_supported;
  __u8 max_aggr_frame_size_valid;
  __u32 max_aggr_frame_size;
  __u8 ipa_ingress_pipe_mode_valid;
  enum ipa_pipe_mode_enum_v01 ipa_ingress_pipe_mode;
  __u8 peripheral_speed_info_valid;
  enum ipa_peripheral_speed_enum_v01 peripheral_speed_info;
  __u8 dl_accumulation_time_limit_valid;
  __u32 dl_accumulation_time_limit;
  __u8 dl_accumulation_pkt_limit_valid;
  __u32 dl_accumulation_pkt_limit;
  __u8 dl_accumulation_byte_limit_valid;
  __u32 dl_accumulation_byte_limit;
  __u8 ul_accumulation_time_limit_valid;
  __u32 ul_accumulation_time_limit;
  __u8 hw_control_flags_valid;
  __u32 hw_control_flags;
  __u8 ul_msi_event_threshold_valid;
  __u32 ul_msi_event_threshold;
  __u8 dl_msi_event_threshold_valid;
  __u32 dl_msi_event_threshold;
  __u8 ul_fifo_size_valid;
  __u32 ul_fifo_size;
  __u8 dl_fifo_size_valid;
  __u32 dl_fifo_size;
  __u8 dl_buf_size_valid;
  __u32 dl_buf_size;
};
struct ipa_config_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
enum ipa_stats_type_enum_v01 {
  IPA_STATS_TYPE_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_STATS_TYPE_INVALID_V01 = 0,
  QMI_IPA_STATS_TYPE_PIPE_V01 = 1,
  QMI_IPA_STATS_TYPE_FILTER_RULES_V01 = 2,
  IPA_STATS_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
struct ipa_pipe_stats_info_type_v01 {
  __u32 pipe_index;
  __u64 num_ipv4_packets;
  __u64 num_ipv4_bytes;
  __u64 num_ipv6_packets;
  __u64 num_ipv6_bytes;
};
struct ipa_stats_type_filter_rule_v01 {
  __u32 filter_rule_index;
  __u64 num_packets;
};
struct ipa_get_data_stats_req_msg_v01 {
  enum ipa_stats_type_enum_v01 ipa_stats_type;
  __u8 reset_stats_valid;
  __u8 reset_stats;
};
struct ipa_get_data_stats_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 ipa_stats_type_valid;
  enum ipa_stats_type_enum_v01 ipa_stats_type;
  __u8 ul_src_pipe_stats_list_valid;
  __u32 ul_src_pipe_stats_list_len;
  struct ipa_pipe_stats_info_type_v01 ul_src_pipe_stats_list[QMI_IPA_MAX_PIPES_V01];
  __u8 dl_dst_pipe_stats_list_valid;
  __u32 dl_dst_pipe_stats_list_len;
  struct ipa_pipe_stats_info_type_v01 dl_dst_pipe_stats_list[QMI_IPA_MAX_PIPES_V01];
  __u8 dl_filter_rule_stats_list_valid;
  __u32 dl_filter_rule_stats_list_len;
  struct ipa_stats_type_filter_rule_v01 dl_filter_rule_stats_list[QMI_IPA_MAX_FILTERS_V01];
};
struct ipa_apn_data_stats_info_type_v01 {
  __u32 mux_id;
  __u64 num_ul_packets;
  __u64 num_ul_bytes;
  __u64 num_dl_packets;
  __u64 num_dl_bytes;
};
struct ipa_get_apn_data_stats_req_msg_v01 {
  __u8 mux_id_list_valid;
  __u32 mux_id_list_len;
  __u32 mux_id_list[QMI_IPA_MAX_APN_V01];
};
struct ipa_get_apn_data_stats_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 apn_data_stats_list_valid;
  __u32 apn_data_stats_list_len;
  struct ipa_apn_data_stats_info_type_v01 apn_data_stats_list[QMI_IPA_MAX_APN_V01];
};
struct ipa_data_usage_quota_info_type_v01 {
  __u32 mux_id;
  __u64 num_Mbytes;
};
#define IPA_DATA_WARNING_QUOTA
struct ipa_set_data_usage_quota_req_msg_v01 {
  __u8 apn_quota_list_valid;
  __u32 apn_quota_list_len;
  struct ipa_data_usage_quota_info_type_v01 apn_quota_list[QMI_IPA_MAX_APN_V01];
  __u8 apn_warning_list_valid;
  __u32 apn_warning_list_len;
  struct ipa_data_usage_quota_info_type_v01 apn_warning_list[QMI_IPA_MAX_APN_V01];
};
struct ipa_set_data_usage_quota_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_data_usage_quota_reached_ind_msg_v01 {
  struct ipa_data_usage_quota_info_type_v01 apn;
  __u8 is_warning_limit_valid;
  __u8 is_warning_limit;
};
struct ipa_stop_data_usage_quota_req_msg_v01 {
  __u8 is_quota_limit_valid;
  __u8 is_quota_limit;
  __u8 is_warning_limit_valid;
  __u8 is_warning_limit;
};
struct ipa_stop_data_usage_quota_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_install_fltr_rule_req_ex_msg_v01 {
  __u8 filter_spec_ex_list_valid;
  __u32 filter_spec_ex_list_len;
  struct ipa_filter_spec_ex_type_v01 filter_spec_ex_list[QMI_IPA_MAX_FILTERS_EX_V01];
  __u8 source_pipe_index_valid;
  __u32 source_pipe_index;
  __u8 num_ipv4_filters_valid;
  __u32 num_ipv4_filters;
  __u8 num_ipv6_filters_valid;
  __u32 num_ipv6_filters;
  __u8 xlat_filter_indices_list_valid;
  __u32 xlat_filter_indices_list_len;
  __u32 xlat_filter_indices_list[QMI_IPA_MAX_FILTERS_EX_V01];
  __u8 filter_spec_ex2_list_valid;
  __u32 filter_spec_ex2_list_len;
  struct ipa_filter_spec_ex2_type_v01 filter_spec_ex2_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 ul_firewall_indices_list_valid;
  __u32 ul_firewall_indices_list_len;
  __u32 ul_firewall_indices_list[QMI_IPA_MAX_FILTERS_V01];
};
struct ipa_install_fltr_rule_resp_ex_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 rule_id_valid;
  __u32 rule_id_len;
  __u32 rule_id[QMI_IPA_MAX_FILTERS_EX_V01];
};
struct ipa_enable_per_client_stats_req_msg_v01 {
  __u8 enable_per_client_stats;
};
struct ipa_enable_per_client_stats_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_per_client_stats_info_type_v01 {
  __u32 client_id;
  __u32 src_pipe_id;
  __u64 num_ul_ipv4_bytes;
  __u64 num_ul_ipv6_bytes;
  __u64 num_dl_ipv4_bytes;
  __u64 num_dl_ipv6_bytes;
  __u32 num_ul_ipv4_pkts;
  __u32 num_ul_ipv6_pkts;
  __u32 num_dl_ipv4_pkts;
  __u32 num_dl_ipv6_pkts;
};
struct ipa_get_stats_per_client_req_msg_v01 {
  __u32 client_id;
  __u32 src_pipe_id;
  __u8 reset_stats_valid;
  __u8 reset_stats;
};
struct ipa_get_stats_per_client_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 per_client_stats_list_valid;
  __u32 per_client_stats_list_len;
  struct ipa_per_client_stats_info_type_v01 per_client_stats_list[QMI_IPA_MAX_PER_CLIENTS_V01];
};
struct ipa_ul_firewall_rule_type_v01 {
  enum ipa_ip_type_enum_v01 ip_type;
  struct ipa_filter_rule_type_v01 filter_rule;
};
struct ipa_configure_ul_firewall_rules_req_msg_v01 {
  __u32 firewall_rules_list_len;
  struct ipa_ul_firewall_rule_type_v01 firewall_rules_list[QMI_IPA_MAX_UL_FIREWALL_RULES_V01];
  __u32 mux_id;
  __u8 disable_valid;
  __u8 disable;
  __u8 are_blacklist_filters_valid;
  __u8 are_blacklist_filters;
};
struct ipa_configure_ul_firewall_rules_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
enum ipa_ul_firewall_status_enum_v01 {
  IPA_UL_FIREWALL_STATUS_ENUM_MIN_ENUM_VAL_V01 = - 2147483647,
  QMI_IPA_UL_FIREWALL_STATUS_SUCCESS_V01 = 0,
  QMI_IPA_UL_FIREWALL_STATUS_FAILURE_V01 = 1,
  IPA_UL_FIREWALL_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647
};
struct ipa_ul_firewall_config_result_type_v01 {
  enum ipa_ul_firewall_status_enum_v01 is_success;
  __u32 mux_id;
};
struct ipa_configure_ul_firewall_rules_ind_msg_v01 {
  struct ipa_ul_firewall_config_result_type_v01 result;
};
struct ipa_mhi_ch_init_info_type_v01 {
  __u8 ch_id;
  __u8 er_id;
  __u32 ch_doorbell_addr;
  __u32 er_doorbell_addr;
  __u32 direction_type;
};
struct ipa_mhi_smmu_info_type_v01 {
  __u64 iova_ctl_base_addr;
  __u64 iova_ctl_size;
  __u64 iova_data_base_addr;
  __u64 iova_data_size;
};
struct ipa_mhi_ready_indication_msg_v01 {
  __u32 ch_info_arr_len;
  struct ipa_mhi_ch_init_info_type_v01 ch_info_arr[QMI_IPA_REMOTE_MHI_CHANNELS_NUM_MAX_V01];
  __u8 smmu_info_valid;
  struct ipa_mhi_smmu_info_type_v01 smmu_info;
};
#define IPA_MHI_READY_INDICATION_MSG_V01_MAX_MSG_LEN 123
struct ipa_mhi_mem_addr_info_type_v01 {
  __u64 pa;
  __u64 iova;
  __u64 size;
};
enum ipa_mhi_brst_mode_enum_v01 {
  IPA_MHI_BRST_MODE_ENUM_MIN_VAL_V01 = IPA_INT_MIN,
  QMI_IPA_BURST_MODE_DEFAULT_V01 = 0,
  QMI_IPA_BURST_MODE_ENABLED_V01 = 1,
  QMI_IPA_BURST_MODE_DISABLED_V01 = 2,
  IPA_MHI_BRST_MODE_ENUM_MAX_VAL_V01 = IPA_INT_MAX,
};
struct ipa_mhi_tr_info_type_v01 {
  __u8 ch_id;
  __u16 poll_cfg;
  enum ipa_mhi_brst_mode_enum_v01 brst_mode_type;
  __u64 ring_iova;
  __u64 ring_len;
  __u64 rp;
  __u64 wp;
};
struct ipa_mhi_er_info_type_v01 {
  __u8 er_id;
  __u32 intmod_cycles;
  __u32 intmod_count;
  __u32 msi_addr;
  __u64 ring_iova;
  __u64 ring_len;
  __u64 rp;
  __u64 wp;
};
struct ipa_mhi_alloc_channel_req_msg_v01 {
  __u32 tr_info_arr_len;
  struct ipa_mhi_tr_info_type_v01 tr_info_arr[QMI_IPA_REMOTE_MHI_CHANNELS_NUM_MAX_V01];
  __u32 er_info_arr_len;
  struct ipa_mhi_er_info_type_v01 er_info_arr[QMI_IPA_REMOTE_MHI_CHANNELS_NUM_MAX_V01];
  __u32 ctrl_addr_map_info_len;
  struct ipa_mhi_mem_addr_info_type_v01 ctrl_addr_map_info[QMI_IPA_REMOTE_MHI_MEMORY_MAPPING_NUM_MAX_V01];
  __u32 data_addr_map_info_len;
  struct ipa_mhi_mem_addr_info_type_v01 data_addr_map_info[QMI_IPA_REMOTE_MHI_MEMORY_MAPPING_NUM_MAX_V01];
};
#define IPA_MHI_ALLOC_CHANNEL_REQ_MSG_V01_MAX_MSG_LEN 808
struct ipa_mhi_ch_alloc_resp_type_v01 {
  __u8 ch_id;
  __u8 is_success;
};
struct ipa_mhi_alloc_channel_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 alloc_resp_arr_valid;
  __u32 alloc_resp_arr_len;
  struct ipa_mhi_ch_alloc_resp_type_v01 alloc_resp_arr[QMI_IPA_REMOTE_MHI_CHANNELS_NUM_MAX_V01];
};
#define IPA_MHI_ALLOC_CHANNEL_RESP_MSG_V01_MAX_MSG_LEN 23
enum ipa_clock_rate_enum_v01 {
  IPA_CLOCK_RATE_ENUM_MIN_ENUM_VAL_V01 = IPA_INT_MIN,
  QMI_IPA_CLOCK_RATE_INVALID_V01 = 0,
  QMI_IPA_CLOCK_RATE_LOW_SVS_V01 = 1,
  QMI_IPA_CLOCK_RATE_SVS_V01 = 2,
  QMI_IPA_CLOCK_RATE_NOMINAL_V01 = 3,
  QMI_IPA_CLOCK_RATE_TURBO_V01 = 4,
  IPA_CLOCK_RATE_ENUM_MAX_ENUM_VAL_V01 = IPA_INT_MAX,
};
struct ipa_mhi_clk_vote_req_msg_v01 {
  __u8 mhi_vote;
  __u8 tput_value_valid;
  __u32 tput_value;
  __u8 clk_rate_valid;
  enum ipa_clock_rate_enum_v01 clk_rate;
};
#define IPA_MHI_CLK_VOTE_REQ_MSG_V01_MAX_MSG_LEN 18
struct ipa_mhi_clk_vote_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_MHI_CLK_VOTE_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_mhi_cleanup_req_msg_v01 {
  __u8 cleanup_valid;
  __u8 cleanup;
};
#define IPA_MHI_CLEANUP_REQ_MSG_V01_MAX_MSG_LEN 4
struct ipa_mhi_cleanup_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_MHI_CLEANUP_RESP_MSG_V01_MAX_MSG_LEN 7
enum ipa_ep_desc_type_enum_v01 {
  IPA_EP_DESC_TYPE_ENUM_MIN_VAL_V01 = IPA_INT_MIN,
  DATA_EP_DESC_TYPE_RESERVED_V01 = 0x00,
  DATA_EP_DESC_TYPE_EMB_CONS_V01 = 0x01,
  DATA_EP_DESC_TYPE_EMB_PROD_V01 = 0x02,
  DATA_EP_DESC_TYPE_RSC_PROD_V01 = 0x03,
  DATA_EP_DESC_TYPE_QDSS_PROD_V01 = 0x04,
  DATA_EP_DESC_TYPE_DPL_PROD_V01 = 0x05,
  DATA_EP_DESC_TYPE_TETH_CONS_V01 = 0x06,
  DATA_EP_DESC_TYPE_TETH_PROD_V01 = 0x07,
  DATA_EP_DESC_TYPE_TETH_RMNET_CONS_V01 = 0x08,
  DATA_EP_DESC_TYPE_TETH_RMNET_PROD_V01 = 0x09,
  DATA_EP_DESC_TYPE_EMB_FLOW_CTL_CONS_V01 = 0x0A,
  DATA_EP_DESC_TYPE_EMB_FLOW_CTL_PROD_V01 = 0x0B,
  DATA_EP_DESC_TYPE_TETH_LL_CONS_V01 = 0x0C,
  DATA_EP_DESC_TYPE_TETH_LL_PROD_V01 = 0x0D,
  IPA_EP_DESC_TYPE_ENUM_MAX_VAL_V01 = IPA_INT_MAX,
};
enum ipa_ic_type_enum_v01 {
  IPA_IC_TYPE_ENUM_MIN_VAL_V01 = IPA_INT_MIN,
  DATA_IC_TYPE_RESERVED_V01 = 0x00,
  DATA_IC_TYPE_MHI_V01 = 0x01,
  DATA_IC_TYPE_MHI_PRIME_V01 = 0x02,
  DATA_IC_TYPE_USB_V01 = 0x03,
  DATA_IC_TYPE_AP_V01 = 0x04,
  DATA_IC_TYPE_Q6_V01 = 0x05,
  DATA_IC_TYPE_UC_V01 = 0x06,
  IPA_IC_TYPE_ENUM_MAX_VAL_V01 = IPA_INT_MAX,
};
enum ipa_ep_status_type_v01 {
  IPA_EP_STATUS_TYPE_MIN_VAL_V01 = IPA_INT_MIN,
  DATA_EP_STATUS_RESERVED_V01 = 0x00,
  DATA_EP_STATUS_STATIC_V01 = 0x01,
  DATA_EP_STATUS_CONNECTED_V01 = 0x02,
  DATA_EP_STATUS_DISCONNECTED_V01 = 0x03,
  IPA_EP_STATUS_TYPE_MAX_VAL_V01 = IPA_INT_MAX,
};
struct ipa_ep_id_type_v01 {
  enum ipa_ic_type_enum_v01 ic_type;
  enum ipa_ep_desc_type_enum_v01 ep_type;
  __u32 ep_id;
  enum ipa_ep_status_type_v01 ep_status;
};
struct ipa_endp_desc_indication_msg_v01 {
  __u8 ep_info_valid;
  __u32 ep_info_len;
  struct ipa_ep_id_type_v01 ep_info[QMI_IPA_ENDP_DESC_NUM_MAX_V01];
  __u8 num_eps_valid;
  __u32 num_eps;
};
#define IPA_ENDP_DESC_INDICATION_MSG_V01_MAX_MSG_LEN 507
enum ipa_aggr_enum_type_v01 {
  IPA_AGGR_ENUM_TYPE_MIN_VAL_V01 = IPA_INT_MIN,
  DATA_AGGR_TYPE_RESERVED_V01 = 0x00,
  DATA_AGGR_TYPE_QMAP_V01 = 0x01,
  DATA_AGGR_TYPE_QMAPv5_V01 = 0x02,
  DATA_AGGR_TYPE_INHERITED_V01 = 0x03,
  IPA_AGGR_ENUM_TYPE_MAX_VAL_V01 = IPA_INT_MAX,
};
struct ipa_mhi_prime_aggr_info_type_v01 {
  enum ipa_ic_type_enum_v01 ic_type;
  enum ipa_ep_desc_type_enum_v01 ep_type;
  __u32 bytes_count;
  __u32 pkt_count;
  enum ipa_aggr_enum_type_v01 aggr_type;
};
#define IPA_MHI_PRIME_AGGR_INFO_REQ_MSG_V01_MAX_MSG_LEN 631
struct ipa_mhi_prime_aggr_info_req_msg_v01 {
  __u8 aggr_info_valid;
  __u32 aggr_info_len;
  struct ipa_mhi_prime_aggr_info_type_v01 aggr_info[QMI_IPA_ENDP_DESC_NUM_MAX_V01];
  __u8 num_eps_valid;
  __u32 num_eps;
};
#define IPA_MHI_PRIME_AGGR_INFO_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_mhi_prime_aggr_info_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
struct ipa_add_offload_connection_req_msg_v01 {
  __u8 num_ipv4_filters_valid;
  __u32 num_ipv4_filters;
  __u8 num_ipv6_filters_valid;
  __u32 num_ipv6_filters;
  __u8 xlat_filter_indices_list_valid;
  __u32 xlat_filter_indices_list_len;
  __u32 xlat_filter_indices_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 filter_spec_ex2_list_valid;
  __u32 filter_spec_ex2_list_len;
  struct ipa_filter_spec_ex2_type_v01 filter_spec_ex2_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 embedded_call_mux_id_valid;
  __u32 embedded_call_mux_id;
  __u8 default_mhi_path_valid;
  __u8 default_mhi_path;
};
#define IPA_ADD_OFFLOAD_CONNECTION_REQ_MSG_V01_MAX_MSG_LEN 11361
struct ipa_add_offload_connection_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
  __u8 filter_handle_list_valid;
  __u32 filter_handle_list_len;
  struct ipa_filter_rule_identifier_to_handle_map_v01 filter_handle_list[QMI_IPA_MAX_FILTERS_V01];
};
#define IPA_ADD_OFFLOAD_CONNECTION_RESP_MSG_V01_MAX_MSG_LEN 523
struct ipa_remove_offload_connection_req_msg_v01 {
  __u8 filter_handle_list_valid;
  __u32 filter_handle_list_len;
  struct ipa_filter_rule_identifier_to_handle_map_v01 filter_handle_list[QMI_IPA_MAX_FILTERS_V01];
  __u8 clean_all_rules_valid;
  __u8 clean_all_rules;
};
#define IPA_REMOVE_OFFLOAD_CONNECTION_REQ_MSG_V01_MAX_MSG_LEN 520
struct ipa_remove_offload_connection_resp_msg_v01 {
  __u8 resp_valid;
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_REMOVE_OFFLOAD_CONNECTION_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_bw_change_ind_msg_v01 {
  __u8 peak_bw_ul_valid;
  __u8 peak_bw_dl_valid;
  __u32 peak_bw_ul;
  __u32 peak_bw_dl;
};
#define IPA_BW_CHANGE_IND_MSG_V01_MAX_MSG_LEN 14
enum ipa_move_nat_type_enum_v01 {
  QMI_IPA_MOVE_NAT_TO_DDR_V01 = 0,
  QMI_IPA_MOVE_NAT_TO_SRAM_V01 = 1,
};
struct ipa_move_nat_req_msg_v01 {
  enum ipa_move_nat_type_enum_v01 nat_move_direction;
};
#define IPA_MOVE_NAT_REQ_MSG_V01_MAX_MSG_LEN 8
struct ipa_move_nat_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_MOVE_NAT_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_move_nat_table_complt_ind_msg_v01 {
  struct ipa_qmi_response_type_v01 nat_table_move_status;
};
#define QMI_IPA_NAT_TABLE_MOVE_COMPLETE_IND_MAX_MSG_LEN_V01 7
struct ipa_wlan_opt_dp_rsrv_filter_req_msg_v01 {
  __u8 num_filters;
  __u32 timeout_val_ms;
  __u32 q6_rtng_table_index;
};
#define IPA_WLAN_OPT_DP_RSRV_FILTER_REQ_MSG_V01_MAX_MSG_LEN 18
struct ipa_wlan_opt_dp_rsrv_filter_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_WLAN_OPT_DP_RSRV_FILTER_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_wlan_opt_dp_rsrv_filter_complt_ind_msg_v01 {
  struct ipa_qmi_response_type_v01 rsrv_filter_status;
};
#define IPA_WLAN_OPT_DP_RSRV_FILTER_COMPLT_IND_MSG_V01_MAX_MSG_LEN 7
struct ip_hdr_v4_address_info_v01 {
  __u32 source;
  __u32 dest;
};
struct ip_hdr_v6_address_info_v01 {
  __u32 source[QMI_IPA_IPV6_WORD_ADDR_LEN_V01];
  __u32 dest[QMI_IPA_IPV6_WORD_ADDR_LEN_V01];
};
struct ipa_wlan_opt_dp_add_filter_req_msg_v01 {
  __u32 filter_idx;
  enum ipa_ip_type_enum_v01 ip_type;
  __u8 v4_addr_valid;
  struct ip_hdr_v4_address_info_v01 v4_addr;
  __u8 v6_addr_valid;
  struct ip_hdr_v6_address_info_v01 v6_addr;
};
#define IPA_WLAN_OPT_DP_ADD_FILTER_REQ_MSG_V01_MAX_MSG_LEN 60
struct ipa_wlan_opt_dp_add_filter_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_WLAN_OPT_DP_ADD_FILTER_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_wlan_opt_dp_add_filter_complt_ind_msg_v01 {
  struct ipa_qmi_response_type_v01 filter_add_status;
  __u32 filter_idx;
  __u8 filter_handle_valid;
  __u32 filter_handle;
};
#define IPA_WLAN_OPT_DP_ADD_FILTER_COMPLT_IND_MSG_V01_MAX_MSG_LEN 21
struct ipa_wlan_opt_dp_remove_filter_req_msg_v01 {
  __u32 filter_idx;
  __u32 filter_handle;
};
#define IPA_WLAN_OPT_DP_REMOVE_FILTER_REQ_MSG_V01_MAX_MSG_LEN 14
struct ipa_wlan_opt_dp_remove_filter_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_WLAN_OPT_DP_REMOVE_FILTER_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_wlan_opt_dp_remove_filter_complt_ind_msg_v01 {
  struct ipa_qmi_response_type_v01 filter_removal_status;
  __u32 filter_idx;
};
#define IPA_WLAN_OPT_DP_REM_FILTER_COMPLT_IND_MSG_V01_MAX_MSG_LEN 14
struct ipa_wlan_opt_dp_remove_all_filter_req_msg_v01 {
  __u8 reserved_valid;
  __u8 reserved;
};
#define IPA_WLAN_OPT_DP_REM_ALL_FILTER_REQ_MSG_V01_MAX_MSG_LEN 4
struct ipa_wlan_opt_dp_remove_all_filter_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_WLAN_OPT_DP_REMOVE_ALL_FILTER_RESP_MSG_V01_MAX_MSG_LEN 7
struct ipa_wlan_opt_dp_remove_all_filter_complt_ind_msg_v01 {
  struct ipa_qmi_response_type_v01 filter_removal_all_status;
};
#define IPA_WLAN_OPT_DP_REM_ALL_FILTER_COMPLT_IND_MSG_V01_MAX_MSG_LEN 7
struct ipa_wlan_opt_dp_set_wlan_per_info_req_msg_v01 {
  __u32 src_wlan_endp_id;
  __u32 dest_wlan_endp_id;
  __u32 dest_apps_endp_id;
  __u32 hdr_len;
  __u32 eth_hdr_offset;
  __u8 hdr_info[QMI_IPA_MAX_ETH_HDR_SIZE_V01];
};
#define IPA_WLAN_OPT_DP_SET_WLAN_PER_INFO_REQ_MSG_V1_MAX_MSG_LEN 102
struct ipa_wlan_opt_dp_set_wlan_per_info_resp_msg_v01 {
  struct ipa_qmi_response_type_v01 resp;
};
#define IPA_WLAN_OPT_DP_SET_WLAN_PER_INFO_RESP_MSG_V1_MAX_MSG_LEN 7
#define QMI_IPA_INDICATION_REGISTER_REQ_V01 0x0020
#define QMI_IPA_INDICATION_REGISTER_RESP_V01 0x0020
#define QMI_IPA_INIT_MODEM_DRIVER_REQ_V01 0x0021
#define QMI_IPA_INIT_MODEM_DRIVER_RESP_V01 0x0021
#define QMI_IPA_MASTER_DRIVER_INIT_COMPLETE_IND_V01 0x0022
#define QMI_IPA_INSTALL_FILTER_RULE_REQ_V01 0x0023
#define QMI_IPA_INSTALL_FILTER_RULE_RESP_V01 0x0023
#define QMI_IPA_FILTER_INSTALLED_NOTIF_REQ_V01 0x0024
#define QMI_IPA_FILTER_INSTALLED_NOTIF_RESP_V01 0x0024
#define QMI_IPA_ENABLE_FORCE_CLEAR_DATAPATH_REQ_V01 0x0025
#define QMI_IPA_ENABLE_FORCE_CLEAR_DATAPATH_RESP_V01 0x0025
#define QMI_IPA_DISABLE_FORCE_CLEAR_DATAPATH_REQ_V01 0x0026
#define QMI_IPA_DISABLE_FORCE_CLEAR_DATAPATH_RESP_V01 0x0026
#define QMI_IPA_CONFIG_REQ_V01 0x0027
#define QMI_IPA_CONFIG_RESP_V01 0x0027
#define QMI_IPA_DISABLE_LINK_LOW_PWR_STATE_REQ_V01 0x0028
#define QMI_IPA_DISABLE_LINK_LOW_PWR_STATE_RESP_V01 0x0028
#define QMI_IPA_ENABLE_LINK_LOW_PWR_STATE_REQ_V01 0x0029
#define QMI_IPA_ENABLE_LINK_LOW_PWR_STATE_RESP_V01 0x0029
#define QMI_IPA_GET_DATA_STATS_REQ_V01 0x0030
#define QMI_IPA_GET_DATA_STATS_RESP_V01 0x0030
#define QMI_IPA_GET_APN_DATA_STATS_REQ_V01 0x0031
#define QMI_IPA_GET_APN_DATA_STATS_RESP_V01 0x0031
#define QMI_IPA_SET_DATA_USAGE_QUOTA_REQ_V01 0x0032
#define QMI_IPA_SET_DATA_USAGE_QUOTA_RESP_V01 0x0032
#define QMI_IPA_DATA_USAGE_QUOTA_REACHED_IND_V01 0x0033
#define QMI_IPA_STOP_DATA_USAGE_QUOTA_REQ_V01 0x0034
#define QMI_IPA_STOP_DATA_USAGE_QUOTA_RESP_V01 0x0034
#define QMI_IPA_INIT_MODEM_DRIVER_CMPLT_REQ_V01 0x0035
#define QMI_IPA_INIT_MODEM_DRIVER_CMPLT_RESP_V01 0x0035
#define QMI_IPA_INSTALL_FILTER_RULE_EX_REQ_V01 0x0037
#define QMI_IPA_INSTALL_FILTER_RULE_EX_RESP_V01 0x0037
#define QMI_IPA_ENABLE_PER_CLIENT_STATS_REQ_V01 0x0038
#define QMI_IPA_ENABLE_PER_CLIENT_STATS_RESP_V01 0x0038
#define QMI_IPA_GET_STATS_PER_CLIENT_REQ_V01 0x0039
#define QMI_IPA_GET_STATS_PER_CLIENT_RESP_V01 0x0039
#define QMI_IPA_INSTALL_UL_FIREWALL_RULES_REQ_V01 0x003A
#define QMI_IPA_INSTALL_UL_FIREWALL_RULES_RESP_V01 0x003A
#define QMI_IPA_INSTALL_UL_FIREWALL_RULES_IND_V01 0x003A
#define QMI_IPA_MHI_CLK_VOTE_REQ_V01 0x003B
#define QMI_IPA_MHI_CLK_VOTE_RESP_V01 0x003B
#define QMI_IPA_MHI_READY_IND_V01 0x003C
#define QMI_IPA_MHI_ALLOC_CHANNEL_REQ_V01 0x003D
#define QMI_IPA_MHI_ALLOC_CHANNEL_RESP_V01 0x003D
#define QMI_IPA_MHI_CLEANUP_REQ_V01 0x003E
#define QMI_IPA_MHI_CLEANUP_RESP_V01 0x003E
#define QMI_IPA_ENDP_DESC_INDICATION_V01 0x003F
#define QMI_IPA_MHI_PRIME_AGGR_INFO_REQ_V01 0x0040
#define QMI_IPA_MHI_PRIME_AGGR_INFO_RESP_V01 0x0040
#define QMI_IPA_ADD_OFFLOAD_CONNECTION_REQ_V01 0x0041
#define QMI_IPA_ADD_OFFLOAD_CONNECTION_RESP_V01 0x0041
#define QMI_IPA_REMOVE_OFFLOAD_CONNECTION_REQ_V01 0x0042
#define QMI_IPA_REMOVE_OFFLOAD_CONNECTION_RESP_V01 0x0042
#define QMI_IPA_BW_CHANGE_INDICATION_V01 0x0044
#define QMI_IPA_MOVE_NAT_REQ_V01 0x0046
#define QMI_IPA_MOVE_NAT_RESP_V01 0x0046
#define QMI_IPA_MOVE_NAT_COMPLETE_IND_V01 0x0046
#define QMI_IPA_WLAN_OPT_DATAPATH_RSRV_FILTER_REQ_V01 0x0049
#define QMI_IPA_WLAN_OPT_DATAPATH_RSRV_FILTER_RESP_V01 0x0049
#define QMI_IPA_WLAN_OPT_DATAPATH_RSRV_FILTER_COMPLT_IND_V01 0x0049
#define QMI_IPA_WLAN_OPT_DATAPATH_ADD_FILTER_REQ_V01 0x004A
#define QMI_IPA_WLAN_OPT_DATAPATH_ADD_FILTER_RESP_V01 0x004A
#define QMI_IPA_WLAN_OPT_DATAPATH_ADD_FILTER_COMPLT_IND_V01 0x004A
#define QMI_IPA_WLAN_OPT_DATAPATH_REMOVE_FILTER_REQ_V01 0x004B
#define QMI_IPA_WLAN_OPT_DATAPATH_REMOVE_FILTER_RESP_V01 0x004B
#define QMI_IPA_WLAN_OPT_DATAPATH_REMOVE_FILTER_COMPLT_IND_V01 0x004B
#define QMI_IPA_WLAN_OPT_DATAPATH_REMOVE_ALL_FILTER_REQ_V01 0x004C
#define QMI_IPA_WLAN_OPT_DATAPATH_REMOVE_ALL_FILTER_RESP_V01 0x004C
#define QMI_IPA_WLAN_OPT_DATAPATH_REMOVE_ALL_FILTER_COMPLT_IND_V01 0x004C
#define QMI_IPA_WLAN_OPT_DATAPATH_SET_WLAN_PER_INFO_REQ_V01 0x004D
#define QMI_IPA_WLAN_OPT_DATAPATH_SET_WLAN_PER_INFO_RESP_V01 0x004D
#define QMI_IPA_INIT_MODEM_DRIVER_REQ_MAX_MSG_LEN_V01 197
#define QMI_IPA_INIT_MODEM_DRIVER_RESP_MAX_MSG_LEN_V01 25
#define QMI_IPA_INDICATION_REGISTER_REQ_MAX_MSG_LEN_V01 16
#define QMI_IPA_INDICATION_REGISTER_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_INSTALL_FILTER_RULE_REQ_MAX_MSG_LEN_V01 33705
#define QMI_IPA_INSTALL_FILTER_RULE_RESP_MAX_MSG_LEN_V01 783
#define QMI_IPA_FILTER_INSTALLED_NOTIF_REQ_MAX_MSG_LEN_V01 1899
#define QMI_IPA_FILTER_INSTALLED_NOTIF_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_MASTER_DRIVER_INIT_COMPLETE_IND_MAX_MSG_LEN_V01 7
#define QMI_IPA_DATA_USAGE_QUOTA_REACHED_IND_MAX_MSG_LEN_V01 19
#define QMI_IPA_ENABLE_FORCE_CLEAR_DATAPATH_REQ_MAX_MSG_LEN_V01 37
#define QMI_IPA_DISABLE_FORCE_CLEAR_DATAPATH_REQ_MAX_MSG_LEN_V01 7
#define QMI_IPA_ENABLE_FORCE_CLEAR_DATAPATH_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_DISABLE_FORCE_CLEAR_DATAPATH_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_CONFIG_REQ_MAX_MSG_LEN_V01 102
#define QMI_IPA_CONFIG_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_DISABLE_LINK_LOW_PWR_STATE_REQ_MAX_MSG_LEN_V01 18
#define QMI_IPA_DISABLE_LINK_LOW_PWR_STATE_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_ENABLE_LINK_LOW_PWR_STATE_REQ_MAX_MSG_LEN_V01 7
#define QMI_IPA_ENABLE_LINK_LOW_PWR_STATE_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_GET_DATA_STATS_REQ_MAX_MSG_LEN_V01 11
#define QMI_IPA_GET_DATA_STATS_RESP_MAX_MSG_LEN_V01 2234
#define QMI_IPA_GET_APN_DATA_STATS_REQ_MAX_MSG_LEN_V01 36
#define QMI_IPA_GET_APN_DATA_STATS_RESP_MAX_MSG_LEN_V01 299
#define QMI_IPA_SET_DATA_USAGE_QUOTA_REQ_MAX_MSG_LEN_V01 200
#define QMI_IPA_SET_DATA_USAGE_QUOTA_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_STOP_DATA_USAGE_QUOTA_REQ_MAX_MSG_LEN_V01 8
#define QMI_IPA_STOP_DATA_USAGE_QUOTA_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_INIT_MODEM_DRIVER_CMPLT_REQ_MAX_MSG_LEN_V01 4
#define QMI_IPA_INIT_MODEM_DRIVER_CMPLT_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_INSTALL_FILTER_RULE_EX_REQ_MAX_MSG_LEN_V01 34021
#define QMI_IPA_INSTALL_FILTER_RULE_EX_RESP_MAX_MSG_LEN_V01 523
#define QMI_IPA_ENABLE_PER_CLIENT_STATS_REQ_MAX_MSG_LEN_V01 4
#define QMI_IPA_ENABLE_PER_CLIENT_STATS_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_GET_STATS_PER_CLIENT_REQ_MAX_MSG_LEN_V01 18
#define QMI_IPA_GET_STATS_PER_CLIENT_RESP_MAX_MSG_LEN_V01 3595
#define QMI_IPA_INSTALL_UL_FIREWALL_RULES_REQ_MAX_MSG_LEN_V01 9875
#define QMI_IPA_INSTALL_UL_FIREWALL_RULES_RESP_MAX_MSG_LEN_V01 7
#define QMI_IPA_INSTALL_UL_FIREWALL_RULES_IND_MAX_MSG_LEN_V01 11
#define QMI_IPA_MAX_MSG_LEN 22685
#endif
