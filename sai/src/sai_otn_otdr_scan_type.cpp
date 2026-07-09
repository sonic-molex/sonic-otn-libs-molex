#include "sai_adapter.h"
#include "otdr_static_config.h"

// ============================================================
// OTN OTDR scan-type config  (SAI_OBJECT_TYPE_OTN_OTDR_SCAN_TYPE)
// ============================================================

sai_status_t
sai_adapter::create_otn_otdr_scan_type(sai_object_id_t *otn_otdr_scan_type_id,
                                       sai_object_id_t switch_id,
                                       uint32_t attr_count,
                                       const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    auto *obj = new otn_otdr_scan_type_obj(switch_metadata_ptr->sai_id_map);
    *otn_otdr_scan_type_id = obj->sai_object_id;
    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_otdr_scan_type_id));

    for (uint32_t i = 0; i < attr_count; i++) {
        sai_status_t rc = set_otn_otdr_scan_type_attribute(obj->sai_object_id, attr_list + i);
        if (rc != SAI_STATUS_SUCCESS)
            return rc;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::remove_otn_otdr_scan_type(sai_object_id_t otn_otdr_scan_type_id)
{
    switch_metadata_ptr->sai_id_map.free_id(otn_otdr_scan_type_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_otdr_scan_type_attribute(sai_object_id_t otn_otdr_scan_type_id,
                                              const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_otdr_scan_type_id) +
                   ", attr id " + std::to_string(attr->id));

    CAST_OBJ(obj, otn_otdr_scan_type_obj, otn_otdr_scan_type_id);

    auto& tbl = otdr_scan_type_config_table::instance();

    switch (attr->id) {

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_OTN_OTDR_SCAN_TYPE:
        obj->scan_type = (sai_otn_otdr_scan_type_t)attr->value.u32;
        tbl.get_or_create(obj->scan_type);   // ensure entry exists with defaults
        break;

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_ACQUISITION_TIME_S:
        obj->acquisition_time_s = attr->value.u32;
        tbl.get_or_create(obj->scan_type).acquisition_time_s = attr->value.u32;
        break;

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_RANGE_M:
        obj->range_m = attr->value.u32;
        tbl.get_or_create(obj->scan_type).range_m = attr->value.u32;
        break;

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_PULSE_WIDTH_NS:
        obj->pulse_width_ns = attr->value.u32;
        tbl.get_or_create(obj->scan_type).pulse_width_ns = attr->value.u32;
        break;

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_WAVELENGTH_MHZ:
        obj->wavelength_mhz = attr->value.u64;
        tbl.get_or_create(obj->scan_type).wavelength_mhz = attr->value.u64;
        break;

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_SAMPLING_RESOLUTION_M:
        obj->sampling_resolution_m = attr->value.u64;
        tbl.get_or_create(obj->scan_type).sampling_resolution_m = attr->value.u64;
        break;

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_OTN_OTDR_FIBER_TYPE:
        obj->fiber_type = (sai_otn_otdr_fiber_type_t)attr->value.u32;
        tbl.get_or_create(obj->scan_type).fiber_type = (sai_otn_otdr_fiber_type_t)attr->value.u32;
        break;

    case SAI_OTN_OTDR_SCAN_TYPE_ATTR_NEGOTIATION:
        obj->negotiation = attr->value.booldata;
        tbl.get_or_create(obj->scan_type).negotiation = attr->value.booldata;
        break;

    default:
        logger::warn("unsupported otn_otdr_scan_type attribute " + std::to_string(attr->id));
        return SAI_STATUS_NOT_SUPPORTED;
    }

    logger::debug(tbl.dump());
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::get_otn_otdr_scan_type_attribute(sai_object_id_t otn_otdr_scan_type_id,
                                              uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    CAST_OBJ(obj, otn_otdr_scan_type_obj, otn_otdr_scan_type_id);

    sai_status_t rc = SAI_STATUS_SUCCESS;
    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_OTN_OTDR_SCAN_TYPE:
            attr_list[i].value.u32 = obj->scan_type;
            break;
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_ACQUISITION_TIME_S:
            attr_list[i].value.u32 = obj->acquisition_time_s;
            break;
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_RANGE_M:
            attr_list[i].value.u32 = obj->range_m;
            break;
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_PULSE_WIDTH_NS:
            attr_list[i].value.u32 = obj->pulse_width_ns;
            break;
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_WAVELENGTH_MHZ:
            attr_list[i].value.u64 = obj->wavelength_mhz;
            break;
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_SAMPLING_RESOLUTION_M:
            attr_list[i].value.u64 = obj->sampling_resolution_m;
            break;
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_OTN_OTDR_FIBER_TYPE:
            attr_list[i].value.u32 = obj->fiber_type;
            break;
        case SAI_OTN_OTDR_SCAN_TYPE_ATTR_NEGOTIATION:
            attr_list[i].value.booldata = obj->negotiation;
            break;
        default:
            logger::warn("unsupported otn_otdr_scan_type attribute " + std::to_string(attr_list[i].id));
            rc = SAI_STATUS_NOT_SUPPORTED;
            break;
        }
    }
    return rc;
}


