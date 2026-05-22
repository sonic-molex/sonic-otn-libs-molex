#include "sai_adapter.h"
#include "dev_util.h"
#include "virtual_otn_dev_mgr.h"

// OTAI OTDR

sai_status_t
sai_adapter::create_otn_otdr(sai_object_id_t *otn_otdr_id,
                             sai_object_id_t switch_id,
                             uint32_t attr_count,
                             const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    otn_otdr_obj *obj = new otn_otdr_obj(switch_metadata_ptr->sai_id_map);
    *otn_otdr_id = obj->sai_object_id;
    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_otdr_id));

    auto& mgr = virtual_otn_device_manager::instance();
    // Virtual device shares the same object id with SAI object
    sai_status_t ret_status = mgr.create_device(obj->sai_object_id, SAI_OBJECT_TYPE_OTN_OTDR);
    if (ret_status == SAI_STATUS_SUCCESS) {
        for (uint32_t i = 0; i < attr_count; i++) {
            sai_status_t tmp = set_otn_otdr_attribute(obj->sai_object_id, attr_list + i);
            if (tmp != SAI_STATUS_SUCCESS) {
                ret_status = tmp;
                break;
            }
        }
    }

    return ret_status;
}

sai_status_t
sai_adapter::remove_otn_otdr(sai_object_id_t otn_otdr_id)
{
    auto& mgr = virtual_otn_device_manager::instance();
    sai_status_t ret = mgr.delete_device(otn_otdr_id);
    if (ret != SAI_STATUS_SUCCESS) {
        logger::warn(std::string(__func__) + ", device not found for object id " + std::to_string(otn_otdr_id));
    }
    switch_metadata_ptr->sai_id_map.free_id(otn_otdr_id);
    return ret;
}

