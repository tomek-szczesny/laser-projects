#!/bin/bash

# Exit if la is not installed
for prog in la_svg2g la_dedup la_bounds la_rot la_pos la_dup la_opt la_g2pdf; do
    if ! command -v "$prog" &> /dev/null; then
        echo "Error: la is not installed (missing $prog)" >&2
        exit 1
    fi
done

TEMP1=$(mktemp)
dirbase="$(dirname -- "$1")"
name="$(basename -- "$1")"
if [[ "$name" == *.* ]]; then
  ext="${name##*.]}"
  base="${name%.*]}"
  outname="${base}.gcode"
else
  base="$name"
  outname="${name}.gcode"
fi
outfile="${dirbase}/${outname}"

cat "$1" | la_svg2g -s 1000 -f 400 | la_dedup | la_rot -minx | la_pos -x 10 -y 10 | \
    la_dup -n 4 | la_opt > "$TEMP1"

echo "; Processed with LA tools" > "$outfile"
echo "; GRBL device profile, absolute coords" >> "$outfile"
cat "$TEMP1" | la_bounds >> "$outfile"
cat >> "$outfile" << EOF
G00         ; Rapid Travel
G17         ; XY plane
G40         ; Cancel Cutter compensation (?)
G21         ; Set units to mm
G54         ; Work Offset
G90         ; Absolute positioning
M4
M8
EOF

cat "$TEMP1" >> "$outfile"

cat >> "$outfile" << EOF
G90
M9
G1S0
M5
G90
G0 X0 Y0
M2

EOF

cat "$outfile" | la_g2pdf -a -g 10 -c -f > "${dirbase}/${base}.preview.pdf"

rm "$TEMP1"
