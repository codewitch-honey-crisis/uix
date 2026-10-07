#ifndef HTCW_UIX_SHAPE_HELPERS_HPP
#define HTCW_UIX_SHAPE_HELPERS_HPP
#include "uix_core.hpp"
namespace uix {
namespace helpers {
/// @brief Shared integer-only, anti-aliased shape drawing used by the slider and switch controls.
/// All borders are stroked inward.
class shape_helpers {
   public:
    /// @brief Collapses a size16 of radiuses to the single uniform radius the aa routines take
    /// (the smaller of the two, ignoring a zero component)
    static int16_t uniform_radius(size16 r) {
        if (r.width == 0) return (int16_t)r.height;
        if (r.height == 0) return (int16_t)r.width;
        return (int16_t)(r.width < r.height ? r.width : r.height);
    }
    /// @brief Indicates whether two rectangles intersect
    static bool overlaps(const srect16& a, const srect16& b) {
        return !(a.x2 < b.x1 || a.x1 > b.x2 || a.y2 < b.y1 || a.y1 > b.y2);
    }
    /// @brief Fills a disc (circle=true, inscribed in the upper-left square of r),
    /// a plain rect (radius <= 0) or an AA rounded rect
    template <typename Destination>
    static void fill_shape(Destination& dst, const srect16& r, uix_pixel color, int16_t radius, bool circle, gfx::mask_draw_cache* dc, const srect16& clip) {
        if (r.x2 < r.x1 || r.y2 < r.y1) {
            return;
        }
        if (circle) {
            gfx::draw::aa_filled_arc(dst, r, color, 0, 360, dc, &clip);
        } else if (radius <= 0) {
            gfx::draw::filled_rectangle(dst, r, color, &clip);
        } else {
            // a border width that spans the box yields a solid rounded rect
            const int16_t w = (int16_t)r.width(), h = (int16_t)r.height();
            gfx::draw::aa_rounded_rectangle(dst, r, color, radius, w < h ? w : h, dc, &clip);
        }
    }
    /// @brief Strokes only the border ring (inward, width bw), leaving the interior untouched
    template <typename Destination>
    static void stroke_shape(Destination& dst, const srect16& r, uix_pixel color, int16_t bw, int16_t radius, bool circle, gfx::mask_draw_cache* dc, const srect16& clip) {
        if (r.x2 < r.x1 || r.y2 < r.y1 || bw <= 0) {
            return;
        }
        if (circle) {
            gfx::draw::aa_arc(dst, r, color, 0, 360, bw, gfx::line_cap::butt, dc, &clip);
        } else if (radius > 0) {
            gfx::draw::aa_rounded_rectangle(dst, r, color, radius, bw, dc, &clip);
        } else {
            const int16_t w = (int16_t)r.width(), h = (int16_t)r.height();
            if (2 * bw >= w || 2 * bw >= h) {
                gfx::draw::filled_rectangle(dst, r, color, &clip);  // all border, no hole
                return;
            }
            // four non-overlapping strips: top, bottom, left, right
            gfx::draw::filled_rectangle(dst, srect16(r.x1, r.y1, r.x2, r.y1 + bw - 1), color, &clip);
            gfx::draw::filled_rectangle(dst, srect16(r.x1, r.y2 - bw + 1, r.x2, r.y2), color, &clip);
            gfx::draw::filled_rectangle(dst, srect16(r.x1, r.y1 + bw, r.x1 + bw - 1, r.y2 - bw), color, &clip);
            gfx::draw::filled_rectangle(dst, srect16(r.x2 - bw + 1, r.y1 + bw, r.x2, r.y2 - bw), color, &clip);
        }
    }
    /// @brief Draws a shape with an inward border of width bw.
    /// Opaque fill: the whole shape in the border color, then the fill inset on top.
    ///   The fill's AA edge blends over the border color, so the seam is perfect.
    /// Translucent fill: stroke the ring only, then fill the hole, so the border never
    ///   bleeds through the fill. On curves the two complementary AA edges can let a
    ///   little background through at the seam; straight integer edges are crisp.
    /// Both paths use the same inner radius (the stroke routines keep the outer radius
    /// on the inner edge, clamped to the inner box) so they look identical.
    /// Skips everything if r doesn't intersect clip.
    template <typename Destination>
    static void draw_bordered(Destination& dst, const srect16& r, uix_pixel fill, uix_pixel border, int16_t bw, int16_t radius, bool circle, gfx::mask_draw_cache* dc, const srect16& clip) {
        if (!overlaps(r, clip)) {
            return;  // nothing of this shape is in the dirty region
        }
        if (bw <= 0) {
            fill_shape(dst, r, fill, radius, circle, dc, clip);
            return;
        }
        if (fill.opacity8() == 255) {
            fill_shape(dst, r, border, radius, circle, dc, clip);
        } else {
            stroke_shape(dst, r, border, bw, radius, circle, dc, clip);
        }
        fill_shape(dst, r.inflate(-bw, -bw), fill, radius, circle, dc, clip);
    }
};
}  // namespace helpers
}  // namespace uix
#endif  // HTCW_UIX_SHAPE_HELPERS_HPP