#pragma once
#include "Theme.hpp"
#include "Policies.hpp"
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Widget_Tracker.H>
#include <FL/fl_draw.H>
#include <string>

namespace mui
{
    class Choice : public policy::CallbackRouter<Fl_Menu_>
    {
    private:
        using Base = policy::CallbackRouter<Fl_Menu_>;

        Fl_Boxtype m_btn_box = FL_GTK_UP_BOX;
        Fl_Boxtype m_btn_dn_box = Theme::schemes::BUTTON_DOWN_BOX;
        bool m_pressed = false;
        bool is_hovered = false;
        std::string m_last_label;

    protected:
        [[nodiscard]] static Fl_Boxtype resolve_hover_box(Fl_Boxtype base) noexcept
        {
            switch (base)
            {
            case FL_GTK_UP_BOX:
                return FL_PLASTIC_UP_BOX;
            case Theme::schemes::ROUNDED_BUTTON_UP_BOX:
                return Theme::schemes::ROUNDED_HOVERED_UP_BOX;
            case FL_DIAMOND_UP_BOX:
                return FL_PLASTIC_THIN_UP_BOX;
            case FL_GTK_THIN_UP_BOX:
                return FL_PLASTIC_UP_BOX;
            default:
                return base;
            }
        }

        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();

            fl_push_clip(x(), y(), w(), h());

            Fl_Boxtype draw_b = m_pressed ? m_btn_dn_box : m_btn_box;
            Fl_Color draw_c = m_pressed ? palette.bg_sec : palette.bg_main;

            if (!m_pressed && is_hovered && active_r())
            {
                const Fl_Boxtype hb = resolve_hover_box(m_btn_box);
                if (hb != m_btn_box)
                    draw_b = hb;
                else
                    draw_c = palette.bg_sec;
            }

            if (damage() & ~FL_DAMAGE_CHILD)
                fl_draw_box(draw_b, x(), y(), w(), h(), draw_c);

            if (const Fl_Menu_Item *m = mvalue())
                m_last_label = m->text ? m->text : "";

            fl_color(policy::resolve_color_active(this, palette.fg_main));
            fl_font(textfont(), textsize());
            fl_draw(m_last_label.c_str(), x() + 6, y(), w() - 24, h(),
                    FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

            const int arrow_x = x() + w() - 14;
            const int arrow_y = y() + (h() - 6) / 2;
            fl_polygon(arrow_x, arrow_y, arrow_x + 8, arrow_y, arrow_x + 4, arrow_y + 6);

            engine::draw_ring(this, 0, Fl::focus() == this);

            fl_pop_clip();
        }

        int handle(int event) override
        {
            const int res = Base::handle(event);

            if (!active_r())
                return res;

            switch (event)
            {
            case FL_FOCUS:
            case FL_UNFOCUS:
                redraw();
                return 1;
            case FL_ENTER:
                is_hovered = true;
                redraw();
                return 1;
            case FL_LEAVE:
                is_hovered = false;
                redraw();
                return 1;
            case FL_KEYBOARD:

                if (Fl::event_key() != ' ' ||
                    (Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META)))
                    return 0;
                [[fallthrough]];

            case FL_PUSH:
            {
                if (!menu() || !menu()->text)
                    return 0;

                if (Fl::visible_focus())
                    Fl::focus(this);

                const auto &palette = ThemeManager::get_palette();
                this->color(palette.bg_main);
                this->selection_color(palette.selection);
                this->textcolor(palette.fg_main);

                m_pressed = true;
                redraw();

                Fl_Menu_::handle(FL_BEFORE_MENU);

                Fl_Widget_Tracker guard(this);
                const Fl_Menu_Item *v = menu()->pulldown(x(), y(), w(), h());
                if (guard.deleted())
                    return 1;

                m_pressed = false;
                is_hovered = (Fl::event_inside(this) != 0);
                redraw();

                if (v && !v->submenu() && v != mvalue())
                {
                    value(v);
                    set_changed();
                    redraw();
                    do_callback();
                }
                return 1;
            }

            default:
                break;
            }

            return res;
        }

    public:
        Choice(int x, int y, int w, int h, const char *l = nullptr)
            : Base(x, y, w, h, l)
        {
            this->down_box(Theme::schemes::MENU_POPUP_BOX);
        }

        template <typename T, void (T::*Method)()>
        Choice &on_select(T *instance)
        {
            this->template bind_callback<T, Method>(instance);
            return *this;
        }

        Choice &on_select(std::function<void()> handler)
        {
            this->bind_callback(std::move(handler));
            return *this;
        }

        [[nodiscard]] const char *text() const
        {
            const Fl_Menu_Item *v = mvalue();
            return v ? v->text : "";
        }
    };
}
