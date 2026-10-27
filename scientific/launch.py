"""Private Align worker entry point. Deliberately has no GUI or app launcher."""
import argparse
import json
from pathlib import Path
from brainana_registration import __version__


def main():
    parser = argparse.ArgumentParser(description='Brainana Align registration worker')
    parser.add_argument('--align-capabilities', action='store_true')
    parser.add_argument('--align-request', type=Path)
    parser.add_argument('--align-output', type=Path)
    parser.add_argument('--events', type=Path)
    parser.add_argument('--cancel', type=Path)
    parser.add_argument('--devices', action='store_true')
    parser.add_argument('--version', action='version', version=__version__)
    args = parser.parse_args()
    if args.align_capabilities:
        from brainana_registration.itk_registration import capabilities
        print(json.dumps(dict(protocol=1, version=__version__,
            runtime='brainana-align-registration', engine_version='0.8.4',
            actions=['register', 'itk-register', 'n4'],
            features=['binary-field-v1', 'direct-field-result', 'phase-profile', 'sampling-v1', 'itk-v1'],
            itk=capabilities(), max_working_voxels=2_000_000)))
        return 0
    if args.devices:
        from brainana_registration.engine import devices
        print(json.dumps(devices()))
        return 0
    if args.align_request:
        if args.align_output is None:
            parser.error('--align-output is required')
        from brainana_registration.align_bridge import run
        return run(args.align_request, args.align_output, args.events, args.cancel)
    parser.error('This private worker requires a registration request')


if __name__ == '__main__':
    raise SystemExit(main())
