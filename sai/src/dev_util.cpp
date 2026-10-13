#include "dev_util.h"
#include <regex>
#include "dev_data.h"

/**
 * component name to dev type map.
 * Need to configure this map for different projects.
 */ 
std::map<std::string, uint32_t> dev_util::dev_map = {
    // OA
    {"OA1-1", DEV_TYPE_BA},
    {"OA1-2", DEV_TYPE_PA},
    {"OA2-1", DEV_TYPE_P_BA},
    {"OA2-2", DEV_TYPE_P_PA},

    // VOA
    {"VOA1-1", DEV_TYPE_BA},
    {"VOA1-2", DEV_TYPE_PA},
    {"VOA2-1", DEV_TYPE_P_BA},
    {"VOA2-2", DEV_TYPE_P_PA},

    // OCM
    {"OCM1-1-1", DEV_TYPE_OCM},
    {"OCM1-1-2", DEV_TYPE_P_OCM},
    {"OCM1-1-3", DEV_TYPE_P_OCM},
    {"OCM1-1-4", DEV_TYPE_P_OCM},
    {"OCM2-1-1", DEV_TYPE_P_OCM},
    {"OCM2-1-2", DEV_TYPE_P_OCM},
    {"OCM2-1-3", DEV_TYPE_P_OCM},
    {"OCM2-1-4", DEV_TYPE_P_OCM},

    // OSC
    {"OSC1-1", DEV_TYPE_BA},
    {"OSC1-2", DEV_TYPE_PA},
    {"OSC2-1", DEV_TYPE_P_BA},
    {"OSC2-2", DEV_TYPE_P_PA},

    // OTDR
    {"OTDR1-1-1", DEV_TYPE_OTDR},
    {"OTDR1-1-2", DEV_TYPE_P_OTDR},
    {"OTDR1-1-3", DEV_TYPE_OTDR},
    {"OTDR1-1-4", DEV_TYPE_P_OTDR},
    {"OTDR2-1-1", DEV_TYPE_OTDR},
    {"OTDR2-1-2", DEV_TYPE_P_OTDR},
    {"OTDR2-1-3", DEV_TYPE_OTDR},
    {"OTDR2-1-4", DEV_TYPE_P_OTDR},

    // WSS
    {"LINE-CARD1", DEV_TYPE_WSS},
    {"LINE-CARD2", DEV_TYPE_WSS1},
    {"LINE-CARD3", DEV_TYPE_WSS2},
    {"LINE-CARD4", DEV_TYPE_WSS3}

};

uint32_t
dev_util::get_dev_type(const std::string &name)
{
    // Step 1: take part before '|'
    auto pipe = name.find('|');
    std::string key = (pipe == std::string::npos)
                        ? name
                        : name.substr(0, pipe);

    // Step 2: progressively generalize the key
    while (!key.empty()) {
        auto it = dev_map.find(key);
        if (it != dev_map.end()) {
            return it->second;
        }

        // Strip last "-<number>"
        auto dash = key.rfind('-');
        if (dash == std::string::npos) {
            break;
        }

        key = key.substr(0, dash);
    }

    return DEV_TYPE_NONE;
}

uint32_t
dev_util::get_dev_index(const std::string &name)
{
    // get the second number after '-', e.g. OCM0-1|191262500 -> 1
    // wss name format: WSS0-2|191262500 -> 2
    static std::regex re("^[^|-]+-(\\d+)");
    std::smatch match;

    if (std::regex_search(name, match, re)) {
        return std::stoi(match[1]);
    }

    // fallback: trailing digits (e.g. LineIn1 -> 1)
    static std::regex tail_re("(\\d+)$");
    if (std::regex_search(name, match, tail_re)) {
        return std::stoi(match[1]);
    }

    return 0;
}

uint32_t
dev_util::get_sub_index(const std::string &name)
{
    // get the number after '|', e.g. OCM0-1|191262500
    static std::regex re("^[^|]+\\|(\\d+)$");
    std::smatch match;

    if (std::regex_search(name, match, re)) {
        return std::stoi(match[1]);
    }

    return 0;
}

std::string
dev_util::get_module_name(const std::string &name)
{
    auto pipe = name.find('|');
    std::string key = (pipe == std::string::npos) ? name : name.substr(0, pipe);
    auto dash = key.rfind('-');
    if (dash == std::string::npos || dash == 0) {
        return key;
    }
    return key.substr(0, dash);
}

namespace {

constexpr const char *kLinePortNa = "N/A";

std::string
line_card_port(uint32_t line, const char *port_dir)
{
    return std::to_string(line) + "/R" + std::to_string(line) + "/" + port_dir;
}

bool
parse_line_role_name(const std::string &name,
                     const std::regex &re,
                     uint32_t &line,
                     uint32_t &instance)
{
    std::smatch match;
    if (!std::regex_match(name, match, re)) {
        return false;
    }
    line = static_cast<uint32_t>(std::stoul(match[1].str()));
    instance = static_cast<uint32_t>(std::stoul(match[2].str()));
    return true;
}

bool
line_instance_ports(uint32_t line,
                    uint32_t instance,
                    uint32_t max_instance,
                    std::string &ingress_port,
                    std::string &egress_port)
{
    if (instance < 1 || instance > max_instance) {
        return false;
    }

    ingress_port.clear();
    egress_port.clear();

    switch (instance) {
    case 1:
        ingress_port = line_card_port(line, "P1/RX");
        egress_port = line_card_port(line, "P2/TX");
        break;
    case 2:
        ingress_port = line_card_port(line, "P2/RX");
        egress_port = line_card_port(line, "P1/TX");
        break;
    case 3:
        if (max_instance < 3) {
            return false;
        }
        ingress_port = line_card_port(line, "P1/RX");
        egress_port = kLinePortNa;
        break;
    case 4:
        if (max_instance < 4) {
            return false;
        }
        ingress_port = kLinePortNa;
        egress_port = line_card_port(line, "P1/TX");
        break;
    case 5:
        if (max_instance < 5) {
            return false;
        }
        ingress_port = line_card_port(line, "P2/RX");
        egress_port = kLinePortNa;
        break;
    case 6:
        if (max_instance < 6) {
            return false;
        }
        ingress_port = kLinePortNa;
        egress_port = line_card_port(line, "P2/TX");
        break;
    default:
        return false;
    }

    return true;
}

} // namespace

bool
dev_util::get_voa_ports(const std::string &name,
                        std::string &ingress_port,
                        std::string &egress_port)
{
    uint32_t line = 0;
    uint32_t instance = 0;
    static const std::regex voa_re(R"(^VOA(\d+)-(\d+)$)", std::regex::icase);
    if (!parse_line_role_name(name, voa_re, line, instance)) {
        return false;
    }
    return line_instance_ports(line, instance, 6, ingress_port, egress_port);
}

bool
dev_util::get_oa_ports(const std::string &name,
                       std::string &ingress_port,
                       std::string &egress_port)
{
    uint32_t line = 0;
    uint32_t instance = 0;
    static const std::regex oa_re(R"(^OA(\d+)-(\d+)$)", std::regex::icase);
    if (!parse_line_role_name(name, oa_re, line, instance)) {
        return false;
    }
    return line_instance_ports(line, instance, 2, ingress_port, egress_port);
}
