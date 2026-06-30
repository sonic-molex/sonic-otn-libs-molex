#include "sai_adapter.h"
#include "virtual_otn_dev_mgr.h"
#include "virtual_otn_device.h"

// OTN device
sai_status_t
sai_adapter::create_otn_device(sai_object_id_t *otn_device_id,
                                sai_object_id_t switch_id,
                                uint32_t attr_count,
                                const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);
    otn_device_obj *obj = new otn_device_obj(switch_metadata_ptr->sai_id_map);
    *otn_device_id = obj->sai_object_id;
    logger::notice(std::string(__func__) + ", object id " + std::to_string(*otn_device_id));
    // TODO, create virtual OTN device

    for (uint32_t i = 0; i < attr_count; i++) {
        sai_status_t tmp_status = set_otn_device_attribute(obj->sai_object_id, attr_list + i);
        logger::debug(std::string(__func__) + " attr " + std::to_string(attr_list[i].id) + " ret " + std::to_string(tmp_status));
        if (tmp_status != SAI_STATUS_SUCCESS) {
            return tmp_status;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::remove_otn_device(sai_object_id_t otn_device_id)
{
    switch_metadata_ptr->sai_id_map.free_id(otn_device_id);
    // TODO, remove virtual OTN device
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_otn_device_attribute(sai_object_id_t otn_device_id,
                                    const sai_attribute_t *attr)
{    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_device_id) +
                   ", attr id " + std::to_string(attr->id));
    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, otn_device_obj, otn_device_id);
    // TODO, set virtual OTN device attribute

    switch (attr->id) {
    case SAI_OTN_DEVICE_ATTR_ADMIN_STATE:
        obj->admin_state = attr->value.booldata;
        // TODO, set admin state to device
        break;
    case SAI_OTN_DEVICE_ATTR_ALARM_ACT_TIME:
        obj->alarm_act_time = attr->value.u32;
        // TODO, set alarm act time to device
        break;
    case SAI_OTN_DEVICE_ATTR_ALARM_DEACT_TIME:
        obj->alarm_deact_time = attr->value.u32;
        // TODO, set alarm deact time to device
        break;
    default:
        logger::warn("unsupported otn device attribute, " + std::to_string(attr->id));
        rc = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
    return rc;
}

sai_status_t
sai_adapter::get_otn_device_attribute(sai_object_id_t otn_device_id,
                                    uint32_t attr_count,
                                    sai_attribute_t *attr_list)
{    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(otn_device_id) +
                   ", attr count " + std::to_string(attr_count));
    sai_status_t rc = SAI_STATUS_SUCCESS;
    // CAST_OBJ(obj, otn_device_obj, otn_device_id);
    // TODO, get virtual OTN device attribute
    return rc;
}

void
sai_adapter::send_alarm_event_data(
        sai_object_id_t object_id,
        sai_otn_alarm_severity_t severity,
        sai_otn_alarm_action_t action,
        std::string& event_name,
        std::string& description,
        std::vector<uint8_t>& raw_data)
{
    if (!switch_metadata_ptr->otn_alarm_event_ntf) {
        logger::warn("OTN alarm event notification function is not set, cannot send alarm event");
        return;
    }

    sai_otn_alarm_event_data_t alarm_data;
    alarm_data.object_id = object_id;
    alarm_data.event_name.count = event_name.size();
    alarm_data.event_name.list = reinterpret_cast<uint8_t*>(const_cast<char*>(event_name.c_str()));
    alarm_data.severity = severity;
    alarm_data.action = action;

    alarm_data.description.count = description.size();
    alarm_data.description.list = reinterpret_cast<uint8_t*>(const_cast<char*>(description.c_str()));

    alarm_data.data.count = raw_data.size();
    alarm_data.data.list = raw_data.data();

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    alarm_data.timestamp.tv_sec  = (uint64_t)ts.tv_sec;
    alarm_data.timestamp.tv_nsec = (uint32_t)ts.tv_nsec;

    switch_metadata_ptr->otn_alarm_event_ntf(1, &alarm_data);
}

void
sai_adapter::report_hal_alarm(
        const std::string& event_name,
        sai_otn_alarm_severity_t severity,
        sai_otn_alarm_action_t action,
        const std::string& description)
{
    /* Environmental/HAL alarms have no SAI object; use the switch object as the
     * resource. The HAL event already states RAISE vs CLEAR, so emit directly
     * (no edge detection). */
    std::string name = event_name;
    std::string desc = description;
    std::vector<uint8_t> raw_data;
    send_alarm_event_data(switch_metadata_ptr->switch_id, severity, action, name, desc, raw_data);
}

void
sai_adapter::evaluate_alarm(
        virtual_otn_device* dev,
        sai_object_id_t object_id,
        const std::string& event_name,
        sai_otn_alarm_severity_t severity,
        bool active,
        const std::string& description,
        std::vector<uint8_t>& raw_data)
{
    if (!dev) {
        logger::warn(std::string(__func__) + ", null device for " + event_name);
        return;
    }

    /* send_alarm_event_data takes non-const refs; copy so callers can pass
     * temporaries/const strings. */
    std::string name = event_name;
    std::string desc = description;

    if (active) {
        /* raise_alarm returns true only on the 0->1 edge */
        if (dev->raise_alarm(event_name)) {
            logger::notice(std::string(__func__) + ", RAISE " + event_name +
                           " on object " + std::to_string(object_id));
            send_alarm_event_data(object_id, severity, SAI_OTN_ALARM_ACTION_RAISE, name, desc, raw_data);
        }
    } else {
        /* clear_alarm returns true only when it was previously active (1->0) */
        if (dev->clear_alarm(event_name)) {
            logger::notice(std::string(__func__) + ", CLEAR " + event_name +
                           " on object " + std::to_string(object_id));
            send_alarm_event_data(object_id, severity, SAI_OTN_ALARM_ACTION_CLEAR, name, desc, raw_data);
        }
    }
}
