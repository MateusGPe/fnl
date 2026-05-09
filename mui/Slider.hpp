
#pragma once
#include "Policies.hpp"
#include <FL/Fl_Slider.H>
#include <algorithm>
#include <utility>

namespace mui
{
    class Slider : public policy::HoverTracker<policy::CallbackRouter<Fl_Slider>>
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();

            color(palette.bg_main);
            selection_color(palette.selection);
            labelcolor(palette.fg_main);

            fl_push_clip(x(), y(), w(), h());
            fl_draw_box(box(), x(), y(), w(), h(), color());

            const bool is_horiz = (type() == FL_HOR_SLIDER ||
                                   type() == FL_HOR_FILL_SLIDER ||
                                   type() == FL_HOR_NICE_SLIDER);

            const int thumb_pad = std::max(palette.metrics.slider_thumb_size,
                                           palette.metrics.slider_thumb_focus_halo_size);

            const int sl_x = x() + (is_horiz ? thumb_pad / 2 : 0);
            const int sl_y = y() + (is_horiz ? 0 : thumb_pad / 2);
            const int sl_w = std::max(0, w() - (is_horiz ? thumb_pad : 0));
            const int sl_h = std::max(0, h() - (is_horiz ? 0 : thumb_pad));

            const double range = maximum() - minimum();
            const double val = (range != 0.0)
                                   ? std::clamp((value() - minimum()) / range, 0.0, 1.0)
                                   : 0.0;

            const Fl_Color filled = active_r() ? palette.selection : fl_inactive(palette.selection);

            int cx, cy;
            core::calculate_slider_thumb_position_and_draw_track(
                x(), y(), w(), h(),
                sl_x, sl_y, sl_w, sl_h,
                val, is_horiz,
                palette.metrics.slider_track_height,
                filled, palette.bg_sec,
                cx, cy);

            draw_halo_if_needed(cx, cy, palette);
            core::draw_slider_thumb(cx, cy, palette.metrics.slider_thumb_size, filled);

            fl_pop_clip();
        }

    private:
        void draw_halo_if_needed(int cx, int cy, const ThemePalette &palette) const
        {
            if (!active_r())
                return;

            const float halo_op = palette.metrics.slider_thumb_hover_halo_opacity;

            if (Fl::focus() == this)
            {
                core::draw_slider_halo(cx, cy,
                                       palette.metrics.slider_thumb_focus_halo_size,
                                       fl_color_average(palette.focus_ring, palette.bg_main, halo_op * 2.0f));
            }
            else if (is_hovered)
            {
                core::draw_slider_halo(cx, cy,
                                       palette.metrics.slider_thumb_focus_halo_size,
                                       fl_color_average(palette.focus_ring, palette.bg_main, halo_op));
            }
        }

    public:
        Slider(int x, int y, int w, int h, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Slider>>(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
        }
    };
}
