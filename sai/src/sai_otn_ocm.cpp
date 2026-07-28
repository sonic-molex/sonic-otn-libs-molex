#include "sai_adapter.h"
#include "dev_util.h"
#include "virtual_otn_dev_mgr.h"
#include <algorithm>
#include <vector>

// OTN OCM
sai_status_t
sai_adapter::create_otn_ocm(sai_object_id_t *otn_ocm_id,
                             sai_object_id_t switch_id,
                             uint32_t attr_count,
                             const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    // First find the matched OCM object if it was existed
    std::vector<sai_object_id_t> ids;
    switch_metadata_ptr->sai_id_map.get_ids(ids);

    std::string name;
    for (uint32_t i = 0; i < attr_count; i++) {
        if (attr_list[i].id == SAI_OTN_OCM_ATTR_NAME) {
            name = attr_list[i].value.chardata;
            break;
        }
    }

    otn_ocm_obj *obj = nullptr;
    for (const auto &it : ids) {
        sai_obj *obj_sai = static_cast<sai_obj *>(switch_metadata_ptr->sai_id_map.get_object(it));
        if (obj_sai->sai_object_type == (sai_object_type_t)SAI_OBJECT_TYPE_OTN_OCM) {
            otn_ocm_obj *obj_temp = static_cast<otn_ocm_obj *>(obj_sai);
            if (obj_temp->dev_name == name) {
                obj = obj_temp;
                break;
            }
        }
    }

    // Then create new object if not found
    if (obj == nullptr) {
        obj = new otn_ocm_obj(switch_metadata_ptr->sai_id_map);
        logger::notice(std::string(__func__) + ", object id " +
                       std::to_string(obj->sai_object_id));
    } else {
        logger::notice(std::string(__func__) + ", found existing object id " +
                       std::to_string(obj->sai_object_id));
    }

    *otn_ocm_id = obj->sai_object_id;

    // create virtual OCM device 
    auto& mgr = virtual_otn_device_manager::instance();
    sai_status_t status = mgr.create_device(obj->sai_object_id,SAI_OBJECT_TYPE_OTN_OCM);

    if (status != SAI_STATUS_SUCCESS)
    {
        logger::error(std::string(__func__) + ", Failed to create virtual OCM device for object id "
                      + std::to_string(obj->sai_object_id) + ", status=" + std::to_string(status));
        delete obj;
        return status;
    }
    
    for (uint32_t i = 0; i < attr_count; i++) {
        status = set_otn_ocm_attribute(obj->sai_object_id, attr_list + i);
        if (status != SAI_STATUS_SUCCESS) {
        logger::warn(std::string(__func__) + ": Failed to set attribute id "
                        + std::to_string(attr_list[i].id) + " (status="
                        + std::to_string(status) + ")");
        }
    }
    if (status == SAI_STATUS_SUCCESS) {
        std::unordered_map<std::string, otn_threshold_range_t> threshold_ranges;
        if (otn_threshold_config::instance().get_threshold_map(obj->dev_name, threshold_ranges)) {
            mgr.set_threshold_ranges(obj->sai_object_id, obj->dev_type, threshold_ranges);
        }
    }

    return status;
}

