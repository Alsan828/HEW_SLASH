from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageEnhance, ImageFilter, ImageFont, ImageOps


ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent
INPUT = OUT / "input"
ASSET = ROOT / "DX21_05_Init" / "asset"

DPI = 300
TRIM_MM = (91, 55)
BLEED_MM = 3
SAFE_MM = 5

CARD = {
    "game_title": "Slash",
    "jp_title": "トライスラッシュ",
    "genre": "2D HIGH-SPEED SLASH ACTION",
    "tagline_jp": "限られた斬撃で、突破しろ。",
    "tagline_en": "READ THE ATTACK. SPEND THE SLASH. BREAK THROUGH.",
    "event": "HEW 2026 EXHIBITION DEMO",
    "edition": "EXHIBITION CARD / NO. 001",
    "name": "________________",
    "team": "嘘つきケーキ",
    "school": "HAL大阪",
    "booth": "BOOTH: ______",
    "contact": "____________________",
    "web": "https://example.com/hew-slash",
    "x": "@hew_slash",
    "qr_caption": "PLAY DEMO",
}

FRONT_SOURCE_CANDIDATES = [
    INPUT / "front.png",
    OUT / "front.png",
]
QR_SOURCE_CANDIDATES = [
    INPUT / "qr.png",
    OUT / "qr.png",
]


def first_existing(paths: list[Path]) -> Path | None:
    for path in paths:
        if path.exists():
            return path
    return None


def qr_source() -> Path | None:
    explicit = first_existing(QR_SOURCE_CANDIDATES)
    if explicit:
        return explicit
    candidates = sorted(OUT.glob("*qr*.png")) + sorted(OUT.glob("*QR*.png")) + sorted(INPUT.glob("*qr*.png")) + sorted(INPUT.glob("*QR*.png"))
    return candidates[0] if candidates else None

COLORS = {
    "ink": (7, 6, 14),
    "deep": (18, 4, 33),
    "violet": (76, 28, 204),
    "violet_2": (132, 56, 255),
    "magenta": (239, 0, 198),
    "red": (255, 9, 48),
    "paper": (244, 241, 255),
    "muted": (177, 163, 216),
    "dim": (77, 63, 111),
}


def mm_to_px(mm: float) -> int:
    return int(round(mm / 25.4 * DPI))


TRIM_W, TRIM_H = (mm_to_px(TRIM_MM[0]), mm_to_px(TRIM_MM[1]))
BLEED = mm_to_px(BLEED_MM)
SAFE = mm_to_px(SAFE_MM)
W, H = TRIM_W + BLEED * 2, TRIM_H + BLEED * 2


