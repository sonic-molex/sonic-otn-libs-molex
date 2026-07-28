#include "otn_threshold_config.h"
#include "logger.h"

#include <fstream>
#include <sstream>
#include <exception>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

otn_threshold_config& otn_threshold_config::instance()
{
    static otn_threshold_config inst;
    return inst;
}

void otn_threshold_config::set_profile_getter(sai_profile_get_value_fn fn)
{
    profile_getter_ = fn;
}

size_t otn_threshold_config::load()
{
    if (loaded_) {
        return table_.size();
    }
    loaded_ = true;

    if (!profile_getter_) {
        logger::warn("otn_threshold_config: no SAI profile getter available; "
                     "thresholds not loaded");
        return 0;
    }

    const char* key = "SAI_OTN_THRESHOLD_FILE";
    const char* path = profile_getter_(0, key);
    if (!path || path[0] == '\0') {
        logger::notice(std::string("otn_threshold_config: ") + key +
                       " not set in sai.profile; no thresholds loaded");
        return 0;
    }

    if (!parse_file(path)) {
        return 0;
    }

    logger::notice("otn_threshold_config: loaded " + std::to_string(table_.size()) +
                   " threshold(s) from " + path);
    logger::notice(dump());
    return table_.size();
}

bool otn_threshold_config::get_threshold_map(const std::string& name, std::unordered_map<std::string, otn_threshold_range_t>& threshold_map) const
{
    auto it = table_.find(name);
    if (it == table_.end()) {
        // A miss is normal when the feature is unconfigured or a device simply has
        // no thresholds; keep it at debug so it does not flood the ERROR channel.
        logger::debug("otn_threshold_config: no threshold map for " + name);
        return false;
    }
    threshold_map = it->second;
    return true;

}

bool otn_threshold_config::parse_file(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        logger::warn("otn_threshold_config: cannot open threshold file " + path);
        return false;
    }

    json root;
    try {
        // ignore_comments = true so the file may carry // or /* */ annotations.
        root = json::parse(ifs, /*callback=*/nullptr, /*allow_exceptions=*/true,
                           /*ignore_comments=*/true);
    } catch (const std::exception& ex) {
        logger::warn("otn_threshold_config: JSON parse error in " + path + ": " + ex.what());
        return false;
    }

    if (!root.is_object()) {
        logger::warn("otn_threshold_config: expected a top-level JSON object in " + path);
        return false;
    }

    for (const auto& item : root.items()) {
        const std::string& comp_name = item.key();

        if (!item.value().is_object()) {
            logger::warn("otn_threshold_config: component '" + comp_name +
                         "' is not a JSON object, skipped");
            continue;
        }

        std::unordered_map<std::string, otn_threshold_range_t> all_ranges;
        for (const auto& comp_item : item.value().items()) {
            const std::string& name = comp_item.key();
            otn_threshold_range_t range;
            try {
                range.high = comp_item.value().at("high-thr").get<double>();
                range.low  = comp_item.value().at("low-thr").get<double>();
            } catch (const std::exception&) {
                logger::warn("otn_threshold_config: entry '" + comp_name + "/" + name +
                            "' missing/invalid high-thr or low-thr, skipped");
                continue;
            }
            all_ranges[name] = range;
        }

        if (all_ranges.empty()) {
            logger::warn("otn_threshold_config: component '" + comp_name +
                         "' has no valid threshold entries, skipped");
            continue;
        }
        table_[comp_name] = all_ranges;
    }

    return true;
}

std::string otn_threshold_config::dump() const
{
    std::ostringstream oss;
    oss << "otn_threshold_config dump (" << table_.size() << " entries):";
    if (table_.empty()) {
        oss << " (empty)";
    }
    for (const auto& kv : table_) {
        for (const auto& kv2 : kv.second) {
            oss << "[" << kv.first << "/" << kv2.first << "] high=" << kv2.second.high
                << " low=" << kv2.second.low << ", ";
        }
    }
    return oss.str();
}
