#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/vector_plot.h"

// SETTINGS //
static const float w = 132;
static const float h = 112;      // Total width and height, have to be multiples of 10mm
static const float t =  50;      // Thickness (width of the side piece)
static const float sw = 16;     // Strap width
static const float mw = 44;      // Middle stripe width
static const float fh = 70;      // Flap height
static const float ro = 28;      // Rounded edge diameter
static const float hs = 4.0;     // hole spacing (from edges and from each other on straght stitches)
static const float rs = ro - hs;  // Diameter of a rounded portion of the stitching path
static const float rh = 0.70;     // Radius of a stitching hole
static const int rs_num = 10;     // Number of stitching holes along the 90deg path arc

static inline void custom_stitch(plot_t *p, float x0, float y0, float x1, float y1, float radius, double spacing, int count) {
    // spacing ignored if count != 0
    double dx = (x1-x0);
    double dy = (y1-y0);
    double d = sqrt(dx*dx + dy*dy);
    if (count > 1) spacing = d/(count-1);
    for (double f=0; f <= d*1.001; f+=spacing) {
        vp_circle(p, x0 + f*dx/d, y0 + f*dy/d, radius);
    }
    return;
}
static inline void stitch(plot_t *p, float x0, float y0, float x1, float y1) {
    custom_stitch(p, x0, y0, x1, y1, rh, hs, 0);
    return;
}
static inline void custom_stitch_arc(plot_t *p, float x0, float y0, float a1, float a2, int number, float radius, float hole_radius) {
    // number per 90 degrees, not of the complete arc
    int rrs = number * round(fabsf(a1-a2)*2/M_PI);
    for (int i = 0; i < rrs; i++) {
        float angle = a1 + (a2-a1)*i/(rrs-1);
        float x = x0 + (radius) * cosf(angle);
        float y = y0 + (radius) * -sinf(angle);
        vp_circle           (p, x, y, hole_radius);
    }
    return;
}
static inline void stitch_arc(plot_t *p, float x0, float y0, float a1, float a2) {
    custom_stitch_arc(p, x0, y0, a1, a2, rs_num, rs, rh);
    return;
}

