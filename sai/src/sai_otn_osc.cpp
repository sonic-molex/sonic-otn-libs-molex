#include "sai_adapter.h"
#include "dev_util.h"
#include "virtual_otn_dev_mgr.h"

// OTN OSC
sai_status_t
sai_adapter::create_otn_osc(sai_object_id_t *otn_osc_id,
                             sai_object_id_t switch_id,
                             uint32_t attr_count,
                             const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    otn_osc_obj *obj = new otn_osc_obj(switch_metadata_ptr->sai_id_map);
    *otn_osc_id = obj->sai_object_id;

    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_osc_id));

    auto& mgr = virtual_otn_device_manager::instance();
    sai_status_t ret_status = mgr.create_device(obj->sai_object_id,SAI_OBJECT_TYPE_OTN_OSC);
    if (ret_status == SAI_STATUS_SUCCESS)
    {
        sai_status_t tmp_status = SAI_STATUS_SUCCESS;
        for (uint32_t i = 0; i < attr_count; i++) {
            tmp_status = set_otn_osc_attribute(obj->sai_object_id, attr_list + i);
            logger::debug(std::string(__func__) + " attr " + std::to_string(attr_list[i].id) + " ret " + std::to_string(tmp_status));
            if (tmp_status != SAI_STATUS_SUCCESS) {
                ret_status = tmp_status;
                break;
            }
        }
        if (tmp_status == SAI_STATUS_SUCCESS) {
            std::unordered_map<std::string, otn_threshold_range_t> threshold_ranges;
            if (otn_threshold_config::instance().get_threshold_map(obj->dev_name, threshold_ranges)) {
                mgr.set_threshold_ranges(obj->sai_object_id, obj->dev_type, threshold_ranges);
            }
        }
    }

    return ret_status;
}

sai_status_t
sai_adapter::remove_otn_osc(sai_object_id_t otn_osc_id)
{
    logger::notice(std::string(__func__) + ", object id " + std::to_string(otn_osc_id));
    
    auto& mgr = virtual_otn_device_manager::instance();
    sai_status_t sai_status_ret = mgr.delete_device(otn_osc_id);
    if (sai_status_ret != SAI_STATUS_SUCCESS)
    {
        logger::warn(std::string(__func__) + ", Did not find Virtual device object id " + std::to_string(otn_osc_id));
    }
    switch_metadata_ptr->sai_id_map.free_id(otn_osc_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_osc_attribute(sai_object_id_t otn_osc_id,
                                    const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_osc_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_osc_obj, otn_osc_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto * vosc_dev = mgr.get_device<virtual_otn_osc_device>(obj->sai_object_id);
    if (!vosc_dev)
    {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(obj->sai_object_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    switch (attr->id) {
    case SAI_OTN_OSC_ATTR_NAME:
    {
        std::string name(attr->value.chardata);
        obj->dev_name = name;
        obj->dev_type = dev_util::get_dev_type(name);
        
        // Set OSC enable.
        vosc_dev->set_name(name);
        vosc_dev->enable();
        vosc_dev->inject_signal();
        vosc_dev->set_input_power(obj->sai_object_id); //simulate input power based on id for testing
        vosc_dev->set_output_frequency(vosc_dev->DEFAULT_OUTPUT_FREQ_KHZ + obj->sai_object_id); // set default freq for reference
        vosc_dev->simulate_signal_path();
        break;
    }
    default:
        rc = SAI_STATUS_NOT_SUPPORTED;
        logger::warn("unsupported otn attenuator attribute, " + std::to_string(attr->id));
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_otn_osc_attribute(sai_object_id_t otn_osc_id,
                                    uint32_t attr_count,
                                    sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_osc_obj, otn_osc_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto * vosc_dev = mgr.get_device<virtual_otn_osc_device>(obj->sai_object_id);
    if (!vosc_dev)
    {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(obj->sai_object_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    // Simulated status
    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_OSC_ATTR_INPUT_POWER:
            attr_list[i].value.s32 = vosc_dev->get_input_power();
            break;
        case SAI_OTN_OSC_ATTR_OUTPUT_POWER:
            attr_list[i].value.s32 = vosc_dev->get_output_power();
            break;
        case SAI_OTN_OSC_ATTR_LASER_BIAS_CURRENT:
            // TODO
            attr_list[i].value.u32 = 0;
            break;
        case SAI_OTN_OSC_ATTR_OUTPUT_FREQUENCY:
            attr_list[i].value.u64 = vosc_dev->get_output_frequency();
            break;
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported otn attenuator attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}
