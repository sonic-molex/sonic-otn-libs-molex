#include "virtual_otn_device.h"
#include "virtual_otn_dev_mgr.h"
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <vector>
#include <utility>
#include <string>
#include "logger.h"

// ===================== Singleton Access =====================
virtual_otn_device_manager& virtual_otn_device_manager::instance() {
    static virtual_otn_device_manager instance;
    return instance;
}

virtual_otn_device* virtual_otn_device_manager::get_device_base(sai_object_id_t object_id)
{
    auto it = g_otn_devices.find(object_id);
    if (it != g_otn_devices.end()) {
        return it->second.get();
    }
    return nullptr;
}

sai_object_id_t virtual_otn_device_manager::get_object_id_by_type_index(
        sai_object_type_extensions_t type, int index)
{
    if (index < 0) {
        return SAI_NULL_OBJECT_ID;
    }
    // Collect (name, object_id) of matching devices, order by name for a stable
    // instance->object mapping, then pick the index-th.
    std::vector<std::pair<std::string, sai_object_id_t>> matches;
    for (const auto& kv : g_otn_devices) {
        if (kv.second && kv.second->get_sai_object_type() == type) {
            matches.emplace_back(kv.second->get_name(), kv.first);
        }
    }
    if ((size_t)index >= matches.size()) {
        logger::warn(std::string(__func__) + " no device of type " + std::to_string(type) +
                     " at index " + std::to_string(index) + " (have " +
                     std::to_string(matches.size()) + ")");
        return SAI_NULL_OBJECT_ID;
    }
    std::sort(matches.begin(), matches.end());
    return matches[index].second;
}

// ===================== Factory Function =====================
sai_status_t virtual_otn_device_manager::create_device(sai_object_id_t object_id, sai_object_type_extensions_t type)
{
    logger::debug(std::string(__func__) + " called with type " + std::to_string(type));
    sai_status_t ret_status = SAI_STATUS_SUCCESS;

    /* Check if object id is already in the map */
    if (g_otn_devices.find(object_id) != g_otn_devices.end()) {
        logger::warn(std::string(__func__) + "object_id " + std::to_string(object_id) + "already exists" );
        ret_status = SAI_STATUS_ITEM_ALREADY_EXISTS;
        return ret_status;
    }

    try {
        switch (type) {
            case SAI_OBJECT_TYPE_OTN_OA:
                g_otn_devices[object_id] = std::make_unique<virtual_otn_oa_device>(object_id, type);
                break;

            case SAI_OBJECT_TYPE_OTN_ATTENUATOR:
                g_otn_devices[object_id] = std::make_unique<virtual_otn_voa_device>(object_id, type);
                break;

            case SAI_OBJECT_TYPE_OTN_OCM:
                g_otn_devices[object_id] = std::make_unique<virtual_otn_ocm_device>(object_id, type);
                break;

            case SAI_OBJECT_TYPE_OTN_OSC:
                g_otn_devices[object_id] = std::make_unique<virtual_otn_osc_device>(object_id, type);
                break;

            case SAI_OBJECT_TYPE_OTN_WSS:
                g_otn_devices[object_id] = std::make_unique<virtual_otn_wss_device>(object_id, type);
                break;

            case SAI_OBJECT_TYPE_OTN_OTDR:
                g_otn_devices[object_id] = std::make_unique<virtual_otn_otdr_device>(object_id, type);
                break;

            default:
                logger::warn(std::string(__func__) + "Current implementation cant handle object type " + std::to_string(type) );
                ret_status = SAI_STATUS_NOT_SUPPORTED;
                break;
        }
    } catch (const std::bad_alloc& e) {
        logger::error(std::string(__func__) + " memory allocation failed: " + e.what());
        ret_status = SAI_STATUS_NO_MEMORY;
    } catch (const std::exception& e) {
        logger::error(std::string(__func__) + " constructor exception: " + e.what());
        ret_status = SAI_STATUS_FAILURE;
    }
    
    logger::debug(std::string(__func__) + " returning " + std::to_string(ret_status));
    return ret_status;
}


sai_status_t virtual_otn_device_manager::delete_device(sai_object_id_t object_id)
{
    auto it = g_otn_devices.find(object_id);
    
    if (it == g_otn_devices.end()) {
        logger::warn(std::string(__func__) + " object_id " + std::to_string(object_id) + " not found");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    // Log what's being destroyed
    logger::debug(std::string(__func__) + " destroying virtual device with object_id " + 
                  std::to_string(object_id));
    
    // Erase from map - unique_ptr automatically deletes the object
    g_otn_devices.erase(it);
    
    return SAI_STATUS_SUCCESS;
}