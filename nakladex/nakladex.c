#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/vector_plot.h"

// SETTINGS //
static const float ss =  4.0;    // Stitch pitch and distance of a stitch from the edge
static const float ro = 30.0;    // Corner radius of the piece
static const float rs = ro - ss; // Radius of the rounded stitching arc
static const float rh =  0.70;   // Radius of a stitching hole
static const float wt =  2 * ss; // Spacer wall thickness (outer edge to inner cutout)

static inline void stitch(plot_t *p, float x0, float y0, float x1, float y1, float pitch) {
    // Pitch-matched straight stitch: endpoints are exact, internal holes
    // distributed evenly to approximate the requested pitch.
    float dx = x1 - x0, dy = y1 - y0;
    float d = hypotf(dx, dy);
    int n = (int)roundf(d / pitch);
    if (n < 1) n = 1;
    for (int i = 0; i <= n; i++) {
        vp_circle(p, x0 + dx * i / n, y0 + dy * i / n, rh);
    }
}

static inline void stitch_arc(plot_t *p, float cx, float cy, float radius, float a1, float a2, float pitch) {
    // Pitch-matched arc stitch, same idea as stitch(). Angles in radians,
    // y down (SVG), so y = cy - radius*sin(angle).
    // Endpoints are skipped: they coincide with the straight stitches'
    // endpoints, which already own those holes.
    float len = radius * fabsf(a2 - a1);
    int n = (int)roundf(len / pitch);
    if (n < 1) n = 1;
    for (int i = 1; i < n; i++) {
        float angle = a1 + (a2 - a1) * i / n;
        vp_circle(p, cx + radius * cosf(angle), cy - radius * sinf(angle), rh);
    }
}

static void parse_args(int argc, char *argv[], float *w, float *l) {
    *w = 45.0f;
    *l = 145.0f;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            *w = atof(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            *l = atof(argv[++i]);
        }
    }
}

int main(int argc, char *argv[]) {

    float w, l;
    parse_args(argc, argv, &w, &l);

    // External dimensions: seam takes 2*ss per stitched edge
    float ext_w = w + 4 * ss;
    float ext_l = l + 2 * ss;

    char base[256], name[300];
    snprintf(base, sizeof(base), "nakladex_w%g_l%g", w, l);

    snprintf(name, sizeof(name), "%s_front.svg", base);
    plot_t p = create_svg(name, ext_w, ext_l);

    // Outline: rectangle with the bottom-right corner replaced by a 90deg arc
    vp_new_path             (&p,   0.0,   0.0);
    vp_line                 (&p,   0.0, ext_l);
    vp_line                 (&p, ext_w - ro, ext_l);
    vp_arc                  (&p, ext_w, ext_l - ro, ro, 1, 0);
    vp_line                 (&p, ext_w,   0.0);
    vp_close_path           (&p);

    // Stitching holes, ss from each stitched edge (side stitches run to the opening edge)
    stitch                  (&p,   ss,           0,        ss, ext_l - ss,   ss); // left
    stitch                  (&p,   ss, ext_l - ss, ext_w - ro, ext_l - ss,   ss); // bottom
    stitch_arc              (&p, ext_w - ro, ext_l - ro, rs, 3*M_PI/2, 2*M_PI, ss); // corner
    stitch                  (&p, ext_w - ss, ext_l - ro, ext_w - ss,         0, ss); // right

    close_plot(&p);
// ------------------------------------------------------------------------- //
    snprintf(name, sizeof(name), "%s_back.svg", base);
    p = create_svg(name, ext_w, ext_l);

    // Outline: identical to front, flipped horizontally
    // (rounded corner now on the bottom-left)
    vp_new_path             (&p, ext_w,   0.0);
    vp_line                 (&p, ext_w, ext_l);
    vp_line                 (&p,    ro, ext_l);
    vp_arc                  (&p,   0.0, ext_l - ro, ro, 0, 0);
    vp_line                 (&p,   0.0,   0.0);
    vp_close_path           (&p);

    // Stitching holes, mirror image of the front
    stitch                  (&p, ext_w - ss,           0, ext_w - ss, ext_l - ss, ss); // right
    stitch                  (&p, ext_w - ss, ext_l - ss,          ro, ext_l - ss, ss); // bottom
    stitch_arc              (&p,          ro, ext_l - ro, rs, 3*M_PI/2,     M_PI, ss); // corner
    stitch                  (&p,          ss, ext_l - ro,           ss,         0, ss); // left

    close_plot(&p);
// ------------------------------------------------------------------------- //
    snprintf(name, sizeof(name), "%s_spacer.svg", base);
    p = create_svg(name, ext_w, ext_l);

    // Outline: same outer shape, interior hollowed out and open at the top (U)
    vp_new_path             (&p,   0.0,   0.0);
    vp_line                 (&p,   0.0, ext_l);
    vp_line                 (&p, ext_w - ro, ext_l);
    vp_arc                  (&p, ext_w, ext_l - ro, ro, 1, 0);
    vp_line                 (&p, ext_w,   0.0);
    vp_line                 (&p, ext_w - wt,   0.0);
    vp_line                 (&p, ext_w - wt, ext_l - ro);
    vp_arc                  (&p, ext_w - ro, ext_l - wt, ro - wt, 0, 0);
    vp_line                 (&p, wt, ext_l - wt);
    vp_line                 (&p, wt,   0.0);
    vp_close_path           (&p);

    // Stitching holes, ss from each stitched edge (side stitches run to the opening edge)
    stitch                  (&p,   ss,           0,        ss, ext_l - ss,   ss); // left
    stitch                  (&p,   ss, ext_l - ss, ext_w - ro, ext_l - ss,   ss); // bottom
    stitch_arc              (&p, ext_w - ro, ext_l - ro, rs, 3*M_PI/2, 2*M_PI, ss); // corner
    stitch                  (&p, ext_w - ss, ext_l - ro, ext_w - ss,         0, ss); // right

    close_plot(&p);
    return 0;
}
