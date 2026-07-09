#!/usr/bin/env python3
# Generates the libsai-otn HAL alarm table (kTable rows) from the canonical
# catalog, so the listener never hand-duplicates otn_alarm_catalog.json.
#
#   gen_alarm_table.py <otn_alarm_catalog.json> <out.inc>
#
# Output is an initializer-list fragment included inside the AlarmDef kTable[]
# array in hal_event_listener.cpp. Each alarm expands to a RAISE row (raise_id,
# is_clear=false) and, if it has a distinct clear_id, a CLEAR row (clear_id,
# is_clear=true) -- so the listener derives raise-vs-clear from the event_id
# (Shasta convention). eventd's displayed severity comes from the device profile
# (default.json); the SAI enum here is best-effort (WARNING -> MINOR, INFO).

import json
import sys

SEV = {
    'CRITICAL':      'SAI_OTN_ALARM_SEVERITY_CRITICAL',
    'MAJOR':         'SAI_OTN_ALARM_SEVERITY_MAJOR',
    'MINOR':         'SAI_OTN_ALARM_SEVERITY_MINOR',
    'WARNING':       'SAI_OTN_ALARM_SEVERITY_MINOR',
    'INFORMATIONAL': 'SAI_OTN_ALARM_SEVERITY_INFO',
}

# sai_object_type string -> SAI extensions enum. Absent (environmental) alarms
# are objectless and use (sai_object_type_extensions_t)0 (reported vs the switch).
SAIOBJ = {
    'oa':   'SAI_OBJECT_TYPE_OTN_OA',
    'voa':  'SAI_OBJECT_TYPE_OTN_ATTENUATOR',
    'wss':  'SAI_OBJECT_TYPE_OTN_WSS',
    'osc':  'SAI_OBJECT_TYPE_OTN_OSC',
    'ocm':  'SAI_OBJECT_TYPE_OTN_OCM',
    'otdr': 'SAI_OBJECT_TYPE_OTN_OTDR',
}


def esc(s):
    return s.replace('\\', '\\\\').replace('"', '\\"')


def main():
    cat_path, out_path = sys.argv[1], sys.argv[2]
    alarms = json.load(open(cat_path))['alarms']

    rows = []
    for a in alarms:
        rp = esc(a.get('resource_prefix', ''))
        name = esc(a['name'])
        sev = SEV[a['severity']]
        obj = SAIOBJ.get(a.get('sai_object_type', ''), '(sai_object_type_extensions_t)0')
        txt = esc(a['text'])
        rows.append((a['raise_id'], 'false', obj, name, sev, rp, txt))
        if a.get('clear_id'):
            rows.append((a['clear_id'], 'true', obj, name, sev, rp, txt))

    out = ['/* AUTO-GENERATED from otn_alarm_catalog.json by gen_alarm_table.py -- DO NOT EDIT. */']
    for event_id, is_clear, obj, name, sev, rp, txt in sorted(rows):
        out.append('    {{ {id}, {ic}, {obj}, "{name}", {sev}, "{rp}", "{txt}" }},'.format(
            id=event_id, ic=is_clear, obj=obj, name=name, sev=sev, rp=rp, txt=txt))
    with open(out_path, 'w') as f:
        f.write('\n'.join(out) + '\n')


if __name__ == '__main__':
    main()
