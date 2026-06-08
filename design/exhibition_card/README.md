# Slash Exhibition Card

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

Current pixel size: 1145 x 720 px including bleed. Trimmed size: 1075 x 650 px.
