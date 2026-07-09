// otn-vhal -- the otn-kvm virtual HAL.
//
// On real hardware the HAL/cmptctrld watches sensors and emits HAL events when a
// condition changes. otn-kvm has no hardware, so this tool is the KVM stand-in
// for that event source -- driven by the user instead of by sensors. It reads
// the shared alarm catalog, frames a HAL event for the requested alarm, and
// sends it straight to libsai-otn's listener socket, which reports it through the
// normal SAI path (send_alarm_event_data -> syncd -> orchagent -> eventd).
//
//   otn-vhal --> HAL_EVENT_DEFAULT_SOCK --> libsai-otn --> ... --> EVENT_DB / show alarm
//
// It is stateless by design: the authoritative "what is currently raised" view
// is EVENT_DB (shown by `show alarm`), so this tool keeps no copy of it.
//
// Usage:
//   otn-vhal list [--category <cat>]
//   otn-vhal raise "<alarm-name>" [--instance N | --object <sai-object-name>]
//   otn-vhal clear "<alarm-name>" [--instance N | --object <sai-object-name>]
//   (current alarms:  show alarm)

#include "hal_event.h"          // SAI wire contract (sai/inc)
#include "otn_alarm_catalog.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#define OTN_VHAL_CATALOG_DEFAULT  "/usr/share/sonic/otn/otn_alarm_catalog.json"

namespace {

void usage()
{
    fprintf(stderr,
        "otn-vhal -- otn-kvm virtual HAL (user-driven alarm event source)\n"
        "usage:\n"
        "  otn-vhal list [--category <cat>]\n"
        "  otn-vhal raise \"<alarm-name>\" [--instance N]\n"
        "  otn-vhal clear \"<alarm-name>\" [--instance N]\n"
        "  (--instance is the unit index for environmental alarms, or the\n"
        "   optical object index; default 0)\n"
        "options:\n"
        "  --catalog <path>   alarm catalog (default " OTN_VHAL_CATALOG_DEFAULT ")\n"
        "  --socket  <path>   libsai HAL socket (default " HAL_EVENT_DEFAULT_SOCK ")\n"
        "current alarms:  show alarm\n");
}

// Frame and send one HAL event to libsai-otn's listener socket. 'event_id' is the
// alarm's raise_id or clear_id (raise/clear is encoded in the id, Shasta-style);
// 'instance' is the affected unit index carried in sub_event_id.
bool send_hal_event(const std::string& sock_path, int event_id, int instance)
{
    struct hal_event payload;
    memset(&payload, 0, sizeof(payload));
    payload.event_id     = event_id;
    payload.sub_event_id = instance;

    unsigned char buf[sizeof(hal_msg_header) + sizeof(payload) + 1];
    memset(buf, 0, sizeof(buf));

    hal_msg_header* hdr = reinterpret_cast<hal_msg_header*>(buf);
    hdr->magic       = htonl(HAL_MSG_MAGIC);
    hdr->version     = HAL_MSG_VERSION;
    hdr->msg_type    = htonl(HAL_MSG_TYPE_DEV_EVENT);
    hdr->status      = htons(0);
    hdr->payload_len = htons(static_cast<uint16_t>(sizeof(payload)));
    memcpy(buf + sizeof(hal_msg_header), &payload, sizeof(payload));

    // Trailing checksum over header+payload (Shasta __UTIL_CalcMsgSum contract).
    const size_t body = sizeof(hal_msg_header) + sizeof(payload);
    buf[body] = hal_msg_checksum(buf, static_cast<unsigned int>(body));

    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("otn-vhal: socket");
        return false;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path.c_str(), sizeof(addr.sun_path) - 1);

    bool ok = sendto(fd, buf, sizeof(buf), 0,
                     (struct sockaddr*)&addr, sizeof(addr)) == (ssize_t)sizeof(buf);
    if (!ok) {
        fprintf(stderr, "otn-vhal: cannot send to %s (%s) -- is syncd/libsai-otn up?\n",
                sock_path.c_str(), strerror(errno));
    }
    close(fd);
    return ok;
}

int do_list(const AlarmCatalog& cat, const std::string& category)
{
    printf("%-32s  %-9s  %-15s  %-12s  %s\n",
           "NAME", "SEVERITY", "ALARM-TYPE", "CATEGORY", "RAISE/CLEAR ID");
    for (const auto& a : cat.all()) {
        if (!category.empty() && a.category != category) {
            continue;
        }
        char ids[32];
        if (a.clear_id) snprintf(ids, sizeof(ids), "%d/%d", a.raise_id, a.clear_id);
        else            snprintf(ids, sizeof(ids), "%d (raise-only)", a.raise_id);
        printf("%-32s  %-9s  %-15s  %-12s  %s\n",
               a.name.c_str(), a.severity.c_str(), a.alarm_type.c_str(),
               a.category.c_str(), ids);
    }
    return 0;
}

int do_raise_clear(const AlarmCatalog& cat, const std::string& sock_path,
                   bool raise, const std::string& name, const std::string& instance_arg)
{
    const AlarmSpec* s = cat.find(name);
    if (!s) {
        fprintf(stderr, "otn-vhal: unknown alarm '%s' (try: otn-vhal list)\n", name.c_str());
        return 1;
    }

    const int event_id = raise ? s->raise_id : s->clear_id;
    if (event_id == 0) {
        fprintf(stderr, "otn-vhal: alarm '%s' has no %s event id (notify-only)\n",
                name.c_str(), raise ? "raise" : "clear");
        return 2;
    }

    const int instance = instance_arg.empty() ? 0 : atoi(instance_arg.c_str());

    if (!send_hal_event(sock_path, event_id, instance)) {
        return 1;
    }

    printf("%s %s instance=%d (event_id=%d) -> %s\n", raise ? "RAISE" : "CLEAR",
           name.c_str(), instance, event_id, sock_path.c_str());
    return 0;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage();
        return 2;
    }

    std::string cmd = argv[1];
    std::string catalog_path = OTN_VHAL_CATALOG_DEFAULT;
    std::string sock_path    = HAL_EVENT_DEFAULT_SOCK;
    std::string category, instance_arg;
    std::string name;

    // positional alarm name for raise/clear is argv[2] (before the flags)
    int i = 2;
    if (cmd == "raise" || cmd == "clear") {
        if (argc < 3) { usage(); return 2; }
        name = argv[2];
        i = 3;
    }
    for (; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--catalog"  && i + 1 < argc) catalog_path = argv[++i];
        else if (a == "--socket"   && i + 1 < argc) sock_path = argv[++i];
        else if (a == "--category" && i + 1 < argc) category = argv[++i];
        else if (a == "--instance" && i + 1 < argc) instance_arg = argv[++i];
        else { usage(); return 2; }
    }

    AlarmCatalog cat;
    if (!cat.load(catalog_path)) {
        fprintf(stderr, "otn-vhal: failed to load alarm catalog %s\n", catalog_path.c_str());
        return 1;
    }

    if (cmd == "list") {
        return do_list(cat, category);
    }
    if (cmd == "raise" || cmd == "clear") {
        return do_raise_clear(cat, sock_path, cmd == "raise", name, instance_arg);
    }

    usage();
    return 2;
}
