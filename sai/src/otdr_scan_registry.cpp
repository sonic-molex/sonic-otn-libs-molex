#include "otdr_scan_registry.h"
#include "logger.h"

otdr_scan_registry& otdr_scan_registry::instance()
{
    static otdr_scan_registry registry;
    return registry;
}

otdr_scan_registry::acquire_result
otdr_scan_registry::acquire(const std::string& module, sai_object_id_t oid, const std::string& name)
{
    std::lock_guard<std::mutex> lock(mtx_);
    slot_state& slot = slots_[module];
    if (slot.active_oid != SAI_NULL_OBJECT_ID) {
        logger::notice("otdr_scan_registry: " + module + " busy with " + slot.active_name + ", rejecting " + name);
        return {SAI_STATUS_OBJECT_IN_USE, slot.generation, slot.active_name};
    }
    slot.active_oid = oid;
    slot.active_name = name;
    slot.generation++;
    logger::notice("otdr_scan_registry: " + module + " acquired by " + name +
                   ", generation " + std::to_string(slot.generation));
    return {SAI_STATUS_SUCCESS, slot.generation, ""};
}

bool otdr_scan_registry::cancel(const std::string& module, sai_object_id_t oid)
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = slots_.find(module);
    if (it == slots_.end() || it->second.active_oid != oid) {
        return false;
    }
    logger::notice("otdr_scan_registry: " + module + " scan on " + it->second.active_name + " cancelled");
    it->second.active_oid = SAI_NULL_OBJECT_ID;
    it->second.active_name.clear();
    it->second.generation++;
    return true;
}

bool otdr_scan_registry::complete(const std::string& module, sai_object_id_t oid, uint64_t generation)
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = slots_.find(module);
    if (it == slots_.end() || it->second.active_oid != oid || it->second.generation != generation) {
        return false;
    }
    it->second.active_oid = SAI_NULL_OBJECT_ID;
    it->second.active_name.clear();
    return true;
}

bool otdr_scan_registry::is_active(const std::string& module, sai_object_id_t oid) const
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = slots_.find(module);
    return it != slots_.end() && it->second.active_oid == oid;
}
