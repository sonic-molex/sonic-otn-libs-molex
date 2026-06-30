#pragma once

#include <string>
#include <memory>
#include <set>
#include <unordered_map>
#include <saiextensions.h>
#include "logger.h"

/* Base class for Virtual OTN device */
class virtual_otn_device {
public:
    virtual_otn_device(sai_object_id_t sai_object_id, sai_object_type_extensions_t sai_object_type)
        : sai_object_id(sai_object_id),
          enabled(false),
          signal_present(false),
          en_sai_object_type(sai_object_type)
    {
        logger::debug(std::string(__func__) + ", sai_id " + std::to_string(sai_object_id) +
        ", sai_type " + std::to_string(sai_object_type));
    }

    void enable() { enabled = true; }
    void disable() { enabled = false; }
    void inject_signal() { signal_present = true; }
    void remove_signal() { signal_present = false; }
    bool is_signal_present() const { return signal_present; }

    void set_name(const std::string& name) { attr_name = name; }
    const std::string& get_name() const { return attr_name; }

    /* Active-alarm tracking for RAISE/CLEAR edge detection.
     * HAL/device alarm conditions are level/state (active or not); eventd wants
     * edges. These let a trigger point emit exactly one RAISE on 0->1 and one
     * CLEAR on 1->0 instead of re-raising on every config write. */
    bool raise_alarm(const std::string& name) { return active_alarms.insert(name).second; }
    bool clear_alarm(const std::string& name) { return active_alarms.erase(name) > 0; }
    bool is_alarm_active(const std::string& name) const { return active_alarms.count(name) > 0; }

    virtual ~virtual_otn_device() {}

private:
    sai_object_id_t sai_object_id;
    bool enabled;
    bool signal_present;
    sai_object_type_extensions_t  en_sai_object_type;
    std::string attr_name;
    std::set<std::string> active_alarms;

};

// ---------------- Derived class ----------------
class virtual_otn_oa_device : public virtual_otn_device {
public:
    // Constructor
    virtual_otn_oa_device(sai_object_id_t id, sai_object_type_extensions_t type)
        : virtual_otn_device(id, type),
          attr_type(SAI_OTN_OA_TYPE_EDFA),
          attr_target_gain(2000),
          attr_min_gain(500),
          attr_max_gain(3500),
          attr_target_gain_tilt(0),
          attr_gain_range(SAI_OTN_OA_GAIN_RANGE_LOW_GAIN_RANGE),
          attr_amp_mode(SAI_OTN_OA_AMP_MODE_CONSTANT_GAIN),
          attr_target_output_power(800),
          attr_max_output_power(2500),
          attr_enabled(false),
          attr_fiber_type_profile(SAI_OTN_OA_FIBER_TYPE_PROFILE_SSMF)
    {
        logger::debug(std::string(__func__) + ", OA Device created with ID " + std::to_string(id));
    }
    
    virtual ~virtual_otn_oa_device() {} 
    
