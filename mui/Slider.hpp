
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
            if (damage() & ~FL_DAMAGE_CHILD)
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

            const double val = core::normalize_value(value(), minimum(), maximum());

            const Fl_Color filled = policy::resolve_color_active(this, palette.selection);

            int cx, cy;
            core::calculate_slider_thumb_position_and_draw_track(
                x(), y(), w(), h(),
                sl_x, sl_y, sl_w, sl_h,
                val, is_horiz,
                palette.metrics.slider_track_height,
                filled, palette.bg_sec,
                cx, cy);

            engine::draw_slider_thumb_with_halo(this, is_hovered, active_r(), cx, cy, filled, palette);

            fl_pop_clip();
        }

    public:
        Slider(int x, int y, int w, int h, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Slider>>(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
        }
    };
}
