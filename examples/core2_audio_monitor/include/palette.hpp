#pragma once
#include <gfx.hpp>
template<typename MappedPixelType> 
struct analyzer_palette {
    static_assert(!MappedPixelType::template has_channel_names<gfx::channel_name::index>::value,"Mapped pixel must not be indexed");
private:
    constexpr static gfx::gfx_result index_to_mapped(int idx,MappedPixelType* result) {
        idx=gfx::helpers::clamp(idx,0,255);
        return convert(palette_data[idx],result);
    }
public:
    using type = analyzer_palette;
    using pixel_type = gfx::indexed_pixel<8>;
    using mapped_pixel_type = MappedPixelType;
    constexpr static const bool writable = false;
    constexpr static const size_t size = 256;
    static const analyzer_palette instance;
    static const gfx::rgb_pixel<16> palette_data[];
    gfx::gfx_result map(pixel_type pixel,mapped_pixel_type* mapped_pixel) const {
        return index_to_mapped(pixel.channel<gfx::channel_name::index>(),mapped_pixel);
    }
    gfx::gfx_result nearest(mapped_pixel_type mapped_pixel,pixel_type* pixel) const {
        
        if(nullptr==pixel) {
            return gfx::gfx_result::invalid_argument;
        }
        mapped_pixel_type mpx;
        gfx::gfx_result r = index_to_mapped(0,&mpx);
        if(gfx::gfx_result::success!=r) {
            return r;
        }
        double least = mpx.difference(mapped_pixel);
        if(0.0==least) {
            pixel->native_value = 0;
            return gfx::gfx_result::success;
        }
        int ii=0;
        for(int i = 1;i<size;++i) {
            r=index_to_mapped(i,&mpx);
            if(gfx::gfx_result::success!=r) {
                return r;
            }
            double cmp = mpx.difference(mapped_pixel);
            if(0.0==cmp) {
                ii=i;
                least = 0.0;
                break;
            }
            if(cmp<least) {
                least = cmp;
                ii=i;
            }
        }
        pixel->template channel<gfx::channel_name::index>(ii);
        return gfx::gfx_result::success;
    }
};
template<typename MappedPixelType>
const analyzer_palette<MappedPixelType> analyzer_palette<MappedPixelType>::instance;

