#pragma once

#include <iostream>
#include <list>
#include <map>
#include <saiextensions.h>
#include <vector>
#include <set>
#include <regex>
#include <optional>
#include <cstring>


#define NULL_HANDLE -1

// object pointer and it's id
class sai_id_map_t
{
public:
    sai_id_map_t()
    {
        // init
        unused_id.clear();
        id_map.clear();
    }

    ~sai_id_map_t()
    {
        free_all();
    }

    void free_id(sai_object_id_t sai_object_id);

    void free_all();

    sai_object_id_t get_new_id(void *obj_ptr)
    {
        sai_object_id_t id;
        if (!unused_id.empty()) {
            id = unused_id.back();
            unused_id.pop_back();
        } else {
            id = id_map.size() + 1;
        }
        id_map[id] = obj_ptr;
        return id;
    }

    void *get_object(sai_object_id_t sai_object_id)
    {
        return id_map[sai_object_id];
    }

    bool find_object(sai_object_id_t sai_object_id)
    {
        return id_map.find(sai_object_id) != id_map.end();
    }

    uint32_t get_ids(std::vector<sai_object_id_t> &ids) {
        for (const auto &it : id_map) {
            ids.push_back(it.first);
        }

        return ids.size();
    }

protected:
    std::map<sai_object_id_t, void *> id_map;
    std::vector<sai_object_id_t> unused_id;
};

class sai_obj
{
public:
    sai_obj(sai_id_map_t &sai_id_map, sai_object_type_t type)
    {
        sai_object_id = sai_id_map.get_new_id(this);
        sai_object_type = type;
    }

    virtual ~sai_obj()
    {
        // free_id(sai_object_id); TODO: fix this
    }

    // variable
    sai_object_id_t sai_object_id;
    sai_object_type_t sai_object_type;
};

class router_interface_obj : public sai_obj
{
public:
    router_interface_obj(sai_id_map_t &sai_id_map) :
        sai_obj(sai_id_map, SAI_OBJECT_TYPE_ROUTER_INTERFACE)
    {
        this->vid = 1;
        this->mac_valid = false;
        this->vid = 0;
        this->type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    }

    // variable
    sai_mac_t mac;
    bool mac_valid;
    uint16_t vid;
    sai_router_interface_type_t type;
};

class otn_obj : public sai_obj
{
public:
    otn_obj(sai_id_map_t &sai_id_map, sai_object_type_extensions_t type) :
        sai_obj(sai_id_map, (sai_object_type_t)type),
        dev_type(0),
        dev_index(0)
    {
    }

    virtual ~otn_obj() {}

    // variable
    uint32_t dev_type;
    uint32_t dev_index;
    std::string dev_name;
};

class otn_device_obj : public otn_obj
{
public:
    otn_device_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_DEVICE),
        admin_state(false),
        alarm_act_time(0),
        alarm_deact_time(0)
    {
    }

    // variable
    bool admin_state;
    uint32_t alarm_act_time;
    uint32_t alarm_deact_time;
};

class otn_attenuator_obj : public otn_obj
{
public:
    otn_attenuator_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_ATTENUATOR),
        block_attn(2550),
        set_attn(800),
        attn_mode(SAI_OTN_ATTENUATOR_MODE_CONSTANT_ATTENUATION),
        target_power(0),
        max_power(500),
        max_power_thr(300),
        enable(true)
    {
    }

    // variable
    const uint32_t block_attn;
    uint32_t set_attn;
    uint32_t attn_mode;
    int32_t target_power;
    int32_t max_power;
    int32_t max_power_thr;
    bool enable;
    std::string ingress_port;
    std::string egress_port;
};

class otn_oa_obj : public otn_obj
{
public:
    otn_oa_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_OA),
        oa_type(SAI_OTN_OA_TYPE_EDFA),
        set_mode(SAI_OTN_OA_AMP_MODE_CONSTANT_GAIN),
        set_gain(2000),
        set_power(800)
    {
    }

    // variable
    int32_t oa_type;
    int32_t set_mode;
    uint32_t set_gain;
    int32_t set_power;
    std::string ingress_port;
    std::string egress_port;
};

class otn_ocm_obj : public otn_obj
{
public:
    otn_ocm_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_OCM)
    {
    }

    // variable
    std::string monitor_port;
    // lower frequency vs object id
    std::map<sai_uint64_t, sai_object_id_t> channels;
};

