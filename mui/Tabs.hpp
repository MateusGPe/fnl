// Tabs.hpp
#pragma once
#include "Theme.hpp"
#include <FL/Fl_Tabs.H>
#include <FL/fl_draw.H>

namespace mui
{
    class Tabs : public Fl_Tabs
    {
    public:
        Tabs(int x, int y, int w, int h, const char *l = nullptr)
            : Fl_Tabs(x, y, w, h, l)
        {
            const auto &palette = ThemeManager::get_palette();

            // Remove the 90s 3D border around the pane
            box(FL_UP_BOX);
            color(palette.bg_sec);
            selection_color(palette.bg_main);
        }
    };
}