#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/vector_plot.h"

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [-w WIDTH] [-h HEIGHT]\n", program_name);
    fprintf(stderr, "  WIDTH and HEIGHT in mm (default: 110mm)\n");
    fprintf(stderr, "  Range: 80-150mm, rounded to nearest 5mm\n");
}

float round_to_5mm(float value) {
    return roundf(value / 5.0f) * 5.0f;
}

int parse_arguments(int argc, char *argv[], float *w, float *h) {
    *w = 110.0f;
    *h = 110.0f;

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

    *w = round_to_5mm(*w);
    *h = round_to_5mm(*h);

    if (*w < 80.0f || *w > 150.0f) {
        fprintf(stderr, "Error: width %.0fmm out of range [80-150mm]\n", *w);
        return 0;
    }
    if (*h < 80.0f || *h > 150.0f) {
        fprintf(stderr, "Error: height %.0fmm out of range [80-150mm]\n", *h);
        return 0;
    }

    printf("Final dimensions: width=%.0fmm, height=%.0fmm\n", *w, *h);
    return 1;
}

int main(int argc, char *argv[]) {

    float w, h;         // Total width and height
    float fh = 70;      // Flap height
    float ro = 20;      // Rounded edge diameter
    float rs = ro - 5;  // Diameter of a rounded portion of the stitching path
    float rh = 0.75;     // Radius of a stitching hole
    int rs_num = 6;     // Number of stitching holes along the 90deg path arc

    if (!parse_arguments(argc, argv, &w, &h)) {
        print_usage(argv[0]);
        return 1;
    }

    int sideholes = (h - ro) / 5;
    int bottomholes = (w - ro - ro)/ 5 - 1;
    int holes = 0;
    

    plot_t front = create_svg("pindelex_front.svg", w, h);

    /* Common settings */
    cairo_set_line_cap      (front.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (front.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (front.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (front.cr, 0.2);

    // Actual design
    // The outline
    vp_new_path             (&front,   0.0,   0.0);
    vp_line                 (&front,   0.0,  h-ro);
    vp_arc                  (&front,    ro,  h   ,   ro, 1, 0);
    vp_line                 (&front,  w-ro,     h);
    vp_arc                  (&front,  w   ,  h-ro,   ro, 1, 0);
    vp_line                 (&front,     w,   0.0);
    vp_close_path           (&front);

    // Stitching holes
    // Left and right side
    for (int i = 0; i < sideholes; i++) {
        float y = 0.0f + (i * 5.0f);
        vp_circle           (&front,   5, y, rh);
        vp_circle           (&front, w-5, y, rh);
        holes += 2;
    }
    // Bottom
    for (int i = 1; i < bottomholes + 1; i++) {
        float x = ro + (i * 5.0f);
        vp_circle           (&front,   x, h-5, rh);
        holes++;
    }
    // Arcs
    for (int i = 0; i < rs_num; i++) {
        float angle = M_PI - (i * M_PI / (2.0f * (rs_num - 1)));
        float x =       ro + (ro - 5.0f) * cosf(angle);
        float y = (h - ro) + (ro - 5.0f) * sinf(angle);
        vp_circle           (&front,   x, y, rh);
        vp_circle           (&front, w-x, y, rh);
        holes += 2;
    }

    close_plot(&front);
// ------------------------------------------------------------------------- //

    plot_t back = create_svg("pindelex_back.svg", w, h+fh);
    cairo_translate         (back.cr,   0.0,   fh);

    /* Common settings */
    cairo_set_line_cap      (back.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (back.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (back.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (back.cr, 0.2);

    // Actual design
    // The outline
    vp_new_path             (&back,      0.0,   0.0);
    vp_line                 (&back,      0.0,  h-ro);
    vp_arc                  (&back,       ro,  h   ,   ro, 1, 0);
    vp_line                 (&back,     w-ro,     h);
    vp_arc                  (&back,     w   ,  h-ro,   ro, 1, 0);
    vp_line                 (&back,        w, 20-fh);
    cairo_curve_to          (back.cr,      w, -fh, w/2+10,    -fh, w/2+10,  -fh);
    vp_line                 (&back,   w/2-10,   -fh);
    cairo_curve_to          (back.cr, w/2-10,   -fh,    0, -fh,      0, -fh+20);
    vp_close_path           (&back);

    // Stitching holes
    // Left and right side
    for (int i = 0; i < ((h - ro) / 5); i++) {
        float y = 0.0f + (i * 5.0f);
        vp_circle           (&back,     5, y, rh);
        vp_circle           (&back,   w-5, y, rh);
    }
    // Bottom
    for (int i = 1; i < ((w - ro - ro) / 5); i++) {
        float x = ro + (i * 5.0f);
        vp_circle           (&back,     x, h-5, rh);
    }
    // Arcs
    for (int i = 0; i < rs_num; i++) {
        float angle = M_PI - (i * M_PI / (2.0f * (rs_num - 1)));
        float x =       ro + (ro - 5.0f) * cosf(angle);
        float y = (h - ro) + (ro - 5.0f) * sinf(angle);
        vp_circle           (&back,     x, y, rh);
        vp_circle           (&back,   w-x, y, rh);
    }
    // Strap holes
    vp_circle               (&back,   w/2, 10,  2);
    vp_circle               (&back,   w/2, 45,  2);

    vp_rect                 (&back,  (w/2)-7, -4, (w/2)+7, 0);

    close_plot(&back);

// ------------------------------------------------------------------------- //

    
    float sidew = 2*h+w; // exaggerated but guaranteed to work
    float sideh = 50.0;
    plot_t side = create_svg("pindelex_side.svg", sidew, sideh);

    // Common settings
    cairo_set_line_cap      (side.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (side.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (side.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (side.cr, 0.2);

    // Plotting is divided into 5 stages: side, curve, bottom, curve, side.
    // All this is repeated twice for each long edge.
    float x = 0, y = 0;
    float lo1 = M_PI * ro / 2 / (rs_num-1); // Length of outer edge arc segment
    float ls1 = M_PI * rs / 2 / (rs_num-1); // Length of stitching arc segment
    float co2 = (lo1 - ls1);
    float co1 =  lo1/2 - co2;


    // Stitching holes, both sides at the same time
    y = 5;
    for (int i = 0; i < sideholes; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += 5.0f;
    }
    for (int i = 0; i < rs_num; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += lo1;
    }
    x = x - lo1 + 5;
    for (int i = 0; i < bottomholes; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += 5.0f;
    }
    for (int i = 0; i < rs_num; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += lo1;
    }
    x = x - lo1 + 5;
    for (int i = 0; i < sideholes; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += 5.0f;
    }

    // Outline = LHS end
    x = 0; y = 0;           vp_new_path(&side,   x,  y);
    y += sideh;             vp_line(&side,   x,  y); vp_stroke(&side);
    

    // Outline - upper
    x = 0; y = 0;
    vp_new_path   (&side,   x,  y);
    // Side 1
    x += (sideholes) * 5.0f;
                            vp_line(&side,   x,  y);
    // Curve 1
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y += 10;  vp_line(&side,   x,  y);
        x += co2; y -= 10;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Bottom
    x += (bottomholes+1) * 5.0f;
                            vp_line(&side,   x,  y);
    // Curve 2
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y += 10;  vp_line(&side,   x,  y);
        x += co2; y -= 10;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Side 2
    x += (sideholes) * 5.0f;
                            vp_line(&side,   x,  y);
    y += sideh;             vp_line(&side,   x,  y); vp_stroke(&side);
    
    // RHS end

    // Outline - lower
    x = 0; y = sideh;
    vp_new_path   (&side,   x,  y);
    // Side 1
    x += (sideholes) * 5.0f;
                            vp_line(&side,   x,  y);
    // Curve 1
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y -= 10;  vp_line(&side,   x,  y);
        x += co2; y += 10;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Bottom
    x += (bottomholes+1) * 5.0f;
                            vp_line(&side,   x,  y);
    // Curve 2
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y -= 10;  vp_line(&side,   x,  y);
        x += co2; y += 10;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Side 2
    x += (sideholes) * 5.0f;
                            vp_line(&side,   x,  y);
    vp_stroke        (&side);

    close_plot(&side);



// ------------------------------------------------------------------------- //

    float strapr = 6;
    float straph = 2*strapr;
    float strapw = 110 + 2*strapr;
    plot_t strap = create_svg("pindelex_strap.svg", strapw, straph);

    /* Common settings */
    cairo_set_line_cap      (strap.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (strap.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (strap.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (strap.cr, 0.2);

    // Actual design
    // The outline
    vp_obround              (&strap, 0, 0, strapw, straph, strapr);
    /*
    vp_new_path           (strap.cr,  strapr, 0);
    vp_arc      (strap.cr,  strapr, strapr, strapr, 1.5 * M_PI, 0.5 * M_PI);
    vp_line           (strap.cr,  strapw-strapr, straph);
    vp_arc      (strap.cr,  strapw-strapr, strapr, strapr, 0.5 * M_PI, 1.5 * M_PI);
    cairo_close_path        (strap.cr);
    cairo_stroke            (strap.cr);
*/

    // Holes
    vp_circle               (&strap,     strapr              , strapr, 2.0);
    vp_circle               (&strap,     strapr + 35         , strapr, 2.0);
    vp_circle               (&strap,     strapw - strapr     , strapr, 2.0);
    vp_circle               (&strap,     strapw - strapr - 35, strapr, 2.0);

    close_plot(&strap);
    return 0;
}

