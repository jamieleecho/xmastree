#!/usr/bin/env python3
import argparse
import re
from collections.abc import Mapping, Sequence
from dataclasses import dataclass

import numpy as np
from colormath.color_conversions import convert_color
from colormath.color_objects import LCHabColor, sRGBColor
from PIL import Image

ENV_FILE_REGEX = re.compile(
    r"^\s*((\*.*)|(\*?([A-Za-z0-9_]+)\s*=\s*([A-Za-z0-9_/\-,]*)\s*))$"
)  # https://regex101.com/r/RhHXrC/1
PALET_FILE_PERMISSIVE_REGEX = re.compile(
    r"^.*PALET.*$"
)  # https://regex101.com/r/aG26vG/1
PALET_FILE_REGEX = re.compile(
    r"^\s*((PALET(\d+))\s*=\s*(\d)\s*,\s*(\d)\s*,\s*(\d)\s*)$"
)  # https://regex101.com/r/NoC5d9/1


@dataclass
class PaletteEntry:
    r: int
    g: int
    b: int


@dataclass
class LCHEntry:
    luminence: float
    chroma: float
    hue: float

    @classmethod
    def from_palette_entry(cls, *, palette_entry: PaletteEntry) -> "LCHEntry":
        srgb = sRGBColor(
            palette_entry.r / 3.0, palette_entry.g / 3.0, palette_entry.b / 3.0
        )
        lch = convert_color(srgb, LCHabColor)
        return cls(luminence=lch.lch_l, chroma=lch.lch_c, hue=lch.lch_h)

    @classmethod
    def from_rgb(cls, r: int, g: int, b: int) -> "LCHEntry":
        srgb = sRGBColor(r / 255.0, g / 255.0, b / 255.0)
        lch = convert_color(srgb, LCHabColor)
        return cls(luminence=lch.lch_l, chroma=lch.lch_c, hue=lch.lch_h)


def _parse_input_palette(palette_path: str) -> Mapping[int, PaletteEntry]:
    with open(palette_path, "r") as file:
        env_file_contents = file.readlines()
    for line in env_file_contents:
        match = ENV_FILE_REGEX.match(line)
        if line and not match:
            raise ValueError(f"Invalid line in palette file {palette_path}: {line}")

    palette_match_permissive = [
        line for line in env_file_contents if PALET_FILE_PERMISSIVE_REGEX.match(line)
    ]
    palette_index_matches = [
        PALET_FILE_REGEX.match(line) for line in palette_match_permissive
    ]
    bad_line_index = None
    try:
        bad_line_index = palette_index_matches.index(None)
    except ValueError:
        pass
    if bad_line_index is not None:
        raise ValueError(
            f"Invalid PALET line in palette file {palette_path}: "
            f"{palette_match_permissive[bad_line_index]}"
        )

    palette_index_to_entry = {
        int(match.group(3)): PaletteEntry(
            r=int(match.group(4)),
            g=int(match.group(5)),
            b=int(match.group(6)),
        )
        for match in palette_index_matches
        if match
    }
    if len(palette_index_to_entry) != len(palette_index_matches):
        raise ValueError(
            f"Duplicate PALET indices found in palette file {palette_path}."
        )
    if any(index < 0 or index > 15 for index in palette_index_to_entry.keys()):
        raise ValueError(
            f"PALET indices must be between 0 and 15 in palette file {palette_path}."
        )
    if any(
        entry.r > 3 or entry.g > 3 or entry.b > 3
        for entry in palette_index_to_entry.values()
    ):
        raise ValueError(
            f"PALET RGB values out of range in palette file {palette_path}."
        )

    return palette_index_to_entry


def _rgb_palette_to_lch_palette(
    rgb_palette: Mapping[int, PaletteEntry],
) -> Mapping[int, LCHEntry]:
    lch_palette = {
        index: LCHEntry.from_palette_entry(palette_entry=entry)
        for index, entry in rgb_palette.items()
    }
    return lch_palette


def _truncate_palette_to_bits_per_pixel(
    *,
    rgb_palette: Mapping[int, PaletteEntry],
    bits_per_pixel: int,
) -> Mapping[int, PaletteEntry]:
    max_palette_size = 2**bits_per_pixel
    truncated_palette = {
        index: entry for index, entry in rgb_palette.items() if index < max_palette_size
    }
    return truncated_palette


def _load_png_image_as_2darray(
    image_path: str,
) -> list[list[tuple[int, int, int, int]]]:
    image = Image.open(image_path).convert("RGBA")
    pixel_data = list(image.getdata())
    width, height = image.size
    pixel_2d_array = [pixel_data[i * width : (i + 1) * width] for i in range(height)]
    return pixel_2d_array


def _convert_png_2d_array_to_2d_palette_indices(
    png_2d_array: list[list[tuple[int, int, int, int]]],
    lch_palette: Mapping[int, LCHEntry],
) -> list[list[int]]:
    height = len(png_2d_array)
    width = len(png_2d_array[0]) if height > 0 else 0
    png_palette_indices_2d_array = [[0 for _ in range(width)] for _ in range(height)]

    for y in range(height):
        for x in range(width):
            r, g, b, a = png_2d_array[y][x]
            pixel_lch = LCHEntry.from_rgb(r, g, b)

            best_index = -1
            best_distance = float("inf")
            for index, palette_lch in lch_palette.items():
                distance = (
                    (pixel_lch.luminence - palette_lch.luminence) ** 2
                    + (pixel_lch.chroma - palette_lch.chroma) ** 2
                    + (pixel_lch.hue - palette_lch.hue) ** 2
                )
                if distance < best_distance:
                    best_distance = distance
                    best_index = index
            png_palette_indices_2d_array[y][x] = best_index

    return png_palette_indices_2d_array


