#pragma once
#include "Policies.hpp"
#include <FL/Fl_Value_Slider.H>
#include <algorithm>
#include <cstdio>
#include <utility>

namespace mui
{
    class ValueSlider : public policy::HoverTracker<policy::CallbackRouter<Fl_Value_Slider>>
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();

            color(palette.bg_main);
            selection_color(palette.selection);
            labelcolor(palette.fg_main);
            textcolor(palette.fg_main);

            fl_push_clip(x(), y(), w(), h());
            if (damage() & ~FL_DAMAGE_CHILD)
                fl_draw_box(box(), x(), y(), w(), h(), color());

            const bool is_horiz = (type() == FL_HOR_SLIDER ||
                                   type() == FL_HOR_FILL_SLIDER ||
                                   type() == FL_HOR_NICE_SLIDER);

            const auto [tx, ty, tw, th, sx, sy, sw, sh] =
                split_areas(is_horiz, palette);

            if (damage() & ~FL_DAMAGE_CHILD)
                fl_draw_box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX,
                            tx, ty, tw, th, palette.input_bg);

            char buf[64];
            format(buf);
            fl_color(policy::resolve_color_active(this, textcolor()));
            fl_font(textfont(), textsize());
            fl_draw(buf, tx, ty, tw, th, FL_ALIGN_CENTER);

            const int thumb_pad = std::max(palette.metrics.slider_thumb_size,
                                           palette.metrics.slider_thumb_focus_halo_size);

            const int sl_x = sx + (is_horiz ? thumb_pad / 2 : 0);
            const int sl_y = sy + (is_horiz ? 0 : thumb_pad / 2);
            const int sl_w = std::max(0, sw - (is_horiz ? thumb_pad : 0));
            const int sl_h = std::max(0, sh - (is_horiz ? 0 : thumb_pad));

            const double val = core::normalize_value(value(), minimum(), maximum());

            const Fl_Color filled = policy::resolve_color_active(this, palette.selection);

            int cx, cy;
            core::calculate_slider_thumb_position_and_draw_track(
                sx, sy, sw, sh,
                sl_x, sl_y, sl_w, sl_h,
                val, is_horiz,
                palette.metrics.slider_track_height,
                filled, palette.bg_sec,
                cx, cy);

            engine::draw_slider_thumb_with_halo(this, is_hovered, active_r(), cx, cy, filled, palette);

            fl_pop_clip();
        }

    private:
        struct Areas
        {
            int tx, ty, tw, th, sx, sy, sw, sh;
        };
        [[nodiscard]] Areas split_areas(bool is_horiz, const ThemePalette &palette) const
        {
            const int gap = palette.metrics.valueslider_gap;
            Areas a{};

            if (is_horiz)
            {
                a.tw = std::clamp(palette.metrics.valueslider_input_width,
                                  0, std::max(0, w() - palette.metrics.valueslider_min_slider_size));
                a.th = h();
                a.tx = x();
                a.ty = y();
                a.sx = x() + a.tw + gap;
                a.sy = y();
                a.sw = w() - a.tw - gap;
                a.sh = h();
            }
            else
            {
                a.tw = w();
                a.th = std::clamp(palette.metrics.valueslider_input_height,
                                  0, std::max(0, h() - palette.metrics.valueslider_min_slider_size));
                a.tx = x();
                a.ty = y();
                a.sx = x();
                a.sy = y() + a.th + gap;
                a.sw = w();
                a.sh = h() - a.th - gap;
            }
            return a;
        }

    public:
        ValueSlider(int x, int y, int w, int h, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Value_Slider>>(x, y, w, h, l)
        {
            Fl_Value_Slider::box(FL_FLAT_BOX);
        }
    };
}
