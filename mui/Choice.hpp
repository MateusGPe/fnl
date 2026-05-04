#pragma once
#include "Theme.hpp"
#include "CoreTypes.hpp"
#include <FL/Fl_Menu_.H>
#include <FL/fl_draw.H>

namespace mui
{
    class Choice : public policy::ChoiceDraw<policy::HoverTracker<policy::CallbackRouter<Fl_Menu_>>>
    {
    protected:
        Fl_Color btn_color;
        Fl_Color btn_hover_color;
        Fl_Color btn_pressed_color;
        Fl_Boxtype btn_box;
        Fl_Boxtype btn_down_box;

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

                redraw();

                Fl_Menu_::handle(FL_BEFORE_MENU);

                Fl_Widget_Tracker wp(this);
                const Fl_Menu_Item *v = menu()->pulldown(x(), y(), w(), h());
                if (wp.deleted())
                    return 1;

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
        Choice(int x, int y, int w, int h, const char *l = nullptr) : policy::ChoiceDraw<policy::HoverTracker<policy::CallbackRouter<Fl_Menu_>>>(x, y, w, h, l)
        {
            this->color(mui::ThemeManager::get_palette().bg_main);
            this->textcolor(mui::ThemeManager::get_palette().fg_main);
            this->selection_color(mui::ThemeManager::get_palette().selection);
            this->down_box(Theme::schemes::MENU_POPUP_BOX);
            btn_box = FL_GTK_UP_BOX;
            btn_down_box = Theme::schemes::BUTTON_DOWN_BOX;
            btn_color = mui::ThemeManager::get_palette().bg_main;
            btn_hover_color = mui::ThemeManager::get_palette().bg_sec;
            btn_pressed_color = mui::ThemeManager::get_palette().bg_sec;
        }

        void textfont(mui::Font f) { Fl_Menu_::textfont(static_cast<Fl_Font>(f)); }
        using Fl_Menu_::textfont;
        void textsize(int s) { Fl_Menu_::textsize(s); }
        using Fl_Menu_::textsize;
        void textcolor(mui::Color c) { Fl_Menu_::textcolor(static_cast<Fl_Color>(c)); }
        using Fl_Menu_::textcolor;
        template <typename T, void (T::*Method)()>
        Choice &on_select(T *instance)
        {
            return on_trigger<T, Method>(instance);
        }
        const char *text() const
        {
            const Fl_Menu_Item *v = Fl_Menu_::mvalue();
            return v ? v->text : "";
        }
    };
}