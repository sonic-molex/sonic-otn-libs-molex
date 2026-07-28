#include "sai_adapter.h"
#include "dev_util.h"
#include "virtual_otn_dev_mgr.h"
#include "virtual_otn_device.h"
#include <algorithm>
#include <cstring>

// ===================== OTN WSS =====================

/* Re-evaluate the WSS media-channel frequency-range alarm. Both bounds must be
 * set (non-zero) and lower must be < upper; otherwise RAISE "Invalid Frequency
 * Range". Called whenever either bound changes so a later valid pair CLEARs it. */
void sai_adapter::check_wss_frequency_alarm(virtual_otn_wss_device* wss_dev, sai_object_id_t object_id)
{
    sai_uint64_t lo = wss_dev->get_lower_frequency();
    sai_uint64_t hi = wss_dev->get_upper_frequency();

    /* Only judge once both bounds have been configured. */
    bool invalid = (lo != 0 && hi != 0 && lo >= hi);

    std::string event_name = "Invalid Frequency Range";
    std::string description = invalid
        ? "Lower frequency " + std::to_string(lo) +
          " is not below upper frequency " + std::to_string(hi)
        : "Frequency range [" + std::to_string(lo) + ", " + std::to_string(hi) + "] is valid";
    std::vector<uint8_t> raw_data;
    sai_adapter::evaluate_alarm(wss_dev, object_id, event_name, SAI_OTN_ALARM_SEVERITY_MINOR,
                                invalid, description, raw_data);
}

