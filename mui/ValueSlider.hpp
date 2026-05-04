// ValueSlider.hpp
#pragma once
#include "Policies.hpp"
#include <FL/Fl_Value_Slider.H>
#include <utility>

namespace mui
{
    class ValueSlider : public policy::ValueSliderDraw<policy::HoverTracker<policy::CallbackRouter<Fl_Value_Slider>>>
    {
    public:
        template <typename... Args>
        ValueSlider(int x, int y, int w, int h, const char *l = nullptr, Args &&...args)
            : policy::ValueSliderDraw<policy::HoverTracker<policy::CallbackRouter<Fl_Value_Slider>>>(x, y, w, h, l, std::forward<Args>(args)...)
        {
            align(FL_ALIGN_CENTER);
            box(FL_FLAT_BOX);
            color(mui::ThemeManager::get_palette().bg_main);
            selection_color(mui::ThemeManager::get_palette().selection);
            textcolor(mui::ThemeManager::get_palette().fg_main);
            labelcolor(mui::ThemeManager::get_palette().fg_main);
        }
    };
}
