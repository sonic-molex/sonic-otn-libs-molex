#include "dev_util.h"
#include <regex>
#include "dev_data.h"

/**
 * component name to dev type map.
 * Need to configure this map for different projects.
 */ 
std::map<std::string, uint32_t> dev_util::dev_map = {
    // OA
    {"OA0-0", DEV_TYPE_BA},
    {"OA0-1", DEV_TYPE_PA},
    {"OA1-0", DEV_TYPE_P_BA},
    {"OA1-1", DEV_TYPE_P_PA},

    // VOA
    {"VOA0-0", DEV_TYPE_BA},
    {"VOA0-1", DEV_TYPE_PA},
    {"VOA1-0", DEV_TYPE_P_BA},
    {"VOA1-1", DEV_TYPE_P_PA},

    // OCM
    {"OCM0", DEV_TYPE_OCM},
    {"OCM1", DEV_TYPE_P_OCM},
    

    // OSC
    {"OSC0-0", DEV_TYPE_BA},
    {"OSC0-1", DEV_TYPE_PA},
    {"OSC1-0", DEV_TYPE_P_BA},
    {"OSC1-1", DEV_TYPE_P_PA},

    // OTDR
    {"OTDR0-0", DEV_TYPE_OTDR},
    {"OTDR1-0", DEV_TYPE_P_OTDR},

    // WSS
    {"LineIn0", DEV_TYPE_WSS},
    {"LineIn1", DEV_TYPE_WSS1},
    {"LineIn2", DEV_TYPE_WSS2},
    {"LineIn3", DEV_TYPE_WSS3}

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
