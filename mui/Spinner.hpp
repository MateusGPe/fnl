#pragma once
#include "Policies.hpp"
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Valuator.H>
#include <utility>

namespace mui
{

    template <typename T, typename... Args>
    bool is_any_of(const T &valor, const Args &...opcoes)
    {
        return ((valor == opcoes) || ...);
    }

    class Spinner : public policy::CallbackRouter<Fl_Spinner>
    {
    private:
        Fl_Widget *m_hovered_button = nullptr;
        bool is_hovered = false;

    protected:
        int handle(int event) override
        {
            // Let HoverTracker do its thing first (sets is_hovered, redraws)
            const int res = policy::CallbackRouter<Fl_Spinner>::handle(event);

            if (is_any_of(event, FL_MOVE, FL_ENTER, FL_LEAVE, FL_FOCUS, FL_UNFOCUS))
            {
                Fl_Widget *over = nullptr;
                if (event != FL_LEAVE)
                {
                    over = Fl::belowmouse();
                    // Only care if we are over one of the buttons
                    if (over != &up_button_ && over != &down_button_)
                    {
                        over = nullptr;
                    }
                }
                if (over != m_hovered_button)
                {
                    m_hovered_button = over;
                }
                if (event == FL_LEAVE || event == FL_ENTER)
                {
                    is_hovered = event == FL_ENTER;
                }
                redraw();
            }
            return res;
        }
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();

            input_.color(palette.input_bg);
            input_.selection_color(palette.selection);
            input_.textcolor(palette.fg_main);

            if (damage() & ~FL_DAMAGE_CHILD)
            {
                fl_draw_box(FL_FLAT_BOX, this->x(), this->y(), this->w(), this->h(), palette.bg_main);
                fl_draw_box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX, x(), y(), w(), h(), palette.input_bg);
            }

            fl_push_clip(input_.x(), input_.y(), input_.w(), input_.h());
            draw_child(input_);
            fl_pop_clip();

            engine::WidgetState up_state;
            up_state.active = active_r();
            up_state.hovered = (m_hovered_button == &up_button_);
            up_state.focused = (Fl::focus() == &up_button_);
            up_state.value = up_button_.value();
            engine::draw_button_top_right(up_button_.x(), up_button_.y(), up_button_.w(), up_button_.h(), up_state, palette);

            engine::WidgetState down_state;
            down_state.active = active_r();
            down_state.hovered = (m_hovered_button == &down_button_);
            down_state.focused = (Fl::focus() == &down_button_);
            down_state.value = down_button_.value();
            engine::draw_button_bottom_right(down_button_.x(), down_button_.y(), down_button_.w(), down_button_.h(), down_state, palette);

            draw_arrow(up_button_, true, palette);
            draw_arrow(down_button_, false, palette);

            const bool is_focused = (Fl::focus() == this || contains(Fl::focus()));
            auto c = engine::draw_ring(this, is_hovered, is_focused);
            if (c != 0)
            {
                fl_line_style(FL_SOLID, palette.metrics.focus_ring_width);
                fl_color(c);
                fl_line(up_button_.x(), up_button_.y(), up_button_.x(), down_button_.y() + down_button_.h());
                fl_line(up_button_.x(), up_button_.y() + up_button_.h(), up_button_.x() + up_button_.w(), up_button_.y() + up_button_.h());
                fl_line_style(0);
            }
        }

    private:
        void draw_arrow(const Fl_Repeat_Button &btn, bool is_up,
                        const ThemePalette &palette) const
        {
            const int cx = btn.x() + btn.w() / 2;
            const int cy = btn.y() + btn.h() / 2;
            const Fl_Color base = btn.value() ? palette.focus_ring : palette.spinner_arrow;
            const Fl_Color c = policy::resolve_color_active(this, base);
            core::draw_spinner_arrow(cx, cy, palette.metrics.spinner_arrow_size, is_up, c);
        }

    public:
        Spinner(int x, int y, int w, int h, const char *label = nullptr)
            : policy::CallbackRouter<Fl_Spinner>(x, y, w, h, label)
        {
            box(FL_NO_BOX);
            input_.box(FL_FLAT_BOX);
            up_button_.box(FL_NO_BOX);
            down_button_.box(FL_NO_BOX);
            up_button_.label("");
            down_button_.label("");
            resize(x, y, w, h);
        }

        void resize(int X, int Y, int W, int H) override
        {
            Fl_Group::resize(X, Y, W, H);

            constexpr int wpad = 4, hpad = 2;
            constexpr int gap = 2;

            const int btn_w = 2 * H / 3;
            const int btn_x = X + W - btn_w;
            const int btn_h_half = H / 2;

            input_.resize(X + wpad, Y + hpad, W - btn_w - gap - wpad, H - hpad * 2);

            up_button_.resize(btn_x, Y, btn_w, btn_h_half);
            down_button_.resize(btn_x, Y + H - btn_h_half, btn_w, btn_h_half);
        }
    };
}