def font(path: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(path, size)


FONT_JP = r"C:\Windows\Fonts\NotoSansJP-VF.ttf"
FONT_SC = r"C:\Windows\Fonts\NotoSansSC-VF.ttf"
FONT_EN = r"C:\Windows\Fonts\bahnschrift.ttf"
FONT_MONO = r"C:\Windows\Fonts\consolab.ttf"


def fit_cover(img: Image.Image, size: tuple[int, int]) -> Image.Image:
    return ImageOps.fit(img.convert("RGBA"), size, method=Image.Resampling.LANCZOS, centering=(0.5, 0.5))


def fit_contain(img: Image.Image, box: tuple[int, int]) -> Image.Image:
    src = img.convert("RGBA")
    ratio = min(box[0] / src.width, box[1] / src.height)
    return src.resize((int(src.width * ratio), int(src.height * ratio)), Image.Resampling.LANCZOS)


def overlay_rect(img: Image.Image, rect: tuple[int, int, int, int], color: tuple[int, int, int], alpha: int) -> None:
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    ImageDraw.Draw(layer).rectangle(rect, fill=(*color, alpha))
    img.alpha_composite(layer)


def draw_text(
    draw: ImageDraw.ImageDraw,
    xy: tuple[int, int],
    text: str,
    fnt: ImageFont.FreeTypeFont,
    fill: tuple[int, int, int] | tuple[int, int, int, int],
    anchor: str = "la",
    stroke_width: int = 0,
    stroke_fill: tuple[int, int, int] | tuple[int, int, int, int] | None = None,
) -> None:
    draw.text(xy, text, font=fnt, fill=fill, anchor=anchor, stroke_width=stroke_width, stroke_fill=stroke_fill)


def text_size(draw: ImageDraw.ImageDraw, text: str, fnt: ImageFont.FreeTypeFont) -> tuple[int, int]:
    box = draw.textbbox((0, 0), text, font=fnt)
    return box[2] - box[0], box[3] - box[1]


def paste_alpha(base: Image.Image, asset_path: Path, box: tuple[int, int, int, int], opacity: float = 1.0) -> None:
    src = Image.open(asset_path).convert("RGBA")
    fitted = fit_contain(src, (box[2] - box[0], box[3] - box[1]))
    if opacity < 1:
        alpha = fitted.getchannel("A").point(lambda p: int(p * opacity))
        fitted.putalpha(alpha)
    base.alpha_composite(fitted, (box[0] + (box[2] - box[0] - fitted.width) // 2, box[1] + (box[3] - box[1] - fitted.height) // 2))


def draw_slash(draw: ImageDraw.ImageDraw, start: tuple[int, int], end: tuple[int, int], width: int, color: tuple[int, int, int], alpha: int = 255) -> None:
    sx, sy = start
    ex, ey = end
    dx, dy = ex - sx, ey - sy
    length = math.hypot(dx, dy) or 1
    nx, ny = -dy / length, dx / length
    taper = width * 0.35
    points = [
        (sx + nx * taper, sy + ny * taper),
        (ex + nx * width, ey + ny * width),
        (ex - nx * taper, ey - ny * taper),
        (sx - nx * width, sy - ny * width),
    ]
    draw.polygon(points, fill=(*color, alpha))


def add_noise(img: Image.Image, opacity: int = 18) -> None:
    rng = random.Random(21)
    noise = Image.new("RGBA", img.size, (0, 0, 0, 0))
    px = noise.load()
    for y in range(0, img.height, 2):
        for x in range(0, img.width, 2):
            v = rng.randrange(0, opacity)
            px[x, y] = (255, 255, 255, v)
    img.alpha_composite(noise)


def add_scanlines(img: Image.Image, color: tuple[int, int, int] = COLORS["magenta"], every: int = 9, alpha: int = 14) -> None:
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    for y in range(0, img.height, every):
        d.line((0, y, img.width, y), fill=(*color, alpha), width=1)
    img.alpha_composite(layer)


def draw_pixel_corner(draw: ImageDraw.ImageDraw, x: int, y: int, sx: int, sy: int, color: tuple[int, int, int], step: int = 9) -> None:
    draw.line((x, y, x + sx * step * 4, y), fill=color, width=step)
    draw.line((x, y, x, y + sy * step * 4), fill=color, width=step)
    draw.line((x + sx * step * 2, y + sy * step, x + sx * step * 4, y + sy * step), fill=color, width=step)
    draw.line((x + sx * step, y + sy * step * 2, x + sx * step, y + sy * step * 4), fill=color, width=step)


def make_front(source: Path | None = None) -> Image.Image:
    source = source or first_existing(FRONT_SOURCE_CANDIDATES)
    if source:
        front = Image.open(source).convert("RGBA")
        img = fit_cover(front, (W, H)).filter(ImageFilter.GaussianBlur(10))
        overlay_rect(img, (0, 0, W, H), COLORS["ink"], 48)
        art = fit_contain(front, (TRIM_W, TRIM_H))
        art_x = BLEED + (TRIM_W - art.width) // 2
        art_y = BLEED + (TRIM_H - art.height) // 2
        shadow = Image.new("RGBA", img.size, (0, 0, 0, 0))
        sd = ImageDraw.Draw(shadow)
        sd.rectangle((art_x - 10, art_y - 10, art_x + art.width + 10, art_y + art.height + 10), fill=(0, 0, 0, 120))
        shadow = shadow.filter(ImageFilter.GaussianBlur(8))
        img.alpha_composite(shadow)
        img.alpha_composite(art, (art_x, art_y))
        add_noise(img, opacity=5)
        return img

    title = Image.open(ASSET / "UI" / "title" / "title1.png")
    img = fit_cover(title, (W, H))
    img = ImageEnhance.Contrast(img).enhance(1.12)
    img = ImageEnhance.Color(img).enhance(1.08)

    overlay_rect(img, (0, 0, W, H), COLORS["ink"], 32)
    add_scanlines(img, every=8, alpha=11)
    add_noise(img, opacity=14)

    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    draw_slash(d, (-70, H - 132), (W - 230, H - 216), 16, COLORS["red"], 212)
    draw_slash(d, (W - 560, 82), (W + 82, 38), 6, COLORS["magenta"], 190)
    draw_slash(d, (122, H + 30), (W - 366, H - 128), 4, COLORS["violet_2"], 150)
    img.alpha_composite(layer)

    d = ImageDraw.Draw(img)
    left = BLEED + SAFE
    top = BLEED + SAFE
    right = W - BLEED - SAFE
    bottom = H - BLEED - SAFE

    draw_text(d, (left, bottom - 122), CARD["event"], font(FONT_MONO, 21), COLORS["paper"], "la")
    draw_text(d, (left, bottom - 86), CARD["genre"], font(FONT_EN, 24), COLORS["magenta"], "la")
    draw_text(d, (right, top + 5), CARD["edition"], font(FONT_MONO, 17), COLORS["muted"], "ra")
    draw_text(d, (right, bottom - 52), CARD["tagline_en"], font(FONT_EN, 18), COLORS["paper"], "ra")

    stroke = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sd = ImageDraw.Draw(stroke)
    draw_pixel_corner(sd, BLEED + 20, BLEED + 20, 1, 1, COLORS["magenta"], 6)
    draw_pixel_corner(sd, W - BLEED - 20, H - BLEED - 20, -1, -1, COLORS["violet_2"], 6)
    img.alpha_composite(stroke)

    return img


def draw_qr_placeholder(img: Image.Image, box: tuple[int, int, int, int]) -> None:
    x0, y0, x1, y1 = box
    size = min(x1 - x0, y1 - y0)
    x0 += (x1 - x0 - size) // 2
    y0 += (y1 - y0 - size) // 2
    x1, y1 = x0 + size, y0 + size

    d = ImageDraw.Draw(img)
    d.rounded_rectangle((x0 - 14, y0 - 14, x1 + 14, y1 + 14), radius=8, fill=COLORS["paper"], outline=COLORS["magenta"], width=5)
    module = size // 17
    pad = (size - module * 17) // 2
    ox, oy = x0 + pad, y0 + pad

    pattern = [
        "11111110010111111",
        "10000010100100001",
        "10111010110101101",
        "10111010000101101",
        "10111011110101101",
        "10000010010100001",
        "11111110101111111",
        "00000000100000000",
        "10110111101100101",
        "01001000100111010",
        "11101110111000111",
        "00010010100101100",
        "11111110101010111",
        "10000010110100010",
        "10111010100111101",
        "10000010011101000",
        "11111110110100111",
    ]
    for row, line in enumerate(pattern):
        for col, value in enumerate(line):
            if value == "1":
                d.rectangle(
                    (ox + col * module, oy + row * module, ox + (col + 1) * module - 1, oy + (row + 1) * module - 1),
                    fill=COLORS["ink"],
                )

    label = "QR"
    f = font(FONT_EN, 38)
    tw, th = text_size(d, label, f)
    d.rounded_rectangle((x0 + size // 2 - tw // 2 - 15, y0 + size // 2 - th // 2 - 11, x0 + size // 2 + tw // 2 + 15, y0 + size // 2 + th // 2 + 13), radius=6, fill=COLORS["paper"])
    draw_text(d, (x0 + size // 2, y0 + size // 2 - 2), label, f, COLORS["ink"], "mm")


def draw_qr_image(img: Image.Image, box: tuple[int, int, int, int]) -> None:
    source = qr_source()
    if not source:
        draw_qr_placeholder(img, box)
        return

    x0, y0, x1, y1 = box
    size = min(x1 - x0, y1 - y0)
    x0 += (x1 - x0 - size) // 2
    y0 += (y1 - y0 - size) // 2
    x1, y1 = x0 + size, y0 + size

    d = ImageDraw.Draw(img)
    d.rounded_rectangle((x0 - 16, y0 - 16, x1 + 16, y1 + 16), radius=8, fill=COLORS["paper"], outline=COLORS["magenta"], width=5)
    qr = Image.open(source).convert("RGBA")
    qr = ImageOps.contain(qr, (size, size), method=Image.Resampling.NEAREST)
    img.alpha_composite(qr, (x0 + (size - qr.width) // 2, y0 + (size - qr.height) // 2))


def draw_feature(draw: ImageDraw.ImageDraw, x: int, y: int, number: str, title: str, body: str) -> None:
    draw.rounded_rectangle((x, y, x + 44, y + 44), radius=4, fill=COLORS["magenta"])
    draw_text(draw, (x + 22, y + 21), number, font(FONT_MONO, 25), COLORS["paper"], "mm")
    draw_text(draw, (x + 61, y - 3), title, font(FONT_JP, 27), COLORS["paper"], "la")
    draw_text(draw, (x + 61, y + 31), body, font(FONT_EN, 17), COLORS["muted"], "la")


def make_back() -> Image.Image:
    img = Image.new("RGBA", (W, H), COLORS["ink"] + (255,))
    bg = fit_cover(Image.open(ASSET / "background" / "1-6background.png").convert("RGBA"), (W, H))
    bg = ImageEnhance.Color(bg).enhance(0.7)
    bg = ImageEnhance.Brightness(bg).enhance(0.45)
    img.alpha_composite(bg)
    overlay_rect(img, (0, 0, W, H), COLORS["deep"], 148)

    d = ImageDraw.Draw(img)
    draw_slash(d, (-90, 150), (W + 72, 76), 18, COLORS["red"], 164)
    draw_slash(d, (W - 120, H - 88), (160, H + 28), 10, COLORS["violet_2"], 92)
    draw_slash(d, (W - 390, 0), (W - 56, H + 36), 3, COLORS["magenta"], 96)

    paste_alpha(img, ASSET / "effect" / "slash_flash4.png", (W - 560, H - 330, W - 60, H - 116), opacity=0.16)
    add_scanlines(img, color=COLORS["violet_2"], every=10, alpha=10)
    add_noise(img, opacity=16)

    d = ImageDraw.Draw(img)
    left = BLEED + SAFE
    top = BLEED + SAFE
    right = W - BLEED - SAFE
    bottom = H - BLEED - SAFE

    title_x = left + 6
    title_y = top + 24
    draw_text(d, (title_x, title_y), CARD["game_title"].upper(), font(FONT_EN, 86), COLORS["paper"], "la", stroke_width=2, stroke_fill=COLORS["red"])
    draw_text(d, (title_x + 7, title_y + 94), CARD["jp_title"], font(FONT_JP, 27), COLORS["paper"], "la")
    draw_text(d, (title_x + 7, title_y + 144), CARD["genre"], font(FONT_EN, 25), COLORS["muted"], "la")

    line_y = title_y + 216
    d.line((title_x + 8, line_y, title_x + 446, line_y), fill=COLORS["magenta"], width=3)
    draw_text(d, (title_x + 8, line_y + 48), "EXHIBITION DEMO", font(FONT_MONO, 27), COLORS["paper"], "la")
    draw_text(d, (title_x + 8, line_y + 88), CARD["edition"], font(FONT_MONO, 19), COLORS["muted"], "la")

    info_x = title_x + 8
    info_y = bottom - 156
    info_w = 566
    d.rounded_rectangle((info_x - 8, info_y - 8, info_x + info_w, info_y + 136), radius=6, fill=(12, 9, 23, 180), outline=COLORS["dim"], width=2)
    label_font = font(FONT_MONO, 16)
    value_font = font(FONT_JP, 24)
    value_font_small = font(FONT_EN, 20)
    rows = [
        ("NAME", CARD["name"], COLORS["paper"], value_font_small),
        ("MAIL", CARD["contact"], COLORS["paper"], value_font_small),
        ("TEAM", CARD["team"], COLORS["paper"], value_font),
        ("SCHOOL", CARD["school"], COLORS["muted"], font(FONT_JP, 21)),
    ]
    for i, (label, value, color, row_font) in enumerate(rows):
        y = info_y + 14 + i * 33
        draw_text(d, (info_x + 10, y + 6), label, label_font, COLORS["dim"], "lt")
        draw_text(d, (info_x + 112, y), value, row_font, color, "lt")

    panel_x0 = right - 330
    panel_y0 = top + 70
    panel_x1 = right
    panel_y1 = bottom - 24
    d.rounded_rectangle((panel_x0, panel_y0, panel_x1, panel_y1), radius=8, fill=(14, 10, 25, 214), outline=COLORS["violet_2"], width=3)

    qr_box = (panel_x0 + 56, panel_y0 + 48, panel_x1 - 56, panel_y0 + 286)
    draw_qr_image(img, qr_box)
    draw_text(d, ((panel_x0 + panel_x1) // 2, panel_y0 + 336), CARD["qr_caption"], font(FONT_EN, 30), COLORS["paper"], "mm")
    draw_text(d, ((panel_x0 + panel_x1) // 2, panel_y0 + 374), "SCAN FOR DEMO", font(FONT_MONO, 16), COLORS["muted"], "mm")
    draw_text(d, ((panel_x0 + panel_x1) // 2, panel_y1 - 38), "HEW 2026", font(FONT_MONO, 18), COLORS["magenta"], "mm")

    draw_pixel_corner(d, BLEED + 20, BLEED + 20, 1, 1, COLORS["violet_2"], 6)
    draw_pixel_corner(d, W - BLEED - 20, H - BLEED - 20, -1, -1, COLORS["magenta"], 6)

    return img


def add_guides(img: Image.Image) -> Image.Image:
    proof = img.copy()
    d = ImageDraw.Draw(proof)
    trim = (BLEED, BLEED, W - BLEED, H - BLEED)
    safe = (BLEED + SAFE, BLEED + SAFE, W - BLEED - SAFE, H - BLEED - SAFE)
    d.rectangle(trim, outline=(0, 255, 255, 190), width=2)
    d.rectangle(safe, outline=(255, 255, 0, 190), width=2)
    mark = 20
    for x, y, sx, sy in [
        (BLEED, BLEED, -1, -1),
        (W - BLEED, BLEED, 1, -1),
        (BLEED, H - BLEED, -1, 1),
        (W - BLEED, H - BLEED, 1, 1),
    ]:
        d.line((x + sx * mark, y, x + sx * 4, y), fill=(0, 255, 255, 220), width=2)
        d.line((x, y + sy * mark, x, y + sy * 4), fill=(0, 255, 255, 220), width=2)
    return proof


def trim_preview(img: Image.Image) -> Image.Image:
    return img.crop((BLEED, BLEED, W - BLEED, H - BLEED))


def save_all(front: Image.Image, back: Image.Image, prefix: str = "slash_card") -> None:
    assets = {
        f"{prefix}_front_print.png": front,
        f"{prefix}_back_print.png": back,
        f"{prefix}_front_proof.png": add_guides(front),
        f"{prefix}_back_proof.png": add_guides(back),
        f"{prefix}_front_trim_preview.png": trim_preview(front),
        f"{prefix}_back_trim_preview.png": trim_preview(back),
    }
    for name, image in assets.items():
        image.convert("RGB").save(OUT / name, dpi=(DPI, DPI), quality=95)

    front.convert("CMYK").save(OUT / f"{prefix}_front_print_cmyk.tif", dpi=(DPI, DPI), compression="tiff_lzw")
    back.convert("CMYK").save(OUT / f"{prefix}_back_print_cmyk.tif", dpi=(DPI, DPI), compression="tiff_lzw")

    front_rgb = front.convert("RGB")
    back_rgb = back.convert("RGB")
    front_rgb.save(OUT / f"{prefix}_print_two_sides.pdf", "PDF", resolution=DPI, save_all=True, append_images=[back_rgb])

    gap = 70
    preview = Image.new("RGB", (TRIM_W * 2 + gap, TRIM_H + 110), (25, 23, 32))
    preview.paste(trim_preview(front).convert("RGB"), (0, 56))
    preview.paste(trim_preview(back).convert("RGB"), (TRIM_W + gap, 56))
    pd = ImageDraw.Draw(preview)
    draw_text(pd, (0, 22), "FRONT", font(FONT_EN, 28), COLORS["paper"], "la")
    draw_text(pd, (TRIM_W + gap, 22), "BACK", font(FONT_EN, 28), COLORS["paper"], "la")
    preview.save(OUT / f"{prefix}_mockup.png", dpi=(DPI, DPI), quality=95)


def write_readme() -> None:
    text = f"""# Slash Exhibition Card

91 x 55 mm Japanese business-card format, plus 3 mm bleed on every side.

## Custom source images

Put exported source images here before running `python make_exhibition_card.py`:

- `input/front.png`: the new front design. It will be fit inside the trim area, with bleed extended from the art.
- `input/qr.png`: the real QR code for the back side.
- Alternatively, put `front.png` and any `*qr*.png` in this folder.

If either file is missing, the script falls back to the generated front or a QR placeholder.

## Use for print

- `slash_card_front_print.png` and `slash_card_back_print.png`: RGB PNG, 300 dpi, with bleed, no guides.
- `slash_card_front_print_cmyk.tif` and `slash_card_back_print_cmyk.tif`: CMYK TIFF, 300 dpi, with bleed, no guides.
- `slash_card_print_two_sides.pdf`: two-page PDF for quick review or print-shop upload.

## Use for checking

- `slash_card_front_proof.png` and `slash_card_back_proof.png`: cyan trim line, yellow safe line.
- `slash_card_front_trim_preview.png` and `slash_card_back_trim_preview.png`: trimmed final face.
- `slash_card_mockup.png`: front/back side-by-side preview.
- `slash_card_v2_*`: the same output set for `front2.png` when that file exists.

## Replace before printing

Edit `CARD` near the top of `make_exhibition_card.py`, especially:

- `booth`
- `name`
- `contact`
- `web`
- `x`
- the QR file at `input/qr.png`

Current pixel size: {W} x {H} px including bleed. Trimmed size: {TRIM_W} x {TRIM_H} px.
"""
    (OUT / "README.md").write_text(text, encoding="utf-8")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    INPUT.mkdir(parents=True, exist_ok=True)
    front = make_front()
    back = make_back()
    save_all(front, back)
    front2_source = first_existing([INPUT / "front2.png", OUT / "front2.png"])
    if front2_source:
        save_all(make_front(front2_source), back, "slash_card_v2")
    write_readme()


if __name__ == "__main__":
    main()
