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

/* HAL event catalog: maps a HAL event_id to an alarm. Every event_name MUST be
 * registered in the eventd profile (device default.json) or eventd drops it.
 * These are environmental alarms with no SAI object of their own. */
struct EnvAlarmDef {
    int                       event_id;
    const char               *event_name;
    sai_otn_alarm_severity_t  severity;
    const char               *resource_prefix;  // + instance, recorded in the description
    const char               *text;
};

const EnvAlarmDef *lookup(int event_id)
{
    static const EnvAlarmDef kTable[] = {
        { 1, "Fan Failure",          SAI_OTN_ALARM_SEVERITY_MAJOR,    "FAN",  "Fan failure detected"       },
        { 2, "Over Temperature",     SAI_OTN_ALARM_SEVERITY_MAJOR,    "TEMP", "Temperature over threshold" },
        { 3, "Power Supply Failure", SAI_OTN_ALARM_SEVERITY_CRITICAL, "PSU",  "Power supply failure"       },
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
        if (n < (ssize_t)(hdrLen + sizeof(struct hal_event))) {
            logger::warn("hal_event: short message ignored");
            continue;
        }

        const hal_msg_header *hdr = reinterpret_cast<const hal_msg_header *>(buf);
        if (ntohl(hdr->msg_type) != HAL_MSG_TYPE_DEV_EVENT) {
            logger::warn("hal_event: unknown msg_type ignored");
            continue;
        }

        struct hal_event ev;
        memcpy(&ev, buf + hdrLen, sizeof(ev));

        const EnvAlarmDef *d = lookup(ev.event_id);
        if (d == nullptr) {
            logger::warn("hal_event: no mapping for event_id " + std::to_string(ev.event_id));
            continue;
        }

        sai_otn_alarm_action_t action =
            ev.action ? SAI_OTN_ALARM_ACTION_RAISE : SAI_OTN_ALARM_ACTION_CLEAR;
        std::string resource = std::string(d->resource_prefix) + std::to_string(ev.instance);
        std::string description = std::string(d->text) + " (" + resource + ")";

        logger::notice(std::string("hal_event: ") + (ev.action ? "RAISE " : "CLEAR ") +
                       d->event_name + " " + resource);

        sai_adapter::report_hal_alarm(d->event_name, d->severity, action, description);
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