// ============================================================
// OTN OTDR fiber profile  (SAI_OBJECT_TYPE_OTN_OTDR_FIBER_PROFILE)
// ============================================================

sai_status_t
sai_adapter::create_otn_otdr_fiber_profile(sai_object_id_t *otn_otdr_fiber_profile_id,
                                           sai_object_id_t switch_id,
                                           uint32_t attr_count,
                                           const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    auto *obj = new otn_otdr_fiber_profile_obj(switch_metadata_ptr->sai_id_map);
    *otn_otdr_fiber_profile_id = obj->sai_object_id;
    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_otdr_fiber_profile_id));

    for (uint32_t i = 0; i < attr_count; i++) {
        sai_status_t rc = set_otn_otdr_fiber_profile_attribute(obj->sai_object_id, attr_list + i);
        if (rc != SAI_STATUS_SUCCESS)
            return rc;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::remove_otn_otdr_fiber_profile(sai_object_id_t otn_otdr_fiber_profile_id)
{
    switch_metadata_ptr->sai_id_map.free_id(otn_otdr_fiber_profile_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_otdr_fiber_profile_attribute(sai_object_id_t otn_otdr_fiber_profile_id,
                                                  const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_otdr_fiber_profile_id) +
                   ", attr id " + std::to_string(attr->id));

    CAST_OBJ(obj, otn_otdr_fiber_profile_obj, otn_otdr_fiber_profile_id);

    auto& tbl = otdr_fiber_profile_config_table::instance();

    switch (attr->id) {

    case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_OTN_OTDR_FIBER_TYPE:
        obj->fiber_type = (sai_otn_otdr_fiber_type_t)attr->value.u32;
        tbl.get_or_create(obj->fiber_type);   // ensure entry exists with defaults
        break;

    case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_REFRACTIVE_INDEX:
        obj->refractive_index = attr->value.s32;
        tbl.get_or_create(obj->fiber_type).refractive_index = attr->value.s32;
        break;

    case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_BACKSCATTER_INDEX:
        obj->backscatter_index = attr->value.s32;
        tbl.get_or_create(obj->fiber_type).backscatter_index = attr->value.s32;
        break;

    case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_REFLECTANCE_THRESHOLD:
        obj->reflectance_threshold = attr->value.s32;
        tbl.get_or_create(obj->fiber_type).reflectance_threshold = attr->value.s32;
        break;

    case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_SPLICE_LOSS_THRESHOLD:
        obj->splice_loss_threshold = attr->value.s32;
        tbl.get_or_create(obj->fiber_type).splice_loss_threshold = attr->value.s32;
        break;

    case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_FIBER_END_THRESHOLD:
        obj->fiber_end_threshold = attr->value.s32;
        tbl.get_or_create(obj->fiber_type).fiber_end_threshold = attr->value.s32;
        break;

    default:
        logger::warn("unsupported otn_otdr_fiber_profile attribute " + std::to_string(attr->id));
        return SAI_STATUS_NOT_SUPPORTED;
    }

    logger::debug(tbl.dump());
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::get_otn_otdr_fiber_profile_attribute(sai_object_id_t otn_otdr_fiber_profile_id,
                                                  uint32_t attr_count,
                                                  sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    CAST_OBJ(obj, otn_otdr_fiber_profile_obj, otn_otdr_fiber_profile_id);

    sai_status_t rc = SAI_STATUS_SUCCESS;
    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_OTN_OTDR_FIBER_TYPE:
            attr_list[i].value.u32 = obj->fiber_type;
            break;
        case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_REFRACTIVE_INDEX:
            attr_list[i].value.s32 = obj->refractive_index;
            break;
        case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_BACKSCATTER_INDEX:
            attr_list[i].value.s32 = obj->backscatter_index;
            break;
        case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_REFLECTANCE_THRESHOLD:
            attr_list[i].value.s32 = obj->reflectance_threshold;
            break;
        case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_SPLICE_LOSS_THRESHOLD:
            attr_list[i].value.s32 = obj->splice_loss_threshold;
            break;
        case SAI_OTN_OTDR_FIBER_PROFILE_ATTR_FIBER_END_THRESHOLD:
            attr_list[i].value.s32 = obj->fiber_end_threshold;
            break;
        default:
            logger::warn("unsupported otn_otdr_fiber_profile attribute " + std::to_string(attr_list[i].id));
            rc = SAI_STATUS_NOT_SUPPORTED;
            break;
        }
    }
    return rc;
}