    // Setters
    void set_type(sai_otn_oa_type_t type) { attr_type = type; }
    bool set_target_gain(sai_uint32_t gain)
    {
        if (gain < attr_min_gain || gain > attr_max_gain) {
            return false; // Invalid gain value
        }
        attr_target_gain = gain;
        return true;
    }
    void set_min_gain(sai_uint32_t gain) { attr_min_gain = gain; }
    void set_max_gain(sai_uint32_t gain) { attr_max_gain = gain; }
    void set_target_gain_tilt(sai_int32_t tilt) { attr_target_gain_tilt = tilt; }
    void set_gain_range(sai_otn_oa_gain_range_t range) { attr_gain_range = range; }
    void set_amp_mode(sai_otn_oa_amp_mode_t mode) { attr_amp_mode = mode; }
    void set_target_output_power(sai_int32_t power) { attr_target_output_power = power; }
    void set_max_output_power(sai_int32_t power) { attr_max_output_power = power; }
    void set_enabled(bool en) { attr_enabled = en; }
    void set_fiber_type_profile(sai_otn_oa_fiber_type_profile_t profile) { attr_fiber_type_profile = profile; }
    void set_ingress_port(const std::string& name) { ingress_port = name; }
    void set_egress_port(const std::string& name) { egress_port = name; }
    // Getters
    sai_otn_oa_type_t get_type() const { return attr_type; }
    sai_uint32_t get_target_gain() const { return attr_target_gain; }
    sai_uint32_t get_min_gain() const { return attr_min_gain; }
    sai_uint32_t get_max_gain() const { return attr_max_gain; }
    sai_int32_t get_target_gain_tilt() const { return attr_target_gain_tilt; }
    sai_otn_oa_gain_range_t get_gain_range() const { return attr_gain_range; }
    sai_otn_oa_amp_mode_t get_amp_mode() const { return attr_amp_mode; }
    sai_int32_t get_target_output_power() const { return attr_target_output_power; }
    sai_int32_t get_max_output_power() const { return attr_max_output_power; }
    bool is_enabled() const { return attr_enabled; }
    sai_otn_oa_fiber_type_profile_t get_fiber_type_profile() const { return attr_fiber_type_profile; }
    const std::string& get_ingress_port() const { return ingress_port; }
    const std::string& get_egress_port() const { return egress_port; }
    sai_uint32_t get_actual_gain() const 
    { 
        // return the target gain for simulation purpose    
        return attr_target_gain; 
    }
    
    sai_uint32_t get_actual_gain_tilt() const 
    { 
        // return the target gain tilt for simulation purpose    
        return attr_target_gain_tilt; 
    }
private:
    // Configurable attributes
    sai_otn_oa_type_t attr_type;
    sai_uint32_t attr_target_gain;
    sai_uint32_t attr_min_gain;
    sai_uint32_t attr_max_gain;
    sai_int32_t  attr_target_gain_tilt;
    sai_otn_oa_gain_range_t attr_gain_range;
    sai_otn_oa_amp_mode_t attr_amp_mode;
    sai_int32_t  attr_target_output_power;
    sai_int32_t  attr_max_output_power;
    bool         attr_enabled;
    sai_otn_oa_fiber_type_profile_t attr_fiber_type_profile;
    std::string ingress_port;
    std::string egress_port;
};



// ===================== VOA Device =====================
class virtual_otn_voa_device : public virtual_otn_device {
public:
    virtual_otn_voa_device(sai_object_id_t id,sai_object_type_extensions_t type)
        : virtual_otn_device(id, type),
          attenuation_mode(SAI_OTN_ATTENUATOR_MODE_CONSTANT_ATTENUATION),
          target_output_power(0),
          attenuation(500),
          enabled(true),
          max_output_power(0),
          max_output_power_threshold(300),
          actual_attenuation(0),
          output_power_total(0),
          optical_return_loss(0)
    {
        logger::debug(std::string(__func__) + ", VOA Device created with ID " + std::to_string(id));
    }

    virtual ~virtual_otn_voa_device() {}
 
    // Setters
    void set_attn_mode(sai_otn_attenuator_mode_t mode) 
    { 
        attenuation_mode = mode;
        logger::debug(std::string(__func__) + ", VOA sai_otn_attenuator_mode_t " + std::to_string(mode)); 
    }
    void set_target_power(sai_int32_t power) { target_output_power = power; }
    
