#pragma once
#include "Theme.hpp"
#include <FL/Fl_Menu_.H>
#include <FL/fl_draw.H>

namespace mui
{
    class Choice : public policy::HoverTracker<policy::CallbackRouter<Fl_Menu_>>
    {
    protected:
        Fl_Boxtype btn_box;
        Fl_Boxtype btn_down_box;
        bool is_pressed = false;

        void draw() override
        {
            fl_push_clip(x(), y(), w(), h());

            const auto& palette = mui::ThemeManager::get_palette();
            Fl_Boxtype draw_b = is_pressed ? btn_down_box : btn_box;
            Fl_Color draw_c = is_pressed ? palette.bg_sec : palette.bg_main;

            if (!is_pressed && is_hovered && active_r())
            {
                Fl_Boxtype hover_b = this->resolve_hover_box(btn_box);
                if (hover_b != btn_box) draw_b = hover_b;
                else draw_c = palette.bg_sec; // Assuming bg_sec is the hover color for Choice
            }

            fl_draw_box(draw_b, x(), y(), w(), h(), draw_c);

            fl_color(active_r() ? textcolor() : fl_inactive(textcolor()));
            fl_font(textfont(), textsize());
            
            if (const Fl_Menu_Item* m = mvalue()) { // mvalue() is from Fl_Menu_
                fl_draw(m->text, x() + 6, y(), w() - 24, h(), FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            }

            int arrow_x = x() + w() - 14;
            int arrow_y = y() + (h() - 6) / 2;
            fl_polygon(arrow_x, arrow_y, arrow_x + 8, arrow_y, arrow_x + 4, arrow_y + 6);

            fl_pop_clip();
        }

        int handle(int event) override
        {
            int res = policy::HoverTracker<policy::CallbackRouter<Fl_Menu_>>::handle(event);
            if (!active_r())
                return res;

            switch (event)
            {
            case FL_ENTER:
                is_hovered = true;
                redraw();
                return 1;
            case FL_LEAVE:
                is_hovered = false;
                redraw();
                return 1;
            case FL_FOCUS:
            case FL_UNFOCUS:
                redraw();
                break;
            case FL_KEYBOARD:
                if (Fl::event_key() != ' ' || (Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META)))
                    return 0;
            case FL_PUSH:
                if (!menu() || !menu()->text)
                    return 0;

                if (Fl::visible_focus())
                    Fl::focus(this);

                // Update colors for the dropdown menu before showing it
                const auto& palette = mui::ThemeManager::get_palette();
                this->color(palette.bg_main);
                this->selection_color(palette.selection);
                this->textcolor(fl_contrast(palette.fg_main, palette.bg_main));
                is_pressed = true;
                redraw();

                Fl_Menu_::handle(FL_BEFORE_MENU);

                Fl_Widget_Tracker wp(this);
                const Fl_Menu_Item *v = menu()->pulldown(x(), y(), w(), h());
                if (wp.deleted())
                    return 1;

                is_pressed = false;
                this->is_hovered = Fl::event_inside(this);
                redraw();
                if (v && !v->submenu())
                {
                    if (v != mvalue())
                    {
                        value(v);
                        set_changed();
                        redraw();
                        do_callback();
                    }
                }
                return 1;
            }

            return res;
        }

    public:
        Choice(int x, int y, int w, int h, const char *l = nullptr) : policy::HoverTracker<policy::CallbackRouter<Fl_Menu_>>(x, y, w, h, l)
        {
            // Colors are now handled dynamically in draw() or handle()
            this->down_box(Theme::schemes::MENU_POPUP_BOX);
            btn_box = FL_GTK_UP_BOX;
            btn_down_box = Theme::schemes::BUTTON_DOWN_BOX;
        }

        template <typename T, void (T::*Method)()>
        Choice &on_select(T *instance)
        {
            this->template bind_callback<T, Method>(instance);
            return *this;
        }
        const char *text() const
        {
            const Fl_Menu_Item *v = Fl_Menu_::mvalue();
            return v ? v->text : "";
        }
    };
}