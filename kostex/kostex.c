#include <stdio.h>
#include <math.h>
#include "../../common/vector_plot.h"
#include "../../common/leathercraft.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Kostex - drawstring bourse for RPG dice.
// Seam-first construction: seams (stitch_*), then rough cut insets.

static inline float deg2rad(float d) { return d * (float)M_PI / 180.0f; }

static const stitch_style_t S = { .hole_d = 1.4f, .pitch = 4.0f, .inset = 4.0f };

typedef struct { float x, y; } Vec2;

int main(void) {
    // --- constants ---
    const int   N       = 7;
    const float r_top   = 100.0f;
    const float r_bot   = 30.0f;
    const float fi_top  = 40.0f;  // rim latitude (deg)
    const float d_holes = 5.0f;   // drawstring hole diameter (mm)
    const float seg_deg = 360.0f / (float)N;

    const float fi_top_r = deg2rad(fi_top);
    const float fi_bot_r = acosf(r_bot / r_top * cosf(fi_top_r)); // derived from r_bot,r_top,fi_top
    const float fi_bot   = fi_bot_r * 180.0f / (float)M_PI;
    const float scale    = r_top / cosf(fi_top_r);
    const float L        = scale * (fi_bot_r - fi_top_r); // meridian length
    const float r_rim_flat = r_bot + L; // unrolled petal length
    const float depth    = scale * (sinf(fi_bot_r) - sinf(fi_top_r)); // bowl depth

    printf("kostex: N=%d r_top=%.2f r_bot=%.2f fi_top=%.1f fi_bot=%.1f L=%.2f flat=%.2f depth=%.2f\n",
           N, r_top, r_bot, fi_top, fi_bot, L, r_rim_flat, depth);

    // --- base: N seam arcs, one per petal ---
    {
        float pad = 10.0f;
        float size = 2.0f * (r_bot + pad);
        plot_t p = create_svg("kostex_base.svg", size, size);
        float cx = size * 0.5f, cy = size * 0.5f;

        for (int i = 0; i < N; i++) {
            float a0 = seg_deg * (float)i;
            float a1 = seg_deg * (float)(i + 1);
            stitch_arc(&p, S, true, false, cx, cy, r_bot, a0, a1);
        }

        vp_circle(&p, cx, cy, r_bot + S.inset); // cutout

        close_plot(&p);
    }

    // --- petal: same bottom seam arc + placeholder outer arc ---
    {
        float pad = 10.0f;
        float size = 2.0f * (r_rim_flat + pad);
        plot_t p = create_svg("kostex_petal.svg", size, size);
        float cx = size * 0.5f, cy = size * 0.5f;

        // Same seam arc as base (first segment) so it fits
        float a0 = -seg_deg * 0.5f;
        float a1 =  seg_deg * 0.5f;
        stitch_arc(&p, S, true, true, cx, cy, r_bot, a0, a1);

        // Side seams: Hermite gore Bezier, same bottom endpoints, endpoints on outer rim
        float a0r = deg2rad(a0), a1r = deg2rad(a1);
        float cos1 = cosf(a1r), sin1 = sinf(a1r);
        float cos0 = cosf(a0r), sin0 = sinf(a0r);

        Vec2 rb = { cx + r_bot * cos1, cy + r_bot * sin1 }; // right/lower bottom
        float hw_top = (float)M_PI / (float)N * r_top;
        Vec2 rt = { cx + r_bot + L, cy + hw_top }; // right/lower top (gore width)
        Vec2 lb = { cx + r_bot * cos0, cy + r_bot * sin0 }; // left/upper bottom
        Vec2 lt = { cx + r_bot + L, cy - hw_top }; // left/upper top (gore width)

        // Gore tangents: d(hw)/d(meridian) = (pi/N)*sin(phi)
        float m_bot = (float)M_PI / (float)N * sinf(fi_bot_r);
        float m_top = (float)M_PI / (float)N * sinf(fi_top_r);
        Vec2 p1, p2;
        // Right/lower: bottom -> top (Gemini Hermite: P1=P0+(H/3,m0*H/3), P2=P3-(H/3,m3*H/3))
        p1 = (Vec2){ rb.x + L / 3.0f, rb.y + m_bot * L / 3.0f };
        p2 = (Vec2){ rt.x - L / 3.0f, rt.y - m_top * L / 3.0f };
        stitch_curve(&p, S, false, false, rb.x, rb.y, p1.x, p1.y, p2.x, p2.y, rt.x, rt.y);

        // Left/upper: bottom -> top
        p1 = (Vec2){ lb.x + L / 3.0f, lb.y - m_bot * L / 3.0f };
        p2 = (Vec2){ lt.x - L / 3.0f, lt.y + m_top * L / 3.0f };
        stitch_curve(&p, S, false, false, lb.x, lb.y, p1.x, p1.y, p2.x, p2.y, lt.x, lt.y);

        // --- cutout (rough inset approximation) ---
        float inset = S.inset;
        float rc_bot = r_bot - inset;
        float delta_a = inset / rc_bot; // extend beyond 360/N by inset on each side
        float a0c = a0r - delta_a;
        float a1c = a1r + delta_a;
        Vec2 rb_c = { cx + rc_bot * cosf(a1c), cy + rc_bot * sinf(a1c) };
        Vec2 lb_c = { cx + rc_bot * cosf(a0c), cy + rc_bot * sinf(a0c) };

        // Top cut arc: level-rim tangent-cone arc, radius r_top/tan(fi_top), through seam holes
        float top_R = r_top / tanf(fi_top_r);
        float top_d = sqrtf(top_R*top_R - hw_top*hw_top);
        float top_cx = rt.x - top_d;
        float top_cy = cy;
        float top_a = asinf(hw_top / top_R);
        float top_ext = inset / top_R;
        Vec2 rt_c = { top_cx + top_R * cosf(top_a + top_ext), top_cy + top_R * sinf(top_a + top_ext) };
        Vec2 lt_c = { top_cx + top_R * cosf(-(top_a + top_ext)), top_cy + top_R * sinf(-(top_a + top_ext)) };

        vp_new_path(&p, rb_c.x, rb_c.y);
        vp_curve(&p, rb_c.x + L/3.0f, rb_c.y + m_bot*L/3.0f,
                     rt_c.x - L/3.0f, rt_c.y - m_top*L/3.0f,
                     rt_c.x, rt_c.y);
        vp_arc(&p, lt_c.x, lt_c.y, top_R, true, false); // level rim arc, extended 4mm each side
        vp_curve(&p, lt_c.x - L/3.0f, lt_c.y + m_top*L/3.0f,
                     lb_c.x + L/3.0f, lb_c.y - m_bot*L/3.0f,
                     lb_c.x, lb_c.y);
        vp_arc(&p, rb_c.x, rb_c.y, rc_bot, false, false); // bottom arc: r_bot-inset, extended
        vp_stroke(&p);

        // Drawstring holes: 2 per petal, evenly spaced around bowl (360/N/2 apart),
        // ~d_holes inward from outer rim (accounting for rim arc curvature).
        float hy_off = hw_top * 0.5f;
        float rim_clear = d_holes * 1.5f; // center distance from rim => ~d_holes edge clearance
        float hx = top_cx + sqrtf((top_R - rim_clear) * (top_R - rim_clear) - hy_off * hy_off);
        vp_circle(&p, hx, cy - hy_off, d_holes * 0.5f);
        vp_circle(&p, hx, cy + hy_off, d_holes * 0.5f);

        // --- reference lines (debug) ---
        // vp_new_path(&p, rb.x, rb.y); vp_line(&p, rt.x, rt.y); vp_stroke(&p);
        // vp_new_path(&p, lb.x, lb.y); vp_line(&p, lt.x, lt.y); vp_stroke(&p);
        // vp_new_path(&p, cx + r_top * cosf(deg2rad(a0)), cy + r_top * sinf(deg2rad(a0)));
        // vp_arc(&p, cx + r_top * cosf(deg2rad(a1)), cy + r_top * sinf(deg2rad(a1)),
        //        r_top, false, false);
        // vp_stroke(&p);

        close_plot(&p);
    }

    return 0;
}
