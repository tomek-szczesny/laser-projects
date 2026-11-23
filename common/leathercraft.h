#ifndef LEATHERCRAFT_H
#define LEATHERCRAFT_H

#include <stdbool.h>
#include <math.h>
#include "vector_plot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    float hole_d;   // hole diameter, mm
    float pitch;    // target stitch pitch along stitch line, mm
    float inset;    // stitch inset from edge, mm
} stitch_style_t;

static inline stitch_style_t stitch_style(float hole_d, float pitch, float inset) {
    return (stitch_style_t){hole_d, pitch, inset};
}

typedef struct {
    float cx, cy;
    float external_r;       // R
    float a0_deg, a1_deg;   // center angles, degrees, 0=+x, positive clockwise
    float theta_deg;        // abs(sweep), expected 0 < theta < 180

    stitch_style_t style;
    float stitch_r;         // R - style.inset

    int n;                  // intervals, >= 1
    int holes;              // n + 1
    float p_o;              // R * theta / n, outer/flat section spacing
    float p_s;              // stitch_r * theta / n, actual stitch pitch
    float co1;              // half flat tooth top
    float co2;              // ramp x
} bend_corner_t;

static inline float lc_deg_to_rad(float deg) {
    return deg * (float)M_PI / 180.0f;
}

static inline float lc_rad_to_deg(float rad) {
    return rad * 180.0f / (float)M_PI;
}

static inline void lc_dir(float angle_deg, float *dx, float *dy) {
    float a = lc_deg_to_rad(angle_deg);
    *dx = cosf(a);
    *dy = sinf(a); // y-down, positive clockwise
}

static inline void lc_left_normal(float angle_deg, float *dx, float *dy) {
    float a = lc_deg_to_rad(angle_deg);
    // left of forward(cos,sin) in y-down visual coords is (sin,-cos)
    *dx = sinf(a);
    *dy = -cosf(a);
}

// Returns number of holes placed.
static inline int stitch_line(plot_t *p, stitch_style_t s,
                              bool incl_start, bool incl_end,
                              float x0, float y0, float x1, float y1) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrtf(dx * dx + dy * dy);
    int n = (int)roundf(len / s.pitch);
    if (n < 1) n = 1;

    int placed = 0;
    float hr = s.hole_d * 0.5f;
    for (int i = 0; i <= n; i++) {
        if (i == 0 && !incl_start) continue;
        if (i == n && !incl_end) continue;
        float t = (float)i / (float)n;
        vp_circle(p, x0 + dx * t, y0 + dy * t, hr);
        placed++;
    }
    return placed;
}

// Returns number of holes placed. Angles in degrees, 0=+x, positive clockwise.
static inline int stitch_arc(plot_t *p, stitch_style_t s,
                             bool incl_start, bool incl_end,
                             float cx, float cy, float r,
                             float a0_deg, float a1_deg) {
    float delta = lc_deg_to_rad(a1_deg - a0_deg);
    float len = fabsf(r * delta);
    int n = (int)roundf(len / s.pitch);
    if (n < 1) n = 1;

    int placed = 0;
    float hr = s.hole_d * 0.5f;
    for (int i = 0; i <= n; i++) {
        if (i == 0 && !incl_start) continue;
        if (i == n && !incl_end) continue;
        float a = lc_deg_to_rad(a0_deg) + delta * (float)i / (float)n;
        vp_circle(p, cx + r * cosf(a), cy + r * sinf(a), hr);
        placed++;
    }
    return placed;
}

