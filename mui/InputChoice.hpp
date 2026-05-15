#pragma once

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <string.h>

#include "Theme.hpp"
#include "Theme/engine.hpp"
#include "Policies.hpp"

namespace mui
{
    class InputMenuButton : public Fl_Menu_Button
    {
    private:
        bool is_hovered = false;

        const Fl_Menu_Item *popup()
        {
            handle(FL_BEFORE_MENU);
            menu_end();
            redraw();
            Fl_Widget_Tracker mb(this);
            const Fl_Menu_Item *m = menu()->pulldown(parent()->x(), y(), parent()->w(), h(), 0, this);
            picked(m);
            if (mb.exists())
                redraw();
            return m;
        }

    public:
        InputMenuButton(int X, int Y, int W, int H, const char *L = 0)
            : Fl_Menu_Button(X, Y, W, H, L)
        {
            box(FL_FLAT_BOX);
        }

        int handle(int e) override
        {
            int res = 0;
            if (!menu() || !menu()->text)
                return 0;

            switch (e)
            {
            case FL_ENTER:
                is_hovered = true;
                redraw();
                return (box() && !type()) ? 1 : 0;
            case FL_LEAVE:
                is_hovered = false;
                redraw();
                return (box() && !type()) ? 1 : 0;
            case FL_PUSH:
                if (!box())
                {
                    if (Fl::event_button() != 3)
                        return 0;
                }
                else if (type())
                {
                    if (!(type() & (1 << (Fl::event_button() - 1))))
                        return 0;
                }
                if (Fl::visible_focus())
                    Fl::focus(this);
                popup();
                return 1;
            case FL_KEYBOARD:
                if (!box())
                    return 0;
                if (Fl::event_key() == ' ' && !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META)))
                {
                    popup();
                    return 1;
                }
                return 0;
            case FL_SHORTCUT:
                if (Fl_Widget::test_shortcut())
                {
                    popup();
                    return 1;
                }
                return test_shortcut() != 0;
            case FL_FOCUS:
            case FL_UNFOCUS:
                if (box() && Fl::visible_focus())
                    redraw();
                return 1;
            default:
                return Fl_Menu_Button::handle(e);
            }
        }

        void draw() override
        {
            if (!box())
                return;

            const auto &palette = ThemeManager::get_palette();
            engine::WidgetState btn_state;
            btn_state.active = active_r();
            btn_state.hovered = is_hovered;
            btn_state.focused = (Fl::focus() == this);
            btn_state.value = (pressed_menu_button_ == this) ? 1 : 0;

            engine::draw_button_right(x(), y(), w(), h(), btn_state, palette);

            fl_color(policy::resolve_color_active(this, palette.fg_main));
            const int arrow_x = x() + w() - 14;
            const int arrow_y = y() + (h() - 6) / 2;
            fl_polygon(arrow_x, arrow_y, arrow_x + 8, arrow_y, arrow_x + 4, arrow_y + 6);
        }
    };

    class InputChoice : public policy::CallbackRouter<Fl_Group>
    {
    protected:
        Fl_Input *inp_;
        InputMenuButton *menu_;
        bool is_hovered = false;

        static void menu_cb(Fl_Widget *, void *data)
        {
            InputChoice *o = static_cast<InputChoice *>(data);
            Fl_Widget_Tracker wp(o);
            const Fl_Menu_Item *item = o->menubutton()->mvalue();

            if (item && item->flags & (FL_SUBMENU | FL_SUBMENU_POINTER))
                return;

            if (!strcmp(o->inp_->value(), o->menu_->text()))
            {
                o->Fl_Widget::clear_changed();
                if (o->when() & FL_WHEN_NOT_CHANGED)
                    o->do_callback(FL_REASON_RESELECTED);
            }
            else
            {
                o->inp_->value(o->menu_->text());
                o->inp_->set_changed();
                o->Fl_Widget::set_changed();
                if (o->when() & (FL_WHEN_CHANGED | FL_WHEN_RELEASE))
                    o->do_callback(FL_REASON_CHANGED);
            }

            if (wp.deleted())
                return;

            if (o->callback() != default_callback)
            {
                o->Fl_Widget::clear_changed();
                o->inp_->clear_changed();
            }
        }

        static void inp_cb(Fl_Widget *, void *data)
        {
            InputChoice *o = static_cast<InputChoice *>(data);
            Fl_Widget_Tracker wp(o);

            if (o->inp_->changed())
            {
                o->Fl_Widget::set_changed();
            }
            else
            {
                o->Fl_Widget::clear_changed();
            }

            if (Fl::callback_reason() == FL_REASON_LOST_FOCUS)
            {
                if (o->when() & FL_WHEN_RELEASE)
                    o->do_callback(FL_REASON_LOST_FOCUS);
            }
            else
            {
                if (o->inp_->changed() && (o->when() & FL_WHEN_CHANGED))
                    o->do_callback(Fl::callback_reason());
                else if (o->when() & FL_WHEN_NOT_CHANGED)
                    o->do_callback(Fl::callback_reason());
            }

            if (wp.deleted())
                return;

            if (o->callback() != default_callback)
                o->Fl_Widget::clear_changed();
        }

        virtual int inp_x() const { return (x() + Fl::box_dx(box())); }
        virtual int inp_y() const { return (y() + Fl::box_dy(box())); }
        virtual int inp_w() const { return (w() - Fl::box_dw(box()) - menu_w()); }
        virtual int inp_h() const { return (h() - Fl::box_dh(box())); }

        virtual int menu_x() const { return x() + w() - menu_w(); }
        virtual int menu_y() const { return y(); }
        virtual int menu_w() const { return 20; }
        virtual int menu_h() const { return h(); }

        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();

            if (damage() & ~FL_DAMAGE_CHILD)
            {
                inp_->color(palette.input_bg);
                Fl_Boxtype btype = Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX;
                draw_box(btype, palette.input_bg);
            }

            fl_push_clip(inp_x(), inp_y(), inp_w(), inp_h());
            draw_child(*inp_);
            fl_pop_clip();

            draw_child(*menu_);

            const bool is_focused = ((Fl::focus() == inp_ || Fl::focus() == menu_) && visible_focus() && Fl::visible_focus());
            if (is_focused)
            {
                engine::draw_focus_ring(x(), y(), w(), h(), palette.focus_ring,
                                        palette.metrics.focus_ring_opacity,
                                        palette.metrics.focus_ring_width,
                                        palette.metrics.radius);

                fl_line_style(FL_SOLID, palette.metrics.focus_ring_width);
                fl_color(palette.focus_ring);
                fl_line(menu_->x(), menu_->y(), menu_->x(), menu_->y() + menu_->h());
                fl_line_style(0);
            }
            if (this->is_hovered)
            {
                engine::draw_focus_ring(x() + 1, y() + 1, w() - 2, h() - 2, palette.selection,
                                        palette.metrics.focus_ring_opacity,
                                        palette.metrics.focus_ring_width,
                                        palette.metrics.radius);
                fl_line_style(FL_SOLID, palette.metrics.focus_ring_width);
                fl_color(palette.selection);
                fl_line(menu_->x()-1, menu_->y()+1, menu_->x()-1, menu_->y() + menu_->h()-1);
                fl_line_style(0);
            }

            draw_label();
        }

        int handle(int e) override
        {
            int res = policy::CallbackRouter<Fl_Group>::handle(e);
            switch (e)
            {
            case FL_ENTER:
                is_hovered = true;
                redraw();
                break;
            case FL_LEAVE:
                is_hovered = false;
                redraw();
                break;
            case FL_FOCUS:
            case FL_UNFOCUS:
                redraw();
                break;
            default:
                break;
            }
            return res;
        }

    public:
        // Dimensions must be passed to properly initialize layout calculations
        InputChoice(int X, int Y, int W, int H, const char *L = 0)
            : policy::CallbackRouter<Fl_Group>(X, Y, W, H, L)
        {
            Fl_Group::box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX);
            align(FL_ALIGN_LEFT);

            inp_ = new Fl_Input(inp_x(), inp_y(), inp_w(), inp_h());
            inp_->callback(inp_cb, (void *)this);
            inp_->box(FL_FLAT_BOX);
            inp_->when(FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED | FL_WHEN_RELEASE);

            menu_ = new InputMenuButton(menu_x(), menu_y(), menu_w(), menu_h());
            menu_->callback(menu_cb, (void *)this);

            end();
        }

        void resize(int X, int Y, int W, int H) override
        {
            Fl_Group::resize(X, Y, W, H);
            inp_->resize(inp_x(), inp_y(), inp_w(), inp_h());
            menu_->resize(menu_x(), menu_y(), menu_w(), menu_h());
        }

        void add(const char *s) { menu_->add(s); }
        int changed() const { return inp_->changed() | Fl_Widget::changed(); }
        void clear_changed()
        {
            inp_->clear_changed();
            Fl_Widget::clear_changed();
        }
        void set_changed() { inp_->set_changed(); }
        void clear() { menu_->clear(); }
        Fl_Boxtype down_box() const { return (menu_->down_box()); }
        void down_box(Fl_Boxtype b) { menu_->down_box(b); }
        const Fl_Menu_Item *menu() { return (menu_->menu()); }
        void menu(const Fl_Menu_Item *m) { menu_->menu(m); }
        Fl_Color textcolor() const { return (inp_->textcolor()); }
        void textcolor(Fl_Color c) { inp_->textcolor(c); }
        Fl_Font textfont() const { return (inp_->textfont()); }
        void textfont(Fl_Font f) { inp_->textfont(f); }
        Fl_Fontsize textsize() const { return (inp_->textsize()); }
        void textsize(Fl_Fontsize s) { inp_->textsize(s); }
        const char *value() const { return (inp_->value()); }
        void value(const char *val) { inp_->value(val); }

        void value(int val)
        {
            menu_->value(val);
            inp_->value(menu_->text(val));
        }

        int update_menubutton()
        {
            for (int i = 0; i < menu_->size(); i++)
            {
                const Fl_Menu_Item &item = menu_->menu()[i];
                if (item.flags & (FL_SUBMENU | FL_SUBMENU_POINTER))
                    continue;
                const char *name = menu_->text(i);
                if (name && strcmp(name, inp_->value()) == 0)
                {
                    menu_->value(i);
                    return 1;
                }
            }
            return 0;
        }

        Fl_Menu_Button *menubutton() { return menu_; }
        Fl_Input *input() { return inp_; }
    };
}