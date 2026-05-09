#pragma once
#include "Theme.hpp"
#include <FL/Fl_Menu_Bar.H>

namespace mui
{
    class MenuBar : public Fl_Menu_Bar
    {
    protected:
        void draw() override
        {
            if (damage() != 0)
            {
                color(ThemeManager::get_palette().bg_main);
                selection_color(ThemeManager::get_palette().selection);
                textcolor(fl_contrast(ThemeManager::get_palette().fg_main, color()));
            }
            fl_push_clip(x(), y(), w(), h());
            Fl_Menu_Bar::draw();

            fl_color(ThemeManager::get_palette().btn_frame.out_top);
            fl_line(x(), y() + h() - 1, x() + w(), y() + h() - 1);
            fl_pop_clip();
        }

        int handle(int event) override
        {
            if (event == FL_PUSH)
            {
                Fl_Boxtype original_box = box();
                box(FL_UP_BOX);
                int result = Fl_Menu_Bar::handle(event);
                box(original_box);
                return result;
            }
            return Fl_Menu_Bar::handle(event);
        }

    public:
        MenuBar(int x, int y, int w, int h, const char *l = nullptr) : Fl_Menu_Bar(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
            down_box(FL_FLAT_BOX);
        }
    };
}