    void set_attenuation(sai_uint32_t value) 
    { 
        attenuation = value; 
        logger::debug(std::string(__func__) + ", VOA set_attenuation " + std::to_string(value));
    }
    void set_enabled(bool en) { enabled = en; }
    void set_max_output_power(sai_int32_t power) { max_output_power = power; }
    void set_max_output_power_threshold(sai_int32_t thresh) { max_output_power_threshold = thresh; }
    void set_ingress_port(const std::string& name) { ingress_port = name; }
    void set_egress_port(const std::string& name) { egress_port = name; }
    // Getters
    sai_otn_attenuator_mode_t get_attn_mode() const { return attenuation_mode; }
    sai_int32_t get_target_power() const { return target_output_power; }
    sai_uint32_t get_attenuation() const { return attenuation; }
    bool get_enabled() const { return enabled;  }
    sai_int32_t get_max_output_power() const { return max_output_power; }
    sai_int32_t get_max_output_power_threshold() const { return max_output_power_threshold; }
    const std::string& get_ingress_port() const { return ingress_port; }
    const std::string& get_egress_port() const { return egress_port; }
    private:
    // Attributes
    sai_otn_attenuator_mode_t attenuation_mode;
    sai_int32_t target_output_power;
    sai_uint32_t attenuation;
    bool enabled;
    sai_int32_t max_output_power;
    sai_int32_t max_output_power_threshold;
    std::string ingress_port;
    std::string egress_port;
    sai_int32_t actual_attenuation;
    sai_int32_t output_power_total;
    sai_int32_t optical_return_loss;
};



// ===================== OCM Channel Class =====================
class virtual_otn_ocm_channel {
public:
    // Constructor
    virtual_otn_ocm_channel( sai_object_id_t sai_ocm_id, uint32_t ch_id)
        : parent_ocm_id(sai_ocm_id), 
          channel_id(ch_id), 
          channel_name(""),
          lower_frequency(0),
          upper_frequency(0),
          power(0),              // READ_ONLY - measured value
          target_power(0)        // READ_ONLY - target value
    {
        logger::debug(std::string(__func__) + ", OCM Channel created: channel id " 
        + std::to_string(ch_id) + ", sai_id = "+std::to_string(sai_ocm_id));
    }
    
    ~virtual_otn_ocm_channel() {}
    
    // Getters (all attributes are CREATE_ONLY or READ_ONLY)
    sai_object_id_t get_channel_id() const { return channel_id; }
    const std::string& get_name() const { return channel_name; }
    sai_uint64_t get_lower_frequency() const { return lower_frequency; }
    sai_uint64_t get_upper_frequency() const { return upper_frequency; }
    sai_int32_t get_power() const { return power; }
    sai_int32_t get_target_power() const { return target_power; }
    
    // Setters
    void set_channel_name(const std::string& name){ channel_name = name; }
    void set_lower_frequency(sai_uint64_t freq) { lower_frequency = freq; }
    void set_upper_frequency(sai_uint64_t freq) { upper_frequency = freq; }

    // Setters for READ_ONLY attributes (internal use - simulating measurements)
    void set_power(sai_int32_t pwr) { power = pwr; }
    void set_target_power(sai_int32_t tgt_pwr) { target_power = tgt_pwr; }

private:
    // CREATE_ONLY attributes
    sai_object_id_t parent_ocm_id;
    uint32_t channel_id;
    std::string channel_name;
    sai_uint64_t lower_frequency;    // MHz
    sai_uint64_t upper_frequency;    // MHz
    
    // READ_ONLY attributes (measured/computed values)
    sai_int32_t power;               // 0.01 dBm units
    sai_int32_t target_power;        // 0.01 dBm units
};


// ===================== OCM Device =====================
class virtual_otn_ocm_device : public virtual_otn_device {
public:
    virtual_otn_ocm_device(sai_object_id_t id, sai_object_type_extensions_t type)
        : virtual_otn_device(id, type),
          monitor_port("")
    {
        logger::debug(std::string(__func__) + ", OCM Device created with ID " + std::to_string(id));
    }
    
    virtual ~virtual_otn_ocm_device() {}
    
    // OCM device attribute setters/getters
    void set_monitor_port(const std::string& port) { monitor_port = port; }
    const std::string& get_monitor_port() const { return monitor_port; }
    
    // Channel management
    sai_status_t add_channel(sai_object_id_t ocm_id, sai_object_id_t ch_id);

    sai_status_t remove_channel(sai_object_id_t channel_id);   
    virtual_otn_ocm_channel* get_channel(sai_object_id_t channel_id);
    sai_status_t remove_all_channels();  
    const std::unordered_map<sai_object_id_t, std::unique_ptr<virtual_otn_ocm_channel>> & 
    get_all_channels() const 
    {
        return channels;
    }
    
