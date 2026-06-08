#include "sai_adapter.h"
#include <sstream>
#include <iomanip>

sai_status_t
sai_adapter::create_switch(sai_object_id_t *switch_id,
                           uint32_t attr_count,
                           const sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    sai_status_t status;
    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_SWITCH_ATTR_INIT_SWITCH:
            if (attr_list[i].value.booldata) {
                // status = init_switch(deafult_mac_set, fdb_notification_set, port_notification_set);
                status = init_switch();
                if (status != SAI_STATUS_SUCCESS) {
                    return status;
                }
            }
            break;
        default:
            set_switch_attribute(switch_metadata_ptr->switch_id, &attr_list[i]);
            break;
        }
    }
    *switch_id = switch_metadata_ptr->switch_id;
    logger::notice(std::string(__func__) + ", object id " + std::to_string(*switch_id));

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::init_switch()
{
    if (switch_list_ptr->size() != 0) {
        logger::warn("currently one switch is supportred, returning operating switch_id: " +
                     std::to_string((*switch_list_ptr)[0]));
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    sai_obj *switch_obj = new sai_obj(switch_metadata_ptr->sai_id_map, SAI_OBJECT_TYPE_SWITCH);
    logger::notice("create new switch, switch_id: " + std::to_string(switch_obj->sai_object_id));

    switch_metadata_ptr->switch_id = switch_obj->sai_object_id;
    switch_list_ptr->push_back(switch_obj->sai_object_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::remove_switch(sai_object_id_t switch_id)
{
    logger::notice(std::string(__func__) + ", object id " + std::to_string(switch_id));

    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t
sai_adapter::get_switch_attribute(sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID:
            attr_list[i].value.oid = switch_metadata_ptr->vids.find(SAI_OBJECT_TYPE_BRIDGE)->second;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_VLAN_ID:
            attr_list[i].value.oid = switch_metadata_ptr->vids.find(SAI_OBJECT_TYPE_VLAN)->second;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID:
            attr_list[i].value.oid = switch_metadata_ptr->vids.find(SAI_OBJECT_TYPE_VIRTUAL_ROUTER)->second;
            break;
        case SAI_SWITCH_ATTR_CPU_PORT:
            attr_list[i].value.oid = switch_metadata_ptr->vids.find(SAI_OBJECT_TYPE_PORT)->second;
            break;
        case SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP:
            attr_list[i].value.oid = switch_metadata_ptr->vids.find(SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP)->second;
            break;
        case SAI_SWITCH_ATTR_PORT_LIST:
            attr_list[i].value.objlist.count = 0;
            break;
        case SAI_SWITCH_ATTR_PORT_NUMBER:
        case SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS:
            attr_list[i].value.u32 = 0;
            break;
        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            memcpy(attr_list[i].value.mac, switch_metadata_ptr->default_switch_mac, 6);
            break;
        case SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO:
            attr_list[i].value.s8list.list[0] = '\0';
            attr_list[i].value.s8list.count = 1;
            break;
        case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_TABLE_GROUP_MINIMUM_PRIORITY:
            attr_list[i].value.u32 = 10;
            break;
        case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_TABLE_GROUP_MAXIMUM_PRIORITY:
        case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
            attr_list[i].value.u32 = 1000;
            break;
        case SAI_SWITCH_ATTR_OTN_ALARM_EVENT_NOTIFY:
            attr_list[i].value.ptr = (void *) switch_metadata_ptr->otn_alarm_event_ntf;
            break;
        case SAI_SWITCH_ATTR_OTN_OTDR_SCAN_COMPLETE_NOTIFY:
            attr_list[i].value.ptr = (void *) switch_metadata_ptr->otn_otdr_scan_complete_ntf;
            break;
        default:
            logger::warn("unsupported switch attribute " + std::to_string(attr_list[i].id));
            return SAI_STATUS_NOT_IMPLEMENTED;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::set_switch_attribute(sai_object_id_t switch_id,
                                  const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(switch_id) +
                   ", attr id " + std::to_string(attr->id));

    switch (attr->id) {
    case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
    {
        memcpy(switch_metadata_ptr->default_switch_mac, attr->value.mac, 6);

        std::ostringstream oss;
        oss << "default switch mac set to:"
            << std::uppercase << std::setfill('0') << std::hex
            << std::setw(2) << static_cast<int>(attr->value.mac[0]) << ":"
            << std::setw(2) << static_cast<int>(attr->value.mac[1]) << ":"
            << std::setw(2) << static_cast<int>(attr->value.mac[2]) << ":"
            << std::setw(2) << static_cast<int>(attr->value.mac[3]) << ":"
            << std::setw(2) << static_cast<int>(attr->value.mac[4]) << ":"
            << std::setw(2) << static_cast<int>(attr->value.mac[5]);
        logger::notice(oss.str());

        break;
    }
    case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
        logger::notice("fdb event notification funciton was set");
        //switch_metadata_ptr->fdb_event_notification_fn = (sai_fdb_event_notification_fn) attr->value.ptr;
        break;
    case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
        logger::notice("port state notification funciton was set");
        //switch_metadata_ptr->port_state_change_notification_fn = (sai_port_state_change_notification_fn) attr->value.ptr;
        break;
    case SAI_SWITCH_ATTR_OTN_ALARM_EVENT_NOTIFY:
        logger::notice("otn alarm event notification funciton was set");
        switch_metadata_ptr->otn_alarm_event_ntf = (sai_otn_alarm_event_notification_fn) attr->value.ptr;
        break;
    case SAI_SWITCH_ATTR_OTN_OTDR_SCAN_COMPLETE_NOTIFY:
        logger::notice("otn otdr scan complete notification function was set");
        switch_metadata_ptr->otn_otdr_scan_complete_ntf = (sai_otn_otdr_scan_complete_notification_fn) attr->value.ptr;
        break;
    default:
        logger::warn("unsupported switch attribute " + std::to_string(attr->id));
        break;
    }
    return SAI_STATUS_SUCCESS;
}
