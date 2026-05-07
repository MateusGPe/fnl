// Slider.hpp
#pragma once
#include "Policies.hpp"
#include <FL/Fl_Slider.H>
#include <utility>
#include <algorithm>

namespace mui
{
    class Slider : public policy::HoverTracker<policy::CallbackRouter<Fl_Slider>>
    {
    protected:
        void draw() override
        {
            fl_push_clip(x(), y(), w(), h());
            fl_draw_box(box(), x(), y(), w(), h(), color());

            bool is_horiz = (type() == FL_HOR_SLIDER || type() == FL_HOR_FILL_SLIDER || type() == FL_HOR_NICE_SLIDER);
            
            const auto& palette = mui::ThemeManager::get_palette();
            
            int track_thickness = palette.metrics.slider_track_height;
            int thumb_size = palette.metrics.slider_thumb_size;
            int halo_size = palette.metrics.slider_thumb_focus_halo_size;
            int thumb_padding = std::max(thumb_size, halo_size);
            
            int slider_x = x() + (is_horiz ? thumb_padding / 2 : 0);
            int slider_y = y() + (is_horiz ? 0 : thumb_padding / 2);
            int slider_w = std::max(0, w() - (is_horiz ? thumb_padding : 0));
            int slider_h = std::max(0, h() - (is_horiz ? 0 : thumb_padding));
            
            double range = maximum() - minimum();
            double val = range != 0.0 ? std::clamp((value() - minimum()) / range, 0.0, 1.0) : 0.0;
            
            int cx, cy;
            
            if (is_horiz) {
                cx = slider_x + static_cast<int>(val * slider_w);
                cy = y() + h() / 2;
                
                core::draw_slider_track_split(
                    slider_x, cx, slider_x + slider_w, cy, 
                    track_thickness, 
                    active_r() ? palette.selection : fl_inactive(palette.selection), 
                    palette.inactive, // Force empty side of track to look disabled
                    true
                );
            } else {
                cx = x() + w() / 2;
                cy = slider_y + static_cast<int>((1.0 - val) * slider_h);
                
                core::draw_slider_track_split(
                    slider_y, cy, slider_y + slider_h, cx, 
                    track_thickness, 
                    active_r() ? palette.selection : fl_inactive(palette.selection), 
                    palette.inactive, // Force empty side of track to look disabled
                    false
                );
            }
            
            if (active_r()) {
                if (Fl::focus() == this) {
                    core::draw_slider_halo(cx, cy, palette.metrics.slider_thumb_focus_halo_size, 
                        fl_color_average(palette.selection, palette.bg_main, palette.metrics.slider_thumb_hover_halo_opacity * 2.0f)); 
                } else if (is_hovered) {
                     core::draw_slider_halo(cx, cy, palette.metrics.slider_thumb_focus_halo_size, 
                        fl_color_average(palette.selection, palette.bg_main, palette.metrics.slider_thumb_hover_halo_opacity)); 
                }
            }
            
            core::draw_slider_thumb(cx, cy, thumb_size, active_r() ? palette.selection : fl_inactive(palette.selection));
            
            fl_pop_clip();
        }

    public:
        template <typename... Args>
        Slider(int x, int y, int w, int h, const char *l = nullptr, Args &&...args)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Slider>>(x, y, w, h, l, std::forward<Args>(args)...)
        {
            Fl_Slider::box(FL_FLAT_BOX);
        }
    };
}
