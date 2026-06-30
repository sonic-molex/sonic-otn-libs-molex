#pragma once

// SAI
#ifdef __cplusplus
extern "C" {
#endif
#include "saiextensions.h"
#ifdef __cplusplus
}
#endif

#include "sai_meta_data.h"

// LOG
#include "logger.h"

// forward declarations; full definitions in virtual_otn_device.h
class virtual_otn_device;
class virtual_otn_wss_device;


#define CHECK_SWITCH_ID(id) \
do {\
    if ((id) != switch_metadata_ptr->switch_id) {\
        return SAI_STATUS_FAILURE;\
    }\
} while(0)

#define CAST_OBJ(o, t, id) t *o = static_cast<t *>(switch_metadata_ptr->sai_id_map.get_object(id))


class sai_adapter
{
public:
    sai_adapter();
    ~sai_adapter();
    // sai api functions
    static sai_status_t create_switch(
            sai_object_id_t *switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_switch(sai_object_id_t switch_id);
    static sai_status_t get_switch_attribute(
            sai_object_id_t switch_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);
    static sai_status_t set_switch_attribute(
            sai_object_id_t switch_id,
            const sai_attribute_t *attr);

    static sai_status_t create_router_interface(
            sai_object_id_t *lag_member_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_router_interface(sai_object_id_t router_interface_id);
    static sai_status_t set_router_interface_attribute(
            sai_object_id_t rif_id,
            const sai_attribute_t *attr);
    static sai_status_t get_router_interface_attribute(
            sai_object_id_t rif_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTN device
    static sai_status_t create_otn_device(
            sai_object_id_t *otn_device_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_device(sai_object_id_t otn_device_id);
    static sai_status_t set_otn_device_attribute(
            sai_object_id_t otn_device_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_device_attribute(
            sai_object_id_t otn_device_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTN attenuator
    static sai_status_t create_otn_attenuator(
            sai_object_id_t *otn_attenuator_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_attenuator(sai_object_id_t otn_attenuator_id);
    static sai_status_t set_otn_attenuator_attribute(
            sai_object_id_t otn_attenuator_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_attenuator_attribute(
            sai_object_id_t otn_attenuator_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTN OA
    static sai_status_t create_otn_oa(
            sai_object_id_t *otn_oa_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_oa(sai_object_id_t otn_oa_id);
    static sai_status_t set_otn_oa_attribute(
            sai_object_id_t otn_oa_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_oa_attribute(
            sai_object_id_t otn_oa_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTAI OCM
    static sai_status_t create_otn_ocm(
            sai_object_id_t *otn_ocm_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_ocm(sai_object_id_t otn_ocm_id);
    static sai_status_t set_otn_ocm_attribute(
            sai_object_id_t otn_ocm_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_ocm_attribute(
            sai_object_id_t otn_ocm_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);
    static sai_status_t create_otn_ocm_channel(
            sai_object_id_t *otn_ocm_channel_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_ocm_channel(sai_object_id_t otn_ocm_channel_id);
    static sai_status_t set_otn_ocm_channel_attribute(
            sai_object_id_t otn_ocm_channel_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_ocm_channel_attribute(
            sai_object_id_t otn_ocm_channel_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTAI OSC
    static sai_status_t create_otn_osc(
            sai_object_id_t *otn_osc_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_osc(sai_object_id_t otn_osc_id);
    static sai_status_t set_otn_osc_attribute(
            sai_object_id_t otn_osc_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_osc_attribute(
            sai_object_id_t otn_osc_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTN WSS
    static sai_status_t create_otn_wss(
            sai_object_id_t *otn_wss_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_wss(sai_object_id_t otn_wss_id);
    static sai_status_t set_otn_wss_attribute(
            sai_object_id_t otn_wss_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_wss_attribute(
            sai_object_id_t otn_wss_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);
    static sai_status_t create_otn_wss_spec_power(
            sai_object_id_t *otn_wss_spec_power_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_wss_spec_power(sai_object_id_t otn_wss_spec_power_id);
    static sai_status_t set_otn_wss_spec_power_attribute(
            sai_object_id_t otn_wss_spec_power_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_wss_spec_power_attribute(
            sai_object_id_t otn_wss_spec_power_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);
    static sai_status_t set_otn_wss_spec_powers_attribute(
            uint32_t object_count,
            const sai_object_id_t *object_id,
            const sai_attribute_t *attr_list,
            sai_bulk_op_error_mode_t mode,
            sai_status_t *object_statuses);
    // OTN OTDR scan type
    static sai_status_t create_otn_otdr_scan_type(
            sai_object_id_t *otn_otdr_scan_type_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_otdr_scan_type(sai_object_id_t otn_otdr_scan_type_id);
    static sai_status_t set_otn_otdr_scan_type_attribute(
            sai_object_id_t otn_otdr_scan_type_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_otdr_scan_type_attribute(
            sai_object_id_t otn_otdr_scan_type_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTN OTDR fiber profile
    static sai_status_t create_otn_otdr_fiber_profile(
            sai_object_id_t *otn_otdr_fiber_profile_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_otdr_fiber_profile(sai_object_id_t otn_otdr_fiber_profile_id);
    static sai_status_t set_otn_otdr_fiber_profile_attribute(
            sai_object_id_t otn_otdr_fiber_profile_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_otdr_fiber_profile_attribute(
            sai_object_id_t otn_otdr_fiber_profile_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OTN OTDR
    static sai_status_t create_otn_otdr(
            sai_object_id_t *otn_otdr_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_otn_otdr(sai_object_id_t otn_otdr_id);
    static sai_status_t set_otn_otdr_attribute(
            sai_object_id_t otn_otdr_id,
            const sai_attribute_t *attr);
    static sai_status_t get_otn_otdr_attribute(
            sai_object_id_t otn_otdr_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);
    // Others
    sai_status_t sai_api_query(sai_api_t sai_api_id, void **api_method_table);
    sai_object_type_t sai_object_type_query(sai_object_id_t);
    sai_status_t sai_query_attribute_capability(
            sai_object_id_t switch_id,
            sai_object_type_t object_type,
            sai_attr_id_t attr_id,
            sai_attr_capability_t *attr_capability);

    //variables
    static std::vector<sai_object_id_t> *switch_list_ptr;
    static switch_metadata *switch_metadata_ptr;

    // api table
    sai_switch_api_t switch_api;
    sai_router_interface_api_t router_interface_api;
    sai_otn_attenuator_api_t otn_attenuator_api;
    sai_otn_oa_api_t otn_oa_api;
    sai_otn_ocm_api_t otn_ocm_api;
    sai_otn_osc_api_t otn_osc_api;
    sai_otn_wss_api_t otn_wss_api;
    sai_otn_otdr_api_t otn_otdr_api;

    /* Report an environmental / HAL alarm that has no SAI object of its own
     * (fan, temperature, power supply, ...). Routes through the normal SAI
     * alarm-notification path (send_alarm_event_data -> syncd -> orchagent ->
     * eventd) using the switch object as the resource. 'event_name' must be
     * registered in the eventd profile (device default.json). */
    static void report_hal_alarm(const std::string& event_name,
                                 sai_otn_alarm_severity_t severity,
                                 sai_otn_alarm_action_t action,
                                 const std::string& description);

private:
    static sai_status_t init_switch();
    static sai_status_t fetch_ocm_data(otn_ocm_obj *obj);
    static sai_status_t set_ocm_waveplan(otn_ocm_obj* obj);
    // hal api call
    static sai_status_t hal_set_voa_attn(uint32_t dev, int32_t attn);
    static sai_status_t hal_get_voa_attn(uint32_t dev, int32_t &attn);
    static sai_status_t hal_set_oa_gain(uint32_t dev, int32_t gain);
    static sai_status_t hal_set_oa_gain_tilt(uint32_t dev, int32_t tilt);
    static sai_status_t hal_set_oa_gain_range(uint32_t dev, int32_t range);
    static sai_status_t hal_set_oa_mode(uint32_t dev, int32_t mode, int32_t val);
    static sai_status_t hal_set_oa_enable(uint32_t dev, bool enable);
    static sai_status_t hal_set_oa_working_state(uint32_t dev, int32_t state);
    static sai_status_t hal_set_oa_in_los_thr(uint32_t dev, int32_t thr, int32_t hys);
    static sai_status_t hal_set_oa_apr_enable(uint32_t dev, bool enable);
    static sai_status_t hal_get_oa_status_data(uint32_t dev, void *data);
    static sai_status_t hal_get_ocm_power(uint32_t dev, uint32_t port_index, void *data);
    static sai_status_t hal_set_ocm_waveplan(uint32_t dev, uint32_t port_index, uint32_t count, void *data);
    static sai_status_t hal_set_osc_enable(uint32_t dev, bool enable);
    static sai_status_t hal_get_osc_status_data(uint32_t dev, void *data);
    static sai_status_t hal_get_otdr_cfg_data(uint32_t dev, void *data);
    static sai_status_t hal_get_otdr_status_data(uint32_t dev, void *data);
    static sai_status_t hal_set_otdr_param(uint32_t dev, uint32_t distance_range, uint32_t pulse_width, double sample_res);
    static sai_status_t hal_set_otdr_thr(uint32_t dev, double reflect_thr, double splice_los_thr, double fiber_end_thr);
    static sai_status_t hal_set_otdr_time(uint32_t dev, uint32_t time);
    static sai_status_t hal_set_otdr_ior(uint32_t dev, double ior);

    static void send_alarm_event_data(
            sai_object_id_t object_id,
            sai_otn_alarm_severity_t severity,
            sai_otn_alarm_action_t action,
            std::string& event_name,
            std::string& description,
            std::vector<uint8_t>& raw_data);

    /* Edge-detecting alarm trigger. Emits a single RAISE when 'active' first
     * becomes true and a single CLEAR when it returns to false, using the
     * device's active-alarm set. 'event_name' must be registered in the eventd
     * profile (device default.json) or eventd drops it. */
    static void evaluate_alarm(
            virtual_otn_device* dev,
            sai_object_id_t object_id,
            const std::string& event_name,
            sai_otn_alarm_severity_t severity,
            bool active,
            const std::string& description,
            std::vector<uint8_t>& raw_data);

    /* Re-evaluate the WSS media-channel frequency-range alarm (lower < upper).
     * Called whenever either bound changes so a later valid pair CLEARs it. */
    static void check_wss_frequency_alarm(
            virtual_otn_wss_device* wss_dev,
            sai_object_id_t object_id);

    std::vector<sai_object_id_t> obj_list;
    switch_metadata metadata;
};