    size_t get_channel_count() const { return channels.size(); }

private:
    // OCM device attributes
    std::string monitor_port;  // SAI_OTN_OCM_ATTR_MONITOR_PORT
    
    // Channel collection
    std::unordered_map<sai_object_id_t, std::unique_ptr<virtual_otn_ocm_channel>> channels;
};


class virtual_otn_osc_device : public virtual_otn_device
{
public:
    virtual_otn_osc_device(sai_object_id_t sai_object_id, sai_object_type_extensions_t type)
        : virtual_otn_device(sai_object_id, type),
          output_power(0.0),
          input_power(0.0),
          output_frequency(0.0)
    {
        logger::debug(std::string(__func__) + " created OSC device id " + std::to_string(sai_object_id));
    }
    
    virtual ~virtual_otn_osc_device() {}

    // Default frequency constant 
    static constexpr sai_uint64_t DEFAULT_OUTPUT_FREQ_KHZ = 198538052; // in kHz

    // === Attribute setters only for simulation ===
    void set_input_power(sai_int32_t power)
    {
        input_power = power;
        logger::debug(std::string(__func__) + " input power set to " + std::to_string(input_power));
    }

    void set_output_frequency(sai_uint64_t freq)
    {
        output_frequency = freq;
        logger::debug(std::string(__func__) + " Output frequency set to " + std::to_string(output_frequency));
    }

    // === Attribute getters ===
    sai_int32_t get_output_power() const { return output_power; }
    sai_int32_t get_input_power() const { return input_power; }
    sai_uint64_t get_output_frequency() const { return output_frequency; }
 
    // === Simulate behavior ===
    void simulate_signal_path()
    {
        if (!is_signal_present())
        {
            logger::warn(std::string(__func__) + ": No input signal on OSC!");
            return;
        }
        // Simple simulation logic — input power affects output power proportionally
        output_power = input_power * 0.95;
        logger::debug(std::string(__func__) + " simulated output power=" + std::to_string(output_power));
    }

private:
    sai_int32_t output_power;           // dBm
    sai_int32_t input_power;            // dBm
    sai_uint64_t output_frequency;      // MHz
 
};


// ===================== WSS Spectrum Power Entry (per frequency slot) =====================
class virtual_otn_wss_spec_power_entry {
public:
    virtual_otn_wss_spec_power_entry(sai_object_id_t parent_wss_id, sai_object_id_t spec_power_id)
        : parent_wss_id(parent_wss_id),
          sai_spec_power_id(spec_power_id),
          lower_frequency(0),
          upper_frequency(0),
          target_power(0),
          attenuation(0),
          actual_attenuation(0)
    {
        logger::debug(std::string(__func__) + ", WSS " + std::to_string(parent_wss_id) +
                      ", spec_power id " + std::to_string(spec_power_id));
    }

    ~virtual_otn_wss_spec_power_entry() {}

    sai_object_id_t get_parent_wss_id() const { return parent_wss_id; }
    sai_object_id_t get_spec_power_id() const { return sai_spec_power_id; }
    sai_uint64_t get_lower_frequency() const { return lower_frequency; }
    sai_uint64_t get_upper_frequency() const { return upper_frequency; }
    sai_int32_t get_target_power() const { return target_power; }
    sai_int32_t get_attenuation() const { return attenuation; }
    sai_int32_t get_actual_attenuation() const { return actual_attenuation; }

    void set_lower_frequency(sai_uint64_t freq) { lower_frequency = freq; }
    void set_upper_frequency(sai_uint64_t freq) { upper_frequency = freq; }
    void set_target_power(sai_int32_t power) { target_power = power; }
    void set_attenuation(sai_int32_t attn) { attenuation = attn; actual_attenuation = attn; }
    void set_actual_attenuation(sai_int32_t attn) { actual_attenuation = attn; }

private:
    sai_object_id_t parent_wss_id;
    sai_object_id_t sai_spec_power_id;
    sai_uint64_t lower_frequency;
    sai_uint64_t upper_frequency;
    sai_int32_t target_power;
    sai_int32_t attenuation;
    sai_int32_t actual_attenuation;
};


