#!/usr/bin/env python3
"""
Simple TMX -> CSV exporter.
Usage:
    python3 tools/tmx_to_csv.py path/to/level1.tmx [output.csv]

It extracts the <data encoding="csv"> content from the first layer and writes it to the output file.
This preserves the exact GIDs as in the TMX (no offsets/changes).
"""
import sys
import xml.etree.ElementTree as ET


def tmx_to_csv(tmx_path, out_path=None):
    tree = ET.parse(tmx_path)
    root = tree.getroot()

    # Find first <layer>/<data> with encoding="csv"
    layer = root.find('layer')
    if layer is None:
        raise SystemExit('No <layer> found in TMX')

    data = layer.find('data')
    if data is None:
        raise SystemExit('No <data> found in first layer')

    encoding = data.get('encoding')
    if encoding != 'csv':
        raise SystemExit(f"Layer data is not encoded as CSV (found: {encoding})")

    csv_text = data.text.strip() if data.text else ''

    if out_path is None:
        # default: same dirname, same basename .csv
        out_path = tmx_path.rsplit('.', 1)[0] + '.csv'

    with open(out_path, 'w', encoding='utf-8') as f:
        f.write(csv_text + '\n')

    print(f'Wrote CSV to: {out_path}')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: tmx_to_csv.py map.tmx [out.csv]')
        sys.exit(1)
    tmx = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else None
    tmx_to_csv(tmx, out)
