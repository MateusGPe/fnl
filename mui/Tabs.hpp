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
        using Base = policy::HoverTracker<policy::CallbackRouter<Fl_Tabs>>;
        Fl_Widget *m_hovered_tab = nullptr;

    protected:
        int handle(int event) override
        {
            const int res = Base::handle(event);
            if (!active_r())
                return res;

            switch (event)
            {
            case FL_MOVE:
            case FL_ENTER:
            case FL_LEAVE:
            {
                Fl_Widget *hw = (event == FL_LEAVE)
                                    ? nullptr
                                    : which(Fl::event_x(), Fl::event_y());

                if (hw != m_hovered_tab)
                {
                    m_hovered_tab = hw;
                    damage(FL_DAMAGE_EXPOSE);
                }
                return 1;
            }
            default:
                break;
            }
            return res;
        }

        void draw() override
        {
            const uchar d = damage();
            const bool redraw_tabs = (d & ~FL_DAMAGE_CHILD) != 0;

            if (redraw_tabs)
            {
                const auto &palette = ThemeManager::get_palette();

                // Main widget background (the empty track behind the tabs)
                color(palette.bg_sec);
                selection_color(palette.bg_main);

                const Fl_Color text_active = active_r() ? palette.selection : fl_inactive(palette.selection);
                const Fl_Color text_inactive = active_r() ? palette.fg_main : fl_inactive(palette.fg_main);

                for (int i = 0; i < children(); ++i)
                {
                    Fl_Widget *c = child(i);

                    // CRITICAL: Force a flat box to strip native FLTK 3D frames/dividers
                    c->box(FL_FLAT_BOX);

                    const bool is_selected = (c == value());
                    const bool is_hovered = (c == m_hovered_tab && !is_selected);

                    if (is_selected)
                    {
                        c->color(palette.bg_main);
                        c->labelcolor(text_active);
                    }
                    else if (is_hovered)
                    {
                        c->color(fl_color_average(palette.bg_main, palette.bg_sec, 0.4f));
                        c->labelcolor(text_inactive);
                    }
                    else
                    {
                        c->color(palette.bg_sec);
                        c->labelcolor(text_inactive);
                    }
                }
            }

            // FLTK calculates geometry and draws flat rectangles/text
            Fl_Tabs::draw();

            if (children() == 0 || !redraw_tabs)
                return;

            const int sel = tab_positions();
            if (sel < 0 || sel >= children())
                return;

            const auto &palette = ThemeManager::get_palette();
            fl_push_clip(x(), y(), w(), h());

            const int tx = x() + tab_pos[sel] + tab_offset;
            const int tw = tab_width[sel];

            const int H = tab_height();
            const bool top = (H >= 0);
            const int th = std::abs(H);

            // pane_y marks the boundary where the tab header meets the content body
            const int pane_y = top ? y() + th : y();
            const int border_y = top ? pane_y - 1 : y() + h() - th;

            // 1. Draw a subtle separator line across the entire tab track
            const Fl_Color border_color = fl_color_average(palette.bg_main, palette.fg_main, 0.85f);
            fl_color(border_color);
            fl_xyline(x(), border_y, x() + w());

            // 2. Erase the border directly beneath the active tab to seamlessly connect it
            fl_color(palette.bg_main);
            fl_xyline(tx, border_y, tx + tw - 1);

            // 3. Draw the modern accent line at the junction, rather than the top
            const Fl_Color accent = active_r() ? palette.selection : fl_inactive(palette.selection);
            fl_color(accent);

            if (top)
            {
                fl_rectf(tx, border_y - 1, tw, 2);
            }
            else
            {
                fl_rectf(tx, border_y, tw, 2);
            }

            fl_pop_clip();
        }

    public:
        Tabs(int x, int y, int w, int h, const char *l = nullptr)
            : Base(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
            handle_overflow(OVERFLOW_PULLDOWN);
        }
    };
}