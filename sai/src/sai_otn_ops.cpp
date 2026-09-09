#include "sai_adapter.h"

sai_status_t
sai_adapter::create_otn_ops(sai_object_id_t *otn_ops_id,
                            sai_object_id_t switch_id,
                            uint32_t attr_count,
                            const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    auto *obj = new otn_ops_obj(switch_metadata_ptr->sai_id_map);
    *otn_ops_id = obj->sai_object_id;

    for (uint32_t i = 0; i < attr_count; ++i) {
        sai_status_t status = set_otn_ops_attribute(*otn_ops_id, &attr_list[i]);
        if (status != SAI_STATUS_SUCCESS) {
            switch_metadata_ptr->sai_id_map.free_id(*otn_ops_id);
            return status;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::remove_otn_ops(sai_object_id_t otn_ops_id)
{
    switch_metadata_ptr->sai_id_map.free_id(otn_ops_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_ops_attribute(sai_object_id_t otn_ops_id,
                                   const sai_attribute_t *attr)
{
    CAST_OBJ(obj, otn_ops_obj, otn_ops_id);

    switch (attr->id) {
    case SAI_OTN_OPS_ATTR_NAME:
        obj->dev_name = attr->value.chardata;
        break;
    case SAI_OTN_OPS_ATTR_REVERTIVE:
        obj->revertive = attr->value.booldata;
        break;
    case SAI_OTN_OPS_ATTR_WAIT_TO_RESTORE_TIME:
        obj->wait_to_restore_time = attr->value.u32;
        break;
    case SAI_OTN_OPS_ATTR_HOLD_OFF_TIME:
        obj->hold_off_time = attr->value.u32;
        break;
    case SAI_OTN_OPS_ATTR_PRIMARY_SWITCH_THRESHOLD:
        obj->primary_switch_threshold = attr->value.s32;
        break;
    case SAI_OTN_OPS_ATTR_PRIMARY_SWITCH_HYSTERESIS:
        obj->primary_switch_hysteresis = attr->value.s32;
        break;
    case SAI_OTN_OPS_ATTR_SECONDARY_SWITCH_THRESHOLD:
        obj->secondary_switch_threshold = attr->value.s32;
        break;
    case SAI_OTN_OPS_ATTR_RELATIVE_SWITCH_THRESHOLD:
        obj->relative_switch_threshold = attr->value.s32;
        break;
    case SAI_OTN_OPS_ATTR_RELATIVE_SWITCH_THRESHOLD_OFFSET:
        obj->relative_switch_threshold_offset = attr->value.s32;
        break;
    case SAI_OTN_OPS_ATTR_FORCE_TO_PORT:
        obj->force_to_port = attr->value.s32;
        if (obj->force_to_port != SAI_OTN_OPS_FORCE_TO_PORT_NONE) {
            obj->active_path = obj->force_to_port == SAI_OTN_OPS_FORCE_TO_PORT_PRIMARY ?
                                   SAI_OTN_OPS_ACTIVE_PATH_PRIMARY :
                                   SAI_OTN_OPS_ACTIVE_PATH_SECONDARY;
        }
        break;
    case SAI_OTN_OPS_ATTR_MANUAL_TO_PORT:
        if ((obj->force_to_port == SAI_OTN_OPS_FORCE_TO_PORT_PRIMARY &&
             attr->value.s32 == SAI_OTN_OPS_MANUAL_TO_PORT_SECONDARY) ||
            (obj->force_to_port == SAI_OTN_OPS_FORCE_TO_PORT_SECONDARY &&
             attr->value.s32 == SAI_OTN_OPS_MANUAL_TO_PORT_PRIMARY)) {
            return SAI_STATUS_OBJECT_IN_USE;
        }
        if (attr->value.s32 == SAI_OTN_OPS_MANUAL_TO_PORT_PRIMARY) {
            obj->active_path = SAI_OTN_OPS_ACTIVE_PATH_PRIMARY;
        } else if (attr->value.s32 == SAI_OTN_OPS_MANUAL_TO_PORT_SECONDARY) {
            obj->active_path = SAI_OTN_OPS_ACTIVE_PATH_SECONDARY;
        }
        break;
    case SAI_OTN_OPS_ATTR_LINE_PRIMARY_IN_ENABLED:
        obj->primary_in_enabled = attr->value.booldata;
        break;
    case SAI_OTN_OPS_ATTR_LINE_SECONDARY_IN_ENABLED:
        obj->secondary_in_enabled = attr->value.booldata;
        break;
    case SAI_OTN_OPS_ATTR_COMMON_IN_ENABLED:
        obj->common_in_enabled = attr->value.booldata;
        break;
    case SAI_OTN_OPS_ATTR_LINE_PRIMARY_IN_TARGET_ATTENUATION:
    case SAI_OTN_OPS_ATTR_LINE_PRIMARY_OUT_TARGET_ATTENUATION:
    case SAI_OTN_OPS_ATTR_LINE_SECONDARY_IN_TARGET_ATTENUATION:
    case SAI_OTN_OPS_ATTR_LINE_SECONDARY_OUT_TARGET_ATTENUATION:
    case SAI_OTN_OPS_ATTR_COMMON_IN_TARGET_ATTENUATION:
    case SAI_OTN_OPS_ATTR_COMMON_OUTPUT_TARGET_ATTENUATION:
        obj->attenuations[attr->id] = attr->value.s32;
        break;
    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::get_otn_ops_attribute(sai_object_id_t otn_ops_id,
                                   uint32_t attr_count,
                                   sai_attribute_t *attr_list)
{
    CAST_OBJ(obj, otn_ops_obj, otn_ops_id);

    for (uint32_t i = 0; i < attr_count; ++i) {
        switch (attr_list[i].id) {
        case SAI_OTN_OPS_ATTR_ACTIVE_PATH:
            attr_list[i].value.s32 = obj->active_path;
            break;
        default:
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    return SAI_STATUS_SUCCESS;
}