// ===================== WSS Device (media channel) =====================
class virtual_otn_wss_device : public virtual_otn_device {
public:
    virtual_otn_wss_device(sai_object_id_t id, sai_object_type_extensions_t type)
        : virtual_otn_device(id, type),
          attr_index(0),
          attr_lower_frequency(0),
          attr_upper_frequency(0),
          attr_admin_state(SAI_OTN_WSS_ADMIN_STATE_ENABLED),
          attr_super_channel(false),
          attr_super_channel_parent(0),
          attr_ase_control_mode(SAI_OTN_WSS_ASE_CONTROL_MODE_AUTO_ASE_FAILURE_AND_RESTORE),
          attr_ase_injection_mode(SAI_OTN_WSS_ASE_INJECTION_MODE_THRESHOLD),
          attr_ase_injection_threshold(0),
          attr_ase_injection_delta(0),
          attr_media_channel_injection_offset(0),
          attr_attenuation_control_mode(SAI_OTN_WSS_ATTENUATION_CONTROL_MODE_ATTENUATION_SET_ATTENUATION),
          attr_attenuation_control_range(SAI_OTN_WSS_ATTENUATION_CONTROL_RANGE_CONTROL_RANGE_FULL),
          attr_max_undershoot_compensation(0),
          attr_max_overshoot_compensation(0),
          attr_oper_status(SAI_OTN_WSS_OPER_STATUS_UP),
          attr_ase_status(SAI_OTN_WSS_ASE_STATUS_NOT_PRESENT)
    {
        logger::debug(std::string(__func__) + ", WSS Device created with ID " + std::to_string(id));
    }

    virtual ~virtual_otn_wss_device() {}

    // WSS media channel attribute setters
    void set_index(sai_uint32_t idx) { attr_index = idx; }
    void set_lower_frequency(sai_uint64_t freq) { attr_lower_frequency = freq; }
    void set_upper_frequency(sai_uint64_t freq) { attr_upper_frequency = freq; }
    void set_admin_state(sai_otn_wss_admin_state_t state) { attr_admin_state = state; }
    void set_super_channel(bool super) { attr_super_channel = super; }
    void set_super_channel_parent(sai_uint32_t parent) { attr_super_channel_parent = parent; }
    void set_ase_control_mode(sai_otn_wss_ase_control_mode_t mode) { attr_ase_control_mode = mode; }
    void set_ase_injection_mode(sai_otn_wss_ase_injection_mode_t mode) { attr_ase_injection_mode = mode; }
    void set_ase_injection_threshold(sai_int32_t val) { attr_ase_injection_threshold = val; }
    void set_ase_injection_delta(sai_int32_t val) { attr_ase_injection_delta = val; }
    void set_media_channel_injection_offset(sai_int32_t val) { attr_media_channel_injection_offset = val; }
    void set_attenuation_control_mode(sai_otn_wss_attenuation_control_mode_t mode) { attr_attenuation_control_mode = mode; }
    void set_attenuation_control_range(sai_otn_wss_attenuation_control_range_t range) { attr_attenuation_control_range = range; }
    void set_max_undershoot_compensation(sai_int32_t val) { attr_max_undershoot_compensation = val; }
    void set_max_overshoot_compensation(sai_int32_t val) { attr_max_overshoot_compensation = val; }
    void set_source_port_name(const std::string& name) { attr_source_port_name = name; }
    void set_dest_port_name(const std::string& name) { attr_dest_port_name = name; }
    void set_oper_status(sai_otn_wss_oper_status_t status) { attr_oper_status = status; }
    void set_ase_status(sai_otn_wss_ase_status_t status) { attr_ase_status = status; }

