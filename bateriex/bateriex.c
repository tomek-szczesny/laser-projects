#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/vector_plot.h"

// SETTINGS //
static const float W      = 156.0;  // Board width
static const float PAD    =  10.0;  // Edge padding and spacing between rows
static const float HGAP   =   2.0;  // Minimum horizontal gap between batteries in a row
static const float CORNER =   3.0;  // Corner radius of the board outline

// Battery dims (standard sizes, no slack: the laser kerf provides clearance)
static const float CR_L = 20.5, CR_W = 3.2;  // CR2032 coin cell, upright on its edge
static const float AA_D = 14.5;              // AA diameter
static const float AAA_D = 10.5;             // AAA diameter
static const float B9_W = 26.5, B9_H = 17.5; // 9V footprint

// Quantities per row and number of rows
static const int CR_PER_ROW = 6,   CR_ROWS = 3;    // 18 CR2032
static const int AA_PER_ROW = 7,   AA_ROWS = 2;    // 14 AA
static const int AAA_PER_ROW = 10, AAA_ROWS = 2;   // 20 AAA
static const int B9_PER_ROW = 4,   B9_ROWS = 1;    // 4 9V

typedef struct {
    float cw, ch;  // cell width (X) and height (Y)
    int   per_row; // cells across the width
    int   rows;    // rows along the length
    int   circle;  // round hole when set
    float rad;     // corner radius for rounded-rect holes
} cell_t;

static void draw_row(plot_t *p, cell_t *c, float x, float y) {
    // Evenly spaced within the usable width (10mm margins stay fixed);
    // gap grows to fill the space, never below HGAP.
    float gap = (c->per_row > 1)
        ? (W - 2 * PAD - c->per_row * c->cw) / (c->per_row - 1)
        : 0.0f;
    for (int j = 0; j < c->per_row; j++) {
        if (c->circle) {
            vp_circle(p, x + c->cw / 2, y + c->cw / 2, c->cw / 2);
        } else if (c->rad > 0) {
            vp_rect_rnd(p, x, y, x + c->cw, y + c->ch, c->rad);
        } else {
            vp_rect(p, x, y, x + c->cw, y + c->ch);
        }
        x += c->cw + gap;
    }
}

int main(void) {
    // Shortest to tallest: CR2032, AAA, 9V, AA
    cell_t cells[] = {
        { CR_L,  CR_W,  CR_PER_ROW,  CR_ROWS,  0, 0.0 },
        { AAA_D, AAA_D, AAA_PER_ROW, AAA_ROWS, 1, 0.0 },
        { B9_W,  B9_H,  B9_PER_ROW,  B9_ROWS,  0, 0.5 },
        { AA_D,  AA_D,  AA_PER_ROW,  AA_ROWS,  1, 0.0 },
    };
    int n = (int)(sizeof(cells) / sizeof(cells[0]));

    float sum = 0;
    int rows = 0;
    for (int i = 0; i < n; i++) {
        sum += cells[i].rows * cells[i].ch;
        rows += cells[i].rows;
        if (cells[i].per_row > 1) {
            float gap = (W - 2 * PAD - cells[i].per_row * cells[i].cw) / (cells[i].per_row - 1);
            if (gap < HGAP)
                fprintf(stderr, "Warning: row gap %.2fmm below minimum %.1fmm\n", gap, HGAP);
        }
    }
    float L = sum + (rows + 1) * PAD;

    plot_t p = create_svg("bateriex.svg", W, L);

    vp_rect_rnd(&p, 0, 0, W, L, CORNER);

    float y = PAD;
    for (int i = 0; i < n; i++) {
        for (int r = 0; r < cells[i].rows; r++) {
            draw_row(&p, &cells[i], PAD, y);
            y += cells[i].ch + PAD;
        }
    }

    close_plot(&p);
// ------------------------------------------------------------------------- //
    // Riser bracket: same outer edge as the board, the battery field
    // replaced by a single rectangular cutout (10mm walls, rounded corners).
    plot_t r = create_svg("bateriex_riser.svg", W, L);

    vp_rect_rnd(&r, 0, 0, W, L, CORNER);
    vp_rect(&r, PAD, PAD, W - PAD, L - PAD);

    close_plot(&r);
    return 0;
}
