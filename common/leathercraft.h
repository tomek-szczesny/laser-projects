#ifndef LEATHERCRAFT_H
#define LEATHERCRAFT_H

/*
 * leathercraft.h - Leathercraft/stitched-part helpers on top of vector_plot.h.
 *
 * Provides pitch-matched stitch holes and bend-corner helpers for laser-cut
 * leather/fabric parts. Coordinates follow vector_plot.h (mm). Angles are in
 * degrees, 0=+x, positive clockwise (SVG/Cairo y-down).
 *
 * Typical workflow (stitch a rounded panel, then make a matching bend strip):
 *
 *   #include "../common/leathercraft.h"
 *
 *   stitch_style_t S = stitch_style(1.4, 4.0, 4.0); // hole_d, pitch, inset
 *   plot_t p = create_svg("part.svg", 132, 112);
 *
 *   vp_new_path(&p, 0, 0);
 *   lc_line_stitched(&p, S, true, false, 0, 84, true);         // edge + holes
 *   bend_corner_t bc = lc_arc_stitched(&p, S, false, false,    // arc + holes
 *                                      28, 112, 28, true, false);
 *   lc_line_stitched(&p, S, false, true, 132, 84, true);
 *   vp_close_path(&p);
 *   close_plot(&p);
 *
 *   // Side strip: lc_bend_strip consumes bc so hole count/spacing matches.
 *   plot_t s = create_svg("side.svg", 360, 50);
 *   vp_new_path(&s, 0, 0);
 *   vp_line(&s, 84, 0);
 *   lc_bend_strip(&s, &bc, 0.0, false); // azimuth 0=+x, material below
 *   vp_stroke(&s);
 *   close_plot(&s);
 *
 * Notes:
 * - lc_* helpers preserve the current Cairo path across hole drawing.
 * - incl_start/incl_end control endpoint ownership when joining segments.
 * - material_left: when facing travel direction, material is on the left.
 */

#include <stdbool.h>
#include <math.h>
#include "vector_plot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Stitch style used by stitch and bend helpers.
 * hole_d: hole diameter (mm). pitch: target distance between holes (mm).
 * inset:  distance from edge to stitch line (mm). */
typedef struct {
    float hole_d;
    float pitch;
    float inset;
} stitch_style_t;

/* Convenience constructor for stitch_style_t. */
static inline stitch_style_t stitch_style(float hole_d, float pitch, float inset) {
    return (stitch_style_t){hole_d, pitch, inset};
}

/* Computed bend-corner geometry. Returned by lc_arc_stitched and consumed by
 * lc_bend_strip so panel corner and strip bend have matching hole counts.
 *
 * p_o: flat strip spacing per interval (outer arc / n).
 * p_s: stitch-line spacing per interval (stitch radius / n).
 * co1: half flat tooth top; co2: notch ramp x. Tooth depth ts is 1.5*inset. */
typedef struct {
    float cx, cy;
    float external_r;
    float a0_deg, a1_deg;   /* center angles, degrees, 0=+x, positive clockwise */
    float theta_deg;        /* abs(sweep), normally 0 < theta < 180 */

    stitch_style_t style;
    float stitch_r;         /* external_r - style.inset */

    int n;                  /* intervals, >= 1 */
    int holes;              /* n + 1 */
    float p_o;
    float p_s;
    float co1;
    float co2;
} bend_corner_t;

static inline float lc_deg_to_rad(float deg) {
    return deg * (float)M_PI / 180.0f;
}

static inline float lc_rad_to_deg(float rad) {
    return rad * 180.0f / (float)M_PI;
}

/* Direction vector for angle_deg (0=+x, positive clockwise, y-down). */
static inline void lc_dir(float angle_deg, float *dx, float *dy) {
    float a = lc_deg_to_rad(angle_deg);
    *dx = cosf(a);
    *dy = sinf(a);
}

/* Left-hand normal for angle_deg in y-down visual coordinates. */
static inline void lc_left_normal(float angle_deg, float *dx, float *dy) {
    float a = lc_deg_to_rad(angle_deg);
    *dx = sinf(a);
    *dy = -cosf(a);
}

/* Draw pitch-matched stitch holes along a straight line.
 * Holes are placed on the line from (x0,y0) to (x1,y1); endpoints may be
 * excluded to avoid duplicates at joins. Returns number of holes placed. */
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

/* Draw pitch-matched stitch holes along a center+angle arc.
 * Angles in degrees, 0=+x, positive clockwise. Returns number of holes placed. */
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

/* Evaluate a cubic Bezier point at parameter t in [0,1].
 * P0 = start, P1/P2 = control points, P3 = end. */