sai_status_t
sai_adapter::remove_otn_ocm(sai_object_id_t otn_ocm_id)
{
    auto& mgr = virtual_otn_device_manager::instance();
    auto * virtual_ocm_dev = mgr.get_device<virtual_otn_ocm_device>(otn_ocm_id);
    if (!virtual_ocm_dev)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    else
    {
        //remove all channels from  ocm virtual device
        virtual_ocm_dev->remove_all_channels();
        
        //delete the device from device manager
        mgr.delete_device(otn_ocm_id);
    }

    switch_metadata_ptr->sai_id_map.free_id(otn_ocm_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_ocm_attribute(sai_object_id_t otn_ocm_id,
                                    const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_ocm_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_ocm_obj, otn_ocm_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto * ocm_dev = mgr.get_device<virtual_otn_ocm_device>(obj->sai_object_id);
    if (!ocm_dev)
    {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(obj->sai_object_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    switch (attr->id) {
    case SAI_OTN_OCM_ATTR_NAME:
    {
        std::string name(attr->value.chardata);
        obj->dev_name = name;
        obj->dev_type = dev_util::get_dev_type(name);
        obj->dev_index = dev_util::get_dev_index(name);
        logger::debug(std::string(__func__) + ", device_type = " + std::to_string(obj->dev_type) +
                     ", device_index = " + std::to_string(obj->dev_index));

        //set virtual device data 
        ocm_dev->set_name(name);
        break;
    }
    case SAI_OTN_OCM_ATTR_MONITOR_PORT:
        obj->monitor_port = std::string(attr->value.chardata);

        //set virtual device data 
        ocm_dev->set_monitor_port(attr->value.chardata);
        break;
    default:
        logger::warn("unsupported otn ocm attribute, " + std::to_string(attr->id));
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_otn_ocm_attribute(sai_object_id_t otn_ocm_id,
                                    uint32_t attr_count,
                                    sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    //CAST_OBJ(obj, otn_ocm_obj, otn_ocm_id);
    // auto& mgr = virtual_otn_device_manager::instance();
    // auto* ocm_dev = mgr.get_device<virtual_otn_ocm_device>(otn_ocm_id);

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_OCM_ATTR_RAW_DATA:
        {
            // OCM raw data: one int8_t per slot, units 0.01 dBm.
            static const uint32_t OCM_SLICE_MAX_NUM = 512;

            if (attr_list[i].value.s8list.count < OCM_SLICE_MAX_NUM) {
                attr_list[i].value.s8list.count = OCM_SLICE_MAX_NUM;
                rc = SAI_STATUS_BUFFER_OVERFLOW;
                break;
            }

            sai_int8_t* buf = attr_list[i].value.s8list.list;
            for (uint32_t s = 0; s < OCM_SLICE_MAX_NUM; s++)
                buf[s] = -30; // -30 dBm dummy value

            attr_list[i].value.s8list.count = OCM_SLICE_MAX_NUM;
            logger::notice(std::string(__func__) + ": RAW_DATA returned " +
                           std::to_string(OCM_SLICE_MAX_NUM) + " slots");
            break;
        }
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported otn ocm attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}

sai_status_t
sai_adapter::create_otn_ocm_channel(sai_object_id_t *otn_ocm_channel_id,
                                     sai_object_id_t switch_id,
                                     uint32_t attr_count,
                                     const sai_attribute_t *attr_list)
{
    otn_ocm_channel_obj *obj = new otn_ocm_channel_obj(switch_metadata_ptr->sai_id_map);
    *otn_ocm_channel_id = obj->sai_object_id;

    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_ocm_channel_id));

    // Pre-scan SAI_OTN_OCM_CHANNEL_ATTR_NAME to capture key identity info[dev_index] before parent lookup
    for (uint32_t i = 0; i < attr_count; i++) {
        const sai_attribute_t &attr = attr_list[i];
        if (attr.id == SAI_OTN_OCM_CHANNEL_ATTR_NAME) {
            std::string name(attr.value.chardata);
            obj->dev_name  = name;
            obj->dev_type  = dev_util::get_dev_type(name);
            obj->dev_index = dev_util::get_dev_index(name);
            logger::debug(std::string(__func__) + ", device_type = " + std::to_string(obj->dev_type) +
                     ", device_index = " + std::to_string(obj->dev_index) + " sub_index = " + std::to_string(dev_util::get_sub_index(name)));
            break;
        }
    }

    // Resolve parent OCM (based on dev_index)
    obj->set_parent(switch_metadata_ptr->sai_id_map);

    // Check if the parent OCM virtual device exists
    auto& mgr = virtual_otn_device_manager::instance();
    auto * virtual_ocm_dev = mgr.get_device<virtual_otn_ocm_device>(obj->ocm_id);
    if (!virtual_ocm_dev)
    {
        logger::warn(std::string(__func__) + ", Parent OCM device not found for object id " + std::to_string(obj->ocm_id));
        
        // lets create the parent OCM virtual device here
        sai_status_t status = mgr.create_device(obj->ocm_id,SAI_OBJECT_TYPE_OTN_OCM);
        if (status != SAI_STATUS_SUCCESS)   
        {
            logger::error(std::string(__func__) + ", Failed to create virtual OCM device for object id "
                          + std::to_string(obj->ocm_id) + ", status=" + std::to_string(status));
            delete obj;
            return status;
        }
        
        // Retrieve the newly created OCM device
        virtual_ocm_dev = mgr.get_device<virtual_otn_ocm_device>(obj->ocm_id);
        if (!virtual_ocm_dev) {
            logger::error(std::string(__func__) + ": Failed to retrieve newly created virtual OCM device");
            delete obj;
            return SAI_STATUS_FAILURE;
        }
    }

    // create the virtual OCM channel object 
    sai_status_t status = virtual_ocm_dev->add_channel(obj->ocm_id, *otn_ocm_channel_id);
    if (status != SAI_STATUS_SUCCESS)
    {
        logger::error(std::string(__func__) + ": Failed to add channel to virtual OCM (status="
                      + std::to_string(status) + ")");
        delete obj;
        return status;
    }

    for (uint32_t i = 0; i < attr_count; i++) {
        status = set_otn_ocm_channel_attribute(obj->sai_object_id, attr_list + i);
        if (status != SAI_STATUS_SUCCESS)
        {
            logger::warn(std::string(__func__) + ": Failed to set attribute id "
                         + std::to_string(attr_list[i].id) + " (status="
                         + std::to_string(status) + ")");
        }
    }
    
    logger::notice(std::string(__func__) + ": Created and initialized OCM channel "
                     + std::to_string(*otn_ocm_channel_id)
                     + " under OCM " + std::to_string(obj->ocm_id));

    return status; 
}

sai_status_t
sai_adapter::remove_otn_ocm_channel(sai_object_id_t otn_ocm_channel_id)
{
    CAST_OBJ(obj, otn_ocm_channel_obj, otn_ocm_channel_id);
    
    auto& mgr = virtual_otn_device_manager::instance();
    auto * virtual_ocm_dev = mgr.get_device<virtual_otn_ocm_device>(obj->ocm_id);
    if (!virtual_ocm_dev)
    {
        logger::warn(std::string(__func__) + ": Virtual Device parent OCM device not found, treating as already removed");
    }
    else
    {
        //remove the channel data from virtual device
        sai_status_t rc = virtual_ocm_dev->remove_channel(otn_ocm_channel_id);
        if (!rc)
        {
            // channel might already be gone — still return success
            logger::warn(std::string(__func__) + ": Virtual Device channel not found in OCM dev, treating as already removed");
        }
    }

    
    switch_metadata_ptr->sai_id_map.free_id(otn_ocm_channel_id);
    

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_ocm_channel_attribute(sai_object_id_t otn_ocm_channel_id,
                                            const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_ocm_channel_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_ocm_channel_obj, otn_ocm_channel_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto * virtual_ocm_dev = mgr.get_device<virtual_otn_ocm_device>(obj->ocm_id);
    if (!virtual_ocm_dev)
    {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(obj->ocm_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    //Get the virtual OCM channel device using the channel id
    virtual_otn_ocm_channel* v_ocm_ch = virtual_ocm_dev->get_channel(otn_ocm_channel_id);
    if (!v_ocm_ch) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    } 
    

    switch (attr->id) {
    case SAI_OTN_OCM_CHANNEL_ATTR_NAME:
    {
        std::string name(attr->value.chardata);
        obj->dev_name = name;
        obj->dev_type = dev_util::get_dev_type(name);
        obj->dev_index = dev_util::get_dev_index(name);

        logger::debug("set ocm channel name " + name +
                       ", type " + std::to_string(obj->dev_type) +
                       ", index " + std::to_string(obj->dev_index));
                       
        //set virtual channel data 
        v_ocm_ch->set_channel_name(name);

        //for simulation set the power here as well
        v_ocm_ch->set_power(otn_ocm_channel_id);
        //for simulation set the target power here as well
        v_ocm_ch->set_target_power(otn_ocm_channel_id + 10);
        break;
    }
    case SAI_OTN_OCM_CHANNEL_ATTR_LOWER_FREQUENCY:
        obj->lower_frequency = attr->value.u64;

        // Update min_frequency for easy searching
        if (obj->lower_frequency < obj->min_frequency) {
            obj->min_frequency = obj->lower_frequency;
        }
        v_ocm_ch->set_lower_frequency(attr->value.u64);
        break;
    case SAI_OTN_OCM_CHANNEL_ATTR_UPPER_FREQUENCY:
        obj->upper_frequency = attr->value.u64;

        //set virtual channel data 
        v_ocm_ch->set_upper_frequency(attr->value.u64);
        break;
    default:
        rc = SAI_STATUS_NOT_SUPPORTED;
        logger::warn("unsupported otn attenuator attribute, " + std::to_string(attr->id));
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_otn_ocm_channel_attribute(sai_object_id_t otn_ocm_channel_id,
                                             uint32_t attr_count,
                                             sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_ocm_channel_obj, otn_ocm_channel_id);

    auto& mgr = virtual_otn_device_manager::instance();
    auto * virtual_ocm_dev = mgr.get_device<virtual_otn_ocm_device>(obj->ocm_id);
    if (!virtual_ocm_dev)
    {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(obj->ocm_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    //Get the virtual OCM channel device using the channel id
    virtual_otn_ocm_channel* v_ocm_ch = virtual_ocm_dev->get_channel(otn_ocm_channel_id);
    if (!v_ocm_ch) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    } 
    
     for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OTN_OCM_CHANNEL_ATTR_POWER:
            // if (obj->min_frequency == obj->lower_frequency) {
            //     CAST_OBJ(obj_parent, otn_ocm_obj, obj->ocm_id);
            //     fetch_ocm_data(obj_parent);
            // }
            attr_list[i].value.s32 = v_ocm_ch->get_power();
            break;
        case SAI_OTN_OCM_CHANNEL_ATTR_TARGET_POWER:
            attr_list[i].value.s32 = v_ocm_ch->get_target_power();
            break;

        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported otn ocm attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}
