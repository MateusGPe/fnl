// Progress.hpp
#pragma once
#include "Theme.hpp"
#include <FL/Fl_Progress.H>
#include <FL/fl_draw.H>
#include <algorithm>

namespace mui
{
    class Progress : public Fl_Progress
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();
            // Dynamically update colors to reflect current theme
            color(palette.bg_main);
            selection_color(palette.selection);
            labelcolor(palette.fg_main);

            fl_push_clip(x(), y(), w(), h());

            fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), color());

            const double range = maximum() - minimum();
            const double val = range != 0.0 ? std::clamp((value() - minimum()) / range, 0.0, 1.0) : 0.0;

            const int track_h = palette.metrics.progress_height;
            const int track_y = y() + (h() - track_h) / 2;
            const int radius = track_h / 2;

            const int fill_w = static_cast<int>(val * w());

            fl_color(active_r() ? palette.selection : fl_inactive(palette.selection));
            fl_rounded_rectf(x(), track_y, w(), track_h, radius);

            const int empty_x = x() + fill_w;
            const int empty_w = w() - fill_w;
            if (empty_w > 0)
            {
                fl_color(active_r() ? palette.bg_sec : fl_inactive(palette.bg_sec));
                fl_push_clip(empty_x, track_y, empty_w, track_h);
                fl_rounded_rectf(x(), track_y, w(), track_h, radius);
                fl_pop_clip();
            }

            if (label() && label()[0])
            {
                fl_color(active_r() ? labelcolor() : fl_inactive(labelcolor()));
                fl_font(labelfont(), labelsize());
                fl_draw(label(), x(), y(), w(), h(), align());
            }

            fl_pop_clip();
        }

    public:
        Progress(int x, int y, int w, int h, const char *l = nullptr)
            : Fl_Progress(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
        }
    };
}