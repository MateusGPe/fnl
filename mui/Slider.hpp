// Slider.hpp
#pragma once
#include "Policies.hpp"
#include <FL/Fl_Slider.H>
#include <utility>

namespace mui
{
    class Slider : public policy::SliderDraw<policy::HoverTracker<policy::CallbackRouter<Fl_Slider>>>
    {
    public:
        template <typename... Args>
        Slider(int x, int y, int w, int h, const char *l = nullptr, Args &&...args)
            : policy::SliderDraw<policy::HoverTracker<policy::CallbackRouter<Fl_Slider>>>(x, y, w, h, l, std::forward<Args>(args)...)
        {
            align(FL_ALIGN_CENTER);
            box(FL_FLAT_BOX);
            color(mui::ThemeManager::get_palette().bg_main);
            selection_color(mui::ThemeManager::get_palette().selection);
            labelcolor(mui::ThemeManager::get_palette().fg_main);
        }
    };
}