static inline void lc_bezier(float t, float x0, float y0, float x1, float y1,
                             float x2, float y2, float x, float y,
                             float *ox, float *oy) {
    float u = 1.0f - t;
    float u2 = u * u;
    float u3 = u2 * u;
    float t2 = t * t;
    float t3 = t2 * t;
    *ox = u3 * x0 + 3.0f * u2 * t * x1 + 3.0f * u * t2 * x2 + t3 * x;
    *oy = u3 * y0 + 3.0f * u2 * t * y1 + 3.0f * u * t2 * y2 + t3 * y;
}

/* Draw pitch-matched stitch holes along a cubic Bezier curve, like stitch_line.
 * Curve starts at (x0,y0); control points (x1,y1) and (x2,y2); end (x,y).
 * Holes are placed at equal arc-length intervals. Returns number of holes placed. */
static inline int stitch_curve(plot_t *p, stitch_style_t s,
                               bool incl_start, bool incl_end,
                               float x0, float y0, float x1, float y1,
                               float x2, float y2, float x, float y) {
    const int NS = 10000;
    float L[NS + 1];
    L[0] = 0.0f;
    float px = x0, py = y0;
    for (int i = 1; i <= NS; i++) {
        float t = (float)i / (float)NS;
        float qx, qy;
        lc_bezier(t, x0, y0, x1, y1, x2, y2, x, y, &qx, &qy);
        float dx = qx - px, dy = qy - py;
        L[i] = L[i - 1] + sqrtf(dx * dx + dy * dy);
        px = qx;
        py = qy;
    }

    float len = L[NS];
    int n = (int)roundf(len / s.pitch);
    if (n < 1) n = 1;

    int placed = 0;
    float hr = s.hole_d * 0.5f;
    for (int i = 0; i <= n; i++) {
        if (i == 0 && !incl_start) continue;
        if (i == n && !incl_end) continue;
        float target = len * (float)i / (float)n;

        int lo = 0, hi = NS;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (L[mid] < target) lo = mid + 1;
            else hi = mid;
        }
        int j = lo;
        if (j < 1) j = 1;
        float seg = L[j] - L[j - 1];
        float frac = (seg > 0.0f) ? (target - L[j - 1]) / seg : 0.0f;
        float t = ((float)(j - 1) + frac) / (float)NS;

        float qx, qy;
        lc_bezier(t, x0, y0, x1, y1, x2, y2, x, y, &qx, &qy);
        vp_circle(p, qx, qy, hr);
        placed++;
    }
    return placed;
}

/* Draw a line from current point to (x,y), like vp_line, and add stitch holes
 * at style.inset on one side. material_left: when facing start->end, holes are
 * on the left. Leaves current point at (x,y). Returns number of holes placed. */
static inline int lc_line_stitched(plot_t *p, stitch_style_t style,
                                   bool incl_start, bool incl_end,
                                   float x, float y, bool material_left) {
    double x0d, y0d;
    cairo_get_current_point(p->cr, &x0d, &y0d);
    float x0 = (float)x0d, y0 = (float)y0d;

    float dx = x - x0;
    float dy = y - y0;
    float len = sqrtf(dx * dx + dy * dy);

    float ndx = 0.0f, ndy = -1.0f;
    if (len > 0.0f) {
        ndx = dy / len;
        ndy = -dx / len;
    }
    if (!material_left) {
        ndx = -ndx;
        ndy = -ndy;
    }

    float hx0 = x0 + ndx * style.inset;
    float hy0 = y0 + ndy * style.inset;
    float hx1 = x  + ndx * style.inset;
    float hy1 = y  + ndy * style.inset;

    cairo_path_t *saved = cairo_copy_path(p->cr);
    int placed = stitch_line(p, style, incl_start, incl_end, hx0, hy0, hx1, hy1);
    cairo_new_path(p->cr);
    cairo_append_path(p->cr, saved);
    cairo_path_destroy(saved);

    vp_line(p, x, y);
    return placed;
}

/* Relative variant of lc_line_stitched; endpoint is current point + (dx,dy). */
static inline int lc_line_stitched_rel(plot_t *p, stitch_style_t style,
                                       bool incl_start, bool incl_end,
                                       float dx, float dy, bool material_left) {
    double x0d, y0d;
    cairo_get_current_point(p->cr, &x0d, &y0d);
    return lc_line_stitched(p, style, incl_start, incl_end,
                            (float)x0d + dx, (float)y0d + dy, material_left);
}

/* Draw a cubic Bezier curve from current point to (x,y), like vp_curve, and
 * add stitch holes at style.inset on one side. material_left: when facing
 * start->end, holes are on the left. Holes are placed at equal arc-length
 * intervals. Leaves current point at (x,y). Returns number of holes placed.
 *
 * Not composed from vp_curve+stitch_curve: a constant-distance offset of a
 * cubic Bezier is not a cubic Bezier, so holes are placed at B(t)+inset*N(t)
 * using the arc-length LUT and B'(t) for normals. */
