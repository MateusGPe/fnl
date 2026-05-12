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

                box(FL_FLAT_BOX);
                color(palette.bg_sec);
                selection_color(palette.bg_main);

                const Fl_Color text_active = active_r() ? palette.selection : fl_inactive(palette.selection);
                const Fl_Color text_inactive = active_r() ? palette.fg_main : fl_inactive(palette.fg_main);

                labelcolor(text_active);

                for (int i = 0; i < children(); ++i)
                {
                    Fl_Widget *c = child(i);
                    c->color(palette.bg_main);

                    const bool is_hovered_and_inactive = (c == m_hovered_tab && c != value());
                    if (is_hovered_and_inactive)
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

            Fl_Tabs::draw();

            if (children() == 0 || !redraw_tabs)
                return;

            const int sel = tab_positions();
            if (sel < 0 || sel >= children())
                return;

            const auto &palette = ThemeManager::get_palette();
            const Fl_Color accent = active_r() ? palette.selection : fl_inactive(palette.selection);

            fl_push_clip(x(), y(), w(), h());

            const int tx = x() + tab_pos[sel] + tab_offset;
            const int tw = tab_width[sel];

            const int H = tab_height();
            const bool top = (H >= 0);
            const int th = std::abs(H);
            const int pane_y = top ? y() + th : y();
            const int border_y = top ? pane_y : y() + h() - th - 1;

            fl_color(palette.inactive);
            fl_xyline(x(), border_y, tx - 1);
            fl_xyline(tx + tw, border_y, x() + w());

            fl_color(accent);
            fl_rectf(tx, top ? y() : y() + h() - 2, tw, 2);

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
