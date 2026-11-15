"""Stage the Fused Sanctuary description against an extracted client Spell.dbc."""
import argparse
from pathlib import Path
import struct

OLD = b"Increases defense rating by $s1 and armor by $s2 while equipped."
NEW = b"Increases defense rating by ${1+$FLOOR(($PL-1)/3)} and armor by ${5*$PL} while equipped."


def stage(source, destination):
    data = bytearray(source.read_bytes())
    magic, count, fields, stride, size = struct.unpack_from("<4s4I", data)
    pool = 20 + count * stride
    if magic != b"WDBC" or fields * 4 != stride or fields <= 170 or pool + size != len(data):
        raise ValueError("Unexpected Spell.dbc layout")
    rows = [pos for pos in range(20, pool, stride) if struct.unpack_from("<I", data, pos)[0] == 1999520]
    if len(rows) != 1:
        raise ValueError("Expected one Fused Sanctuary row")
    field = rows[0] + 170 * 4
    start = pool + struct.unpack_from("<I", data, field)[0]
    if data[start:data.index(0, start)] != OLD:
        raise ValueError("Unexpected existing tooltip")
    struct.pack_into("<I", data, field, size)
    struct.pack_into("<I", data, 16, size + len(NEW) + 1)
    data.extend(NEW + b"\0")
    with destination.open("xb") as output:
        output.write(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    stage(args.source, args.destination)