static inline int lc_curve_stitched(plot_t *p, stitch_style_t style,
                                    bool incl_start, bool incl_end,
                                    float x1, float y1, float x2, float y2,
                                    float x, float y, bool material_left) {
    double x0d, y0d;
    cairo_get_current_point(p->cr, &x0d, &y0d);
    float x0 = (float)x0d, y0 = (float)y0d;

    const int NS = 10000;
    float L[NS + 1];
    L[0] = 0.0f;
    float px = x0, py = y0;
    for (int i = 1; i <= NS; i++) {
        float t = (float)i / (float)NS;
        float qx, qy;
        lc_bezier(t, x0, y0, x1, y1, x2, y2, x, y, &qx, &qy);
        float dx = qx - px, dy = qy - py;
        L[i] = L[i - 1] + sqrtf(dx * dx + dy * dy);
        px = qx;
        py = qy;
    }

    float len = L[NS];
    int n = (int)roundf(len / style.pitch);
    if (n < 1) n = 1;

    cairo_path_t *saved = cairo_copy_path(p->cr);
    int placed = 0;
    float hr = style.hole_d * 0.5f;
    for (int i = 0; i <= n; i++) {
        if (i == 0 && !incl_start) continue;
        if (i == n && !incl_end) continue;
        float target = len * (float)i / (float)n;

        int lo = 0, hi = NS;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (L[mid] < target) lo = mid + 1;
            else hi = mid;
        }
        int j = lo;
        if (j < 1) j = 1;
        float seg = L[j] - L[j - 1];
        float frac = (seg > 0.0f) ? (target - L[j - 1]) / seg : 0.0f;
        float t = ((float)(j - 1) + frac) / (float)NS;

        float qx, qy;
        lc_bezier(t, x0, y0, x1, y1, x2, y2, x, y, &qx, &qy);

        float u = 1.0f - t;
        float tx = 3.0f * u * u * (x1 - x0) + 6.0f * u * t * (x2 - x1) + 3.0f * t * t * (x - x2);
        float ty = 3.0f * u * u * (y1 - y0) + 6.0f * u * t * (y2 - y1) + 3.0f * t * t * (y - y2);
        float tl = sqrtf(tx * tx + ty * ty);
        float ndx = 0.0f, ndy = -1.0f;
        if (tl > 0.0f) {
            ndx = ty / tl;
            ndy = -tx / tl;
        }
        if (!material_left) {
            ndx = -ndx;
            ndy = -ndy;
        }

        vp_circle(p, qx + ndx * style.inset, qy + ndy * style.inset, hr);
        placed++;
    }
    cairo_new_path(p->cr);
    cairo_append_path(p->cr, saved);
    cairo_path_destroy(saved);

    vp_curve(p, x1, y1, x2, y2, x, y);
    return placed;
}

/* Draw a buttonhole: circle at (cx,cy) plus a slit.
 * angle_deg=0 points SE; positive clockwise (y-down). */
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

/* Return an empty/invalid bend_corner_t (used on arc geometry errors). */
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

/* Draw an endpoint-based arc like vp_arc, plus stitch holes at
 * stitch_r = r - style.inset. Returns bend_corner_t for lc_bend_strip.
 * Leaves current point at (x,y). */
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

    float sweep = (float)(a2 - a1);
    bc.theta_deg = fabsf(lc_rad_to_deg(sweep));
    float theta = fabsf(sweep);

    float l_s = bc.stitch_r * theta;
    bc.n = (int)roundf(l_s / style.pitch);
    if (bc.n < 1) bc.n = 1;
    bc.holes = bc.n + 1;

    bc.p_s = l_s / (float)bc.n;
    float l_o = bc.external_r * theta;
    bc.p_o = l_o / (float)bc.n;
    bc.co2 = bc.p_o - bc.p_s;
    bc.co1 = bc.p_o * 0.5f - bc.co2;

    cairo_path_t *saved = cairo_copy_path(p->cr);
    stitch_arc(p, style, incl_start, incl_end, bc.cx, bc.cy, bc.stitch_r,
               bc.a0_deg, bc.a1_deg);
    cairo_new_path(p->cr);
    cairo_append_path(p->cr, saved);
    cairo_path_destroy(saved);

    vp_arc(p, x, y, r, cw, large_arc);

    return bc;
}

/* Extend current path with a bend-strip notch profile starting at current point.
 * Use bend_corner_t from lc_arc_stitched for matching hole count/spacing.
 * azimuth_deg: travel direction (0=+x, positive clockwise).
 * material_left: when facing azimuth, material/notches are on the left.
 * Draws holes as separate stroked paths; outline remains unstroked with current
 * point at strip end. Notch depth ts is fixed to 1.5 * style.inset. */
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
}

#endif