template<typename MappedPixelType>
const gfx::rgb_pixel<16> analyzer_palette<MappedPixelType>::palette_data[] = {
    gfx::rgb_pixel<16>(0, 0, 3),   gfx::rgb_pixel<16>(0, 0, 3),   gfx::rgb_pixel<16>(0, 0, 3),
    gfx::rgb_pixel<16>(0, 1, 3),   gfx::rgb_pixel<16>(0, 1, 3),   gfx::rgb_pixel<16>(0, 1, 3),
    gfx::rgb_pixel<16>(0, 1, 3),   gfx::rgb_pixel<16>(0, 2, 3),   gfx::rgb_pixel<16>(0, 2, 3),
    gfx::rgb_pixel<16>(0, 2, 3),   gfx::rgb_pixel<16>(0, 2, 3),   gfx::rgb_pixel<16>(0, 3, 3),
    gfx::rgb_pixel<16>(0, 3, 3),   gfx::rgb_pixel<16>(0, 3, 3),   gfx::rgb_pixel<16>(0, 3, 3),
    gfx::rgb_pixel<16>(0, 4, 3),   gfx::rgb_pixel<16>(0, 4, 3),   gfx::rgb_pixel<16>(0, 4, 3),
    gfx::rgb_pixel<16>(0, 4, 3),   gfx::rgb_pixel<16>(0, 5, 3),   gfx::rgb_pixel<16>(0, 5, 3),
    gfx::rgb_pixel<16>(0, 5, 3),   gfx::rgb_pixel<16>(0, 5, 2),   gfx::rgb_pixel<16>(0, 6, 2),
    gfx::rgb_pixel<16>(0, 6, 2),   gfx::rgb_pixel<16>(0, 6, 2),   gfx::rgb_pixel<16>(0, 6, 2),
    gfx::rgb_pixel<16>(0, 7, 2),   gfx::rgb_pixel<16>(0, 7, 2),   gfx::rgb_pixel<16>(0, 7, 2),
    gfx::rgb_pixel<16>(0, 7, 2),   gfx::rgb_pixel<16>(0, 8, 2),   gfx::rgb_pixel<16>(0, 8, 2),
    gfx::rgb_pixel<16>(0, 8, 2),   gfx::rgb_pixel<16>(0, 8, 2),   gfx::rgb_pixel<16>(0, 8, 2),
    gfx::rgb_pixel<16>(0, 9, 2),   gfx::rgb_pixel<16>(0, 9, 2),   gfx::rgb_pixel<16>(0, 9, 2),
    gfx::rgb_pixel<16>(0, 9, 2),   gfx::rgb_pixel<16>(0, 10, 2),  gfx::rgb_pixel<16>(0, 10, 2),
    gfx::rgb_pixel<16>(0, 10, 2),  gfx::rgb_pixel<16>(0, 10, 2),  gfx::rgb_pixel<16>(0, 11, 2),
    gfx::rgb_pixel<16>(0, 11, 2),  gfx::rgb_pixel<16>(0, 11, 2),  gfx::rgb_pixel<16>(0, 11, 2),
    gfx::rgb_pixel<16>(0, 12, 2),  gfx::rgb_pixel<16>(0, 12, 2),  gfx::rgb_pixel<16>(0, 12, 2),
    gfx::rgb_pixel<16>(0, 12, 2),  gfx::rgb_pixel<16>(0, 13, 2),  gfx::rgb_pixel<16>(0, 13, 2),
    gfx::rgb_pixel<16>(0, 13, 2),  gfx::rgb_pixel<16>(0, 13, 2),  gfx::rgb_pixel<16>(0, 14, 2),
    gfx::rgb_pixel<16>(0, 14, 2),  gfx::rgb_pixel<16>(0, 14, 2),  gfx::rgb_pixel<16>(0, 14, 2),
    gfx::rgb_pixel<16>(0, 15, 2),  gfx::rgb_pixel<16>(0, 15, 2),  gfx::rgb_pixel<16>(0, 15, 2),
    gfx::rgb_pixel<16>(0, 15, 2),  gfx::rgb_pixel<16>(0, 16, 2),  gfx::rgb_pixel<16>(0, 16, 1),
    gfx::rgb_pixel<16>(0, 16, 1),  gfx::rgb_pixel<16>(0, 16, 1),  gfx::rgb_pixel<16>(0, 16, 1),
    gfx::rgb_pixel<16>(0, 17, 1),  gfx::rgb_pixel<16>(0, 17, 1),  gfx::rgb_pixel<16>(0, 17, 1),
    gfx::rgb_pixel<16>(0, 17, 1),  gfx::rgb_pixel<16>(0, 18, 1),  gfx::rgb_pixel<16>(0, 18, 1),
    gfx::rgb_pixel<16>(0, 18, 1),  gfx::rgb_pixel<16>(0, 18, 1),  gfx::rgb_pixel<16>(0, 19, 1),
    gfx::rgb_pixel<16>(0, 19, 1),  gfx::rgb_pixel<16>(0, 19, 1),  gfx::rgb_pixel<16>(0, 19, 1),
    gfx::rgb_pixel<16>(0, 20, 1),  gfx::rgb_pixel<16>(0, 20, 1),  gfx::rgb_pixel<16>(0, 20, 1),
    gfx::rgb_pixel<16>(0, 20, 1),  gfx::rgb_pixel<16>(0, 21, 1),  gfx::rgb_pixel<16>(0, 21, 1),
    gfx::rgb_pixel<16>(0, 21, 1),  gfx::rgb_pixel<16>(0, 21, 1),  gfx::rgb_pixel<16>(0, 22, 1),
    gfx::rgb_pixel<16>(0, 22, 1),  gfx::rgb_pixel<16>(0, 22, 1),  gfx::rgb_pixel<16>(0, 22, 1),
    gfx::rgb_pixel<16>(0, 23, 1),  gfx::rgb_pixel<16>(0, 23, 1),  gfx::rgb_pixel<16>(0, 23, 1),
    gfx::rgb_pixel<16>(0, 23, 1),  gfx::rgb_pixel<16>(0, 23, 1),  gfx::rgb_pixel<16>(0, 24, 1),
    gfx::rgb_pixel<16>(0, 24, 1),  gfx::rgb_pixel<16>(0, 24, 1),  gfx::rgb_pixel<16>(0, 24, 1),
    gfx::rgb_pixel<16>(0, 25, 1),  gfx::rgb_pixel<16>(0, 25, 1),  gfx::rgb_pixel<16>(0, 25, 1),
    gfx::rgb_pixel<16>(0, 25, 1),  gfx::rgb_pixel<16>(0, 26, 1),  gfx::rgb_pixel<16>(0, 26, 0),
    gfx::rgb_pixel<16>(0, 26, 0),  gfx::rgb_pixel<16>(0, 26, 0),  gfx::rgb_pixel<16>(0, 27, 0),
    gfx::rgb_pixel<16>(0, 27, 0),  gfx::rgb_pixel<16>(0, 27, 0),  gfx::rgb_pixel<16>(0, 27, 0),
    gfx::rgb_pixel<16>(0, 28, 0),  gfx::rgb_pixel<16>(0, 28, 0),  gfx::rgb_pixel<16>(0, 28, 0),
    gfx::rgb_pixel<16>(0, 28, 0),  gfx::rgb_pixel<16>(0, 29, 0),  gfx::rgb_pixel<16>(0, 29, 0),
    gfx::rgb_pixel<16>(0, 29, 0),  gfx::rgb_pixel<16>(0, 29, 0),  gfx::rgb_pixel<16>(0, 30, 0),
    gfx::rgb_pixel<16>(0, 30, 0),  gfx::rgb_pixel<16>(0, 30, 0),  gfx::rgb_pixel<16>(0, 30, 0),
    gfx::rgb_pixel<16>(0, 31, 0),  gfx::rgb_pixel<16>(0, 31, 0),  gfx::rgb_pixel<16>(0, 31, 0),
    gfx::rgb_pixel<16>(0, 31, 0),  gfx::rgb_pixel<16>(1, 31, 0),  gfx::rgb_pixel<16>(1, 30, 0),
    gfx::rgb_pixel<16>(2, 31, 0),  gfx::rgb_pixel<16>(2, 31, 0),  gfx::rgb_pixel<16>(3, 31, 0),
    gfx::rgb_pixel<16>(3, 30, 0),  gfx::rgb_pixel<16>(3, 30, 0),  gfx::rgb_pixel<16>(4, 30, 0),
    gfx::rgb_pixel<16>(4, 30, 0),  gfx::rgb_pixel<16>(5, 30, 0),  gfx::rgb_pixel<16>(5, 30, 0),
    gfx::rgb_pixel<16>(5, 30, 0),  gfx::rgb_pixel<16>(6, 30, 0),  gfx::rgb_pixel<16>(6, 29, 0),
    gfx::rgb_pixel<16>(7, 29, 0),  gfx::rgb_pixel<16>(7, 29, 0),  gfx::rgb_pixel<16>(8, 30, 0),
    gfx::rgb_pixel<16>(8, 29, 0),  gfx::rgb_pixel<16>(8, 29, 0),  gfx::rgb_pixel<16>(9, 29, 0),
    gfx::rgb_pixel<16>(9, 28, 0),  gfx::rgb_pixel<16>(10, 28, 0), gfx::rgb_pixel<16>(10, 28, 0),
    gfx::rgb_pixel<16>(10, 28, 0), gfx::rgb_pixel<16>(11, 28, 0), gfx::rgb_pixel<16>(11, 28, 0),
    gfx::rgb_pixel<16>(12, 28, 0), gfx::rgb_pixel<16>(12, 28, 0), gfx::rgb_pixel<16>(12, 27, 0),
    gfx::rgb_pixel<16>(13, 27, 0), gfx::rgb_pixel<16>(13, 27, 0), gfx::rgb_pixel<16>(14, 28, 0),
    gfx::rgb_pixel<16>(14, 27, 0), gfx::rgb_pixel<16>(15, 27, 0), gfx::rgb_pixel<16>(15, 27, 0),
    gfx::rgb_pixel<16>(15, 27, 0), gfx::rgb_pixel<16>(15, 26, 0), gfx::rgb_pixel<16>(15, 26, 0),
    gfx::rgb_pixel<16>(15, 25, 0), gfx::rgb_pixel<16>(15, 25, 0), gfx::rgb_pixel<16>(15, 24, 0),
    gfx::rgb_pixel<16>(15, 24, 0), gfx::rgb_pixel<16>(15, 23, 0), gfx::rgb_pixel<16>(15, 23, 0),
    gfx::rgb_pixel<16>(15, 22, 0), gfx::rgb_pixel<16>(15, 22, 0), gfx::rgb_pixel<16>(15, 21, 0),
    gfx::rgb_pixel<16>(15, 21, 0), gfx::rgb_pixel<16>(16, 22, 0), gfx::rgb_pixel<16>(15, 21, 0),
    gfx::rgb_pixel<16>(15, 21, 0), gfx::rgb_pixel<16>(15, 20, 0), gfx::rgb_pixel<16>(15, 20, 0),
    gfx::rgb_pixel<16>(15, 19, 0), gfx::rgb_pixel<16>(15, 19, 0), gfx::rgb_pixel<16>(15, 18, 0),
    gfx::rgb_pixel<16>(15, 18, 0), gfx::rgb_pixel<16>(15, 17, 0), gfx::rgb_pixel<16>(15, 17, 0),
    gfx::rgb_pixel<16>(15, 16, 0), gfx::rgb_pixel<16>(15, 16, 0), gfx::rgb_pixel<16>(15, 15, 0),
    gfx::rgb_pixel<16>(15, 15, 0), gfx::rgb_pixel<16>(15, 15, 0), gfx::rgb_pixel<16>(15, 15, 0),
    gfx::rgb_pixel<16>(15, 14, 0), gfx::rgb_pixel<16>(15, 14, 0), gfx::rgb_pixel<16>(15, 14, 0),
    gfx::rgb_pixel<16>(15, 14, 0), gfx::rgb_pixel<16>(15, 13, 0), gfx::rgb_pixel<16>(15, 13, 0),
    gfx::rgb_pixel<16>(15, 13, 0), gfx::rgb_pixel<16>(15, 13, 0), gfx::rgb_pixel<16>(15, 12, 0),
    gfx::rgb_pixel<16>(15, 12, 0), gfx::rgb_pixel<16>(15, 12, 0), gfx::rgb_pixel<16>(15, 12, 0),
    gfx::rgb_pixel<16>(15, 11, 0), gfx::rgb_pixel<16>(15, 11, 0), gfx::rgb_pixel<16>(15, 11, 0),
    gfx::rgb_pixel<16>(15, 11, 0), gfx::rgb_pixel<16>(15, 10, 0), gfx::rgb_pixel<16>(15, 10, 0),
    gfx::rgb_pixel<16>(15, 10, 0), gfx::rgb_pixel<16>(15, 10, 0), gfx::rgb_pixel<16>(15, 10, 0),
    gfx::rgb_pixel<16>(15, 9, 0),  gfx::rgb_pixel<16>(15, 9, 0),  gfx::rgb_pixel<16>(15, 9, 0),
    gfx::rgb_pixel<16>(15, 9, 0),  gfx::rgb_pixel<16>(15, 8, 0),  gfx::rgb_pixel<16>(15, 8, 0),
    gfx::rgb_pixel<16>(15, 8, 0),  gfx::rgb_pixel<16>(15, 8, 0),  gfx::rgb_pixel<16>(15, 7, 0),
    gfx::rgb_pixel<16>(15, 7, 0),  gfx::rgb_pixel<16>(15, 7, 0),  gfx::rgb_pixel<16>(15, 7, 0),
    gfx::rgb_pixel<16>(15, 6, 0),  gfx::rgb_pixel<16>(15, 6, 0),  gfx::rgb_pixel<16>(15, 6, 0),
    gfx::rgb_pixel<16>(15, 6, 0),  gfx::rgb_pixel<16>(15, 5, 0),  gfx::rgb_pixel<16>(15, 5, 0),
    gfx::rgb_pixel<16>(15, 5, 0),  gfx::rgb_pixel<16>(15, 5, 0),  gfx::rgb_pixel<16>(15, 5, 0),
    gfx::rgb_pixel<16>(15, 4, 0),  gfx::rgb_pixel<16>(15, 4, 0),  gfx::rgb_pixel<16>(15, 4, 0),
    gfx::rgb_pixel<16>(15, 4, 0),  gfx::rgb_pixel<16>(15, 3, 0),  gfx::rgb_pixel<16>(15, 3, 0),
    gfx::rgb_pixel<16>(15, 3, 0),  gfx::rgb_pixel<16>(15, 3, 0),  gfx::rgb_pixel<16>(15, 2, 0),
    gfx::rgb_pixel<16>(15, 2, 0),  gfx::rgb_pixel<16>(15, 2, 0),  gfx::rgb_pixel<16>(15, 2, 0),
    gfx::rgb_pixel<16>(15, 1, 0),  gfx::rgb_pixel<16>(15, 1, 0),  gfx::rgb_pixel<16>(15, 1, 0),
    gfx::rgb_pixel<16>(15, 1, 0),  gfx::rgb_pixel<16>(15, 0, 0),  gfx::rgb_pixel<16>(15, 0, 0),
    gfx::rgb_pixel<16>(15, 0, 0)};