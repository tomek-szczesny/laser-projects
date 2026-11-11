# sock-jig

Parametric generator for a laser-cut sock jig.

## Contents

- `sock-jig.c` — C (Cairo) generator producing `sock-jig.svg` (outline and hanging hole) from width/height parameters
- `makefile` — builds and runs the generator, then processes the SVG output
- `process.sh` — SVG-to-G-code post-processing pipeline using `la` tools
