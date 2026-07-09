#!/usr/bin/env python3
# Generates (or checks) the device eventd profile default.json from the canonical
# alarm catalog, so the eventd event list is never hand-duplicated.
#
#   gen_default_json.py <otn_alarm_catalog.json> <default.json>            # rewrite
#   gen_default_json.py --check <otn_alarm_catalog.json> <default.json>    # verify in sync (exit 1 on drift)
#
# eventd needs every alarm name registered here (with its severity) or it drops
# the event; the severity shown by `show alarm` comes from this file. Non-catalog
# system events already present (e.g. SYSTEM_STATE) are preserved, in order,
# ahead of the catalog alarms.

import collections
import json
import sys


# Non-alarm system events to keep as-is (everything else in events[] is fully
# catalog-managed -- an event NOT in the catalog and NOT here is dropped).
SYSTEM_EVENTS = {"SYSTEM_STATE"}


def build_events(catalog_path, default_path):
    catalog = json.load(open(catalog_path))
    current = json.load(open(default_path), object_pairs_hook=collections.OrderedDict)

    # preserve only known system events (not arbitrary non-catalog entries, or a
    # removed alarm would linger); the catalog is authoritative for all alarms.
    preserved = [e for e in current.get('events', []) if e.get('name') in SYSTEM_EVENTS]

    events = list(preserved)
    for a in catalog['alarms']:
        events.append(collections.OrderedDict([
            ('name', a['name']),
            ('severity', a['severity']),
            ('enable', 'true'),
            ('message', a['text']),
        ]))

    out = collections.OrderedDict()
    if '__README__' in current:
        out['__README__'] = current['__README__']
    out['events'] = events
    return out


def dumps(obj):
    return json.dumps(obj, indent=4) + '\n'


def main():
    args = sys.argv[1:]
    check = False
    if args and args[0] == '--check':
        check = True
        args = args[1:]
    catalog_path, default_path = args[0], args[1]

    generated = dumps(build_events(catalog_path, default_path))

    if check:
        current = open(default_path).read()
        if current != generated:
            sys.stderr.write(
                "default.json is OUT OF SYNC with otn_alarm_catalog.json.\n"
                "Regenerate: python3 gen_default_json.py %s %s\n" % (catalog_path, default_path))
            sys.exit(1)
        print("default.json in sync with catalog")
        return

    open(default_path, 'w').write(generated)
    print("wrote %s from %s" % (default_path, catalog_path))


if __name__ == '__main__':
    main()