class otn_ocm_channel_obj : public otn_obj
{
public:
    otn_ocm_channel_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_OCM_CHANNEL),
        ocm_id(0),
        lower_frequency(0),
        upper_frequency(0),
        power(0)
    {
    }

    void set_parent(sai_id_map_t &sai_id_map) {
        std::vector<sai_object_id_t> ids;
        sai_id_map.get_ids(ids);

        otn_ocm_obj *parent_obj = NULL;
        for (const auto &it : ids) {
            sai_obj *obj = static_cast<sai_obj *>(sai_id_map.get_object(it));
            if (obj->sai_object_type == (sai_object_type_t)SAI_OBJECT_TYPE_OTN_OCM) {
                otn_ocm_obj *obj_temp = static_cast<otn_ocm_obj *>(obj);
                if (obj_temp->dev_index == this->dev_index) {
                    parent_obj = obj_temp;
                    break;
                }
            }
        }

        if (parent_obj == NULL) {
            // Create ocm first, for future using
            parent_obj = new otn_ocm_obj(sai_id_map);
            parent_obj->dev_index = this->dev_index;
        }

        this->ocm_id = parent_obj->sai_object_id;
        parent_obj->channels[this->lower_frequency] = this->sai_object_id;
    }

    // variable
    sai_object_id_t ocm_id;
    sai_uint64_t lower_frequency;
    sai_uint64_t upper_frequency;
    sai_int32_t power;

    static sai_uint64_t min_frequency;
};

class otn_osc_obj : public otn_obj
{
public:
    otn_osc_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_OSC),
        freq(198538052)
    {
    }

    // variable
    const uint64_t freq;
};

class otn_wss_obj : public otn_obj
{
public:
    otn_wss_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_WSS),
        lower_frequency(0),
        upper_frequency(0),
        admin_state(SAI_OTN_WSS_ADMIN_STATE_ENABLED),
        super_channel(false),
        super_channel_parent(0),
        ase_control_mode(SAI_OTN_WSS_ASE_CONTROL_MODE_AUTO_ASE_FAILURE_AND_RESTORE),
        ase_injection_mode(SAI_OTN_WSS_ASE_INJECTION_MODE_THRESHOLD),
        ase_injection_threshold(0),
        ase_injection_delta(0),
        media_channel_injection_offset(0),
        attenuation_control_mode(SAI_OTN_WSS_ATTENUATION_CONTROL_MODE_ATTENUATION_SET_ATTENUATION),
        attenuation_control_range(SAI_OTN_WSS_ATTENUATION_CONTROL_RANGE_CONTROL_RANGE_FULL),
        max_undershoot_compensation(0),
        max_overshoot_compensation(0)
    {
    }

    // variable
    sai_uint64_t lower_frequency;
    sai_uint64_t upper_frequency;
    sai_otn_wss_admin_state_t admin_state;
    bool super_channel;
    sai_uint32_t super_channel_parent;
    sai_otn_wss_ase_control_mode_t ase_control_mode;
    sai_otn_wss_ase_injection_mode_t ase_injection_mode;
    sai_int32_t ase_injection_threshold;
    sai_int32_t ase_injection_delta;
    sai_int32_t media_channel_injection_offset;
    sai_otn_wss_attenuation_control_mode_t attenuation_control_mode;
    sai_otn_wss_attenuation_control_range_t attenuation_control_range;
    sai_int32_t max_undershoot_compensation;
    sai_int32_t max_overshoot_compensation;
    std::string source_port_name;
    std::string dest_port_name;
    std::map<sai_uint64_t, sai_object_id_t> spec_power_entries;  /* lower_frequency -> spec_power_id */
};

class otn_wss_spec_power_obj : public otn_obj
{
public:
    otn_wss_spec_power_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_WSS_SPEC_POWER),
        parent_wss_id(0),
        lower_frequency(0),
        upper_frequency(0),
        target_power(0),
        attenuation(0),
        actual_attenuation(0)
    {
    }

    void set_parent(sai_id_map_t &sai_id_map) {
        std::vector<sai_object_id_t> ids;
        sai_id_map.get_ids(ids);

        otn_wss_obj *parent_obj = NULL;
        for (const auto &it : ids) {
            sai_obj *obj = static_cast<sai_obj *>(sai_id_map.get_object(it));
            if (obj->sai_object_type == (sai_object_type_t)SAI_OBJECT_TYPE_OTN_WSS) {
                otn_wss_obj *obj_temp = static_cast<otn_wss_obj *>(obj);
                if (!this->dev_name.empty() && obj_temp->source_port_name == this->dev_name) {
                    parent_obj = obj_temp;
                    break;
                }
                if (obj_temp->dev_index == this->dev_index) {
                    parent_obj = obj_temp;
                    break;
                }
            }
        }

        if (parent_obj == NULL) {
            // Create wss device first, for future use
            parent_obj = new otn_wss_obj(sai_id_map);
            parent_obj->dev_index = this->dev_index;
            if (!this->dev_name.empty()) {
                parent_obj->source_port_name = this->dev_name;
            }
        }

        this->parent_wss_id = parent_obj->sai_object_id;
        parent_obj->spec_power_entries[this->lower_frequency] = this->sai_object_id;
    }
    
    // variable
    sai_object_id_t parent_wss_id;
    sai_uint64_t lower_frequency;
    sai_uint64_t upper_frequency;
    sai_int32_t target_power;
    sai_int32_t attenuation;
    sai_int32_t actual_attenuation;
};

