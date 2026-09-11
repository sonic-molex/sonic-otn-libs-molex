#include "virtual_otn_device.h"
#include "dev_util.h"
#include "otdr_scan_registry.h"
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <chrono>
#include <climits>
#include "logger.h"

/* OCM Device */
// Channel management
   sai_status_t virtual_otn_ocm_device::add_channel(sai_object_id_t parent_ocm_id, sai_object_id_t channel_id)
    {
        if (channels.find(channel_id) != channels.end()) {
            logger::warn(std::string(__func__) + ": Channel already exists: " + std::to_string(channel_id));
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }

        // map of channel ids to ocm channel objects
        channels[channel_id] = std::make_unique<virtual_otn_ocm_channel>(parent_ocm_id, channel_id);

        logger::debug(std::string(__func__) + ": Added OCM channel "
                      + std::to_string(channel_id) + " to OCM " + std::to_string(parent_ocm_id));

        return SAI_STATUS_SUCCESS;
    }

sai_status_t virtual_otn_ocm_device::remove_channel(sai_object_id_t channel_id)
{
    auto it = channels.find(channel_id);
    if (it == channels.end()) {
        logger::warn(std::string(__func__) + " channel not found: " + std::to_string(channel_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    channels.erase(it);
    logger::debug(std::string(__func__) + " removed channel " + std::to_string(channel_id));
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t virtual_otn_ocm_device::remove_all_channels()
{
    logger::warn(std::string(__func__) + ": removing all channels (" + std::to_string(channels.size()) + ")");
    channels.clear();
    logger::warn(std::string(__func__) + ": removed all channels");

    return SAI_STATUS_SUCCESS;
}

virtual_otn_ocm_channel* virtual_otn_ocm_device::get_channel(sai_object_id_t channel_id)
{
    logger::debug(std::string(__func__) + " channel_id " + std::to_string(channel_id));
    auto it = channels.find(channel_id);
    return (it != channels.end()) ? it->second.get() : nullptr;
}


/* WSS Device - spectrum power entry management */
sai_status_t virtual_otn_wss_device::add_spec_power(sai_object_id_t spec_power_id, sai_object_id_t parent_wss_id)
{
    if (spec_power_entries.find(spec_power_id) != spec_power_entries.end()) {
        logger::warn(std::string(__func__) + ": Spec power already exists: " + std::to_string(spec_power_id));
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }
    spec_power_entries[spec_power_id] =
        std::make_unique<virtual_otn_wss_spec_power_entry>(parent_wss_id, spec_power_id);
    logger::debug(std::string(__func__) + ": Added WSS spec power " + std::to_string(spec_power_id) +
                  " to WSS " + std::to_string(parent_wss_id));
    return SAI_STATUS_SUCCESS;
}

sai_status_t virtual_otn_wss_device::remove_spec_power(sai_object_id_t spec_power_id)
{
    auto it = spec_power_entries.find(spec_power_id);
    if (it == spec_power_entries.end()) {
        logger::warn(std::string(__func__) + ": Spec power not found: " + std::to_string(spec_power_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    spec_power_entries.erase(it);
    logger::debug(std::string(__func__) + ": Removed WSS spec power " + std::to_string(spec_power_id));
    return SAI_STATUS_SUCCESS;
}

virtual_otn_wss_spec_power_entry* virtual_otn_wss_device::get_spec_power(sai_object_id_t spec_power_id)
{
    auto it = spec_power_entries.find(spec_power_id);
    return (it != spec_power_entries.end()) ? it->second.get() : nullptr;
}

sai_status_t virtual_otn_wss_device::remove_all_spec_power()
{
    logger::debug(std::string(__func__) + ": removing all spec power entries (" +
                 std::to_string(spec_power_entries.size()) + ")");
    spec_power_entries.clear();
    return SAI_STATUS_SUCCESS;
}


/* OTDR Device */
std::string virtual_otn_otdr_device::get_module_name() const
{
    return dev_util::get_module_name(get_name());
}

std::string virtual_otn_otdr_device::sor_temp_path(const std::string& instance)
{
    return "/host/otn/otdr-sors/.otdr_result_" + instance + ".sor";
}

sai_otn_otdr_status_t virtual_otn_otdr_device::get_scanning_status() const
{
    bool active = otdr_scan_registry::instance().is_active(get_module_name(), get_sai_object_id());
    return active ? SAI_OTN_OTDR_STATUS_MEASURING : SAI_OTN_OTDR_STATUS_IDLE;
}

bool virtual_otn_otdr_device::cancel_scan()
{
    return otdr_scan_registry::instance().cancel(get_module_name(), get_sai_object_id());
}

sai_status_t virtual_otn_otdr_device::trigger_scan(
        sai_object_id_t otdr_id,
        sai_otn_otdr_scan_complete_notification_fn ntf_fn,
        std::string* busy_name)
{
    const std::string name = get_name();
    const std::string module = get_module_name();

    auto acquired = otdr_scan_registry::instance().acquire(module, otdr_id, name);
    if (acquired.status != SAI_STATUS_SUCCESS) {
        if (busy_name) {
            *busy_name = acquired.busy_name;
        }
        return acquired.status;
    }

    const uint32_t acq_time = get_acquisition_time_s();
    const uint64_t generation = acquired.generation;
    const std::string sor_path = sor_temp_path(name);
    logger::notice("virtual_otn_otdr_device::trigger_scan: starting scan thread, otdr_id=" +
                   std::to_string(otdr_id) + " module=" + module +
                   " acquisition_time=" + std::to_string(acq_time) + "s");

    // The thread captures values only: the device may be removed while it sleeps
    std::thread([otdr_id, generation, module, name, acq_time, sor_path, ntf_fn]() {
        logger::notice("virtual_otn_otdr_device scan thread: " + name + " sleeping " + std::to_string(acq_time) + "s");
        std::this_thread::sleep_for(std::chrono::seconds(acq_time));

        // The slot is released before the callback fires, so a following STATUS read sees IDLE
        if (!otdr_scan_registry::instance().complete(module, otdr_id, generation)) {
            logger::notice("virtual_otn_otdr_device scan thread: " + name + " scan cancelled or superseded, dropping completion");
            return;
        }

        FILE *fdst = fopen(sor_path.c_str(), "wb");
        if (fdst) {
            fclose(fdst);
            logger::notice("virtual_otn_otdr_device scan thread: empty SOR created at " + sor_path);
        } else {
            logger::warn("virtual_otn_otdr_device scan thread: cannot create SOR at " + sor_path);
        }

        // Synthetic scan result with 3 events
        sai_otn_otdr_event_t events[3];
        events[0].type           = SAI_OTN_OTDR_EVENT_TYPE_REFLECTION;
        events[0].distance_m     = 0;
        events[0].loss_db        = 50;      // 0.50 dB
        events[0].reflection_db  = -1400;   // -14.00 dB

        events[1].type           = SAI_OTN_OTDR_EVENT_TYPE_LOSS;
        events[1].distance_m     = 25000;
        events[1].loss_db        = 35;      // 0.35 dB
        events[1].reflection_db  = -1000;   // -10.00 dB

        events[2].type           = SAI_OTN_OTDR_EVENT_TYPE_END_OF_FIBER;
        events[2].distance_m     = 50000;
        events[2].loss_db        = 300;     // 3.00 dB
        events[2].reflection_db  = -1600;   // -16.00 dB

        sai_otn_otdr_scan_complete_data_t result;
        result.otdr_id         = otdr_id;
        result.total_length_m  = 50000;
        result.total_loss_db   = 1000;   // 10.00 dB
        result.orl_db          = -3000;  // -30.00 dB
        result.average_loss_db = 20;     // 0.20 dB/km
        result.event_count     = 3;
        result.events          = events;

        if (ntf_fn) {
            logger::notice("virtual_otn_otdr_device scan thread: firing scan-complete callback, otdr_id=" +
                           std::to_string(otdr_id));
            ntf_fn(1, &result);
            logger::notice("virtual_otn_otdr_device scan thread: callback returned");
        } else {
            logger::warn("virtual_otn_otdr_device scan thread: no callback registered, dropping notification");
        }
    }).detach();

    return SAI_STATUS_SUCCESS;
}

