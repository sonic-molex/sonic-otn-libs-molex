#include "sai_adapter.h"

std::vector<sai_object_id_t> *sai_adapter::switch_list_ptr;
switch_metadata *sai_adapter::switch_metadata_ptr;


sai_adapter::sai_adapter()
{
    // ptr
    switch_list_ptr = &obj_list;
    switch_metadata_ptr = &metadata;

    // api set
    switch_api.create_switch = &sai_adapter::create_switch;
    switch_api.remove_switch = &sai_adapter::remove_switch;
    switch_api.get_switch_attribute = &sai_adapter::get_switch_attribute;
    switch_api.set_switch_attribute = &sai_adapter::set_switch_attribute;

    router_interface_api.create_router_interface = &sai_adapter::create_router_interface;
    router_interface_api.remove_router_interface = &sai_adapter::remove_router_interface;
    router_interface_api.set_router_interface_attribute = &sai_adapter::set_router_interface_attribute;
    router_interface_api.get_router_interface_attribute = &sai_adapter::get_router_interface_attribute;

    otn_attenuator_api.create_otn_attenuator = &sai_adapter::create_otn_attenuator;
    otn_attenuator_api.remove_otn_attenuator = &sai_adapter::remove_otn_attenuator;
    otn_attenuator_api.set_otn_attenuator_attribute = &sai_adapter::set_otn_attenuator_attribute;
    otn_attenuator_api.get_otn_attenuator_attribute = &sai_adapter::get_otn_attenuator_attribute;

    otn_oa_api.create_otn_oa = &sai_adapter::create_otn_oa;
    otn_oa_api.remove_otn_oa = &sai_adapter::remove_otn_oa;
    otn_oa_api.set_otn_oa_attribute = &sai_adapter::set_otn_oa_attribute;
    otn_oa_api.get_otn_oa_attribute = &sai_adapter::get_otn_oa_attribute;

    otn_ocm_api.create_otn_ocm = &sai_adapter::create_otn_ocm;
    otn_ocm_api.remove_otn_ocm = &sai_adapter::remove_otn_ocm;
    otn_ocm_api.set_otn_ocm_attribute = &sai_adapter::set_otn_ocm_attribute;
    otn_ocm_api.get_otn_ocm_attribute = &sai_adapter::get_otn_ocm_attribute;
    otn_ocm_api.create_otn_ocm_channel = &sai_adapter::create_otn_ocm_channel;
    otn_ocm_api.remove_otn_ocm_channel = &sai_adapter::remove_otn_ocm_channel;
    otn_ocm_api.set_otn_ocm_channel_attribute = &sai_adapter::set_otn_ocm_channel_attribute;
    otn_ocm_api.get_otn_ocm_channel_attribute = &sai_adapter::get_otn_ocm_channel_attribute;

    otn_osc_api.create_otn_osc = &sai_adapter::create_otn_osc;
    otn_osc_api.remove_otn_osc = &sai_adapter::remove_otn_osc;
    otn_osc_api.set_otn_osc_attribute = &sai_adapter::set_otn_osc_attribute;
    otn_osc_api.get_otn_osc_attribute = &sai_adapter::get_otn_osc_attribute;

    otn_wss_api.create_otn_wss = &sai_adapter::create_otn_wss;
    otn_wss_api.remove_otn_wss = &sai_adapter::remove_otn_wss;
    otn_wss_api.set_otn_wss_attribute = &sai_adapter::set_otn_wss_attribute;
    otn_wss_api.get_otn_wss_attribute = &sai_adapter::get_otn_wss_attribute;
    otn_wss_api.create_otn_wsss = nullptr;
    otn_wss_api.remove_otn_wsss = nullptr;
    otn_wss_api.set_otn_wsss_attribute = nullptr;
    otn_wss_api.get_otn_wsss_attribute = nullptr;
    otn_wss_api.create_otn_wss_spec_power = &sai_adapter::create_otn_wss_spec_power;
    otn_wss_api.remove_otn_wss_spec_power = &sai_adapter::remove_otn_wss_spec_power;
    otn_wss_api.set_otn_wss_spec_power_attribute = &sai_adapter::set_otn_wss_spec_power_attribute;
    otn_wss_api.get_otn_wss_spec_power_attribute = &sai_adapter::get_otn_wss_spec_power_attribute;
    otn_wss_api.create_otn_wss_spec_powers = nullptr;
    otn_wss_api.remove_otn_wss_spec_powers = nullptr;
    otn_wss_api.set_otn_wss_spec_powers_attribute = &sai_adapter::set_otn_wss_spec_powers_attribute;
    otn_wss_api.get_otn_wss_spec_powers_attribute = nullptr;

    otn_otdr_api.create_otn_otdr = &sai_adapter::create_otn_otdr;
    otn_otdr_api.remove_otn_otdr = &sai_adapter::remove_otn_otdr;
    otn_otdr_api.set_otn_otdr_attribute = &sai_adapter::set_otn_otdr_attribute;
    otn_otdr_api.get_otn_otdr_attribute = &sai_adapter::get_otn_otdr_attribute;
    otn_otdr_api.create_otn_otdr_scan_type = &sai_adapter::create_otn_otdr_scan_type;
    otn_otdr_api.remove_otn_otdr_scan_type = &sai_adapter::remove_otn_otdr_scan_type;
    otn_otdr_api.set_otn_otdr_scan_type_attribute = &sai_adapter::set_otn_otdr_scan_type_attribute;
    otn_otdr_api.get_otn_otdr_scan_type_attribute = &sai_adapter::get_otn_otdr_scan_type_attribute;
    otn_otdr_api.create_otn_otdr_fiber_profile = &sai_adapter::create_otn_otdr_fiber_profile;
    otn_otdr_api.remove_otn_otdr_fiber_profile = &sai_adapter::remove_otn_otdr_fiber_profile;
    otn_otdr_api.set_otn_otdr_fiber_profile_attribute = &sai_adapter::set_otn_otdr_fiber_profile_attribute;
    otn_otdr_api.get_otn_otdr_fiber_profile_attribute = &sai_adapter::get_otn_otdr_fiber_profile_attribute;

    logger::notice("sai adapter initialized");
}