class otn_otdr_obj : public otn_obj
{
public:
    otn_otdr_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_OTDR),
        range_m(60), pulse_width_ns(3000), acquisition_time_s(60),
        wavelength_mhz(193414489ULL), sampling_resolution_m(1000), negotiation(true),
        refractive_index(1467900), backscatter_index(-8100),
        reflectance_threshold(-4000), splice_loss_threshold(35),
        fiber_end_threshold(300)
    {
    }

    // CREATE_ONLY identity fields
    std::string name;
    std::string parent_port;

    // CREATE_AND_SET scan parameters
    sai_uint32_t range_m;
    sai_uint32_t pulse_width_ns;
    sai_uint32_t acquisition_time_s;
    sai_uint64_t wavelength_mhz;
    sai_uint64_t sampling_resolution_m;   // scaled by 100 (e.g. 1000 = 10.00 m)

    // CREATE_AND_SET fiber properties
    sai_otn_otdr_fiber_type_t  fiber_type;
    bool         negotiation;
    sai_int32_t  refractive_index;      // scaled by 1e6 (e.g. 1467900 = 1.4679)
    sai_int32_t  backscatter_index;     // scaled by 100 (e.g. -8100 = -81.00 dB/km)

    // CREATE_AND_SET detection thresholds
    sai_int32_t  reflectance_threshold;   // scaled by 100 (e.g. -4000 = -40.00 dB)
    sai_int32_t  splice_loss_threshold;   // scaled by 100 (e.g. 35 = 0.35 dB)
    sai_int32_t  fiber_end_threshold;     // scaled by 100 (e.g. 300 = 3.00 dB)
};

class otn_otdr_scan_type_obj : public otn_obj
{
public:
    otn_otdr_scan_type_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_OTDR_SCAN_TYPE),
        scan_type(SAI_OTN_OTDR_SCAN_TYPE_SHORT),
        acquisition_time_s(60), range_m(50000), pulse_width_ns(3000),
        wavelength_mhz(200000000ULL), sampling_resolution_m(1000),
        fiber_type(SAI_OTN_OTDR_FIBER_TYPE_SSMF), negotiation(true)
    {}

    // CREATE_ONLY key
    sai_otn_otdr_scan_type_t  scan_type;
    // CREATE_AND_SET
    sai_uint32_t acquisition_time_s;
    sai_uint32_t range_m;
    sai_uint32_t pulse_width_ns;
    sai_uint64_t wavelength_mhz;
    sai_uint64_t sampling_resolution_m;
    sai_otn_otdr_fiber_type_t fiber_type;
    bool         negotiation;
};

class otn_otdr_fiber_profile_obj : public otn_obj
{
public:
    otn_otdr_fiber_profile_obj(sai_id_map_t &sai_id_map) :
        otn_obj(sai_id_map, SAI_OBJECT_TYPE_OTN_OTDR_FIBER_PROFILE),
        fiber_type(SAI_OTN_OTDR_FIBER_TYPE_SSMF),
        refractive_index(1467900), backscatter_index(-8200),
        reflectance_threshold(-4000), splice_loss_threshold(35),
        fiber_end_threshold(300)
    {}

    // CREATE_ONLY key
    sai_otn_otdr_fiber_type_t fiber_type;
    // CREATE_AND_SET
    sai_int32_t refractive_index;
    sai_int32_t backscatter_index;
    sai_int32_t reflectance_threshold;
    sai_int32_t splice_loss_threshold;
    sai_int32_t fiber_end_threshold;
};

class switch_metadata
{ 
public:
    switch_metadata() : virtual_id(0x0F000000), otn_alarm_event_ntf(nullptr), otn_otdr_scan_complete_ntf(nullptr)
    {
        memset(default_switch_mac, 0, 6);

        sai_object_id_t id = virtual_id;
        vids[SAI_OBJECT_TYPE_VIRTUAL_ROUTER] = ++id;
        vids[SAI_OBJECT_TYPE_VLAN] = ++id;
        vids[SAI_OBJECT_TYPE_PORT] = ++id;
        vids[SAI_OBJECT_TYPE_BRIDGE] = ++id;
        vids[SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP] = ++id;
    }

    // variable
    const uint32_t virtual_id;
    sai_object_id_t switch_id;
    sai_mac_t default_switch_mac;
    sai_id_map_t sai_id_map;

    // OTN alarm event notification callback
    sai_otn_alarm_event_notification_fn otn_alarm_event_ntf;

    // OTN OTDR scan complete notification callback
    sai_otn_otdr_scan_complete_notification_fn otn_otdr_scan_complete_ntf;

    // id mapping for virtual devices
    std::map<sai_object_type_t, sai_object_id_t> vids;

};
