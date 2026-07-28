#pragma once

#include "virtual_otn_device.h"
#include "virtual_otn_dev_mgr.h"
#include <memory>


class virtual_otn_device_manager {
public:
    // Singleton accessor
    static virtual_otn_device_manager& instance();

    // Factory functions
    sai_status_t create_device(sai_object_id_t object_id, sai_object_type_extensions_t type);
    sai_status_t delete_device(sai_object_id_t object_id);
    sai_status_t set_threshold_ranges(sai_object_id_t object_id, uint32_t device_type, std::unordered_map<std::string, otn_threshold_range_t>& threshold_ranges);

    // Base getter
    virtual_otn_device* get_device_base(sai_object_id_t object_id);

    // Resolve the SAI object_id of the index-th device of a given type, ordered
    // deterministically by device name (so instance 0/1 -> OA0-0/OA0-1). Used to
    // attach object-bound HAL alarms to their SAI object. Returns
    // SAI_NULL_OBJECT_ID if there is no such device.
    sai_object_id_t get_object_id_by_type_index(sai_object_type_extensions_t type, int index);

    // Typed getter
    template <typename T>
    T* get_device(sai_object_id_t object_id) {
        virtual_otn_device* base = get_device_base(object_id);
        if (base) {
            return dynamic_cast<T*>(base);
        }
        return nullptr;
    }

private:
    virtual_otn_device_manager() = default;
    ~virtual_otn_device_manager() = default;

    // Disable copy/move
    virtual_otn_device_manager(const virtual_otn_device_manager&) = delete;
    virtual_otn_device_manager& operator=(const virtual_otn_device_manager&) = delete;
    virtual_otn_device_manager(virtual_otn_device_manager&&) = delete;
    virtual_otn_device_manager& operator=(virtual_otn_device_manager&&) = delete;

    // Internal map
    std::unordered_map<sai_object_id_t, std::unique_ptr<virtual_otn_device>> g_otn_devices;
};