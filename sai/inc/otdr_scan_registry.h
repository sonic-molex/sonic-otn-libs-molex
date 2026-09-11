#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <saiextensions.h>

// One physical OTDR per slot (module): tracks which instance currently owns the slot's single scan.
class otdr_scan_registry {
public:
    struct acquire_result {
        sai_status_t status;
        uint64_t generation;
        std::string busy_name;
    };

    static otdr_scan_registry& instance();

    // TRIGGER: takes the slot for oid, or returns SAI_STATUS_OBJECT_IN_USE naming the current holder.
    acquire_result acquire(const std::string& module, sai_object_id_t oid, const std::string& name);

    // CANCEL: frees the slot only when oid is the holder; false when this instance had no scan.
    bool cancel(const std::string& module, sai_object_id_t oid);

    // Scan thread wake-up: frees the slot only when oid still holds it with the same generation.
    bool complete(const std::string& module, sai_object_id_t oid, uint64_t generation);

    // True while oid owns the slot's scan; siblings of a busy slot read false.
    bool is_active(const std::string& module, sai_object_id_t oid) const;

private:
    struct slot_state {
        sai_object_id_t active_oid = SAI_NULL_OBJECT_ID;
        std::string active_name;
        uint64_t generation = 0;
    };

    otdr_scan_registry() = default;

    mutable std::mutex mtx_;
    std::unordered_map<std::string, slot_state> slots_;
};
