// ValueSlider.hpp
#pragma once
#include "Policies.hpp"
#include <FL/Fl_Value_Slider.H>
#include <utility>
#include <algorithm>
#include <cstdio>

namespace mui
{
    class ValueSlider : public policy::HoverTracker<policy::CallbackRouter<Fl_Value_Slider>>
    {
    protected:
        void draw() override
        {
            fl_push_clip(x(), y(), w(), h());
            fl_draw_box(box(), x(), y(), w(), h(), color());
            const auto &palette = mui::ThemeManager::get_palette();

            int t_w = w(), t_h = h();
            int s_x = x(), s_y = y(), s_w = w(), s_h = h();
            int t_x = x(), t_y = y();

            bool is_horiz = (type() == FL_HOR_SLIDER || type() == FL_HOR_FILL_SLIDER || type() == FL_HOR_NICE_SLIDER);

            int text_area_w = palette.metrics.valueslider_input_width;
            int text_area_h = palette.metrics.valueslider_input_height;
            int gap = palette.metrics.valueslider_gap;

            if (is_horiz)
            {
                text_area_w = std::clamp(text_area_w, 0, std::max(0, w() - palette.metrics.valueslider_min_slider_size));
                t_w = text_area_w;
                t_x = x();
                s_x = x() + text_area_w + gap;
                s_w = w() - text_area_w - gap;
            }
            else
            {
                text_area_h = std::clamp(text_area_h, 0, std::max(0, h() - palette.metrics.valueslider_min_slider_size));
                t_h = text_area_h;
                t_y = y();
                s_y = y() + text_area_h + gap;
                s_h = h() - text_area_h - gap;
            }

            fl_draw_box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX, t_x, t_y, t_w, t_h, palette.input_bg);

            char buf[64];
            format(buf);
            fl_color(active_r() ? textcolor() : fl_inactive(textcolor()));
            fl_font(textfont(), textsize());
            fl_draw(buf, t_x, t_y, t_w, t_h, FL_ALIGN_CENTER);

            int track_thickness = palette.metrics.slider_track_height;
            int thumb_size = palette.metrics.slider_thumb_size;
            int halo_size = palette.metrics.slider_thumb_focus_halo_size;
            int thumb_padding = std::max(thumb_size, halo_size);

            int slider_x = s_x + (is_horiz ? thumb_padding / 2 : 0);
            int slider_y = s_y + (is_horiz ? 0 : thumb_padding / 2);
            int slider_w = std::max(0, s_w - (is_horiz ? thumb_padding : 0));
            int slider_h = std::max(0, s_h - (is_horiz ? 0 : thumb_padding));

            double range = maximum() - minimum();
            double val = range != 0.0 ? std::clamp((value() - minimum()) / range, 0.0, 1.0) : 0.0;

            int cx, cy;

            if (is_horiz)
            {
                cx = slider_x + static_cast<int>(val * slider_w);
                cy = s_y + s_h / 2;

                core::draw_slider_track_split(
                    slider_x, cx, slider_x + slider_w, cy,
                    track_thickness,
                    active_r() ? palette.selection : fl_inactive(palette.selection),
                    palette.inactive, // Force empty side of track to look disabled
                    true);
            }
            else
            {
                cx = s_x + s_w / 2;
                cy = slider_y + static_cast<int>((1.0 - val) * slider_h);

                core::draw_slider_track_split(
                    slider_y, cy, slider_y + slider_h, cx,
                    track_thickness,
                    active_r() ? palette.selection : fl_inactive(palette.selection),
                    palette.inactive, // Force empty side of track to look disabled
                    false);
            }

            if (active_r())
            {
                if (Fl::focus() == this)
                {
                    core::draw_slider_halo(cx, cy, palette.metrics.slider_thumb_focus_halo_size,
                                           fl_color_average(palette.selection, palette.bg_main, palette.metrics.slider_thumb_hover_halo_opacity * 2.0f));
                }
                else if (is_hovered)
                {
                    core::draw_slider_halo(cx, cy, palette.metrics.slider_thumb_focus_halo_size,
                                           fl_color_average(palette.selection, palette.bg_main, palette.metrics.slider_thumb_hover_halo_opacity));
                }
            }

            core::draw_slider_thumb(cx, cy, thumb_size, active_r() ? palette.selection : fl_inactive(palette.selection));

            fl_pop_clip();
        }

    public:
        template <typename... Args>
        ValueSlider(int x, int y, int w, int h, const char *l = nullptr, Args &&...args)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Value_Slider>>(x, y, w, h, l, std::forward<Args>(args)...)
        {
            textcolor(mui::ThemeManager::get_palette().fg_main);
            Fl_Value_Slider::box(FL_FLAT_BOX);
        }
    };
}