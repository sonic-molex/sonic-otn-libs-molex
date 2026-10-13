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
    // OTDR{n}-{s}-{i} -> "OTDR{n}-{s}"; otherwise strip the last "-<index>" (e.g. OTDR1-2 -> OTDR1)
    static std::string get_module_name(const std::string &name);

    // VOA{line}-{instance} -> {line}/R{line}/P*/RX|TX (instance 1..6)
    static bool get_voa_ports(const std::string &name,
                              std::string &ingress_port,
                              std::string &egress_port);

    // OA{line}-{instance} -> {line}/R{line}/P*/RX|TX (instance 1..2, west/east pair)
    static bool get_oa_ports(const std::string &name,
                             std::string &ingress_port,
                             std::string &egress_port);

private:
    static std::map<std::string, uint32_t> dev_map;
};