    // WSS media channel attribute getters
    sai_uint32_t get_index() const { return attr_index; }
    sai_uint64_t get_lower_frequency() const { return attr_lower_frequency; }
    sai_uint64_t get_upper_frequency() const { return attr_upper_frequency; }
    sai_otn_wss_admin_state_t get_admin_state() const { return attr_admin_state; }
    bool get_super_channel() const { return attr_super_channel; }
    sai_uint32_t get_super_channel_parent() const { return attr_super_channel_parent; }
    sai_otn_wss_ase_control_mode_t get_ase_control_mode() const { return attr_ase_control_mode; }
    sai_otn_wss_ase_injection_mode_t get_ase_injection_mode() const { return attr_ase_injection_mode; }
    sai_int32_t get_ase_injection_threshold() const { return attr_ase_injection_threshold; }
    sai_int32_t get_ase_injection_delta() const { return attr_ase_injection_delta; }
    sai_int32_t get_media_channel_injection_offset() const { return attr_media_channel_injection_offset; }
    sai_otn_wss_attenuation_control_mode_t get_attenuation_control_mode() const { return attr_attenuation_control_mode; }
    sai_otn_wss_attenuation_control_range_t get_attenuation_control_range() const { return attr_attenuation_control_range; }
    sai_int32_t get_max_undershoot_compensation() const { return attr_max_undershoot_compensation; }
    sai_int32_t get_max_overshoot_compensation() const { return attr_max_overshoot_compensation; }
    const std::string& get_source_port_name() const { return attr_source_port_name; }
    const std::string& get_dest_port_name() const { return attr_dest_port_name; }
    sai_otn_wss_oper_status_t get_oper_status() const { return attr_oper_status; }
    sai_otn_wss_ase_status_t get_ase_status() const { return attr_ase_status; }

    // Spectrum power entry management (child of WSS)
    sai_status_t add_spec_power(sai_object_id_t spec_power_id, sai_object_id_t wss_id);
    sai_status_t remove_spec_power(sai_object_id_t spec_power_id);
    virtual_otn_wss_spec_power_entry* get_spec_power(sai_object_id_t spec_power_id);
    sai_status_t remove_all_spec_power();
    size_t get_spec_power_count() const { return spec_power_entries.size(); }

private:
    sai_uint32_t attr_index;
    sai_uint64_t attr_lower_frequency;
    sai_uint64_t attr_upper_frequency;
    sai_otn_wss_admin_state_t attr_admin_state;
    bool attr_super_channel;
    sai_uint32_t attr_super_channel_parent;
    sai_otn_wss_ase_control_mode_t attr_ase_control_mode;
    sai_otn_wss_ase_injection_mode_t attr_ase_injection_mode;
    sai_int32_t attr_ase_injection_threshold;
    sai_int32_t attr_ase_injection_delta;
    sai_int32_t attr_media_channel_injection_offset;
    sai_otn_wss_attenuation_control_mode_t attr_attenuation_control_mode;
    sai_otn_wss_attenuation_control_range_t attr_attenuation_control_range;
    sai_int32_t attr_max_undershoot_compensation;
    sai_int32_t attr_max_overshoot_compensation;
    std::string attr_source_port_name;
    std::string attr_dest_port_name;
    sai_otn_wss_oper_status_t attr_oper_status;
    sai_otn_wss_ase_status_t attr_ase_status;
    std::unordered_map<sai_object_id_t, std::unique_ptr<virtual_otn_wss_spec_power_entry>> spec_power_entries;
};


// ===================== OTDR Device =====================
class virtual_otn_otdr_device : public virtual_otn_device {
public:
    virtual_otn_otdr_device(sai_object_id_t id, sai_object_type_extensions_t type)
        : virtual_otn_device(id, type),
          parent_port(""),
          range_m(60),
          pulse_width_ns(3000),
          acquisition_time_s(60),
          wavelength_mhz(193414489ULL),
          sampling_resolution_m(1000),
          fiber_type(SAI_OTN_OTDR_FIBER_TYPE_SSMF),
          scan_type(SAI_OTN_OTDR_SCAN_TYPE_CUSTOM),
          negotiation(true),
          refractive_index(1467900),
          backscatter_index(-8100),
          reflectance_threshold(-4000),
          splice_loss_threshold(35),
          fiber_end_threshold(300),
          scanning_status(SAI_OTN_OTDR_STATUS_IDLE)
    {
        logger::debug(std::string(__func__) + ", OTDR Device created with ID " + std::to_string(id));
    }

