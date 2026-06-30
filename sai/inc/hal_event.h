#ifndef HAL_EVENT_H
#define HAL_EVENT_H

#include <stdint.h>

/* HAL-event message format: the contract between a HAL event source and
 * libsai-otn. On real hardware the HAL/cmptctrld sends these; on otn-kvm a test
 * generator stands in. libsai-otn receives them and reports the corresponding
 * alarm through the normal SAI alarm-notification path.
 *
 * Framed envelope:  [ hal_msg_header (32 B) ][ payload ][ checksum (1 B) ]
 * Multi-byte header fields are network byte order. The checksum byte is
 * reserved but not yet computed/validated.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_EVENT_DEFAULT_SOCK "/var/tmp/alarm_hal_event.sock"

#define HAL_MSG_MAGIC            (0xAA55EE77u)
#define HAL_MSG_VERSION          (0x01)
#define HAL_MSG_MAX_LEN          (1024)
#define HAL_MSG_HDR_RSVD_LEN     (16)

/* message types (header.msg_type) */
#define HAL_MSG_TYPE_DEV_EVENT   (0x00010001u)

/* 32-byte fixed header. Multi-byte fields are network byte order on the wire. */
typedef struct {
    uint32_t magic;        /* HAL_MSG_MAGIC                       */
    uint8_t  version;      /* HAL_MSG_VERSION                     */
    uint8_t  seq;          /* sequence number (unused, 0)         */
    uint8_t  slot;         /* slot id (unused, 0)                 */
    uint8_t  rsvd0;        /* reserved                            */
    uint32_t msg_type;     /* HAL_MSG_TYPE_*                      */
    uint16_t status;       /* status flags (unused, 0)            */
    uint16_t payload_len;  /* payload bytes following this header */
    uint8_t  rsvd[HAL_MSG_HDR_RSVD_LEN];
} hal_msg_header;

/* dev-event payload: self-describing so the receiver needs no device lookup.
 *   event_id : which condition (maps to an alarm in libsai-otn's catalog)
 *   instance : affected unit index (e.g. fan/psu number)
 *   action   : 1 = RAISE, 0 = CLEAR
 */
struct hal_event {
    int event_id;
    int instance;
    int action;
};

#ifdef __cplusplus
}
#endif

#endif /* HAL_EVENT_H */