def _write_mvicon_file(
    *,
    output_icon_path: str,
    png_palette_indices_2d_array: list[list[int]],
    bits_per_pixel: int,  # 1, 2 or 4
) -> None:
    height = len(png_palette_indices_2d_array)
    width = len(png_palette_indices_2d_array[0]) if height > 0 else 0

    with open(output_icon_path, "wb") as file:
        pixels_per_byte = 8 // bits_per_pixel
        row_bytes = width // pixels_per_byte

        for y in range(height):
            row_data = bytearray(row_bytes)
            for x in range(width):
                palette_index = png_palette_indices_2d_array[y][x]
                byte_index = x // pixels_per_byte
                row_data[byte_index] = (
                    row_data[byte_index] << bits_per_pixel
                ) | palette_index
            file.write(row_data)


def _create_mvicon_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Convert a 24x24 PNG image to a Multi Vue icon file."
    )
    parser.add_argument("input_png", type=str, help="Path to the input PNG image file.")
    parser.add_argument(
        "input_palette",
        type=str,
        help="Path to the input palette file is in the Multi Vue env.file format.",
    )
    parser.add_argument("output_icon", type=str, help="Path to the output icon file.")
    return parser


@dataclass
class IndexedImage:
    image: list[list[int]]
    rgb_palette: Mapping[int, PaletteEntry]


def _reduce_image_colors(
    *,
    input_png_path: str,
    input_palette_path: str,
    bits_per_pixel: int,
) -> IndexedImage:
    rgb_palette = _parse_input_palette(input_palette_path)
    truncated_rgb_palette = _truncate_palette_to_bits_per_pixel(
        rgb_palette=rgb_palette,
        bits_per_pixel=bits_per_pixel,
    )
    lch_palette = _rgb_palette_to_lch_palette(truncated_rgb_palette)
    png_2d_array = _load_png_image_as_2darray(input_png_path)
    png_palette_indices_2d_array = _convert_png_2d_array_to_2d_palette_indices(
        png_2d_array, lch_palette
    )
    return IndexedImage(
        rgb_palette=truncated_rgb_palette, image=png_palette_indices_2d_array
    )


def convert_png_to_mvicon(
    *,
    input_png_path: str,
    input_palette_path: str,
    output_icon_path: str,
    bits_per_pixel: int,
) -> None:
    indexed_image = _reduce_image_colors(
        input_png_path=input_png_path,
        input_palette_path=input_palette_path,
        bits_per_pixel=bits_per_pixel,
    )
    _write_mvicon_file(
        output_icon_path=output_icon_path,
        png_palette_indices_2d_array=indexed_image.image,
        bits_per_pixel=bits_per_pixel,
    )


def png_to_mvicon(args: Sequence[str] | None = None) -> None:
    parser = _create_mvicon_arg_parser()
    parsed_args = parser.parse_args(args)
    convert_png_to_mvicon(
        input_png_path=parsed_args.input_png,
        input_palette_path=parsed_args.input_palette,
        output_icon_path=parsed_args.output_icon,
        bits_per_pixel=2,
    )


def indexed_image_to_image(indexed_image: IndexedImage) -> Image:
    def two_to_8_bit(b: int):
        return (b + 1) * 85

    def palette_entry_to_8_bit(p: PaletteEntry) -> tuple[int, int, int]:
        return (
            two_to_8_bit(p.r),
            two_to_8_bit(p.g),
            two_to_8_bit(p.b),
        )

    rgb_image = [
        [palette_entry_to_8_bit(indexed_image.rgb_palette[pixel]) for pixel in row]
        for row in indexed_image.image
    ]

    return Image.fromarray(np.array(rgb_image, dtype=np.uint8))


def convert_png_to_coco_png(
    *,
    input_png_path: str,
    input_palette_path: str,
    output_png_path: str,
    bits_per_pixel: int,
) -> None:
    indexed_image = _reduce_image_colors(
        input_png_path=input_png_path,
        input_palette_path=input_palette_path,
        bits_per_pixel=bits_per_pixel,
    )
    image = indexed_image_to_image(
        indexed_image=indexed_image,
    )
    image.save(output_png_path, format="PNG")


def _create_png_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Convert a PNG image to a CoCo 3 palette PNG."
    )
    parser.add_argument("input_png", type=str, help="Path to the input PNG image file.")
    parser.add_argument(
        "input_palette",
        type=str,
        help="Path to the input palette file is in the Multi Vue env.file format.",
    )
    parser.add_argument("output_png", type=str, help="Path to the output PNG file.")
    return parser


def png_to_coco_png(args: Sequence[str] | None = None) -> None:
    parser = _create_png_arg_parser()
    parsed_args = parser.parse_args(args)
    convert_png_to_coco_png(
        input_png_path=parsed_args.input_png,
        input_palette_path=parsed_args.input_palette,
        output_png_path=parsed_args.output_png,
        bits_per_pixel=4,
    )
