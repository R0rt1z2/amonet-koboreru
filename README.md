# amonet-koboreru

This is a Preloader exploit for some MediaTek-based Amazon devices, including Echo Shows, Fire Tablets, and possibly some Fire TV devices.

It provides persistent arbitrary code execution at the highest privilege level (EL3).

## How it works

MediaTek wraps most firmware partitions in a 512-byte header, and a single partition can chain several sub-images, each with its own header.

`tee1` is one of those. It holds ATF followed by TEE. The header carries the size and the load address, but it is not covered by the signature, and the Preloader copies the image to whatever address it names before checking that signature.

We give the TEE sub-image a load address of `0xFFFFFFFF`, which makes the Preloader fall back to a default that was never set and is just 0, and a size that reaches a bit past its own block device struct.

The copy therefore lands our payload in SRAM and replaces the struct's read method with a pointer to it.

That copy still has to get past `check_part_overlapped`, which keeps a single range called `lk_region` and is meant to stop one image from being loaded on top of another:

```c
for (i = 0; i < 4; i++) {  /* "lk", "lk_a", "lk_b", "uboot" */
    if (strcmp(name_table[i], hdr->name) == 0) {
        lk_region.start = start;
        lk_region.end   = start + hdr->dsize - 1;
        return; /* claims the range, nothing to check */
    }
}

if (overlaps(start, end, lk_region))
    panic();
```

An image whose header name is in that table claims the range it is about to occupy and returns right there, and anything else is checked against the claimed range and takes the Preloader down if it lands inside it.

Those four are partition names, `lk` on most devices and `lk_a` or `lk_b` on the ones with slots, but what the function actually compares them against is the name in the image header, which is `LK` on all of them.

Nothing ever matches, so nothing ever claims a range and `lk_region` keeps its initial value of zero. Our load starts at zero too, which counts as inside it, so a sub-image still called `TEE` would panic here.

Calling it `lk` instead puts us on the branch that claims the range and returns, and the overlap test is never reached.

The struct is overwritten partway through that same transfer, and since the read method is fetched again for every block, the very next read of the image we are still loading already goes through our pointer.

The payload now runs with the Preloader's privileges.

It patches it in memory to accept unsigned Download Agents and unsigned ATF/TEE images, clears the RPMB anti-rollback counters, restores the block device and loads the real LK from another partition.

It then returns an error, which the Preloader takes as a failed read, so it gives up on `tee1` and loads the real ATF and TEE from `tee2`. All of it lives in flash, so it comes back on every boot.

## Building

Building requires `arm-none-eabi-gcc`, GNU Make, and Python 3 with [liblk](https://github.com/R0rt1z2/liblk).

You can build the payload for every device, or just one with `DEVICE=`:

```bash
make
make DEVICE=checkers
```

The result is written to `build/<device>/payload.bin`.

Then craft the TEE image, using a donor TEE from the device's stock firmware:

```bash
./create_tee_image.py <donor.img> build/checkers/payload.bin tz.img -d checkers
```

## Usage

> [!NOTE]
> Amazon patched this around 2022, so recent Preloaders are not vulnerable. You need to downgrade the Preloader to an older version first.
>
> To check whether a given Preloader is affected, grep it for `check_part_overlapped done`. If the string is there, it is most likely vulnerable.

Flash the malicious TEE image onto `tee1` and the original (donor) TEE image onto `tee2`.

On Amazon devices, you also need to ensure that the original LK is flashed to the LK partition.

With the exploit in place, you can load a custom TEE and a custom LK. Flash your custom LK to whatever partition the device configuration specifies (see `LK_PART_NAME`).

If you wish to replace TEE/ATF, flash your custom image to the `tee2` partition.

> [!CAUTION]
> Be careful about what you flash. If you damage `tee1` (which holds the malicious TZ image) or `lk` (which contains the original LK image), you could hard brick the device.

The exploit also provides an insecure Preloader USBDL mode, which allows you to flash partitions using tools like MTKClient.

To access this mode, on Echo devices at least, press and hold only the Volume Down and Mute (Power) keys with the USB cable connected while the device is booting.

This will cause the device to enter a Preloader USBDL loop, with which you can establish a handshake. Check `dmesg` and look for the following:

```
[  352.325834] usb 3-9: new high-speed USB device number 11 using xhci_hcd
[  352.449489] usb 3-9: New USB device found, idVendor=0e8d, idProduct=2000, bcdDevice= 1.00
[  352.449495] usb 3-9: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[  352.449496] usb 3-9: Product: MT8163 Preloader
[  352.449497] usb 3-9: Manufacturer: PWNED
```

## License

MIT, see [LICENSE](LICENSE). Copyright (c) 2026 Roger Ortiz, Ben Grisdale.