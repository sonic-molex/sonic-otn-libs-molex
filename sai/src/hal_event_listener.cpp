#include "hal_event.h"
#include "hal_event_listener.h"
#include "sai_adapter.h"
#include "logger.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

/* HAL alarm catalog: maps a HAL event_id to an alarm. Generated from
 * otn_alarm_catalog.json (the single source of truth); also mirrored into the
 * device eventd profile (default.json), or eventd drops the event. Following the
 * Shasta convention, raise and clear are distinct event ids -- each expands to a
 * RAISE row and (if it has one) a CLEAR row, so 'is_clear' tells the action.
 * Every alarm is reported against the switch object with "<resource_prefix>
 * <instance>" as the resource in the description. Severities are the closest SAI
 * enum value -- the shown severity comes from the eventd profile, not here. */
struct AlarmDef {
    int                          event_id;
    bool                         is_clear;         // true => this id is the CLEAR variant
    sai_object_type_extensions_t sai_object_type;  // object-bound alarm; 0 = objectless (environmental)
    const char                  *event_name;
    sai_otn_alarm_severity_t     severity;
    const char                  *resource_prefix;  // objectless: + instance, in the description
    const char                  *text;
};

const AlarmDef *lookup(int event_id)
{
    // Generated from otn_alarm_catalog.json at build time (gen_alarm_table.py)
    // so this table is never hand-duplicated -- the catalog is the single source.
    static const AlarmDef kTable[] = {
#include "otn_alarm_table.gen.inc"
    };
    for (const auto &e : kTable)
        if (e.event_id == event_id) return &e;
    return nullptr;
}

void listener_loop()
{
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) {
        logger::error("hal_event: socket() failed");
        return;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, HAL_EVENT_DEFAULT_SOCK, sizeof(addr.sun_path) - 1);

    unlink(HAL_EVENT_DEFAULT_SOCK);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        logger::error(std::string("hal_event: bind() failed on ") + HAL_EVENT_DEFAULT_SOCK);
        close(fd);
        return;
    }

    logger::notice(std::string("hal_event: listening on ") + HAL_EVENT_DEFAULT_SOCK);

    unsigned char buf[HAL_MSG_MAX_LEN];
    const size_t hdrLen = sizeof(hal_msg_header);

    for (;;) {
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, nullptr, nullptr);
        if (n < 0) {
            if (errno == EINTR) continue;
            logger::error("hal_event: recvfrom() failed");
            break;
        }
        if (n < (ssize_t)(hdrLen + 1)) {
            logger::warn("hal_event: short message ignored");
            continue;
        }

        const hal_msg_header *hdr = reinterpret_cast<const hal_msg_header *>(buf);
        if (ntohl(hdr->msg_type) != HAL_MSG_TYPE_DEV_EVENT) {
            logger::warn("hal_event: unknown msg_type ignored");
            continue;
        }

        // Validate framing + checksum (Shasta UTIL_ChkOopMsg contract):
        // frame = header + payload + 1 trailing checksum byte.
        const size_t plen  = ntohs(hdr->payload_len);
        const size_t frame = hdrLen + plen + 1;
        if ((size_t)n < frame) {
            logger::warn("hal_event: short message ignored");
            continue;
        }
        if ((uint8_t)buf[frame - 1] != hal_msg_checksum(buf, (unsigned int)(frame - 1))) {
            logger::warn("hal_event: checksum mismatch, message dropped");
            continue;
        }
        if (plen < sizeof(struct hal_event)) {
            logger::warn("hal_event: payload too small, ignored");
            continue;
        }

        struct hal_event ev;
        memcpy(&ev, buf + hdrLen, sizeof(ev));

        const AlarmDef *d = lookup(ev.event_id);
        if (d == nullptr) {
            logger::warn("hal_event: no mapping for event_id " + std::to_string(ev.event_id));
            continue;
        }

        /* Shasta convention: the event_id (via the catalog) says raise-vs-clear;
         * sub_event_id is the affected instance. */
        const int instance = ev.sub_event_id;
        sai_otn_alarm_action_t action =
            d->is_clear ? SAI_OTN_ALARM_ACTION_CLEAR : SAI_OTN_ALARM_ACTION_RAISE;
        const bool raise = !d->is_clear;

        if (d->sai_object_type != (sai_object_type_extensions_t)0) {
            /* Object-bound alarm: attach to the instance-th SAI object of this
             * type so orchagent resolves the real resource name (e.g. "OA0-1"). */
            logger::notice(std::string("hal_event: ") + (raise ? "RAISE " : "CLEAR ") +
                           d->event_name + " [instance " + std::to_string(instance) + "]");
            sai_adapter::report_hal_object_alarm(d->sai_object_type, instance,
                                                 d->event_name, d->severity, action, d->text);
        } else {
            /* Objectless (environmental): reported against the switch, with the
             * unit as "<PREFIX><instance>" in the description. */
            std::string resource = std::string(d->resource_prefix) + std::to_string(instance);
            std::string description = std::string(d->text) + " (" + resource + ")";
            logger::notice(std::string("hal_event: ") + (raise ? "RAISE " : "CLEAR ") +
                           d->event_name + " " + resource);
            sai_adapter::report_hal_alarm(d->event_name, d->severity, action, description);
        }
    }

    unlink(HAL_EVENT_DEFAULT_SOCK);
    close(fd);
}

std::once_flag g_once;

}  // namespace

namespace hal_listener {

void start_listener()
{
    std::call_once(g_once, []() {
        std::thread(listener_loop).detach();
    });
}

}  // namespace hal_listener
