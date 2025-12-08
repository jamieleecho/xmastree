import pytest

from xmastree_utilities.png_to_mvicon import (
    ENV_FILE_REGEX,
    PALET_FILE_PERMISSIVE_REGEX,
    PALET_FILE_REGEX,
    LCHEntry,
    PaletteEntry,
)


def test_ENV_FILE_REGEX_invalid_line() -> None:
    invalid_line = "INVALID LINE"
    match = ENV_FILE_REGEX.match(invalid_line)
    assert match is None


@pytest.mark.parametrize(
    "test_line",
    [
        ("VAR=VALUE",),
        (" VAR = VALUE "),
        (""),
        ("   "),
        ("* this is a comment"),
        (" * "),
    ],
)
def test_ENV_FILE_REGEX_valid_line(
    test_line: str,
) -> None:
    valid_line = " VAR = VALUE"
    match = ENV_FILE_REGEX.match(valid_line)
    assert match is not None


@pytest.mark.parametrize(
    "test_line",
    [
        "PALET2=1,2,3",
        "  PALET2 = 1 , 2 , 3 ",
        "PALET16=0,0,0",
        "PALET0=3,3,3",
        "PALET=hello world",
    ],
)
def test_PALET_FILE_PERMISSIVE_REGEX_valid_line(
    test_line: str,
) -> None:
    match = PALET_FILE_PERMISSIVE_REGEX.match(test_line)
    assert match is not None


@pytest.mark.parametrize(
    "test_line",
    [
        "TEST=2",
        "*comment=ignore",
    ],
)
def test_PALET_FILE_PERMISSIVE_REGEX_invalid_line(test_line: str) -> None:
    match = PALET_FILE_PERMISSIVE_REGEX.match(test_line)
    assert match is None


@pytest.mark.parametrize(
    "test_line,expected_index,expected_red,expected_green,expected_blue",
    [
        ("PALET5=2,1,0", 5, 2, 1, 0),
        ("  PALET0 = 2 , 1 , 0 ", 0, 2, 1, 0),
        ("  PALET15 = 2 , 1 , 3 ", 15, 2, 1, 3),
        ("  PALET17 = 9 , 1 , 3 ", 17, 9, 1, 3),
        ("  PALET123 = 8, 1 , 3 ", 123, 8, 1, 3),
    ],
)
def test_PALET_FILE_REGEX_valid_line(
    test_line: str,
    expected_index: int,
    expected_red: int,
    expected_green: int,
    expected_blue: int,
) -> None:
    match = PALET_FILE_REGEX.match(test_line)
    assert match is not None


@pytest.mark.parametrize(
    "test_line",
    [
        "PALET9=21,1,0",
        "PALET2=1,1,20",
        "PALET4=1,21,0",
    ],
)
def test_PALET_FILE_REGEX_invalid_line(
    test_line: str,
) -> None:
    match = PALET_FILE_REGEX.match(test_line)
    assert match is None


@pytest.mark.parametrize(
    "entry,expected_lch_entry",
    [
        (PaletteEntry(r=0, g=0, b=0), LCHEntry(luminence=0.0, chroma=0.0, hue=360.0)),
        (
            PaletteEntry(r=3, g=3, b=3),
            LCHEntry(
                luminence=99.99998453333127,
                chroma=0.00857377398931557,
                hue=266.9285772201555,
            ),
        ),
        (
            PaletteEntry(r=3, g=0, b=0),
            LCHEntry(
                luminence=53.23896002513146,
                chroma=104.5490632458832,
                hue=39.998956384033,
            ),
        ),
    ],
)
def test_LCHEntry_from_rgb(
    entry: PaletteEntry,
    expected_lch_entry: LCHEntry,
) -> None:
    lch_entry = LCHEntry.from_palette_entry(palette_entry=entry)
    assert lch_entry == expected_lch_entry
