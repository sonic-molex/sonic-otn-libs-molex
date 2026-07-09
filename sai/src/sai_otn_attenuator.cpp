#include "sai_adapter.h"
#include "dev_util.h"
#include "virtual_otn_dev_mgr.h"

// OTN attenuator
sai_status_t
sai_adapter::create_otn_attenuator(sai_object_id_t *otn_attenuator_id,
                                    sai_object_id_t switch_id,
                                    uint32_t attr_count,
                                    const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    otn_attenuator_obj *obj = new otn_attenuator_obj(switch_metadata_ptr->sai_id_map);
    *otn_attenuator_id = obj->sai_object_id;

    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_attenuator_id));
    
    auto& mgr = virtual_otn_device_manager::instance();
    sai_status_t ret_status = mgr.create_device(obj->sai_object_id,SAI_OBJECT_TYPE_OTN_ATTENUATOR);

    if (ret_status == SAI_STATUS_SUCCESS)
    {
        for (uint32_t i = 0; i < attr_count; i++) {
            sai_status_t tmp_status = set_otn_attenuator_attribute(obj->sai_object_id, attr_list + i);
            logger::debug(std::string(__func__) + " attr " + std::to_string(attr_list[i].id) + " ret " + std::to_string(tmp_status));
            if (tmp_status != SAI_STATUS_SUCCESS) {
                ret_status = tmp_status;
                break;
            }
        }
    }

    return ret_status;
}

sai_status_t
sai_adapter::remove_otn_attenuator(sai_object_id_t otn_attenuator_id)
{
    auto& mgr = virtual_otn_device_manager::instance();
    sai_status_t sai_status_ret = mgr.delete_device(otn_attenuator_id);
    if (sai_status_ret != SAI_STATUS_SUCCESS)
    {
        logger::warn(std::string(__func__) + ", Did not find Virtual device object id " + std::to_string(otn_attenuator_id));
    }
    switch_metadata_ptr->sai_id_map.free_id(otn_attenuator_id);
    return sai_status_ret;
}

sai_status_t
sai_adapter::set_otn_attenuator_attribute(sai_object_id_t otn_attenuator_id,
                                          const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_attenuator_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_attenuator_obj, otn_attenuator_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto * voa_dev = mgr.get_device<virtual_otn_voa_device>(obj->sai_object_id);
    if (!voa_dev)
    {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(obj->sai_object_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
         
    switch (attr->id) {
    case SAI_OTN_ATTENUATOR_ATTR_NAME:
    {
        std::string name(attr->value.chardata);
        obj->dev_name = name;
        obj->dev_type = dev_util::get_dev_type(name);

        // TODO, set port name by name instead of reading from module
        obj->ingress_port = name + "-IN";
        obj->egress_port = name + "-OUT";

        voa_dev->set_name(name);
        voa_dev->set_ingress_port(obj->ingress_port);
        voa_dev->set_egress_port(obj->egress_port);

        break;
    }
    case SAI_OTN_ATTENUATOR_ATTR_ATTENUATION_MODE:
    {
        obj->attn_mode = attr->value.u32;

        //set into virtual device 
        voa_dev->set_attn_mode((sai_otn_attenuator_mode_t)attr->value.u32);
        break;
    }
    case SAI_OTN_ATTENUATOR_ATTR_TARGET_OUTPUT_POWER:
    {
        obj->target_power = attr->value.s32;
        // TODO, set target output power to module
        voa_dev->set_max_output_power(attr->value.s32);
        break;
    }
    case SAI_OTN_ATTENUATOR_ATTR_ATTENUATION:
    {
        obj->set_attn = attr->value.u32;

        voa_dev->set_attenuation(attr->value.s32);

        /* Attenuation valid range [0, 6000] in 0.01 dB units (0..60 dB).
         * RAISE when outside, CLEAR once a valid value is set. */
        static const sai_int32_t ATTN_MIN = 0;
        static const sai_int32_t ATTN_MAX = 6000;
        bool out_of_range = attr->value.s32 < ATTN_MIN || attr->value.s32 > ATTN_MAX;
        std::string event_name = "Attenuation Out of Range";
        std::string description = out_of_range
            ? "Attenuation " + std::to_string(attr->value.s32) +
              " is out of range [" + std::to_string(ATTN_MIN) + ", " + std::to_string(ATTN_MAX) + "]"
            : "Attenuation " + std::to_string(attr->value.s32) + " is within range";
        std::vector<uint8_t> raw_data = {(uint8_t)(attr->value.s32 & 0xFF), (uint8_t)((attr->value.s32 >> 8) & 0xFF), (uint8_t)((attr->value.s32 >> 16) & 0xFF), (uint8_t)((attr->value.s32 >> 24) & 0xFF)};
        evaluate_alarm(voa_dev, obj->sai_object_id, event_name, SAI_OTN_ALARM_SEVERITY_MINOR,
                       out_of_range, description, raw_data);
        break;
    }
    case SAI_OTN_ATTENUATOR_ATTR_ENABLED:
    {
        obj->enable = attr->value.booldata;

        voa_dev->set_enabled(attr->value.booldata);
        break;
    }
    case SAI_OTN_ATTENUATOR_ATTR_MAX_OUTPUT_POWER:
    {
        obj->max_power = attr->value.s32;
        // TODO, set max output power to module

        voa_dev->set_max_output_power(attr->value.s32);
        break;
    }
    case SAI_OTN_ATTENUATOR_ATTR_MAX_OUTPUT_POWER_THRESHOLD:
    {
        obj->max_power_thr = attr->value.s32;
        // TODO, set max output power threshold to module
        voa_dev->set_max_output_power_threshold(attr->value.s32);
        break;
    }
    default:
        logger::warn("unsupported otn attenuator attribute, " + std::to_string(attr->id));
        rc = SAI_STATUS_NOT_SUPPORTED;
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_otn_attenuator_attribute(sai_object_id_t otn_attenuator_id,
                                           uint32_t attr_count,
                                           sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_attenuator_obj, otn_attenuator_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto * voa_dev = mgr.get_device<virtual_otn_voa_device>(obj->sai_object_id);
    if (!voa_dev)
    {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(obj->sai_object_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_ATTENUATOR_ATTR_INGRESS_PORT:
            std::strncpy(attr_list[i].value.chardata, voa_dev->get_ingress_port().c_str(), sizeof(attr_list[i].value.chardata));
            break;
        case SAI_OTN_ATTENUATOR_ATTR_EGRESS_PORT:
            std::strncpy(attr_list[i].value.chardata, voa_dev->get_egress_port().c_str(), sizeof(attr_list[i].value.chardata));
            break;
        case SAI_OTN_ATTENUATOR_ATTR_ACTUAL_ATTENUATION:
        {
            // TODO, simulate actual attenuation from module
            attr_list[i].value.u32 = voa_dev->get_attenuation();
            break;
        }
        case SAI_OTN_ATTENUATOR_ATTR_SYSTEM_DERIVED_TARGET_OUTPUT_POWER:
        case SAI_OTN_ATTENUATOR_ATTR_OUTPUT_POWER_TOTAL:
        case SAI_OTN_ATTENUATOR_ATTR_OPTICAL_RETURN_LOSS:
            attr_list[i].value.s32 = 0;
            break;

        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported otn attenuator attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}
