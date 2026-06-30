// HAL-event generator -- stand-in for the HAL event source on otn-kvm.
//
// On real hardware the HAL/cmptctrld emits these events; otn-kvm has none, so
// this tool frames one HAL event and sends it to libsai-otn's listener socket.
// libsai-otn then reports the alarm through the SAI notification path
// (send_alarm_event_data -> syncd -> orchagent -> eventd -> show alarm).
//
// Usage:  hal-event-generator [socket-path] [event-id] [instance] [action]
//   action : 1 = RAISE (default), 0 = CLEAR

#include "hal_event.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : HAL_EVENT_DEFAULT_SOCK;

    struct hal_event payload;
    payload.event_id = (argc > 2) ? atoi(argv[2]) : 1;
    payload.instance = (argc > 3) ? atoi(argv[3]) : 0;
    payload.action   = (argc > 4) ? atoi(argv[4]) : 1;   // default RAISE

    // Frame: [32-byte header][payload][1-byte checksum]
    unsigned char buf[sizeof(hal_msg_header) + sizeof(payload) + 1];
    memset(buf, 0, sizeof(buf));

    hal_msg_header *hdr = reinterpret_cast<hal_msg_header *>(buf);
    hdr->magic       = htonl(HAL_MSG_MAGIC);
    hdr->version     = HAL_MSG_VERSION;
    hdr->msg_type    = htonl(HAL_MSG_TYPE_DEV_EVENT);
    hdr->status      = htons(0);
    hdr->payload_len = htons(static_cast<uint16_t>(sizeof(payload)));

    memcpy(buf + sizeof(hal_msg_header), &payload, sizeof(payload));
    // checksum byte left 0 (reserved, not validated yet)
    size_t msgLen = sizeof(hal_msg_header) + sizeof(payload) + 1;

    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) { perror("[hal-event-generator] socket"); return 1; }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (sendto(fd, buf, msgLen, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[hal-event-generator] sendto");
        close(fd);
        return 1;
    }

    printf("[hal-event-generator] sent HAL event: event_id=%d instance=%d action=%s (%zu bytes) -> %s\n",
           payload.event_id, payload.instance, payload.action ? "RAISE" : "CLEAR", msgLen, path);
    close(fd);
    return 0;
}
