# HAL event → alarm (otn-kvm)

Lets the platform raise **environmental alarms** (fan, temperature, power supply
— things with no SAI object of their own) through the **same path** as the
optical SAI alarms, so there is one alarm pipeline, owned by orchagent:

```
HAL event source ──socket──▶ libsai-otn listener ──▶ report_hal_alarm
   (hal-event-generator           (hal_event_listener.cpp)        │
    on otn-kvm)                                                   ▼
                                          send_alarm_event_data → otn_alarm_event_ntf (syncd)
                                          → redis notif → orchagent (publishAlarmEvent)
                                          → eventd → EVENT_DB → show alarm
```

On real hardware the HAL/cmptctrld is the event source; on otn-kvm
`hal-event-generator` stands in. The listener runs **inside libsai-otn** (syncd's
address space) so it can invoke the registered notification callback directly —
it does **not** publish to eventd itself (orchagent owns that).

## Pieces

| File | Role |
|---|---|
| `inc/hal_event.h` | wire contract: `hal_msg_header` + `hal_event {event_id, instance, action}` |
| `src/hal_event_listener.cpp` | listener thread + event catalog (event_id → name/severity); calls `report_hal_alarm` |
| `sai_adapter::report_hal_alarm` (`sai_otn_device.cpp`) | emits via `send_alarm_event_data` using the **switch** object as the resource |
| `test/hal_event_generator.cpp` | `hal-event-generator` — the HAL stand-in/injector |

The listener starts from `init_switch()` (once the switch object exists).

## Event catalog (`hal_event_listener.cpp`)

| event_id | event_name | severity | resource detail |
|---|---|---|---|
| 1 | Fan Failure | MAJOR | FAN`<instance>` |
| 2 | Over Temperature | MAJOR | TEMP`<instance>` |
| 3 | Power Supply Failure | CRITICAL | PSU`<instance>` |

Resource (object) is the **switch** (env entities have no SAI object); the unit
detail (e.g. `FAN0`) goes in the alarm description. Every `event_name` must be
registered in the eventd profile —
`device/molex/x86_64-otn-kvm_x86_64-r0/default.json` — or eventd drops it.

## Verify on otn-kvm

```bash
# inside the syncd container (after the switch is up)
hal-event-generator /var/tmp/alarm_hal_event.sock 1 0 1   # Fan Failure, FAN0, RAISE
show alarm                                                # expect a "Fan Failure" row
hal-event-generator /var/tmp/alarm_hal_event.sock 1 0 0   # CLEAR
```

## Notes / TODO

- The 1-byte checksum in `hal_msg_header` is reserved, not yet validated.
- The HAL event carries the RAISE/CLEAR action, so the listener emits directly
  (no edge detection). If a source ever sends repeats, add dedup in
  `report_hal_alarm` (the SAI `evaluate_alarm` edge logic is the model).
