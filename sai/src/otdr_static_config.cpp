#include "otdr_static_config.h"
#include "logger.h"

#include <sstream>

namespace {
const char* scan_type_to_str(sai_otn_otdr_scan_type_t t)
{
    switch (t) {
    case SAI_OTN_OTDR_SCAN_TYPE_SHORT:  return "short";
    case SAI_OTN_OTDR_SCAN_TYPE_MEDIUM: return "medium";
    case SAI_OTN_OTDR_SCAN_TYPE_LONG:   return "long";
    case SAI_OTN_OTDR_SCAN_TYPE_AUTO:   return "auto";
    case SAI_OTN_OTDR_SCAN_TYPE_CUSTOM: return "custom";
    default:                            return "unknown";
    }
}

const char* fiber_type_to_str(sai_otn_otdr_fiber_type_t t)
{
    switch (t) {
    case SAI_OTN_OTDR_FIBER_TYPE_DSF:  return "DSF";
    case SAI_OTN_OTDR_FIBER_TYPE_LEAF: return "LEAF";
    case SAI_OTN_OTDR_FIBER_TYPE_SSMF: return "SSMF";
    case SAI_OTN_OTDR_FIBER_TYPE_TWC:  return "TWC";
    case SAI_OTN_OTDR_FIBER_TYPE_TWRS: return "TWRS";
    default:                           return "unknown";
    }
}
} // namespace

// ---------- otdr_port_config_table ----------

otdr_port_config_table& otdr_port_config_table::instance()
{
    static otdr_port_config_table inst;
    return inst;
}

otdr_scan_params_t& otdr_port_config_table::get_or_create(const std::string& port)
{
    auto it = table_.find(port);
    if (it == table_.end()) {
        logger::notice("otdr_port_config_table: create entry port=" + port);
        it = table_.emplace(port, otdr_scan_params_t{}).first;
    }
    return it->second;
}

const otdr_scan_params_t* otdr_port_config_table::get(const std::string& port) const
{
    auto it = table_.find(port);
    return (it == table_.end()) ? nullptr : &it->second;
}

std::string otdr_port_config_table::dump(const std::string& port) const
{
    std::ostringstream oss;
    oss << "otdr_port_config_table dump for port=" << port << ":";
    auto it = table_.find(port);
    if (it == table_.end()) {
        oss << " (no entry)";
    } else {
        const auto& p = it->second;
        oss << "\n  acquisition_time_s=" << p.acquisition_time_s
            << " range_m=" << p.range_m
            << " pulse_width_ns=" << p.pulse_width_ns
            << " wavelength_mhz=" << p.wavelength_mhz
            << " sampling_resolution_m=" << p.sampling_resolution_m
            << " fiber_type=" << fiber_type_to_str(p.fiber_type)
            << " negotiation=" << (p.negotiation ? "true" : "false")
            << " refractive_index=" << p.refractive_index
            << " backscatter_index=" << p.backscatter_index
            << " reflectance_threshold=" << p.reflectance_threshold
            << " splice_loss_threshold=" << p.splice_loss_threshold
            << " fiber_end_threshold=" << p.fiber_end_threshold;
    }
    return oss.str();
}

// ---------- otdr_scan_type_config_table ----------

otdr_scan_type_config_table& otdr_scan_type_config_table::instance()
{
    static otdr_scan_type_config_table inst;
    return inst;
}

otdr_scan_type_params_t& otdr_scan_type_config_table::get_or_create(sai_otn_otdr_scan_type_t scan_type)
{
    auto it = table_.find(scan_type);
    if (it == table_.end()) {
        logger::notice(std::string("otdr_scan_type_config_table: create entry scan_type=")
                       + scan_type_to_str(scan_type));
        it = table_.emplace(scan_type, otdr_scan_type_params_t{}).first;
    }
    return it->second;
}

const otdr_scan_type_params_t* otdr_scan_type_config_table::get(sai_otn_otdr_scan_type_t scan_type) const
{
    auto it = table_.find(scan_type);
    return (it == table_.end()) ? nullptr : &it->second;
}

std::string otdr_scan_type_config_table::dump() const
{
    std::ostringstream oss;
    oss << "otdr_scan_type_config_table dump:";
    if (table_.empty()) {
        oss << " (empty)";
    }
    for (const auto& kv : table_) {
        const auto& p = kv.second;
        oss << "\n  [" << scan_type_to_str(kv.first) << "]"
            << " acquisition_time_s=" << p.acquisition_time_s
            << " range_m=" << p.range_m
            << " pulse_width_ns=" << p.pulse_width_ns
            << " wavelength_mhz=" << p.wavelength_mhz
            << " sampling_resolution_m=" << p.sampling_resolution_m
            << " fiber_type=" << fiber_type_to_str(p.fiber_type)
            << " negotiation=" << (p.negotiation ? "true" : "false");
    }
    return oss.str();
}

// ---------- otdr_fiber_profile_config_table ----------

otdr_fiber_profile_config_table& otdr_fiber_profile_config_table::instance()
{
    static otdr_fiber_profile_config_table inst;
    return inst;
}

otdr_fiber_profile_params_t& otdr_fiber_profile_config_table::get_or_create(sai_otn_otdr_fiber_type_t fiber_type)
{
    auto it = table_.find(fiber_type);
    if (it == table_.end()) {
        logger::notice(std::string("otdr_fiber_profile_config_table: create entry fiber_type=")
                       + fiber_type_to_str(fiber_type));
        it = table_.emplace(fiber_type, otdr_fiber_profile_params_t{}).first;
    }
    return it->second;
}

const otdr_fiber_profile_params_t* otdr_fiber_profile_config_table::get(sai_otn_otdr_fiber_type_t fiber_type) const
{
    auto it = table_.find(fiber_type);
    return (it == table_.end()) ? nullptr : &it->second;
}

std::string otdr_fiber_profile_config_table::dump() const
{
    std::ostringstream oss;
    oss << "otdr_fiber_profile_config_table dump:";
    if (table_.empty()) {
        oss << " (empty)";
    }
    for (const auto& kv : table_) {
        const auto& p = kv.second;
        oss << "\n  [" << fiber_type_to_str(kv.first) << "]"
            << " refractive_index=" << p.refractive_index
            << " backscatter_index=" << p.backscatter_index
            << " reflectance_threshold=" << p.reflectance_threshold
            << " splice_loss_threshold=" << p.splice_loss_threshold
            << " fiber_end_threshold=" << p.fiber_end_threshold;
    }
    return oss.str();
}
