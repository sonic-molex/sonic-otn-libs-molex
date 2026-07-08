# HAL event → alarm (otn-kvm)

Raises OTN alarms through the **same path** as the native optical SAI alarms, so
there is one alarm pipeline owned by orchagent:

```
otn-vhal (virtual HAL) ──HAL socket──▶ libsai-otn listener (hal_event_listener.cpp)
                                                    │
              object-bound  → report_hal_object_alarm (resolve instance -> SAI object_id)
              environmental → report_hal_alarm (switch object; "<PREFIX><instance>")
                                                    │
                     send_alarm_event_data → otn_alarm_event_ntf (syncd)
                     → redis notif → orchagent (publishAlarmEvent; object_id -> resource name)
                     → eventd → EVENT_DB → show alarm
```

On real hardware the HAL/cmptctrld emits these events; on otn-kvm there is no
hardware, so **`otn-vhal`** is the KVM virtual HAL that stands in, driven by the
user. The listener runs **inside libsai-otn** (syncd's address space) and invokes
the registered notification callback directly — it does not publish to eventd
itself (orchagent owns that). `otn-vhal` is stateless; the "currently raised" view
is EVENT_DB (`show alarm`).

## Wire contract — Shasta-compatible

Frame is byte-compatible with a Shasta OOP message (`util-lib/util_msg.h`):

```
[ hal_msg_header (32B) ][ payload (8B) ][ checksum (1B) ]
```

- **Header** mirrors `ST_OOP_MSG_HEADER` (magic `0xAA55EE77`, `msg_type` = OID
  `DEF_OID_DEV_EVENT` `0x00010001`); multi-byte fields are network order.
- **Payload** is Shasta's DEV_EVENT payload — two native-order ints
  `struct hal_event { int event_id; int sub_event_id; }`:
  - `event_id` selects the alarm **and** raise-vs-clear — raise and clear are
    **distinct ids** (Shasta convention, e.g. `DEF_EVENT_ID_OA_COMM_FAIL 23` /
    `_CLEAR_OA_COMM_FAIL 24`). Mapped to an alarm by the catalog.
  - `sub_event_id` = the affected **instance** (fan/psu/sensor number, or optical
    object index). No action bit.
- **Checksum** = `hal_msg_checksum` = Shasta `__UTIL_CalcMsgSum` (`0xFF` minus the
  8-bit sum of header+payload); written on send, validated on receive.

A genuine Shasta HAL DEV_EVENT frame decodes here unchanged.

## Two alarm sources, one encoding

The catalog carries both, distinguished only by their `event_id` range:

- **`hal-event`** — real Shasta HAL dev-events, using the actual `DEF_EVENT_ID_*`
  raise/clear pairs from `util-lib/util_event.h` (KVM-relevant subset: OA/WSS/OSC/
  OTDR/OCM comm-fails). A real HAL frame lands here. Shasta's notify-only triggers
  (FAN/PWR/CPU) are excluded — they have no clear id and aren't standalone alarms.
- **OLS / derived** — thermal, fan, power, optical (incl. amp LOS/ORL/APR). These
  are OpenROADM alarms *derived* on real hardware (read from HAL state, not sent
  as dev-events), so they get **reserved ids ≥ 4096** and are injectable on the
  KVM via `otn-vhal`.

Alarm attributes follow the **OpenROADM** model: `alarm_type` (communication /
qualityOfService / processingError / equipment / environmental) and
`resource_type` (device / port / circuit-pack / …), per `dsc-lib/DSC_alarm.h`.

## Pieces

| File | Role |
|---|---|
| `sai/alarm/otn_alarm_catalog.json` | **single source of truth** (raise_id, clear_id, name, severity, alarm_type, resource_type, resource_prefix, category, text). Everything else is generated from it. |
| `sai/alarm/gen_alarm_table.py` / `gen_default_json.py` | generators for the listener `kTable` and the device `default.json` |
| `inc/hal_event.h` | wire contract: `hal_msg_header` + `hal_event {event_id, sub_event_id}` + `hal_msg_checksum` (Shasta-compatible) |
| `src/hal_event_listener.cpp` | listener thread; `event_id`→alarm (+ raise/clear via `is_clear`), `sub_event_id`=instance, calls `report_hal_alarm` |
| `sai_adapter::report_hal_alarm` (`sai_otn_device.cpp`) | environmental alarm — emits against the switch object |
| `sai_adapter::report_hal_object_alarm` (`sai_otn_device.cpp`) | object-bound alarm — resolves instance→SAI object_id (`get_object_id_by_type_index`) and emits against it, so orchagent maps it to the real resource name |
| `platform/OtnVirtualHal/otn_vhal.cpp` | **`otn-vhal`** — the KVM virtual HAL (`list` / `raise` / `clear`) |
| `platform/OtnVirtualHal/otn_alarm_catalog.{h,cpp}` | catalog loader for otn-vhal |

The listener starts from `init_switch()` (once the switch object exists).

## Single source of truth — generated artifacts

**Edit `sai/alarm/otn_alarm_catalog.json` and nothing else.** Derived:

1. `src/hal_event_listener.cpp` `kTable` → generated at build time by
   `alarm/gen_alarm_table.py` (CMake `add_custom_command`) into `otn_alarm_table.gen.inc`.
   Each alarm expands to a RAISE row and (if `clear_id`≠0) a CLEAR row.
2. `device/molex/x86_64-otn-kvm_x86_64-r0/default.json` → regenerate with
   `alarm/gen_default_json.py <catalog> <default.json>`; every `event_name` must be
   listed (with severity) or eventd drops it, and the shown severity comes from
   this file. `make check_default_json` (or CI) fails if it drifts.

## Verify on otn-kvm

```bash
# inside the syncd container (SAI socket is local to it)
otn-vhal list                                        # the catalog
otn-vhal raise "OA Communication Failure" --instance 0   # a real Shasta dev-event (id 23)
otn-vhal raise "Loss of Signal" --instance 1             # OLS-derived alarm (id 4136)
show alarm                                            # currently-raised (EVENT_DB)
otn-vhal clear "OA Communication Failure" --instance 0
```

## Notes / TODO

- Header, payload, and checksum are Shasta-compatible: a genuine Shasta HAL
  DEV_EVENT (`DEF_EVENT_ID_*` + instance) decodes and maps to an alarm here.
- Raise/clear come from **distinct event ids** (Shasta convention); the listener
  emits directly (no edge detection). Duplicate raise/clear is harmless — eventd
  keys the current alarm by `type-id|resource`.
- Only clearable Shasta dev-events (the `*_COMM_FAIL` raise/clear pairs) are
  included. Shasta's notify-only triggers (`FAN`/`PWR`/`CPU`) are excluded — they
  carry no clear id and are re-read signals, not standalone alarms; the fine
  fan/power alarms cover those clearably.
- `alarm_type` / `resource_type` are OpenROADM classification carried in the
  catalog. They are not yet propagated into the `otn-alarm` event params
  (type-id/resource/severity/text/action) — a future orchagent/eventd extension.
- Resource resolution: **object-bound** alarms (those with `sai_object_type`)
  attach to the instance-th SAI object of that type — orchagent maps the object_id
  to the real component name via its `vid2Name` table (e.g. `OA0-1`), so the
  `show alarm` resource is correct. `sub_event_id` (instance) indexes the objects
  of that type ordered by name (0 → `OA0-0`, 1 → `OA0-1`). **Environmental**
  alarms are objectless (reported against the switch) with `"<PREFIX><instance>"`
  in the description. Note: object resolution needs the SAI objects to exist
  (created by orchagent from config) — verifiable only on a running KVM, not in a
  standalone unit build.