    virtual ~virtual_otn_otdr_device() {}

    // Setters
    void set_parent_port(const std::string& port)     { parent_port = port; }
    void set_range_m(sai_uint32_t v)                  { range_m = v; }
    void set_pulse_width_ns(sai_uint32_t v)           { pulse_width_ns = v; }
    void set_acquisition_time_s(sai_uint32_t v)       { acquisition_time_s = v; }
    void set_wavelength_mhz(sai_uint64_t v)           { wavelength_mhz = v; }
    void set_sampling_resolution_m(sai_uint64_t v)    { sampling_resolution_m = v; }
    void set_fiber_type(sai_otn_otdr_fiber_type_t v)         { fiber_type = v; }
    void set_scan_type(sai_otn_otdr_scan_type_t v)           { scan_type = v; }
    void set_negotiation(bool v)                      { negotiation = v; }
    void set_refractive_index(sai_int32_t v)          { refractive_index = v; }
    void set_backscatter_index(sai_int32_t v)         { backscatter_index = v; }
    void set_reflectance_threshold(sai_int32_t v)     { reflectance_threshold = v; }
    void set_splice_loss_threshold(sai_int32_t v)     { splice_loss_threshold = v; }
    void set_fiber_end_threshold(sai_int32_t v)       { fiber_end_threshold = v; }
    void set_scanning_status(sai_otn_otdr_status_t v) { scanning_status = v; }

    // Launch a background scan thread: sleeps acquisition_time_s, copies SOR file, fires ntf_fn
    void trigger_scan(sai_object_id_t otdr_id, sai_otn_otdr_scan_complete_notification_fn ntf_fn);

    // Getters
    const std::string& get_parent_port() const        { return parent_port; }
    sai_uint32_t get_range_m() const                  { return range_m; }
    sai_uint32_t get_pulse_width_ns() const           { return pulse_width_ns; }
    sai_uint32_t get_acquisition_time_s() const       { return acquisition_time_s; }
    sai_uint64_t get_wavelength_mhz() const           { return wavelength_mhz; }
    sai_uint64_t get_sampling_resolution_m() const          { return sampling_resolution_m; }
    sai_otn_otdr_fiber_type_t get_fiber_type() const         { return fiber_type; }
    sai_otn_otdr_scan_type_t get_scan_type() const           { return scan_type; }
    bool get_negotiation() const                      { return negotiation; }
    sai_int32_t get_refractive_index() const               { return refractive_index; }
    sai_int32_t get_backscatter_index() const              { return backscatter_index; }
    sai_int32_t get_reflectance_threshold() const          { return reflectance_threshold; }
    sai_int32_t get_splice_loss_threshold() const          { return splice_loss_threshold; }
    sai_int32_t get_fiber_end_threshold() const            { return fiber_end_threshold; }
    sai_otn_otdr_status_t get_scanning_status() const          { return scanning_status; }

private:
    std::string  parent_port;
    sai_uint32_t range_m;
    sai_uint32_t pulse_width_ns;
    sai_uint32_t acquisition_time_s;
    sai_uint64_t wavelength_mhz;
    sai_uint64_t sampling_resolution_m;
    sai_otn_otdr_fiber_type_t  fiber_type;
    sai_otn_otdr_scan_type_t   scan_type;
    bool         negotiation;
    sai_int32_t       refractive_index;
    sai_int32_t       backscatter_index;
    sai_int32_t       reflectance_threshold;
    sai_int32_t       splice_loss_threshold;
    sai_int32_t       fiber_end_threshold;
    sai_otn_otdr_status_t scanning_status;   // 0 = idle (simulated)
};
