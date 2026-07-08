#include "otn_alarm_catalog.h"
#include "Json.hpp"

#include <fstream>

using json = nlohmann::json;

bool AlarmCatalog::load(const std::string& path)
{
    m_path = path;
    m_alarms.clear();
    m_by_name.clear();

    std::ifstream f(path);
    if (!f.is_open()) {
        return false;
    }

    json j;
    try {
        f >> j;
    } catch (const std::exception&) {
        return false;
    }

    if (!j.contains("alarms") || !j["alarms"].is_array()) {
        return false;
    }

    for (const auto& a : j["alarms"]) {
        AlarmSpec s;
        s.raise_id        = a.value("raise_id", 0);
        s.clear_id        = a.value("clear_id", 0);
        s.name            = a.value("name", std::string());
        s.severity        = a.value("severity", std::string());
        s.alarm_type      = a.value("alarm_type", std::string());
        s.resource_type   = a.value("resource_type", std::string());
        s.category        = a.value("category", std::string());
        s.resource_prefix = a.value("resource_prefix", std::string());
        s.text            = a.value("text", std::string());

        if (s.name.empty()) {
            continue;
        }
        m_by_name[s.name] = m_alarms.size();
        m_alarms.push_back(std::move(s));
    }

    return !m_alarms.empty();
}

const AlarmSpec* AlarmCatalog::find(const std::string& name) const
{
    auto it = m_by_name.find(name);
    return (it == m_by_name.end()) ? nullptr : &m_alarms[it->second];
}