sai_status_t
sai_adapter::create_otn_wss(sai_object_id_t *otn_wss_id,
                            sai_object_id_t switch_id,
                            uint32_t attr_count,
                            const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    // First find a matching WSS object if it already exists (match by SOURCE_PORT_NAME or NAME)
    std::vector<sai_object_id_t> ids;
    switch_metadata_ptr->sai_id_map.get_ids(ids);

    std::string name;
    std::string source_port_name;
    bool index_in_list = false;
    for (uint32_t i = 0; i < attr_count; i++) {
        if (attr_list[i].id == SAI_OTN_WSS_ATTR_NAME) {
            name = attr_list[i].value.chardata;
        } else if (attr_list[i].id == SAI_OTN_WSS_ATTR_INDEX) {
            index_in_list = true;
        } else if (attr_list[i].id == SAI_OTN_WSS_ATTR_SOURCE_PORT_NAME) {
            source_port_name = attr_list[i].value.chardata;
        }
    }

    otn_wss_obj *obj = nullptr;
    for (const auto &it : ids) {
        sai_obj *obj_sai = static_cast<sai_obj *>(switch_metadata_ptr->sai_id_map.get_object(it));
        if (obj_sai->sai_object_type == (sai_object_type_t)SAI_OBJECT_TYPE_OTN_WSS) {
            otn_wss_obj *obj_temp = static_cast<otn_wss_obj *>(obj_sai);
            if (!source_port_name.empty()) {
                if (obj_temp->source_port_name == source_port_name) {
                    obj = obj_temp;
                    break;
                }
            } else if (!name.empty()) {
                if (obj_temp->dev_name == name) {
                    obj = obj_temp;
                    break;
                }
            }
        }
    }

    // Create new object if not found
    bool created_new = false;
    if (obj == nullptr) {
        obj = new otn_wss_obj(switch_metadata_ptr->sai_id_map);
        created_new = true;
        logger::notice(std::string(__func__) + ", object id " + std::to_string(obj->sai_object_id));
    } else {
        logger::notice(std::string(__func__) + ", found existing object id " + std::to_string(obj->sai_object_id));
    }

    *otn_wss_id = obj->sai_object_id;

    auto& mgr = virtual_otn_device_manager::instance();
    if (created_new) {
        sai_status_t status = mgr.create_device(obj->sai_object_id, SAI_OBJECT_TYPE_OTN_WSS);
        if (status != SAI_STATUS_SUCCESS) {
            logger::error(std::string(__func__) + ", Failed to create virtual WSS device, status=" + std::to_string(status));
            switch_metadata_ptr->sai_id_map.free_id(obj->sai_object_id);
            return status;
        }
    }

    for (uint32_t i = 0; i < attr_count; i++) {
        sai_status_t tmp_status = set_otn_wss_attribute(obj->sai_object_id, attr_list + i);
        logger::debug(std::string(__func__) + " attr " + std::to_string(attr_list[i].id) + " ret " + std::to_string(tmp_status));
        if (tmp_status != SAI_STATUS_SUCCESS) {
            if (created_new) {
                mgr.delete_device(obj->sai_object_id);
                switch_metadata_ptr->sai_id_map.free_id(obj->sai_object_id);
            }
            return tmp_status;
        }
    }

    /* When NAME is provided but INDEX was not in the create list, derive from name (e.g. WSS0-2|191262500 -> 2) */
    if (!index_in_list && !name.empty()) {
        obj->dev_index = dev_util::get_dev_index(name);
        auto* wss_dev = mgr.get_device<virtual_otn_wss_device>(obj->sai_object_id);
        if (wss_dev) {
            wss_dev->set_index(obj->dev_index);
        }
    }

    std::unordered_map<std::string, otn_threshold_range_t> threshold_ranges;
    if (otn_threshold_config::instance().get_threshold_map(obj->dev_name, threshold_ranges)) {
        mgr.set_threshold_ranges(obj->sai_object_id, obj->dev_type, threshold_ranges);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::remove_otn_wss(sai_object_id_t otn_wss_id)
{
    logger::notice(std::string(__func__) + ", object id " + std::to_string(otn_wss_id));

    CAST_OBJ(obj, otn_wss_obj, otn_wss_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto* wss_dev = mgr.get_device<virtual_otn_wss_device>(otn_wss_id);
    if (wss_dev) {
        wss_dev->remove_all_spec_power();
        mgr.delete_device(otn_wss_id);
    }

    /* Remove all spec_power children from id_map */
    for (const auto &it : obj->spec_power_entries) {
        switch_metadata_ptr->sai_id_map.free_id(it.second);
    }
    obj->spec_power_entries.clear();

    switch_metadata_ptr->sai_id_map.free_id(otn_wss_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_wss_attribute(sai_object_id_t otn_wss_id,
                                  const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_wss_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_wss_obj, otn_wss_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto* wss_dev = mgr.get_device<virtual_otn_wss_device>(obj->sai_object_id);

    switch (attr->id) {
    case SAI_OTN_WSS_ATTR_INDEX:
        obj->dev_index = attr->value.u32;
        if (wss_dev) wss_dev->set_index(attr->value.u32);
        break;
    case SAI_OTN_WSS_ATTR_NAME:
        obj->dev_name = attr->value.chardata;
        if (wss_dev) wss_dev->set_name(attr->value.chardata);
        break;
    case SAI_OTN_WSS_ATTR_LOWER_FREQUENCY:
        obj->lower_frequency = attr->value.u64;
        if (wss_dev) {
            wss_dev->set_lower_frequency(attr->value.u64);
            check_wss_frequency_alarm(wss_dev, obj->sai_object_id);
        }
        break;
    case SAI_OTN_WSS_ATTR_UPPER_FREQUENCY:
        obj->upper_frequency = attr->value.u64;
        if (wss_dev) {
            wss_dev->set_upper_frequency(attr->value.u64);
            check_wss_frequency_alarm(wss_dev, obj->sai_object_id);
        }
        break;
    case SAI_OTN_WSS_ATTR_ADMIN_STATE:
        obj->admin_state = (sai_otn_wss_admin_state_t)attr->value.s32;
        if (wss_dev) wss_dev->set_admin_state((sai_otn_wss_admin_state_t)attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_SUPER_CHANNEL:
        obj->super_channel = attr->value.booldata;
        if (wss_dev) wss_dev->set_super_channel(attr->value.booldata);
        break;
    case SAI_OTN_WSS_ATTR_SUPER_CHANNEL_PARENT:
        obj->super_channel_parent = attr->value.u32;
        if (wss_dev) wss_dev->set_super_channel_parent(attr->value.u32);
        break;
    case SAI_OTN_WSS_ATTR_ASE_CONTROL_MODE:
        obj->ase_control_mode = (sai_otn_wss_ase_control_mode_t)attr->value.s32;
        if (wss_dev) wss_dev->set_ase_control_mode((sai_otn_wss_ase_control_mode_t)attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_ASE_INJECTION_MODE:
        obj->ase_injection_mode = (sai_otn_wss_ase_injection_mode_t)attr->value.s32;
        if (wss_dev) wss_dev->set_ase_injection_mode((sai_otn_wss_ase_injection_mode_t)attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_ASE_INJECTION_THRESHOLD:
        obj->ase_injection_threshold = attr->value.s32;
        if (wss_dev) wss_dev->set_ase_injection_threshold(attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_ASE_INJECTION_DELTA:
        obj->ase_injection_delta = attr->value.s32;
        if (wss_dev) wss_dev->set_ase_injection_delta(attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_MEDIA_CHANNEL_INJECTION_OFFSET:
        obj->media_channel_injection_offset = attr->value.s32;
        if (wss_dev) wss_dev->set_media_channel_injection_offset(attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_ATTENUATION_CONTROL_MODE:
        obj->attenuation_control_mode = (sai_otn_wss_attenuation_control_mode_t)attr->value.s32;
        if (wss_dev) wss_dev->set_attenuation_control_mode((sai_otn_wss_attenuation_control_mode_t)attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_ATTENUATION_CONTROL_RANGE:
        obj->attenuation_control_range = (sai_otn_wss_attenuation_control_range_t)attr->value.s32;
        if (wss_dev) wss_dev->set_attenuation_control_range((sai_otn_wss_attenuation_control_range_t)attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_MAX_UNDERSHOOT_COMPENSATION:
        obj->max_undershoot_compensation = attr->value.s32;
        if (wss_dev) wss_dev->set_max_undershoot_compensation(attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_MAX_OVERSHOOT_COMPENSATION:
        obj->max_overshoot_compensation = attr->value.s32;
        if (wss_dev) wss_dev->set_max_overshoot_compensation(attr->value.s32);
        break;
    case SAI_OTN_WSS_ATTR_SOURCE_PORT_NAME:
        obj->source_port_name = attr->value.chardata;
        if (wss_dev) wss_dev->set_source_port_name(attr->value.chardata);
        if (obj->dev_index == 0) {
            obj->dev_index = dev_util::get_dev_index(obj->source_port_name);
            if (wss_dev) wss_dev->set_index(obj->dev_index);
        }
        break;
    case SAI_OTN_WSS_ATTR_DEST_PORT_NAME:
        obj->dest_port_name = attr->value.chardata;
        if (wss_dev) wss_dev->set_dest_port_name(attr->value.chardata);
        break;
    default:
        rc = SAI_STATUS_NOT_SUPPORTED;
        logger::warn("unsupported otn wss attribute, " + std::to_string(attr->id));
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_otn_wss_attribute(sai_object_id_t otn_wss_id,
                                  uint32_t attr_count,
                                  sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_wss_obj, otn_wss_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto* wss_dev = mgr.get_device<virtual_otn_wss_device>(obj->sai_object_id);

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_WSS_ATTR_INDEX:
            attr_list[i].value.u32 = wss_dev ? wss_dev->get_index() : obj->dev_index;
            break;
        case SAI_OTN_WSS_ATTR_NAME: {
            const std::string& s = wss_dev ? wss_dev->get_name() : obj->dev_name;
            size_t len = std::min(sizeof(attr_list[i].value.chardata) - 1, s.size());
            std::memcpy(attr_list[i].value.chardata, s.c_str(), len);
            attr_list[i].value.chardata[len] = '\0';
            break;
        }
        case SAI_OTN_WSS_ATTR_LOWER_FREQUENCY:
            attr_list[i].value.u64 = wss_dev ? wss_dev->get_lower_frequency() : obj->lower_frequency;
            break;
        case SAI_OTN_WSS_ATTR_UPPER_FREQUENCY:
            attr_list[i].value.u64 = wss_dev ? wss_dev->get_upper_frequency() : obj->upper_frequency;
            break;
        case SAI_OTN_WSS_ATTR_ADMIN_STATE:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_admin_state() : obj->admin_state;
            break;
        case SAI_OTN_WSS_ATTR_SUPER_CHANNEL:
            attr_list[i].value.booldata = wss_dev ? wss_dev->get_super_channel() : obj->super_channel;
            break;
        case SAI_OTN_WSS_ATTR_SUPER_CHANNEL_PARENT:
            attr_list[i].value.u32 = wss_dev ? wss_dev->get_super_channel_parent() : obj->super_channel_parent;
            break;
        case SAI_OTN_WSS_ATTR_ASE_CONTROL_MODE:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_ase_control_mode() : obj->ase_control_mode;
            break;
        case SAI_OTN_WSS_ATTR_ASE_INJECTION_MODE:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_ase_injection_mode() : obj->ase_injection_mode;
            break;
        case SAI_OTN_WSS_ATTR_ASE_INJECTION_THRESHOLD:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_ase_injection_threshold() : obj->ase_injection_threshold;
            break;
        case SAI_OTN_WSS_ATTR_ASE_INJECTION_DELTA:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_ase_injection_delta() : obj->ase_injection_delta;
            break;
        case SAI_OTN_WSS_ATTR_MEDIA_CHANNEL_INJECTION_OFFSET:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_media_channel_injection_offset() : obj->media_channel_injection_offset;
            break;
        case SAI_OTN_WSS_ATTR_ATTENUATION_CONTROL_MODE:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_attenuation_control_mode() : obj->attenuation_control_mode;
            break;
        case SAI_OTN_WSS_ATTR_ATTENUATION_CONTROL_RANGE:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_attenuation_control_range() : obj->attenuation_control_range;
            break;
        case SAI_OTN_WSS_ATTR_MAX_UNDERSHOOT_COMPENSATION:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_max_undershoot_compensation() : obj->max_undershoot_compensation;
            break;
        case SAI_OTN_WSS_ATTR_MAX_OVERSHOOT_COMPENSATION:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_max_overshoot_compensation() : obj->max_overshoot_compensation;
            break;
        case SAI_OTN_WSS_ATTR_SOURCE_PORT_NAME: {
            const std::string& s = wss_dev ? wss_dev->get_source_port_name() : obj->source_port_name;
            size_t len = std::min(sizeof(attr_list[i].value.chardata) - 1, s.size());
            std::memcpy(attr_list[i].value.chardata, s.c_str(), len);
            attr_list[i].value.chardata[len] = '\0';
            break;
        }
        case SAI_OTN_WSS_ATTR_DEST_PORT_NAME: {
            const std::string& s = wss_dev ? wss_dev->get_dest_port_name() : obj->dest_port_name;
            size_t len = std::min(sizeof(attr_list[i].value.chardata) - 1, s.size());
            std::memcpy(attr_list[i].value.chardata, s.c_str(), len);
            attr_list[i].value.chardata[len] = '\0';
            break;
        }
        case SAI_OTN_WSS_ATTR_OPER_STATUS:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_oper_status() : SAI_OTN_WSS_OPER_STATUS_UP;
            break;
        case SAI_OTN_WSS_ATTR_ASE_STATUS:
            attr_list[i].value.s32 = wss_dev ? wss_dev->get_ase_status() : SAI_OTN_WSS_ASE_STATUS_NOT_PRESENT;
            break;
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported otn wss attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}

// ===================== OTN WSS SPEC POWER =====================

sai_status_t
sai_adapter::create_otn_wss_spec_power(sai_object_id_t *otn_wss_spec_power_id,
                                       sai_object_id_t switch_id,
                                       uint32_t attr_count,
                                       const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    otn_wss_spec_power_obj *obj = new otn_wss_spec_power_obj(switch_metadata_ptr->sai_id_map);
    *otn_wss_spec_power_id = obj->sai_object_id;

    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_wss_spec_power_id));

    // Pre-scan SOURCE_PORT_NAME to capture key identity before parent lookup
    for (uint32_t i = 0; i < attr_count; i++) {
        const sai_attribute_t &attr = attr_list[i];
        if (attr.id == SAI_OTN_WSS_SPEC_POWER_ATTR_SOURCE_PORT_NAME) {
            std::string name(attr.value.chardata);
            obj->dev_name  = name;
            obj->dev_type  = dev_util::get_dev_type(name);
            obj->dev_index = dev_util::get_dev_index(name);
            logger::debug(std::string(__func__) + ", device_type = " + std::to_string(obj->dev_type) +
                     ", device_index = " + std::to_string(obj->dev_index) + " sub_index = " + std::to_string(dev_util::get_sub_index(name)));
            break;
        }
    }

    // Create parent WSS metadata object if it doesn't exist, or retireve existing parent
    obj->set_parent(switch_metadata_ptr->sai_id_map);

    // Check if the parent WSS virtual device exists
    auto& mgr = virtual_otn_device_manager::instance();
    auto* wss_dev = mgr.get_device<virtual_otn_wss_device>(obj->parent_wss_id);
    if (!wss_dev)
    {
        logger::warn(std::string(__func__) + ", Parent WSS device not found for object id " + std::to_string(obj->parent_wss_id));

        // create the parent WSS virtual device here
        sai_status_t status = mgr.create_device(obj->parent_wss_id,SAI_OBJECT_TYPE_OTN_WSS);
        if (status != SAI_STATUS_SUCCESS)
        {
            logger::error(std::string(__func__) + ", Failed to create virtual WSS device for object id "
                          + std::to_string(obj->parent_wss_id) + ", status=" + std::to_string(status));
            switch_metadata_ptr->sai_id_map.free_id(obj->sai_object_id);
            return status;
        }

        // Retrieve the newly created WSS device
        wss_dev = mgr.get_device<virtual_otn_wss_device>(obj->parent_wss_id);
        if (!wss_dev) {
            logger::error(std::string(__func__) + ": Failed to retrieve newly created virtual WSS device");
            switch_metadata_ptr->sai_id_map.free_id(obj->sai_object_id);
            return SAI_STATUS_FAILURE;
        }
    }

    // Create the virtual WSS spec power entry
    sai_status_t status = wss_dev->add_spec_power(obj->sai_object_id, obj->parent_wss_id);
    if (status != SAI_STATUS_SUCCESS) {
        logger::error(std::string(__func__) + ": Failed to add spec power to virtual WSS (status="
                      + std::to_string(status) + ")");
        switch_metadata_ptr->sai_id_map.free_id(obj->sai_object_id);
        return status;
    }

    for (uint32_t i = 0; i < attr_count; i++) {
        status = set_otn_wss_spec_power_attribute(obj->sai_object_id, attr_list + i);
        if (status != SAI_STATUS_SUCCESS) {
            logger::warn(std::string(__func__) + ": Failed to set attribute id "
                         + std::to_string(attr_list[i].id) + " (status="
                         + std::to_string(status) + ")");
        }
    }

    logger::notice(std::string(__func__) + ": Created and initialized WSS spec power "
                  + std::to_string(*otn_wss_spec_power_id)
                  + " under WSS " + std::to_string(obj->parent_wss_id));

    return status;
}

sai_status_t
sai_adapter::remove_otn_wss_spec_power(sai_object_id_t otn_wss_spec_power_id)
{
    logger::notice(std::string(__func__) + ", object id " + std::to_string(otn_wss_spec_power_id));

    CAST_OBJ(obj, otn_wss_spec_power_obj, otn_wss_spec_power_id);

    /* Remove from parent WSS virtual device */
    auto& mgr = virtual_otn_device_manager::instance();
    auto* wss_dev = mgr.get_device<virtual_otn_wss_device>(obj->parent_wss_id);
    if (wss_dev) {
        wss_dev->remove_spec_power(otn_wss_spec_power_id);
    }

    /* Unlink from parent WSS metadata */
    if (switch_metadata_ptr->sai_id_map.find_object(obj->parent_wss_id)) {
        sai_obj *parent = static_cast<sai_obj *>(switch_metadata_ptr->sai_id_map.get_object(obj->parent_wss_id));
        if (parent->sai_object_type == (sai_object_type_t)SAI_OBJECT_TYPE_OTN_WSS) {
            otn_wss_obj *wss_obj = static_cast<otn_wss_obj *>(parent);
            wss_obj->spec_power_entries.erase(obj->lower_frequency);
        }
    }

    switch_metadata_ptr->sai_id_map.free_id(otn_wss_spec_power_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_wss_spec_power_attribute(sai_object_id_t otn_wss_spec_power_id,
                                              const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_wss_spec_power_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_wss_spec_power_obj, otn_wss_spec_power_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto* wss_dev = (obj->parent_wss_id ? mgr.get_device<virtual_otn_wss_device>(obj->parent_wss_id) : nullptr);
    virtual_otn_wss_spec_power_entry* spec_entry = wss_dev ? wss_dev->get_spec_power(otn_wss_spec_power_id) : nullptr;

    if (!spec_entry) {
        logger::warn(std::string(__func__) + ": Spec power not found: " + std::to_string(otn_wss_spec_power_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    switch (attr->id) {
    case SAI_OTN_WSS_SPEC_POWER_ATTR_SOURCE_PORT_NAME:
        obj->dev_name = attr->value.chardata;
        break;
    case SAI_OTN_WSS_SPEC_POWER_ATTR_LOWER_FREQUENCY:
        obj->lower_frequency = attr->value.u64;
        spec_entry->set_lower_frequency(attr->value.u64);
        break;
    case SAI_OTN_WSS_SPEC_POWER_ATTR_UPPER_FREQUENCY:
        obj->upper_frequency = attr->value.u64;
        spec_entry->set_upper_frequency(attr->value.u64);
        break;
    case SAI_OTN_WSS_SPEC_POWER_ATTR_TARGET_POWER:
        obj->target_power = attr->value.s32;
        spec_entry->set_target_power(attr->value.s32);
        break;
    case SAI_OTN_WSS_SPEC_POWER_ATTR_ATTENUATION:
        obj->attenuation = attr->value.s32;
        obj->actual_attenuation = attr->value.s32;
        spec_entry->set_attenuation(attr->value.s32);
        break;
    default:
        rc = SAI_STATUS_NOT_SUPPORTED;
        logger::warn("unsupported otn wss spec power attribute, " + std::to_string(attr->id));
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_otn_wss_spec_power_attribute(sai_object_id_t otn_wss_spec_power_id,
                                             uint32_t attr_count,
                                             sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_wss_spec_power_obj, otn_wss_spec_power_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto* wss_dev = (obj->parent_wss_id ? mgr.get_device<virtual_otn_wss_device>(obj->parent_wss_id) : nullptr);
    virtual_otn_wss_spec_power_entry* spec_entry = wss_dev ? wss_dev->get_spec_power(otn_wss_spec_power_id) : nullptr;

    if (!spec_entry) {
        logger::warn(std::string(__func__) + ": Spec power not found: " + std::to_string(otn_wss_spec_power_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_WSS_SPEC_POWER_ATTR_SOURCE_PORT_NAME: {
            size_t len = obj->dev_name.copy(attr_list[i].value.chardata, sizeof(attr_list[i].value.chardata) - 1);
            attr_list[i].value.chardata[len] = '\0';
            break;
        }
        case SAI_OTN_WSS_SPEC_POWER_ATTR_LOWER_FREQUENCY:
            attr_list[i].value.u64 = spec_entry->get_lower_frequency();
            break;
        case SAI_OTN_WSS_SPEC_POWER_ATTR_UPPER_FREQUENCY:
            attr_list[i].value.u64 = spec_entry->get_upper_frequency();
            break;
        case SAI_OTN_WSS_SPEC_POWER_ATTR_TARGET_POWER:
            attr_list[i].value.s32 = spec_entry->get_target_power();
            break;
        case SAI_OTN_WSS_SPEC_POWER_ATTR_ATTENUATION:
            attr_list[i].value.s32 = spec_entry->get_attenuation();
            break;
        case SAI_OTN_WSS_SPEC_POWER_ATTR_ACTUAL_ATTENUATION:
            attr_list[i].value.s32 = spec_entry->get_actual_attenuation();
            break;
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported otn wss spec power attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}

sai_status_t
sai_adapter::set_otn_wss_spec_powers_attribute(uint32_t object_count,
                                               const sai_object_id_t *object_id,
                                               const sai_attribute_t *attr_list,
                                               sai_bulk_op_error_mode_t mode,
                                               sai_status_t *object_statuses)
{
    if (!object_id || !attr_list || !object_statuses)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    bool any_failure = false;
    for (uint32_t i = 0; i < object_count; i++)
    {
        object_statuses[i] = set_otn_wss_spec_power_attribute(object_id[i], &attr_list[i]);
        if (object_statuses[i] != SAI_STATUS_SUCCESS)
        {
            any_failure = true;
            if (mode == SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR)
            {
                for (uint32_t j = i + 1; j < object_count; j++)
                {
                    object_statuses[j] = SAI_STATUS_NOT_EXECUTED;
                }
                break;
            }
        }
    }

    return any_failure ? SAI_STATUS_FAILURE : SAI_STATUS_SUCCESS;
}
