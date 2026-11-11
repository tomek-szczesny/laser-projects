# Laser Projects

A collection of personal laser-cutter project generators and design files.

This repository includes C (Cairo-based) vector generators, SVG/Boxes.py-based design projects, a shared plotting helper library, and per-project processing scripts.

## Repository layout

- `common/` — shared code used by C-based projects
  - `vector_plot.h` — lightweight Cairo SVG/PDF plotting helpers (`plot_t`, `create_svg`, `vp_*` drawing primitives)
- `smaczex/` — parametric generator for a pouch for dog snacks (C)
- `bateriex/` — parametric battery organizer board generator (C)
- `nakladex/` — parametric generator of cases for clip-on sunglasses (and other flat things) (C)
- `sock-jig/` — parametric sock-jig outline generator (C)
- `szufladex/` — modular storage/drawer system (SVG/Boxes.py-based design files)
- `rozen-box/` — case for my PC, based on a laptop motherboard (SVG/Boxes.py-based design files)

Each project directory keeps its own `process.sh` (and `makefile` where applicable).

## Prerequisites

- GCC and Cairo development libraries
- [la suite](https://github.com/tomek-szczesny/la) (CLI tools for G-code processing) available in `PATH`

See each project's `README.md` for project-specific details.