sai_status_t
sai_adapter::set_otn_otdr_attribute(sai_object_id_t otn_otdr_id,
                                    const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_otdr_id) +
                   ", attr id " + std::to_string(attr->id));

    CAST_OBJ(obj, otn_otdr_obj, otn_otdr_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto* otdr_dev = mgr.get_device<virtual_otn_otdr_device>(obj->sai_object_id);
    if (!otdr_dev) {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(otn_otdr_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    switch (attr->id) {

    case SAI_OTN_OTDR_ATTR_NAME:
    {
        std::string name(attr->value.chardata);
        obj->name = name;
        obj->dev_type = dev_util::get_dev_type(name);
        otdr_dev->set_name(name);
        break;
    }
    case SAI_OTN_OTDR_ATTR_PARENT_PORT:
        obj->parent_port = attr->value.chardata;
        otdr_dev->set_parent_port(attr->value.chardata);
        break;

    case SAI_OTN_OTDR_ATTR_ACQUISITION_TIME_S:
        obj->acquisition_time_s = attr->value.u32;
        otdr_dev->set_acquisition_time_s(attr->value.u32);
        break;

    case SAI_OTN_OTDR_ATTR_RANGE_M:
        obj->range_m = attr->value.u32;
        otdr_dev->set_range_m(attr->value.u32);
        break;

    case SAI_OTN_OTDR_ATTR_PULSE_WIDTH_NS:
        obj->pulse_width_ns = attr->value.u32;
        otdr_dev->set_pulse_width_ns(attr->value.u32);
        break;

    case SAI_OTN_OTDR_ATTR_WAVELENGTH_MHZ:
        obj->wavelength_mhz = attr->value.u64;
        otdr_dev->set_wavelength_mhz(attr->value.u64);
        break;

    case SAI_OTN_OTDR_ATTR_SAMPLING_RESOLUTION_M:
        obj->sampling_resolution_m = attr->value.u64;
        otdr_dev->set_sampling_resolution_m(attr->value.u64);
        break;

    case SAI_OTN_OTDR_ATTR_FIBER_TYPE:
        otdr_dev->set_fiber_type((sai_otn_otdr_fiber_type_t)attr->value.u32);
        break;

    case SAI_OTN_OTDR_ATTR_NEGOTIATION:
        obj->negotiation = attr->value.booldata;
        otdr_dev->set_negotiation(attr->value.booldata);
        break;

    case SAI_OTN_OTDR_ATTR_REFRACTIVE_INDEX:
        obj->refractive_index = attr->value.s32;
        otdr_dev->set_refractive_index(attr->value.s32);
        break;

    case SAI_OTN_OTDR_ATTR_BACKSCATTER_INDEX:
        obj->backscatter_index = attr->value.s32;
        otdr_dev->set_backscatter_index(attr->value.s32);
        break;

    case SAI_OTN_OTDR_ATTR_REFLECTANCE_THRESHOLD:
        obj->reflectance_threshold = attr->value.s32;
        otdr_dev->set_reflectance_threshold(attr->value.s32);
        break;

    case SAI_OTN_OTDR_ATTR_SPLICE_LOSS_THRESHOLD:
        obj->splice_loss_threshold = attr->value.s32;
        otdr_dev->set_splice_loss_threshold(attr->value.s32);
        break;

    case SAI_OTN_OTDR_ATTR_FIBER_END_THRESHOLD:
        obj->fiber_end_threshold = attr->value.s32;
        otdr_dev->set_fiber_end_threshold(attr->value.s32);
        break;

    default:
        logger::warn("unsupported otn otdr attribute, " + std::to_string(attr->id));
        return SAI_STATUS_NOT_SUPPORTED;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::get_otn_otdr_attribute(sai_object_id_t otn_otdr_id,
                                    uint32_t attr_count,
                                    sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    CAST_OBJ(obj, otn_otdr_obj, otn_otdr_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto* otdr_dev = mgr.get_device<virtual_otn_otdr_device>(obj->sai_object_id);
    if (!otdr_dev) {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(otn_otdr_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sai_status_t rc = SAI_STATUS_SUCCESS;
    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_OTDR_ATTR_NAME:
            std::strncpy(attr_list[i].value.chardata, otdr_dev->get_name().c_str(), sizeof(attr_list[i].value.chardata) - 1);
            attr_list[i].value.chardata[sizeof(attr_list[i].value.chardata) - 1] = '\0';
            break;
        case SAI_OTN_OTDR_ATTR_PARENT_PORT:
            std::strncpy(attr_list[i].value.chardata, otdr_dev->get_parent_port().c_str(), sizeof(attr_list[i].value.chardata) - 1);
            attr_list[i].value.chardata[sizeof(attr_list[i].value.chardata) - 1] = '\0';
            break;
        case SAI_OTN_OTDR_ATTR_ACQUISITION_TIME_S:
            attr_list[i].value.u32 = otdr_dev->get_acquisition_time_s();
            break;
        case SAI_OTN_OTDR_ATTR_RANGE_M:
            attr_list[i].value.u32 = otdr_dev->get_range_m();
            break;
        case SAI_OTN_OTDR_ATTR_PULSE_WIDTH_NS:
            attr_list[i].value.u32 = otdr_dev->get_pulse_width_ns();
            break;
        case SAI_OTN_OTDR_ATTR_WAVELENGTH_MHZ:
            attr_list[i].value.u64 = otdr_dev->get_wavelength_mhz();
            break;
        case SAI_OTN_OTDR_ATTR_SAMPLING_RESOLUTION_M:
            attr_list[i].value.u64 = otdr_dev->get_sampling_resolution_m();
            break;
        case SAI_OTN_OTDR_ATTR_FIBER_TYPE:
            attr_list[i].value.u32 = otdr_dev->get_fiber_type();
            break;
        case SAI_OTN_OTDR_ATTR_NEGOTIATION:
            attr_list[i].value.booldata = otdr_dev->get_negotiation();
            break;
        case SAI_OTN_OTDR_ATTR_REFRACTIVE_INDEX:
            attr_list[i].value.s32 = otdr_dev->get_refractive_index();
            break;
        case SAI_OTN_OTDR_ATTR_BACKSCATTER_INDEX:
            attr_list[i].value.s32 = otdr_dev->get_backscatter_index();
            break;
        case SAI_OTN_OTDR_ATTR_REFLECTANCE_THRESHOLD:
            attr_list[i].value.s32 = otdr_dev->get_reflectance_threshold();
            break;
        case SAI_OTN_OTDR_ATTR_SPLICE_LOSS_THRESHOLD:
            attr_list[i].value.s32 = otdr_dev->get_splice_loss_threshold();
            break;
        case SAI_OTN_OTDR_ATTR_FIBER_END_THRESHOLD:
            attr_list[i].value.s32 = otdr_dev->get_fiber_end_threshold();
            break;
        case SAI_OTN_OTDR_ATTR_STATUS:
            attr_list[i].value.u32 = otdr_dev->get_scanning_status();
            break;
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported otn otdr attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}