// Draws a line like vp_line, adds stitch holes on one side at style.inset.
// material_left: when looking from start to end, holes are on the left.
// Current point ends at (x,y).
static inline int lc_line_stitched(plot_t *p, stitch_style_t style,
                                   bool incl_start, bool incl_end,
                                   float x, float y, bool material_left) {
    double x0d, y0d;
    cairo_get_current_point(p->cr, &x0d, &y0d);
    float x0 = (float)x0d, y0 = (float)y0d;

    float dx = x - x0;
    float dy = y - y0;
    float len = sqrtf(dx * dx + dy * dy);
    int n = (int)roundf(len / style.pitch);
    if (n < 1) n = 1;

    // Left normal in y-down visual coords: (dy,-dx) normalized.
    float ndx = 0.0f, ndy = -1.0f;
    if (len > 0.0f) {
        ndx = dy / len;
        ndy = -dx / len;
    }
    if (!material_left) {
        ndx = -ndx;
        ndy = -ndy;
    }

    cairo_path_t *saved = cairo_copy_path(p->cr);

    int placed = 0;
    float hr = style.hole_d * 0.5f;
    for (int i = 0; i <= n; i++) {
        if (i == 0 && !incl_start) continue;
        if (i == n && !incl_end) continue;
        float t = (float)i / (float)n;
        float pxp = x0 + dx * t + ndx * style.inset;
        float pyp = y0 + dy * t + ndy * style.inset;
        vp_circle(p, pxp, pyp, hr);
        placed++;
    }

    cairo_new_path(p->cr);
    cairo_append_path(p->cr, saved);
    cairo_path_destroy(saved);

    vp_line(p, x, y);
    return placed;
}

// Relative variant: endpoint is (current_x + dx, current_y + dy).
static inline int lc_line_stitched_rel(plot_t *p, stitch_style_t style,
                                       bool incl_start, bool incl_end,
                                       float dx, float dy, bool material_left) {
    double x0d, y0d;
    cairo_get_current_point(p->cr, &x0d, &y0d);
    return lc_line_stitched(p, style, incl_start, incl_end,
                            (float)x0d + dx, (float)y0d + dy, material_left);
}

// 0 deg = SE, positive clockwise (y-down).
static inline void vp_buttonhole(plot_t *p,
                                 float cx, float cy, float angle_deg,
                                 float hole_d, float slit_len) {
    vp_circle(p, cx, cy, hole_d * 0.5f);
    float a = lc_deg_to_rad(angle_deg + 45.0f);
    float dx = cosf(a) * slit_len;
    float dy = sinf(a) * slit_len;
    vp_new_path(p, cx, cy);
    vp_line(p, cx + dx, cy + dy);
    vp_stroke(p);
}

static inline bend_corner_t lc_bend_corner_zero(void) {
    bend_corner_t bc;
    bc.cx = 0; bc.cy = 0;
    bc.external_r = 0;
    bc.a0_deg = 0; bc.a1_deg = 0; bc.theta_deg = 0;
    bc.style = stitch_style(0, 1, 0);
    bc.stitch_r = 0;
    bc.n = 0; bc.holes = 0;
    bc.p_o = 0; bc.p_s = 0; bc.co1 = 0; bc.co2 = 0;
    return bc;
}

