#include "sai_adapter.h"
#include "dev_util.h"
#include "virtual_otn_dev_mgr.h"
#include "otdr_static_config.h"

namespace {

// Resolve config from static tables and push to the virtual device before scan.
void apply_scan_config(virtual_otn_otdr_device* dev,
                       const std::string& port,
                       sai_otn_otdr_scan_type_t scan_type)
{
    if (scan_type == SAI_OTN_OTDR_SCAN_TYPE_CUSTOM) {
        const auto* p = otdr_port_config_table::instance().get(port);
        if (!p) {
            logger::warn(std::string("apply_scan_config: no CUSTOM config for port=") + port);
            return;
        }
        dev->set_acquisition_time_s(p->acquisition_time_s);
        dev->set_range_m(p->range_m);
        dev->set_pulse_width_ns(p->pulse_width_ns);
        dev->set_wavelength_mhz(p->wavelength_mhz);
        dev->set_sampling_resolution_m(p->sampling_resolution_m);
        dev->set_fiber_type(p->fiber_type);
        dev->set_negotiation(p->negotiation);
        dev->set_refractive_index(p->refractive_index);
        dev->set_backscatter_index(p->backscatter_index);
        dev->set_reflectance_threshold(p->reflectance_threshold);
        dev->set_splice_loss_threshold(p->splice_loss_threshold);
        dev->set_fiber_end_threshold(p->fiber_end_threshold);
        logger::debug(otdr_port_config_table::instance().dump(port));

    } else if (scan_type == SAI_OTN_OTDR_SCAN_TYPE_AUTO) {
        // hardware decides, nothing to push

    } else {
        // SHORT / MEDIUM / LONG
        const auto* sc = otdr_scan_type_config_table::instance().get(scan_type);
        if (!sc) {
            logger::warn(std::string("apply_scan_config: no config for scan_type=")
                         + std::to_string(static_cast<int>(scan_type)));
            return;
        }
        dev->set_acquisition_time_s(sc->acquisition_time_s);
        dev->set_range_m(sc->range_m);
        dev->set_pulse_width_ns(sc->pulse_width_ns);
        dev->set_wavelength_mhz(sc->wavelength_mhz);
        dev->set_sampling_resolution_m(sc->sampling_resolution_m);
        dev->set_fiber_type(sc->fiber_type);
        dev->set_negotiation(sc->negotiation);

        const auto* fp = otdr_fiber_profile_config_table::instance().get(sc->fiber_type);
        if (fp) {
            dev->set_refractive_index(fp->refractive_index);
            dev->set_backscatter_index(fp->backscatter_index);
            dev->set_reflectance_threshold(fp->reflectance_threshold);
            dev->set_splice_loss_threshold(fp->splice_loss_threshold);
            dev->set_fiber_end_threshold(fp->fiber_end_threshold);
        } else {
            logger::warn(std::string("apply_scan_config: no fiber profile for fiber_type=")
                         + std::to_string(static_cast<int>(sc->fiber_type)));
        }
        logger::debug(otdr_scan_type_config_table::instance().dump());
        logger::debug(otdr_fiber_profile_config_table::instance().dump());
    }
}

} // namespace

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
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).acquisition_time_s = attr->value.u32;
        break;

    case SAI_OTN_OTDR_ATTR_RANGE_M:
        obj->range_m = attr->value.u32;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).range_m = attr->value.u32;
        break;

    case SAI_OTN_OTDR_ATTR_PULSE_WIDTH_NS:
        obj->pulse_width_ns = attr->value.u32;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).pulse_width_ns = attr->value.u32;
        break;

    case SAI_OTN_OTDR_ATTR_WAVELENGTH_MHZ:
        obj->wavelength_mhz = attr->value.u64;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).wavelength_mhz = attr->value.u64;
        break;

    case SAI_OTN_OTDR_ATTR_SAMPLING_RESOLUTION_M:
        obj->sampling_resolution_m = attr->value.u64;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).sampling_resolution_m = attr->value.u64;
        break;

    case SAI_OTN_OTDR_ATTR_FIBER_TYPE:
        break;

    case SAI_OTN_OTDR_ATTR_NEGOTIATION:
        obj->negotiation = attr->value.booldata;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).negotiation = attr->value.booldata;
        break;

    case SAI_OTN_OTDR_ATTR_REFRACTIVE_INDEX:
        obj->refractive_index = attr->value.s32;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).refractive_index = attr->value.s32;
        break;

    case SAI_OTN_OTDR_ATTR_BACKSCATTER_INDEX:
        obj->backscatter_index = attr->value.s32;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).backscatter_index = attr->value.s32;
        break;

    case SAI_OTN_OTDR_ATTR_REFLECTANCE_THRESHOLD:
        obj->reflectance_threshold = attr->value.s32;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).reflectance_threshold = attr->value.s32;
        break;

    case SAI_OTN_OTDR_ATTR_SPLICE_LOSS_THRESHOLD:
        obj->splice_loss_threshold = attr->value.s32;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).splice_loss_threshold = attr->value.s32;
        break;

    case SAI_OTN_OTDR_ATTR_FIBER_END_THRESHOLD:
        obj->fiber_end_threshold = attr->value.s32;
        if (!obj->parent_port.empty())
            otdr_port_config_table::instance().get_or_create(obj->parent_port).fiber_end_threshold = attr->value.s32;
        break;

    case SAI_OTN_OTDR_ATTR_SCAN_TYPE:
        otdr_dev->set_scan_type((sai_otn_otdr_scan_type_t)attr->value.u32);
        break;

    case SAI_OTN_OTDR_ATTR_OPERATION:
    {
        sai_otn_otdr_operation_t op = (sai_otn_otdr_operation_t)attr->value.u32;
        if (op == SAI_OTN_OTDR_OPERATION_CANCEL) {
            logger::notice(std::string(__func__) + ", CANCEL: setting status to IDLE");
            otdr_dev->set_scanning_status(SAI_OTN_OTDR_STATUS_IDLE);
        } else {
            if (otdr_dev->get_scanning_status() == SAI_OTN_OTDR_STATUS_MEASURING) {
                logger::notice(std::string(__func__) + ", scan already in progress, rejecting");
                return SAI_STATUS_FAILURE;
            }
            apply_scan_config(otdr_dev, obj->parent_port, otdr_dev->get_scan_type());
            otdr_dev->trigger_scan(obj->sai_object_id,
                                   switch_metadata_ptr->otn_otdr_scan_complete_ntf);
        }
        break;
    }

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
            attr_list[i].value.u32 = obj->acquisition_time_s;
            break;
        case SAI_OTN_OTDR_ATTR_RANGE_M:
            attr_list[i].value.u32 = obj->range_m;
            break;
        case SAI_OTN_OTDR_ATTR_PULSE_WIDTH_NS:
            attr_list[i].value.u32 = obj->pulse_width_ns;
            break;
        case SAI_OTN_OTDR_ATTR_WAVELENGTH_MHZ:
            attr_list[i].value.u64 = obj->wavelength_mhz;
            break;
        case SAI_OTN_OTDR_ATTR_SAMPLING_RESOLUTION_M:
            attr_list[i].value.u64 = obj->sampling_resolution_m;
            break;
        case SAI_OTN_OTDR_ATTR_FIBER_TYPE:
            attr_list[i].value.u32 = obj->fiber_type;
            break;
        case SAI_OTN_OTDR_ATTR_NEGOTIATION:
            attr_list[i].value.booldata = obj->negotiation;
            break;
        case SAI_OTN_OTDR_ATTR_REFRACTIVE_INDEX:
            attr_list[i].value.s32 = obj->refractive_index;
            break;
        case SAI_OTN_OTDR_ATTR_BACKSCATTER_INDEX:
            attr_list[i].value.s32 = obj->backscatter_index;
            break;
        case SAI_OTN_OTDR_ATTR_REFLECTANCE_THRESHOLD:
            attr_list[i].value.s32 = obj->reflectance_threshold;
            break;
        case SAI_OTN_OTDR_ATTR_SPLICE_LOSS_THRESHOLD:
            attr_list[i].value.s32 = obj->splice_loss_threshold;
            break;
        case SAI_OTN_OTDR_ATTR_FIBER_END_THRESHOLD:
            attr_list[i].value.s32 = obj->fiber_end_threshold;
            break;
        case SAI_OTN_OTDR_ATTR_SCAN_TYPE:
            attr_list[i].value.u32 = otdr_dev->get_scan_type();
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
