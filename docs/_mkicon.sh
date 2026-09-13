#!/usr/bin/env bash
# Temporary helper: rasterize favicon.svg into PNG + ICO for crawlers that
# don't support SVG favicons (Google results, link previews).
set -e
cd "$(dirname "$0")"

echo "Checking available converters..."
HAVE_RSVG=$(command -v rsvg-convert || true)
HAVE_CONVERT=$(command -v convert || true)
HAVE_MAGICK=$(command -v magick || true)
echo "rsvg-convert: ${HAVE_RSVG:-no}"
echo "convert:      ${HAVE_CONVERT:-no}"
echo "magick:       ${HAVE_MAGICK:-no}"

# Render a big PNG first (from SVG), then downscale to the sizes we need.
if [ -n "$HAVE_RSVG" ]; then
    rsvg-convert -w 512 -h 512 favicon.svg -o favicon-512.png
elif [ -n "$HAVE_MAGICK" ]; then
    magick -background none favicon.svg -resize 512x512 favicon-512.png
elif [ -n "$HAVE_CONVERT" ]; then
    convert -background none favicon.svg -resize 512x512 favicon-512.png
else
    echo "NO_CONVERTER"
    exit 3
fi

# Standard sizes from the 512 master.
CONV="${HAVE_MAGICK:-$HAVE_CONVERT}"
"$CONV" favicon-512.png -resize 180x180 apple-touch-icon.png
"$CONV" favicon-512.png -resize 32x32 favicon-32.png
"$CONV" favicon-512.png -resize 16x16 favicon-16.png
# Multi-size .ico (16+32+48) for classic favicon slots / crawlers.
"$CONV" favicon-512.png -define icon:auto-resize=48,32,16 favicon.ico

echo "Done. Produced:"
ls -la favicon.ico apple-touch-icon.png favicon-16.png favicon-32.png favicon-512.png
