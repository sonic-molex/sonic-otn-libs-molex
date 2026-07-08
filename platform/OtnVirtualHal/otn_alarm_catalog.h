#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/* One alarm definition, loaded from otn_alarm_catalog.json (the single source of
 * truth shared with the SAI listener and the device eventd profile). Raise and
 * clear are distinct HAL event ids (Shasta convention); clear_id == 0 means the
 * event has no distinct clear id (notify-only). */
struct AlarmSpec {
    int         raise_id = 0;
    int         clear_id = 0;
    std::string name;
    std::string severity;         // CRITICAL|MAJOR|MINOR|WARNING|INFORMATIONAL (eventd)
    std::string alarm_type;       // OpenROADM: communication|qualityOfService|processingError|equipment|environmental
    std::string resource_type;    // OpenROADM: device|port|circuit-pack|connection|...
    std::string category;         // hal-event|thermal|fan|power|optical
    std::string resource_prefix;
    std::string text;
};

/* Loads and indexes the alarm catalog. */
class AlarmCatalog {
public:
    // Returns false if the file is missing/unparseable or contains no alarms.
    bool load(const std::string& path);

    const AlarmSpec* find(const std::string& name) const;
    const std::vector<AlarmSpec>& all() const { return m_alarms; }
    size_t size() const { return m_alarms.size(); }
    const std::string& path() const { return m_path; }

private:
    std::vector<AlarmSpec>                 m_alarms;
    std::unordered_map<std::string, size_t> m_by_name;
    std::string                           m_path;
};
