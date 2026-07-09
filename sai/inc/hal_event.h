#ifndef HAL_EVENT_H
#define HAL_EVENT_H

#include <stdint.h>

/* HAL-event message format: the contract between a HAL event source and
 * libsai-otn. On real hardware the HAL/cmptctrld sends these; on otn-kvm a test
 * generator stands in. libsai-otn receives them and reports the corresponding
 * alarm through the normal SAI alarm-notification path.
 *
 * Framed envelope:  [ hal_msg_header (32 B) ][ payload ][ checksum (1 B) ]
 * Multi-byte header fields are network byte order. The trailing checksum byte is
 * computed and validated (see hal_msg_checksum) using Shasta's __UTIL_CalcMsgSum
 * algorithm, so the header + checksum framing is byte-compatible with a Shasta
 * OOP message (ST_OOP_MSG_HEADER, OID DEF_OID_DEV_EVENT).
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

/* dev-event payload -- byte-compatible with Shasta's DEV_EVENT payload
 * (util-lib-cpp/util_event.cpp Event::event_send): two native-order ints.
 * Shasta convention:
 *   event_id     : which alarm AND raise-vs-clear -- raise and clear are DISTINCT
 *                  event ids (e.g. DEF_EVENT_ID_OA_COMM_FAIL / _CLEAR_OA_COMM_FAIL).
 *                  Mapped to an alarm by the catalog (otn_alarm_catalog.json).
 *   sub_event_id : the affected instance -- fan/psu/sensor number, or the optical
 *                  object index. (No action bit; action is carried by event_id.)
 */
struct hal_event {
    int event_id;
    int sub_event_id;
};

/* Frame checksum: 0xFF minus the 8-bit running sum of every byte preceding the
 * trailing checksum byte (i.e. the header + payload). Byte-for-byte identical to
 * Shasta's __UTIL_CalcMsgSum (util-lib/util_msg.c), so the sender/receiver here
 * and a genuine Shasta OOP frame validate each other. Sender writes it as the
 * final byte; receiver recomputes over (frame_len - 1) and compares. */
static inline uint8_t hal_msg_checksum(const void *buf, unsigned int len)
{
    const unsigned char *p = (const unsigned char *)buf;
    uint8_t sum = 0;
    unsigned int i;
    for (i = 0; i < len; ++i) {
        sum = (uint8_t)(sum + p[i]);
    }
    return (uint8_t)(0xFF - sum);
}

#ifdef __cplusplus
}
#endif

#endif /* HAL_EVENT_H */
