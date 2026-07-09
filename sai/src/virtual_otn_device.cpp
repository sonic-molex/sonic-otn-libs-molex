#include "virtual_otn_device.h"
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <chrono>
#include <climits>
#include "logger.h"

sai_status_t virtual_otn_device::set_threshold_ranges(uint32_t device_type, std::unordered_map<std::string, otn_threshold_range_t>& threshold_ranges)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;

    for (auto& item : threshold_ranges) {
        const std::string key = item.first;
        const otn_threshold_range_t& range = item.second;
        
        // TODO: Based on different types of devices, threshold ranges are set differently.
        // Function signature example is: int EDFA_SetOscRxThrHys(uint32_t uiDevType, int16_t sThr, int16_t sHys);
        logger::notice(std::string(__func__) + " setting threshold range for " + key + " to " + std::to_string(range.low) + " - " + std::to_string(range.high));
    }
    return rc;
}

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
void virtual_otn_otdr_device::trigger_scan(
        sai_object_id_t otdr_id,
        sai_otn_otdr_scan_complete_notification_fn ntf_fn)
{
    if (get_scanning_status() == SAI_OTN_OTDR_STATUS_MEASURING) {
        logger::warn("virtual_otn_otdr_device::trigger_scan: scan already in progress, ignoring");
        return;
    }

    uint32_t acq_time = get_acquisition_time_s();
    logger::notice("virtual_otn_otdr_device::trigger_scan: starting scan thread, otdr_id=" +
                   std::to_string(otdr_id) + " acquisition_time=" + std::to_string(acq_time) + "s");

    set_scanning_status(SAI_OTN_OTDR_STATUS_MEASURING);

    std::thread([this, otdr_id, acq_time, ntf_fn]() {
        logger::notice("virtual_otn_otdr_device scan thread: sleeping " + std::to_string(acq_time) + "s");
        std::this_thread::sleep_for(std::chrono::seconds(acq_time));
        logger::notice("virtual_otn_otdr_device scan thread: awoke, preparing SOR file and result");

        const char *sorSrc = "/host/otn/otdr-sors/.otdr_result.sor";
        FILE *fdst = fopen(sorSrc, "wb");
        if (fdst) {
            fclose(fdst);
            logger::notice(std::string("virtual_otn_otdr_device scan thread: empty SOR created at ") + sorSrc);
        } else {
            logger::warn(std::string("virtual_otn_otdr_device scan thread: cannot create SOR at ") + sorSrc);
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

        set_scanning_status(SAI_OTN_OTDR_STATUS_IDLE);
        logger::notice("virtual_otn_otdr_device scan thread: status set to IDLE");

        if (ntf_fn) {
            logger::notice("virtual_otn_otdr_device scan thread: firing scan-complete callback, otdr_id=" +
                           std::to_string(otdr_id));
            ntf_fn(1, &result);
            logger::notice("virtual_otn_otdr_device scan thread: callback returned");
        } else {
            logger::warn("virtual_otn_otdr_device scan thread: no callback registered, dropping notification");
        }
    }).detach();
}