// Like vp_arc: draws arc from current point to (x,y), using endpoint-based
// radius/cw/large_arc semantics. Additionally plots stitch holes at
// stitch_r = r - style.inset and returns a bend_corner_t for lc_bend_strip.
// Current point ends at (x,y), like vp_arc.
static inline bend_corner_t lc_arc_stitched(plot_t *p, stitch_style_t style,
                                            bool incl_start, bool incl_end,
                                            float x, float y, float r,
                                            bool cw, bool large_arc) {
    double x0, y0;
    cairo_get_current_point(p->cr, &x0, &y0);

    double dx = x - x0, dy = y - y0;
    double dist = sqrt(dx * dx + dy * dy);
    if (dist == 0 || dist > 2.0000001 * r) {
        printf("lc_arc_stitched warning: dist = %f, radius = %f. This arc will not be plotted.\n", dist, r);
        return lc_bend_corner_zero();
    }

    double mx = (x0 + x) / 2, my = (y0 + y) / 2;
    double h_squared = r * r - dist * dist / 4;
    if (h_squared < 0) h_squared = 0;
    double h = sqrt(h_squared);

    double px = -dy / dist, py = dx / dist;

    double cx = mx + h * px * (large_arc != cw ? -1 : 1);
    double cy = my + h * py * (large_arc != cw ? -1 : 1);

    double a1 = atan2(y0 - cy, x0 - cx);
    double a2 = atan2(y - cy, x - cx);

    double angle_diff = a2 - a1;
    while (angle_diff > M_PI) angle_diff -= 2 * M_PI;
    while (angle_diff < -M_PI) angle_diff += 2 * M_PI;

    if (large_arc && fabs(angle_diff) <= M_PI) {
        a1 += 2 * M_PI;
    }
    if (!large_arc && fabs(angle_diff) > M_PI) {
        a1 += 2 * M_PI;
    }

    bend_corner_t bc;
    bc.cx = (float)cx;
    bc.cy = (float)cy;
    bc.external_r = r;
    bc.style = style;
    bc.stitch_r = r - style.inset;
    bc.a0_deg = lc_rad_to_deg((float)a1);
    bc.a1_deg = lc_rad_to_deg((float)a2);

    // Sweep in the direction actually drawn.
    float sweep = (float)(a2 - a1); // radians, signed in cairo math coords
    bc.theta_deg = fabsf(lc_rad_to_deg(sweep));
    float theta = fabsf(sweep);

    float l_s = bc.stitch_r * theta;
    bc.n = (int)roundf(l_s / style.pitch);
    if (bc.n < 1) bc.n = 1;
    bc.holes = bc.n + 1;

    bc.p_s = l_s / (float)bc.n;
    float l_o = bc.external_r * theta;
    bc.p_o = l_o / (float)bc.n;
    bc.co2 = bc.p_o - bc.p_s;               // = style.inset * theta / n
    bc.co1 = bc.p_o * 0.5f - bc.co2;        // = p_s - p_o/2

    // Stitch holes first; they use their own paths, so preserve current path.
    cairo_path_t *saved = cairo_copy_path(p->cr);
    stitch_arc(p, style, incl_start, incl_end, bc.cx, bc.cy, bc.stitch_r,
               bc.a0_deg, bc.a1_deg);
    cairo_new_path(p->cr);
    cairo_append_path(p->cr, saved);
    cairo_path_destroy(saved);

    // Draw the arc exactly like vp_arc, leaving current point at endpoint.
    vp_arc(p, x, y, r, cw, large_arc);

    return bc;
}

// Extends current path with a bend-strip notch profile, starting at current
// point. Holes are drawn as separate paths; outline path is preserved.
// ts is fixed to 1.5 * style.inset. Ends with current point at strip end.
static inline void lc_bend_strip(plot_t *p, const bend_corner_t *bc,
                                 float azimuth_deg, bool material_left) {
    double x0d, y0d;
    cairo_get_current_point(p->cr, &x0d, &y0d);
    float x = (float)x0d;
    float y = (float)y0d;

    float fdx, fdy, ndx, ndy;
    lc_dir(azimuth_deg, &fdx, &fdy);
    lc_left_normal(azimuth_deg, &ndx, &ndy);
    if (!material_left) {
        ndx = -ndx;
        ndy = -ndy;
    }

    float s = bc->style.inset;
    float ts = 1.5f * s;
    float hr = bc->style.hole_d * 0.5f;

    // Preserve current path across hole drawing.
    cairo_path_t *saved = cairo_copy_path(p->cr);

    for (int i = 0; i <= bc->n; i++) {
        float u = bc->p_o * (float)i;
        float hx = x + fdx * u + ndx * s;
        float hy = y + fdy * u + ndy * s;
        vp_circle(p, hx, hy, hr);
    }

    cairo_new_path(p->cr);
    cairo_append_path(p->cr, saved);
    cairo_path_destroy(saved);

    // Notch outline: co1 flat, co2 ramp in, co2 ramp out, co1 flat per interval.
    float u = 0.0f;
    for (int i = 0; i < bc->n; i++) {
        u += bc->co1;
        vp_line(p, x + fdx * u, y + fdy * u);

        u += bc->co2;
        vp_line(p, x + fdx * u + ndx * ts, y + fdy * u + ndy * ts);

        u += bc->co2;
        vp_line(p, x + fdx * u, y + fdy * u);

        u += bc->co1;
        vp_line(p, x + fdx * u, y + fdy * u);
    }
    // Current point is now at strip end; path remains unstroked.
}

#endif
