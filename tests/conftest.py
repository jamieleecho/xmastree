from pathlib import Path
from typing import Mapping

import pytest

from xmastree_utilities.png_to_mvicon import (
    LCHEntry,
    PaletteEntry,
    _parse_input_palette,
    _rgb_palette_to_lch_palette,
    _truncate_palette_to_bits_per_pixel,
)


@pytest.fixture(scope="module")
def fixtures_path() -> Path:
    """Return the path to the fixtures directory."""
    return Path(__file__).parent / "fixtures"


@pytest.fixture(scope="module")
def images_path(fixtures_path: Path) -> Path:
    """Return the path to the images directory."""
    return fixtures_path / "images"


@pytest.fixture(scope="module")
def xmastree_image_path(images_path: Path) -> Path:
    """Return the path to the xmastree images directory."""
    return images_path / "xmastree.png"


@pytest.fixture(scope="module")
def palettes_path(fixtures_path: Path) -> Path:
    """Return the path to the palettes directory."""
    return fixtures_path / "palettes"


@pytest.fixture(scope="module")
def default_palette_path(palettes_path: Path) -> Path:
    """Return the path to the default palette file."""
    return palettes_path / "default-palette.txt"


@pytest.fixture(scope="module")
def default_palette(default_palette_path: Path) -> Mapping[int, PaletteEntry]:
    """Return the default palette as a mapping from index to PaletteEntry."""
    return _parse_input_palette(palette_path=str(default_palette_path))


@pytest.fixture(scope="module")
def truncated_palette(
    default_palette: Mapping[int, PaletteEntry],
) -> Mapping[int, PaletteEntry]:
    """Return the truncated palette to the given bits per pixel."""
    return _truncate_palette_to_bits_per_pixel(
        rgb_palette=default_palette,
        bits_per_pixel=2,
    )


@pytest.fixture(scope="module")
def default_lch_palette(
    truncated_palette: Mapping[int, PaletteEntry],
) -> Mapping[int, LCHEntry]:
    """Return the default palette converted to LCHEntry."""
    return _rgb_palette_to_lch_palette(rgb_palette=truncated_palette)
