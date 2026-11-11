#ifndef VECTOR_PLOT_H
#define VECTOR_PLOT_H

#include <stdio.h>
#include <cairo.h>
#include <cairo-svg.h>
#include <cairo-pdf.h>

typedef struct {
    FILE *f;
} stream_t;

static cairo_status_t write_to_stream(void *closure, const unsigned char *data, unsigned int length) {
    stream_t *s = (stream_t *)closure;
    if (fwrite(data, 1, length, s->f) == length) {
        return CAIRO_STATUS_SUCCESS;
    }
    return CAIRO_STATUS_WRITE_ERROR;
}

typedef struct {
    cairo_t *cr;
    cairo_surface_t *surface;
    stream_t *stream;
} plot_t;

static plot_t create_svg(const char *filename, float width, float height) {
    plot_t p;
    p.stream = malloc(sizeof(stream_t));
    p.stream->f = fopen(filename, "wb");

    p.surface = cairo_svg_surface_create_for_stream(write_to_stream, p.stream, width, height);
    cairo_svg_surface_set_document_unit(p.surface, CAIRO_SVG_UNIT_MM);
    p.cr = cairo_create(p.surface);

    /* Common settings */
    cairo_set_line_cap      (p.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (p.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (p.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (p.cr, 0.2);
    cairo_reset_clip        (p.cr);
    return p;
}

static plot_t create_pdf(const char *filename, float width, float height) {
    plot_t p;
    p.stream = malloc(sizeof(stream_t));
    p.stream->f = fopen(filename, "wb");

    p.surface = cairo_pdf_surface_create_for_stream(write_to_stream, p.stream, width, height);
    p.cr = cairo_create(p.surface);

    /* Common settings */
    cairo_set_line_cap      (p.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (p.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (p.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (p.cr, 0.2);
    cairo_reset_clip        (p.cr);
    return p;
}

static void close_plot(plot_t *p) {
    cairo_destroy(p->cr);
    cairo_surface_destroy(p->surface);
    fclose(p->stream->f);
    free(p->stream);
}

static inline void vp_circle(plot_t *p, float x, float y, float r) {
    cairo_new_path(p->cr);
    cairo_arc(p->cr, x, y, r, 0, 2 * M_PI);
    cairo_stroke(p->cr);
}

static inline void vp_rect(plot_t *p, float x1, float y1, float x2, float y2) {
    cairo_new_path(p->cr);
    float w = fabs(x2 - x1), h = fabs(y2 - y1);
    float x = fmin(x1, x2), y = fmin(y1, y2);
    cairo_rectangle(p->cr, x, y, w, h);
    cairo_stroke(p->cr);
}

static inline void vp_rect_rnd(plot_t *p, float x1, float y1, float x2, float y2, float r) {
    cairo_new_path(p->cr);
    float w = fabs(x2 - x1), h = fabs(y2 - y1);
    float x = fmin(x1, x2), y = fmin(y1, y2);
    float rd = fmin(r, fmin(w, h) / 2);

    cairo_move_to(p->cr, x + rd, y);
    cairo_line_to(p->cr, x + w - rd, y);
    cairo_arc(p->cr, x + w - rd, y + rd, rd, -M_PI/2, 0);
    cairo_line_to(p->cr, x + w, y + h - rd);
    cairo_arc(p->cr, x + w - rd, y + h - rd, rd, 0, M_PI/2);
    cairo_line_to(p->cr, x + rd, y + h);
    cairo_arc(p->cr, x + rd, y + h - rd, rd, M_PI/2, M_PI);
    cairo_line_to(p->cr, x, y + rd);
    cairo_arc(p->cr, x + rd, y + rd, rd, M_PI, 3*M_PI/2);
    cairo_close_path(p->cr);
    cairo_stroke(p->cr);
}

static inline void vp_obround(plot_t *p, float x1, float y1, float x2, float y2, float r) {
    cairo_new_path(p->cr);
    float dx = fabs(x2 - x1), dy = fabs(y2 - y1);
    float x = fmin(x1, x2), y = fmin(y1, y2);
    float rd = fmin(r, fmin(dx, dy) / 2);

    if (dx > dy) {
        cairo_arc(p->cr, x + rd, y + dy/2, rd, M_PI/2, 3*M_PI/2);
        cairo_arc(p->cr, x + dx - rd, y + dy/2, rd, -M_PI/2, M_PI/2);
    } else {
        cairo_arc(p->cr, x + dx/2, y + rd, rd, M_PI, 0);
        cairo_arc(p->cr, x + dx/2, y + dy - rd, rd, 0, M_PI);
    }
    cairo_close_path(p->cr);
    cairo_stroke(p->cr);
}

static inline void vp_new_path(plot_t *p, float x, float y) {
    cairo_new_path(p->cr);
    cairo_move_to(p->cr, x, y);
}

static inline void vp_close_path(plot_t *p) {
    cairo_close_path(p->cr);
    cairo_stroke(p->cr);
}

static inline void vp_stroke(plot_t *p) {
    cairo_stroke(p->cr);
}

static inline void vp_line(plot_t *p, float x, float y) {
    cairo_line_to(p->cr, x, y);
}

static inline void vp_line_rel(plot_t *p, float dx, float dy) {
    cairo_rel_line_to(p->cr, dx, dy);
}

static inline void vp_arc(plot_t *p, float x, float y, float r, bool cw, bool large_arc) {
    double x0, y0;
    cairo_get_current_point(p->cr, &x0, &y0);

    double dx = x - x0, dy = y - y0;
    double dist = sqrt(dx*dx + dy*dy);
    if (dist == 0 || dist > 2.0000001*r) {
        printf("vp_arc warning: dist = %f, radius = %f. This arc will not be plotted.\n", dist, r);
        return;
    }

    double mx = (x0 + x) / 2, my = (y0 + y) / 2;

    // Convoluted way to avoid cases where r is almost equal to dist and sqrt returns NaN
    double h_squared = r*r - dist*dist/4;
    if (h_squared < 0) h_squared = 0;
    double h = sqrt(h_squared);

    double px = -dy / dist, py = dx / dist;

    double cx, cy;
    cx = mx + h*px*(large_arc != cw ? -1 : 1);
    cy = my + h*py*(large_arc != cw ? -1 : 1);

    double a1 = atan2(y0 - cy, x0 - cx);
    double a2 = atan2(y - cy, x - cx);

    // large_arc logic - forces angle > pi when set
    double angle_diff = a2 - a1;
    while (angle_diff > M_PI) angle_diff -= 2*M_PI;
    while (angle_diff < -M_PI) angle_diff += 2*M_PI;

    if (large_arc && fabs(angle_diff) <= M_PI) {
        a1 += 2*M_PI;  // Swap to get the larger arc
    }
    if (!large_arc && fabs(angle_diff) > M_PI) {
        a1 += 2*M_PI;  // Swap to get the smaller arc
    }

    if (cw) cairo_arc_negative(p->cr, cx, cy, r, a1, a2);
    else cairo_arc(p->cr, cx, cy, r, a1, a2);
}

static inline void vp_arc_rel(plot_t *p, float dx, float dy, float r, bool cw, bool large_arc) {
    double x0, y0;
    cairo_get_current_point(p->cr, &x0, &y0);
    vp_arc(p, x0 + dx, y0 + dy, r, cw, large_arc);
}


#endif
