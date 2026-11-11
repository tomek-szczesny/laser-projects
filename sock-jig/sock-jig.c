#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/vector_plot.h"

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [-w WIDTH] [-h HEIGHT]\n", program_name);
}

int parse_arguments(int argc, char *argv[], float *w, float *h) {
    *w = 95.0f;
    *h = 400.0f;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -w requires a value\n");
                return 0;
            }
            *w = atof(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -h requires a value\n");
                return 0;
            }
            *h = atof(argv[++i]);
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
            return 0;
        }
    }

    printf("Final dimensions: width=%.0fmm, height=%.0fmm\n", *w, *h);
    return 1;
}

int main(int argc, char *argv[]) {

    float w, h;         // Total width and height
    float ro = 7;      // Rounded corner diameter
    float rh = 4;     // Radius of a hole for hanging
    float so = 70;      // "Sockicity" of the sock's front.. Don't ask

    if (!parse_arguments(argc, argv, &w, &h)) {
        print_usage(argv[0]);
        return 1;
    }

    plot_t jig = create_svg("sock-jig.svg", w, h);

    /* Common settings */
    cairo_set_line_cap      (jig.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (jig.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (jig.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (jig.cr, 0.2);

    // Actual design
    // The outline
    cairo_move_to           (jig.cr,    0.0,   so);
    cairo_line_to           (jig.cr,    0.0,  h-ro);
    cairo_arc_negative      (jig.cr,     ro,  h-ro,   ro, M_PI, M_PI / 2);
    cairo_line_to           (jig.cr,   w-ro,     h);
    cairo_arc_negative      (jig.cr,   w-ro,  h-ro,   ro, M_PI/2, 0);
    cairo_line_to           (jig.cr,      w, so);
    cairo_curve_to          (jig.cr,      w, 0, w/2,    0, w/2,  0);
    cairo_curve_to          (jig.cr, w/2,   0,    0, 0,      0, so);
    cairo_close_path        (jig.cr);
    cairo_stroke            (jig.cr);

    // Hanging Hole
    cairo_arc               (jig.cr,   w/2, h-(3*rh), rh, 0, 2 * M_PI);
    cairo_stroke            (jig.cr);

    close_plot(&jig);

    return 0;
}