sai_adapter::~sai_adapter()
{
    logger::notice("sai adapter closed");
}

sai_status_t
sai_adapter::sai_api_query(sai_api_t sai_api_id, void **api_method_table)
{
    switch ((uint32_t)sai_api_id) {
    case SAI_API_SWITCH:
        *api_method_table = &switch_api;
        break;
    case SAI_API_ROUTER_INTERFACE:
        *api_method_table = &router_interface_api;
        break;
    case SAI_API_OTN_ATTENUATOR:
        *api_method_table = &otn_attenuator_api;
        break;
    case SAI_API_OTN_OA:
        *api_method_table = &otn_oa_api;
        break;
    case SAI_API_OTN_OCM:
        *api_method_table = &otn_ocm_api;
        break;
    case SAI_API_OTN_OSC:
        *api_method_table = &otn_osc_api;
        break;
    case SAI_API_OTN_WSS:
        *api_method_table = &otn_wss_api;
        break;
    case SAI_API_OTN_OTDR:
        *api_method_table = &otn_otdr_api;
        break;
    default:
        logger::notice("unsupported api request made " + std::to_string(sai_api_id));
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

sai_object_type_t
sai_adapter::sai_object_type_query(sai_object_id_t sai_object_id)
{
    // top level, switch
    if (switch_metadata_ptr->switch_id == sai_object_id) {
        return SAI_OBJECT_TYPE_SWITCH;
    }

    // otn sai
    if (switch_metadata_ptr->sai_id_map.find_object(sai_object_id)) {
        return static_cast<sai_obj *>(switch_metadata_ptr->sai_id_map.get_object(sai_object_id))->sai_object_type;
    }

    // vid
    for (const auto &id : switch_metadata_ptr->vids) {
        if (sai_object_id == id.second) {
            return id.first;
        }
    }

    return SAI_OBJECT_TYPE_NULL;
}

sai_status_t
sai_adapter::sai_query_attribute_capability(
        sai_object_id_t switch_id,
        sai_object_type_t object_type,
        sai_attr_id_t attr_id,
        sai_attr_capability_t *attr_capability)
{
    if (object_type == SAI_OBJECT_TYPE_SWITCH) {
        switch (attr_id) {
        case SAI_SWITCH_ATTR_OTN_ALARM_EVENT_NOTIFY:
        case SAI_SWITCH_ATTR_OTN_OTDR_SCAN_COMPLETE_NOTIFY:
            attr_capability->create_implemented = false;
            attr_capability->set_implemented = true;
            attr_capability->get_implemented = true;
            break;
        default:
            attr_capability->create_implemented = false;
            attr_capability->set_implemented = false;
            attr_capability->get_implemented = false;
            break;
        }
    } else {
        attr_capability->create_implemented = false;
        attr_capability->set_implemented = false;
        attr_capability->get_implemented = false;
    }

    return SAI_STATUS_SUCCESS;
}
