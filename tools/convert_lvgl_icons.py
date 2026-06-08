#!/usr/bin/env python3

import argparse
import struct
import subprocess
from pathlib import Path


LV_IMG_CF_TRUE_COLOR_ALPHA = 5


def read_png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as source:
        header = source.read(24)

    if header[:8] != b"\x89PNG\r\n\x1a\n" or header[12:16] != b"IHDR":
        raise ValueError(f"{path} is not a PNG file")

    return struct.unpack(">II", header[16:24])


def output_stem(path: Path) -> str:
    if path.suffix.lower() == ".png":
        return path.stem

    # Handle source files accidentally named like "cursor_to_originpng".
    if path.name.lower().endswith("png"):
        return path.name[:-3]

    return path.stem


def convert_icon(source: Path, destination: Path) -> None:
    width, height = read_png_size(source)
    if width > 2047 or height > 2047:
        raise ValueError(f"{source} exceeds LVGL v8's 2047 px dimension limit")

    rgba = subprocess.run(
        ["convert", str(source), "-depth", "8", "rgba:-"],
        check=True,
        capture_output=True,
    ).stdout

    expected_size = width * height * 4
    if len(rgba) != expected_size:
        raise ValueError(
            f"{source} decoded to {len(rgba)} bytes; expected {expected_size}"
        )

    header = LV_IMG_CF_TRUE_COLOR_ALPHA | (width << 10) | (height << 21)
    pixels = bytearray(width * height * 3)

    for pixel_index in range(width * height):
        source_offset = pixel_index * 4
        destination_offset = pixel_index * 3
        red, green, blue, alpha = rgba[source_offset : source_offset + 4]
        rgb565 = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
        struct.pack_into("<HB", pixels, destination_offset, rgb565, alpha)

    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open("wb") as output:
        output.write(struct.pack("<I", header))
        output.write(pixels)

    print(f"{source} -> {destination} ({width}x{height}, RGB565A8)")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert PNG icons to LVGL v8 RGB565A8 binary images."
    )
    parser.add_argument("source", type=Path, help="Directory containing PNG files")
    parser.add_argument("destination", type=Path, help="Directory for generated .bin files")
    args = parser.parse_args()

    sources = sorted(path for path in args.source.iterdir() if path.is_file())
    if not sources:
        raise SystemExit(f"No files found in {args.source}")

    for source in sources:
        destination = args.destination / f"{output_stem(source)}.bin"
        convert_icon(source, destination)


if __name__ == "__main__":
    main()
