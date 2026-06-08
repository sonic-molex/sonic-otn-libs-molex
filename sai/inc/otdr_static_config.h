#pragma once

#include <map>
#include <string>
#include <saiextensions.h>

// ---------- Per-port customised scan params (CUSTOM scan type) ----------

struct otdr_scan_params_t
{
    sai_uint32_t acquisition_time_s     = 60;
    sai_uint32_t range_m                = 60000;
    sai_uint32_t pulse_width_ns         = 3000;
    sai_uint64_t wavelength_mhz         = 193414489ULL;
    sai_uint64_t sampling_resolution_m  = 1000;   // x100, i.e. 10.00 m
    sai_otn_otdr_fiber_type_t fiber_type = SAI_OTN_OTDR_FIBER_TYPE_SSMF;
    bool         negotiation            = true;
    sai_int32_t  refractive_index       = 1467900;  // x1e6
    sai_int32_t  backscatter_index      = -8200;    // x100
    sai_int32_t  reflectance_threshold  = -4000;    // x100
    sai_int32_t  splice_loss_threshold  = 35;       // x100
    sai_int32_t  fiber_end_threshold    = 300;      // x100
};

// Keyed by port name.  Populated when SAI_OTN_OTDR_ATTR_* scan/fiber params
// are set on a per-port OTDR object.  Consulted at trigger time when
// scan_type == CUSTOM.
class otdr_port_config_table
{
public:
    static otdr_port_config_table& instance();

    otdr_scan_params_t& get_or_create(const std::string& port);
    const otdr_scan_params_t* get(const std::string& port) const;
    std::string dump(const std::string& port) const;

private:
    otdr_port_config_table() = default;
    otdr_port_config_table(const otdr_port_config_table&) = delete;
    otdr_port_config_table& operator=(const otdr_port_config_table&) = delete;

    std::map<std::string, otdr_scan_params_t> table_;
};


// ---------- Common scan-type config (SHORT / MEDIUM / LONG) ----------

struct otdr_scan_type_params_t
{
    sai_uint32_t acquisition_time_s     = 60;
    sai_uint32_t range_m                = 50000;
    sai_uint32_t pulse_width_ns         = 3000;
    sai_uint64_t wavelength_mhz         = 200000000ULL;
    sai_uint64_t sampling_resolution_m  = 1000;   // x100
    sai_otn_otdr_fiber_type_t fiber_type = SAI_OTN_OTDR_FIBER_TYPE_SSMF;
    bool         negotiation            = true;
};

// Keyed by scan type.  Populated via create/set_otn_otdr_scan_type.
// Consulted at trigger time for SHORT / MEDIUM / LONG scans.
class otdr_scan_type_config_table
{
public:
    static otdr_scan_type_config_table& instance();

    otdr_scan_type_params_t& get_or_create(sai_otn_otdr_scan_type_t scan_type);
    const otdr_scan_type_params_t* get(sai_otn_otdr_scan_type_t scan_type) const;
    std::string dump() const;

private:
    otdr_scan_type_config_table() = default;
    otdr_scan_type_config_table(const otdr_scan_type_config_table&) = delete;
    otdr_scan_type_config_table& operator=(const otdr_scan_type_config_table&) = delete;

    std::map<sai_otn_otdr_scan_type_t, otdr_scan_type_params_t> table_;
};


// ---------- Fiber-profile config ----------

struct otdr_fiber_profile_params_t
{
    sai_int32_t  refractive_index       = 1467900;  // x1e6
    sai_int32_t  backscatter_index      = -8200;    // x100
    sai_int32_t  reflectance_threshold  = -4000;    // x100
    sai_int32_t  splice_loss_threshold  = 35;       // x100
    sai_int32_t  fiber_end_threshold    = 300;      // x100
};

// Keyed by fiber type.  Populated via create/set_otn_otdr_fiber_profile.
// Consulted at trigger time chained from otdr_scan_type_params_t::fiber_type.
class otdr_fiber_profile_config_table
{
public:
    static otdr_fiber_profile_config_table& instance();

    otdr_fiber_profile_params_t& get_or_create(sai_otn_otdr_fiber_type_t fiber_type);
    const otdr_fiber_profile_params_t* get(sai_otn_otdr_fiber_type_t fiber_type) const;
    std::string dump() const;

private:
    otdr_fiber_profile_config_table() = default;
    otdr_fiber_profile_config_table(const otdr_fiber_profile_config_table&) = delete;
    otdr_fiber_profile_config_table& operator=(const otdr_fiber_profile_config_table&) = delete;

    std::map<sai_otn_otdr_fiber_type_t, otdr_fiber_profile_params_t> table_;
};