int main(int argc, char *argv[]) {


    int holes = 0;  // Counter
    plot_t p;
    
    p = create_svg("hole_test.svg", w, h);

    for (int i = 0; i < 6; i++) {
        vp_circle               (&p,   10*i, 10, 2.25);
        vp_new_path             (&p,   10*i, 7.5);
        vp_line                 (&p,   10*i,  16+2*i);
        vp_stroke               (&p);
    }
    // i = 3 gives the nicest hole. 2.25mm was too much (->2.0mm)

    close_plot(&p);
// ------------------------------------------------------------------------- //
    p = create_svg("cut_test.svg", w, h);

    vp_rect                     (&p, 0, 0, 4*hs, 8*hs);
    stitch                      (&p, 1*hs, 0, 1*hs, 8*hs);
    stitch                      (&p, 3*hs, 0, 3*hs, 8*hs);

    close_plot(&p);
// ------------------------------------------------------------------------- //
    p = create_svg("smaczex_front.svg", w, h);

    vp_new_path             (&p,   0.0,   0.0);
    vp_line                 (&p,   0.0,  h-ro);
    vp_arc                  (&p,    ro,     h,   ro, 1, 0);
    vp_line                 (&p,  w-ro,     h);
    vp_arc                  (&p,     w,  h-ro,   ro, 1, 0);
    vp_line                 (&p,     w,     0);
    vp_close_path           (&p);

    // Stitching holes
    stitch                  (&p, hs, 0, hs, h-ro-hs);
    stitch_arc              (&p, ro, h-ro, M_PI, 3*M_PI/2);
    stitch                  (&p, ro+hs, h-hs, w-ro-hs, h-hs);
    stitch_arc              (&p, w-ro, h-ro, 3*M_PI/2, 4*M_PI/2);
    stitch                  (&p, w-hs, h-ro-hs, w-hs, 0);

    // More stitches for stiffener
    stitch                  (&p, 3*hs, 1*hs, w-3*hs, 1*hs);
    custom_stitch           (&p,w/2, 7*hs,   3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    custom_stitch           (&p,w/2, 7*hs, w-3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    stitch                  (&p, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch                  (&p,w-3*hs, 2*hs,w-3*hs, 3*hs);
    vp_circle               (&p,  w/2, 4*hs,  1.5); 

    
    close_plot(&p);
// ------------------------------------------------------------------------- //
    p = create_svg("smaczex_front_stiffener.svg", w, h);

    vp_new_path             (&p,   2*hs,   0.0);
    vp_line                 (&p,   2*hs,  4*hs);
    vp_arc                  (&p,   3*hs,  5*hs, hs, 1, 0);
    vp_line                 (&p,  (w/2),  8*hs);
    vp_line                 (&p, w-3*hs,  5*hs);
    vp_arc                  (&p, w-2*hs,  4*hs, hs, 1, 0);
    vp_line                 (&p, w-2*hs,  0*hs);
    vp_close_path           (&p);

    // More stitches for stiffener
    stitch                  (&p, 3*hs, 1*hs, w-3*hs, 1*hs);
    custom_stitch           (&p,w/2, 7*hs,   3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    custom_stitch           (&p,w/2, 7*hs, w-3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    stitch                  (&p, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch                  (&p,w-3*hs, 2*hs,w-3*hs, 3*hs);
    vp_circle               (&p,  w/2, 4*hs,  1.5); 
    
    close_plot(&p);
// ------------------------------------------------------------------------- //
    p = create_svg("smaczex_back.svg", w, 2*h);
    cairo_translate(p.cr, 0, 3*hs);   // lets me draw at negative y coords

    vp_new_path             (&p,   0.0,   -2*hs);
    vp_line                 (&p,   0.0,  h-ro);
    vp_arc                  (&p,    ro,     h,   ro, 1, 0);
    vp_line                 (&p,  w-ro,     h);
    vp_arc                  (&p,     w,  h-ro,   ro, 1, 0);
    vp_line                 (&p,     w,   -2*hs);
    vp_close_path           (&p);

    // Stitching holes
    stitch                  (&p, hs, 0, hs, h-ro-hs);
    stitch_arc              (&p, ro, h-ro, M_PI, 3*M_PI/2);
    stitch                  (&p, ro+hs, h-hs, w-ro-hs, h-hs);
    stitch_arc              (&p, w-ro, h-ro, 3*M_PI/2, 4*M_PI/2);
    stitch                  (&p, w-hs, h-ro-hs, w-hs, 0);

    // Flap stitches
    stitch                  (&p, hs, -hs, w-hs, -hs);

    // Strap related holes
    vp_rect                 (&p,  (w-sw)/2, 0, (w+sw)/2, 4);
    vp_circle               (&p,   w/2, 14,  2);
    vp_circle               (&p,   w/2, 54,  2);

    stitch                  (&p,  (w-12*hs)/2, 9, (w+12*hs)/2, 9);
    stitch                  (&p,  (w-12*hs)/2, 59, (w+12*hs)/2, 59);



    //stitch                  (&p, flrw-hs, h-hs-hs, flrw-hs, 0);
    //stitch                  (&p, w-flrw+hs, h-hs-hs, w-flrw+hs, 0);

    //stitch_arc              (&p, w/2, ro+hs, 0, 4*M_PI/2);
    
    close_plot(&p);
// ------------------------------------------------------------------------- //
    p = create_svg("smaczex_back_stiffener.svg", w, 2*h);
    cairo_translate(p.cr, 0, 3*hs);   // lets me draw at negative y coords


    vp_rect                 (&p,  (w-14*hs)/2, 9-hs, (w+14*hs)/2, 59+hs);
    // Strap related holes
    vp_circle               (&p,   w/2, 14,  2);
    vp_circle               (&p,   w/2, 54,  2);

    stitch                  (&p,  (w-12*hs)/2, 9, (w+12*hs)/2, 9);
    stitch                  (&p,  (w-12*hs)/2, 59, (w+12*hs)/2, 59);



    //stitch                  (&p, flrw-hs, h-hs-hs, flrw-hs, 0);
    //stitch                  (&p, w-flrw+hs, h-hs-hs, w-flrw+hs, 0);

    //stitch_arc              (&p, w/2, ro+hs, 0, 4*M_PI/2);
    
    close_plot(&p);
// ------------------------------------------------------------------------- //
    p = create_svg("smaczex_flap.svg", w, 2*h);
    cairo_translate(p.cr, 0, 3*hs);   // lets me draw at negative y coords
    float fl = t+12;       // flap length

    float f1 = 3*hs*w/(w-6*hs);

    vp_new_path             (&p,   0.0,   -2*hs);
    vp_line                 (&p,   0.0, fl+2*hs-f1);
    vp_line                 (&p,   (w-0*hs)/2, fl+2*hs);
    vp_line                 (&p,   (w+0*hs)/2, fl+2*hs);
    vp_line                 (&p,     w, fl+2*hs-f1);
    vp_line                 (&p,     w,   -2*hs);
    vp_close_path           (&p);

    // Flap stitches
    stitch                  (&p, hs, -hs, w-hs, -hs);

    vp_circle               (&p,  w/2, fl-2*hs, 2.25);
    vp_new_path             (&p,  w/2, fl-2*hs+2.25);
    vp_line_rel             (&p,    0, 7);
    vp_stroke               (&p);

    cairo_translate(p.cr, 0, t-12);
    /*
    vp_new_path             (&p,   2*hs,   0.0);
    vp_line                 (&p,   2*hs,  4*hs);
    vp_arc                  (&p,   3*hs,  5*hs, hs, 1, 0);
    vp_line                 (&p,  (w/2),  8*hs);
    vp_line                 (&p, w-3*hs,  5*hs);
    vp_arc                  (&p, w-2*hs,  4*hs, hs, 1, 0);
    vp_line                 (&p, w-2*hs,  0*hs);
    vp_close_path           (&p);
*/

    // More stitches for stiffener
    stitch                  (&p, 3*hs, 1*hs, w-3*hs, 1*hs);
    custom_stitch           (&p,w/2, 7*hs,   3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    custom_stitch           (&p,w/2, 7*hs, w-3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    stitch                  (&p, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch                  (&p,w-3*hs, 2*hs,w-3*hs, 3*hs);
    vp_circle               (&p,  w/2, 4*hs,  2.25); 
    vp_new_path             (&p,  w/2, 4*hs+2.25);
    vp_line_rel             (&p,    0, 7);
    vp_stroke               (&p);

    close_plot(&p);
// ------------------------------------------------------------------------- //
    p = create_svg("smaczex_flap_stiffener.svg", w, 2*h);

    vp_new_path             (&p,   2*hs,   0.0);
    vp_line                 (&p,   2*hs,  4*hs);
    vp_arc                  (&p,   3*hs,  5*hs, hs, 1, 0);
    vp_line                 (&p,  (w/2),  8*hs);
    vp_line                 (&p, w-3*hs,  5*hs);
    vp_arc                  (&p, w-2*hs,  4*hs, hs, 1, 0);
    vp_line                 (&p, w-2*hs,  0*hs);
    vp_close_path           (&p);

    // More stitches for stiffener
    stitch                  (&p, 3*hs, 1*hs, w-3*hs, 1*hs);
    custom_stitch           (&p,w/2, 7*hs,   3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    custom_stitch           (&p,w/2, 7*hs, w-3*hs, 4*hs, rh, 0, w/hs/2 - 1);
    stitch                  (&p, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch                  (&p,w-3*hs, 2*hs,w-3*hs, 3*hs);
    vp_circle               (&p,  w/2, 4*hs,  2.25); 
    vp_new_path             (&p,  w/2, 4*hs+2.25);
    vp_line_rel             (&p,    0, 7);
    vp_stroke               (&p);

    close_plot(&p);
// ------------------------------------------------------------------------- //

    static const int sideholes = (h - ro) / hs;
    static const int bottomholes = (w - ro - ro)/ hs - 1;
    static const int ts = 1.5*hs;   // "tooth" height
    float sidew = 2*h+w; // exaggerated but guaranteed to work
    float sideh = t;
    plot_t side = create_svg("smaczex_side.svg", sidew, sideh);

    // Plotting is divided into 5 stages: side, curve, bottom, curve, side.
    // All this is repeated twice for each long edge.
    float x = 0, y = 0;
    float lo1 = M_PI * ro / 2 / (rs_num-1); // Length of outer edge arc segment
    float ls1 = M_PI * rs / 2 / (rs_num-1); // Length of stitching arc segment
    float co2 = (lo1 - ls1);
    float co1 =  lo1/2 - co2;

    // Stitching holes, both sides at the same time
    y = hs;
    for (int i = 0; i < sideholes; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += hs;
    }
    for (int i = 0; i < rs_num; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += lo1;
    }
    x = x - lo1 + hs;
    for (int i = 0; i < bottomholes; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += hs;
    }
    for (int i = 0; i < rs_num; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += lo1;
    }
    x = x - lo1 + hs;
    for (int i = 0; i < sideholes; i++) {
        vp_circle   (&side,   x,       y, rh);
        vp_circle   (&side,   x, sideh-y, rh);
        x += hs;
    }

    // Outline = LHS end
    x = 0; y = 0;           vp_new_path(&side,   x,  y);
    y += sideh;             vp_line(&side,   x,  y); vp_stroke(&side);

    // Outline - upper
    x = 0; y = 0;
    vp_new_path   (&side,   x,  y);
    // Side 1
    x += (sideholes) * hs;
                            vp_line(&side,   x,  y);
    // Curve 1
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y += ts;  vp_line(&side,   x,  y);
        x += co2; y -= ts;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Bottom
    x += (bottomholes+1) * hs;
                            vp_line(&side,   x,  y);
    // Curve 2
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y += ts;  vp_line(&side,   x,  y);
        x += co2; y -= ts;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Side 2
    x += (sideholes) * hs;
                            vp_line(&side,   x,  y);
    y += sideh;             vp_line(&side,   x,  y); vp_stroke(&side);
    
    // RHS end

    // Outline - lower
    x = 0; y = sideh;
    vp_new_path   (&side,   x,  y);
    // Side 1
    x += (sideholes) * hs;
                            vp_line(&side,   x,  y);
    // Curve 1
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y -= ts;  vp_line(&side,   x,  y);
        x += co2; y += ts;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Bottom
    x += (bottomholes+1) * hs;
                            vp_line(&side,   x,  y);
    // Curve 2
    for (int i = 1; i < rs_num; i++) {
        x += co1;           vp_line(&side,   x,  y);
        x += co2; y -= ts;  vp_line(&side,   x,  y);
        x += co2; y += ts;  vp_line(&side,   x,  y);
        x += co1;           vp_line(&side,   x,  y);
    }
    // Side 2
    x += (sideholes) * hs;
                            vp_line(&side,   x,  y);
    vp_stroke        (&side);

    close_plot(&side);

// ------------------------------------------------------------------------- //

    float strapr = sw/2;
    float straph = 2*strapr;
    float strapw = 110 + 2*strapr;
    plot_t strap = create_svg("smaczex_strap.svg", strapw, straph);

    // Actual design
    // The outline
    vp_obround              (&strap, 0, 0, strapw, straph, strapr);

    // Holes
    vp_circle               (&strap,     strapr              , strapr, 2.0);
    vp_circle               (&strap,     strapr + 40         , strapr, 2.0);
    vp_circle               (&strap,     strapw - strapr     , strapr, 2.0);
    vp_circle               (&strap,     strapw - strapr - 40, strapr, 2.0);

    close_plot(&strap);
    return 0;
}

