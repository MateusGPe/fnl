#pragma once

#include "Types.hpp"
#include <algorithm>
#include <cmath>

namespace mui
{
    class PixelBlender
    {
    public:
        static inline void blend_pixels(BlendMode mode, float fr, float fg, float fb, float br, float bg, float bb, float alpha, uchar *out)
        {
            float out_r = fr, out_g = fg, out_b = fb;

            if (mode == BlendMode::Multiply)
            {
                out_r = (fr * br) / 255.0f;
                out_g = (fg * bg) / 255.0f;
                out_b = (fb * bb) / 255.0f;
            }
            else if (mode == BlendMode::Screen)
            {
                out_r = 255.0f - ((255.0f - fr) * (255.0f - br)) / 255.0f;
                out_g = 255.0f - ((255.0f - fg) * (255.0f - bg)) / 255.0f;
                out_b = 255.0f - ((255.0f - fb) * (255.0f - bb)) / 255.0f;
            }
            else if (mode == BlendMode::Overlay)
            {
                out_r = (br < 128) ? (2.0f * fr * br / 255.0f) : (255.0f - 2.0f * (255.0f - fr) * (255.0f - br) / 255.0f);
                out_g = (bg < 128) ? (2.0f * fg * bg / 255.0f) : (255.0f - 2.0f * (255.0f - fg) * (255.0f - bg) / 255.0f);
                out_b = (bb < 128) ? (2.0f * fb * bb / 255.0f) : (255.0f - 2.0f * (255.0f - fb) * (255.0f - bb) / 255.0f);
            }

            out[0] = static_cast<uchar>(std::clamp(out_r * alpha + br * (1.0f - alpha), 0.0f, 255.0f));
            out[1] = static_cast<uchar>(std::clamp(out_g * alpha + bg * (1.0f - alpha), 0.0f, 255.0f));
            out[2] = static_cast<uchar>(std::clamp(out_b * alpha + bb * (1.0f - alpha), 0.0f, 255.0f));
        }

        static inline void bilerp(const uchar *fg_data, float src_px, float src_py, int img_w, int img_h, int fg_ld, int fg_d, float &fr, float &fg, float &fb, float &fa)
        {
            const int x1_full = static_cast<int>(std::floor(src_px));
            const int y1_full = static_cast<int>(std::floor(src_py));
            const float wx = src_px - x1_full;
            const float wy = src_py - y1_full;

            const int x1 = std::clamp(x1_full, 0, img_w - 1);
            const int y1 = std::clamp(y1_full, 0, img_h - 1);
            const int x2 = std::min(x1 + 1, img_w - 1);
            const int y2 = std::min(y1 + 1, img_h - 1);

            const int idx11 = y1 * fg_ld + x1 * fg_d;
            const int idx21 = y1 * fg_ld + x2 * fg_d;
            const int idx12 = y2 * fg_ld + x1 * fg_d;
            const int idx22 = y2 * fg_ld + x2 * fg_d;

            auto lerp_chan = [&](int offset)
            {
                const float c11 = fg_data[idx11 + offset];
                const float c21 = fg_data[idx21 + offset];
                const float c12 = fg_data[idx12 + offset];
                const float c22 = fg_data[idx22 + offset];
                return (c11 * (1.0f - wx) + c21 * wx) * (1.0f - wy) + (c12 * (1.0f - wx) + c22 * wx) * wy;
            };

            fr = lerp_chan(0);
            fg = lerp_chan(1);
            fb = lerp_chan(2);
            fa = (fg_d == 4) ? lerp_chan(3) : 255.0f;
        }
    };
}