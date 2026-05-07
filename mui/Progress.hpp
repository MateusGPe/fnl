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
            fl_push_clip(x(), y(), w(), h());

            const auto& palette = ThemeManager::get_palette();

            // Clear the widget area using the assigned background color
            fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), color());

            double range = maximum() - minimum();
            double val = range != 0.0 ? std::clamp((value() - minimum()) / range, 0.0, 1.0) : 0.0;

            int track_h = palette.metrics.progress_height;
            int track_y = y() + (h() - track_h) / 2;
            int radius = palette.metrics.progress_corner_radius;

            // Draw the empty background track
            fl_color(active_r() ? palette.bg_sec : fl_inactive(palette.bg_sec));
            fl_rounded_rectf(x(), track_y, w(), track_h, radius);

            // Draw the active filled track
            int fill_w = static_cast<int>(val * w());
            if (fill_w > 0)
            {
                // Clip the fill so it naturally chops off at the exact pixel percentage,
                // while ensuring the underlying rect is wide enough to not mangle the corner radius.
                fl_push_clip(x(), track_y, fill_w, track_h);
                fl_color(active_r() ? palette.selection : fl_inactive(palette.selection));
                fl_rounded_rectf(x(), track_y, std::max(fill_w, track_h * 2), track_h, radius);
                fl_pop_clip();
            }

            // Draw the standard label over the bar if one is provided
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
            // Set up sane flat defaults to prevent native artifacts
            box(FL_FLAT_BOX);
            color(ThemeManager::get_palette().bg_main);
            selection_color(ThemeManager::get_palette().selection);
            labelcolor(ThemeManager::get_palette().fg_main);
        }
    };
}