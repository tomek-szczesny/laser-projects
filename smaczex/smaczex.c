#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/vector_plot.h"
#include "../common/leathercraft.h"

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
static const stitch_style_t S = { .hole_d = 2*rh, .pitch = hs, .inset = hs };

int main(int argc, char *argv[]) {

    int holes = 0;  // Counter
    plot_t p;
    bend_corner_t bc_side = lc_bend_corner_zero();
    
    p = create_svg("smaczex_front.svg", w, h);

    vp_new_path             (&p,   0.0,   0.0);
    lc_line_stitched        (&p, S, true,  true,   0.0,  h-ro, true);
    bc_side = lc_arc_stitched(&p, S, false, true,    ro,     h, ro, true, false);
    lc_line_stitched        (&p, S, false, true,  w-ro,     h, true);
    lc_arc_stitched         (&p, S, false, true,     w,  h-ro, ro, true, false);
    lc_line_stitched        (&p, S, false, true,     w,   0.0, true);
    vp_close_path           (&p);

    // More stitches for stiffener
    stitch_line(&p, S, true, true, 3*hs, 1*hs, w-3*hs, 1*hs);
    stitch_line             (&p, S, true, true, w/2, 7*hs,   3*hs, 4*hs);
    stitch_line             (&p, S, false, true, w/2, 7*hs, w-3*hs, 4*hs);
    stitch_line(&p, S, true, true, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch_line(&p, S, true, true, w-3*hs, 2*hs,w-3*hs, 3*hs);
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
    stitch_line(&p, S, true, true, 3*hs, 1*hs, w-3*hs, 1*hs);
    stitch_line             (&p, S, true, true, w/2, 7*hs,   3*hs, 4*hs);
    stitch_line             (&p, S, false, true, w/2, 7*hs, w-3*hs, 4*hs);
    stitch_line(&p, S, true, true, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch_line(&p, S, true, true, w-3*hs, 2*hs,w-3*hs, 3*hs);
    vp_circle               (&p,  w/2, 4*hs,  1.5); 
    
    close_plot(&p);

// ------------------------------------------------------------------------- //

    p = create_svg("smaczex_back.svg", w, 2*h);
    cairo_translate(p.cr, 0, 3*hs);   // lets me draw at negative y coords

    vp_new_path             (&p,   0.0,   -2*hs);
    vp_line                 (&p,   0.0,    0.0);              // unstitched top-left
    lc_line_stitched        (&p, S, true,  true,   0.0,  h-ro, true);
    lc_arc_stitched         (&p, S, false, true,    ro,     h, ro, true, false);
    lc_line_stitched        (&p, S, false, true,  w-ro,     h, true);
    lc_arc_stitched         (&p, S, false, true,     w,  h-ro, ro, true, false);
    lc_line_stitched        (&p, S, false, true,     w,   0.0, true);
    vp_line                 (&p,     w,   -2*hs);              // unstitched top-right
    vp_close_path           (&p);

    // Flap stitches
    stitch_line(&p, S, true, true, hs, -hs, w-hs, -hs);

    // Strap related holes
    vp_rect                 (&p,  (w-sw)/2, 0, (w+sw)/2, 4);
    vp_circle               (&p,   w/2, 14,  2);
    vp_circle               (&p,   w/2, 54,  2);

    stitch_line(&p, S, true, true, (w-12*hs)/2, 9, (w+12*hs)/2, 9);
    stitch_line(&p, S, true, true, (w-12*hs)/2, 59, (w+12*hs)/2, 59);

    close_plot(&p);

// ------------------------------------------------------------------------- //

    p = create_svg("smaczex_back_stiffener.svg", w, 2*h);
    cairo_translate(p.cr, 0, 3*hs);   // lets me draw at negative y coords

    vp_rect                 (&p,  (w-14*hs)/2, 9-hs, (w+14*hs)/2, 59+hs);
    // Strap related holes
    vp_circle               (&p,   w/2, 14,  2);
    vp_circle               (&p,   w/2, 54,  2);

    stitch_line(&p, S, true, true, (w-12*hs)/2, 9, (w+12*hs)/2, 9);
    stitch_line(&p, S, true, true, (w-12*hs)/2, 59, (w+12*hs)/2, 59);
    
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
    stitch_line(&p, S, true, true, hs, -hs, w-hs, -hs);

    vp_circle               (&p,  w/2, fl-2*hs, 2.25);
    vp_new_path             (&p,  w/2, fl-2*hs+2.25);
    vp_line_rel             (&p,    0, 7);
    vp_stroke               (&p);

    cairo_translate(p.cr, 0, t-12);

    // More stitches for stiffener
    stitch_line(&p, S, true, true, 3*hs, 1*hs, w-3*hs, 1*hs);
    stitch_line             (&p, S, true, true, w/2, 7*hs,   3*hs, 4*hs);
    stitch_line             (&p, S, false, true, w/2, 7*hs, w-3*hs, 4*hs);
    stitch_line(&p, S, true, true, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch_line(&p, S, true, true, w-3*hs, 2*hs,w-3*hs, 3*hs);
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
    stitch_line(&p, S, true, true, 3*hs, 1*hs, w-3*hs, 1*hs);
    stitch_line             (&p, S, true, true, w/2, 7*hs,   3*hs, 4*hs);
    stitch_line             (&p, S, false, true, w/2, 7*hs, w-3*hs, 4*hs);
    stitch_line(&p, S, true, true, 3*hs, 2*hs, 3*hs, 3*hs);
    stitch_line(&p, S, true, true, w-3*hs, 2*hs,w-3*hs, 3*hs);
    vp_circle               (&p,  w/2, 4*hs,  2.25); 
    vp_new_path             (&p,  w/2, 4*hs+2.25);
    vp_line_rel             (&p,    0, 7);
    vp_stroke               (&p);

    close_plot(&p);

// ------------------------------------------------------------------------- //

    float sidew = 2*h+w; // exaggerated but guaranteed to work
    float sideh = t;
    plot_t side = create_svg("smaczex_side.svg", sidew, sideh);

    // Upper edge: facing +x, material below (+y) => material_left=false
    vp_new_path(&side, 0.0f, 0.0f);
    lc_line_stitched_rel(&side, S, true,  false, h-ro,    0.0f, false);
    lc_bend_strip       (&side, &bc_side, 0.0f, false);
    lc_line_stitched_rel(&side, S, false, false, w-ro-ro, 0.0f, false);
    lc_bend_strip       (&side, &bc_side, 0.0f, false);
    lc_line_stitched_rel(&side, S, false, true,  h-ro,    0.0f, false);
    vp_line_rel(&side, 0.0f, sideh);   // RHS end
    vp_stroke(&side);

    // Lower edge: facing +x, material above (-y) => material_left=true
    vp_new_path(&side, 0.0f, sideh);
    lc_line_stitched_rel(&side, S, true,  false, h-ro,    0.0f, true);
    lc_bend_strip       (&side, &bc_side, 0.0f, true);
    lc_line_stitched_rel(&side, S, false, false, w-ro-ro, 0.0f, true);
    lc_bend_strip       (&side, &bc_side, 0.0f, true);
    lc_line_stitched_rel(&side, S, false, true,  h-ro,    0.0f, true);
    vp_stroke(&side);

    // LHS end
    vp_new_path(&side, 0.0f, 0.0f);
    vp_line(&side, 0.0f, sideh);
    vp_stroke(&side);

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

// ------------------------------------------------------------------------- //

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
    stitch_line(&p, S, true, true, 1*hs, 0, 1*hs, 8*hs);
    stitch_line(&p, S, true, true, 3*hs, 0, 3*hs, 8*hs);

    close_plot(&p);

// ------------------------------------------------------------------------- //

    return 0;
}

