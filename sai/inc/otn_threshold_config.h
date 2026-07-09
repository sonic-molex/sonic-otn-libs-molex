#pragma once

#include <map>
#include <string>
#include <unordered_map>

// SAI (for sai_profile_get_value_fn)
#ifdef __cplusplus
extern "C" {
#endif
#include "saiextensions.h"
#ifdef __cplusplus
}
#endif

// A single named threshold parameter, expressed as a high/low range, matching
// the { "high-thr": <num>, "low-thr": <num> } value of each entry in the JSON
// threshold file referenced by SAI_OTN_THRESHOLD_FILE in sai.profile.
struct otn_threshold_range_t
{
    double high = 0.0;
    double low  = 0.0;
};

// Device-global optical threshold store.
//
// Populated once at switch init (create_switch -> init_switch) by reading the
// path from the SAI_OTN_THRESHOLD_FILE profile key and parsing the JSON. Entries
// are keyed by their name (a vendor namespace such as
// "olt-booster-amp-app-in-los-high-threshold") and consulted by the components
// that program the corresponding hardware thresholds.
class otn_threshold_config
{
public:
    static otn_threshold_config& instance();

    // Store the SAI profile get_value callback (from sai_api_initialize).
    void set_profile_getter(sai_profile_get_value_fn fn);

    // Resolve SAI_OTN_THRESHOLD_FILE via the profile getter, parse it and cache
    // the entries. Idempotent: the first call does the work, later calls are
    // no-ops. Returns the number of thresholds currently held.
    size_t load();

    // Get the threshold map for a given name.
    bool get_threshold_map(const std::string& name, std::unordered_map<std::string, otn_threshold_range_t>& threshold_map) const;

    // Human-readable dump of all held thresholds (for logging / debugging).
    std::string dump() const;

private:
    otn_threshold_config() = default;
    otn_threshold_config(const otn_threshold_config&) = delete;
    otn_threshold_config& operator=(const otn_threshold_config&) = delete;

    bool parse_file(const std::string& path);

    sai_profile_get_value_fn profile_getter_ = nullptr;
    std::map<std::string, std::unordered_map<std::string, otn_threshold_range_t>> table_;
    bool loaded_ = false;
};
