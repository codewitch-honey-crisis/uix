#pragma once
#include <gfx.hpp>
// the SSD1306 is a bit odd, in that it packs 8 pixels in a byte, arranged VERTICALLY, but then the bytes are arranged HORIZONTALLY
// since lcd_init.h does no translation, we do it here, such that when draws happen, they happen to this surface, which translates
// the memory into a format usable by the ssd1306
class ssd1306_surface_adapter final {
public:
    using type = ssd1306_surface_adapter;
    using pixel_type = gfx::gsc_pixel<1>;
    using palette_type = gfx::palette<pixel_type,pixel_type>;
    using caps = gfx::gfx_caps< false,false,false,false,false,true,false>;
private:
    // use an inner bitmap over our memory
    using bitmap_type = gfx::bitmap<pixel_type,palette_type>;
    bitmap_type m_bmp;  
    ssd1306_surface_adapter(const ssd1306_surface_adapter& rhs)=delete;
    ssd1306_surface_adapter& operator=(const ssd1306_surface_adapter& rhs)=delete;
public:
    ssd1306_surface_adapter(gfx::size16 dimensions,void* data, const palette_type* palette=nullptr) :
            m_bmp(gfx::size16(dimensions.width*8,dimensions.height/8),data,palette) {

    }
    gfx::size16 dimensions() const {
        return gfx::size16(m_bmp.dimensions().width/8,m_bmp.dimensions().height*8);
    }
    gfx::rect16 bounds() const {
        return dimensions().bounds();
    }
    gfx::gfx_result point(gfx::point16 location, pixel_type color) {
        const int x = (location.x * 8) + (7-(location.y % 8));
        const int y = location.y / 8;
        return m_bmp.point(gfx::point16(x,y),color);
    }
    gfx::gfx_result point(gfx::point16 location, pixel_type* out_color) const {
        const int x = (location.x * 8) + (7-(location.y % 8));
        const int y = location.y / 8;
        return m_bmp.point(gfx::point16(x,y),out_color);
    }
    gfx::gfx_result fill(const gfx::rect16& bounds, pixel_type color) {
        const gfx::rect16 b = bounds.normalize();
        for(int y = b.y1;y<=b.y2;++y) {
            for(int x = b.x1;x<=b.x2;++x) {
                point(gfx::point16(x,y),color);
            }
        }
        return gfx::gfx_result::success;
    }
    gfx::gfx_result clear(const gfx::rect16& bounds) {
        return fill(bounds,pixel_type());
    }
    void* begin() {
        return m_bmp.begin();
    }
};