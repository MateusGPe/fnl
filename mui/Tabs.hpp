// Tabs.hpp
#pragma once
#include "Theme.hpp"
#include "Policies.hpp"
#include <FL/Fl_Tabs.H>
#include <FL/fl_draw.H>
#include <cmath>

namespace mui
{
    class Tabs : public policy::HoverTracker<policy::CallbackRouter<Fl_Tabs>>
    {
    private:
        Fl_Widget *m_last_hovered_tab = nullptr;

    protected:
        int handle(int event) override
        {
            int res = policy::HoverTracker<policy::CallbackRouter<Fl_Tabs>>::handle(event);

            if (!active_r())
                return res;

            switch (event)
            {
            case FL_MOVE:
            case FL_ENTER:
            case FL_LEAVE:
            {
                Fl_Widget *hw = (event == FL_LEAVE) ? nullptr : which(Fl::event_x(), Fl::event_y());

                if (hw != m_last_hovered_tab)
                {
                    m_last_hovered_tab = hw;
                    // FL_DAMAGE_EXPOSE explicitly tells FLTK to only redraw the tab bar, bypassing child pane repaints.
                    damage(FL_DAMAGE_EXPOSE);
                }
                res = 1;
                break;
            }
            }
            return res;
        }

        void draw() override
        {
            // Capture damage state before Fl_Tabs::draw() clears it
            uchar d = damage();

            // ENHANCEMENT: Only update the tab bar colors/styles if the tab bar itself
            // is damaged. If only a child is updating (FL_DAMAGE_CHILD), skip this loop.
            if (d & (FL_DAMAGE_ALL | FL_DAMAGE_EXPOSE | FL_DAMAGE_SCROLL))
            {
                const auto &palette = ThemeManager::get_palette();

                // Configure flat widgets to eliminate the 3D structures
                box(FL_FLAT_BOX);
                color(palette.bg_sec);
                selection_color(palette.bg_main);

                Fl_Color text_active = active_r() ? palette.selection : fl_inactive(palette.selection);
                Fl_Color text_inactive = active_r() ? palette.fg_main : fl_inactive(palette.fg_main);

                labelcolor(text_active);

                for (int i = 0; i < children(); ++i)
                {
                    Fl_Widget *c = child(i);
                    c->color(palette.bg_main);

                    if (c == m_last_hovered_tab && c != value())
                    {
                        c->selection_color(fl_color_average(palette.bg_main, palette.bg_sec, 0.5f));
                        c->labelcolor(text_active);
                    }
                    else
                    {
                        c->selection_color(palette.bg_sec);
                        c->labelcolor(text_inactive);
                    }
                }
            }

            // Let FLTK perform its native clipping, child updating, and drawing algorithms
            // NOTE: This call clears the widget's damage flags.
            Fl_Tabs::draw();

            if (children() == 0)
                return;

            // ENHANCEMENT: Only draw custom overlays if the tab area was redrawn
            if (d & (FL_DAMAGE_ALL | FL_DAMAGE_EXPOSE | FL_DAMAGE_SCROLL))
            {
                int sel = tab_positions();

                if (sel >= 0 && sel < children())
                {
                    const auto &palette = ThemeManager::get_palette();
                    Fl_Color text_active = active_r() ? palette.selection : fl_inactive(palette.selection);

                    fl_push_clip(x(), y(), w(), h());

                    // Leverage Fl_Tabs protected native arrays for O(1) coordinate lookups
                    int tx = x() + tab_pos[sel] + tab_offset;
                    int tw = tab_width[sel];

                    int H = tab_height();
                    bool top = (H >= 0);
                    int th = std::abs(H);

                    // Align perfectly with FLTK's native child_area_y calculations
                    int pane_y = top ? y() + th : y();
                    int border_y = top ? pane_y : y() + h() - th - 1;

                    // Draw a subtle 1px border spanning the width, skipping the active tab's
                    // region to create a visual "stem" connecting the tab to the child pane.
                    fl_color(palette.inactive);
                    fl_xyline(x(), border_y, tx - 1);
                    fl_xyline(tx + tw, border_y, x() + w());

                    // Draw the modern primary accent line on the active tab
                    fl_color(text_active);
                    int accent_y = top ? y() : y() + h() - 2;
                    fl_rectf(tx, accent_y, tw, 2);

                    fl_pop_clip();
                }
            }
        }

    public:
        Tabs(int x, int y, int w, int h, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Tabs>>(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
            handle_overflow(OVERFLOW_PULLDOWN);
        }
    };
}