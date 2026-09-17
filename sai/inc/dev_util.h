#pragma once

#include <map>
#include <string>
#include <cstdint>

class dev_util
{
public:
    static uint32_t get_dev_type(const std::string &name);
    static uint32_t get_dev_index(const std::string &name);
    static uint32_t get_sub_index(const std::string &name);
    // "OTDR1-2" -> "OTDR1": the module (slot) an instance belongs to, part before "|" and before the last "-<index>"
    static std::string get_module_name(const std::string &name);

private:
    static std::map<std::string, uint32_t> dev_map;
};
